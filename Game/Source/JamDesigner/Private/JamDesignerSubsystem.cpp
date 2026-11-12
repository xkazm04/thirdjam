#include "JamDesignerSubsystem.h"
#include "JamDesignerPrompt.h"
#include "JamDesignerTransport.h"
#include "JamCartridge.h"
#include "JamEventBus.h"
#include "JamJson.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformMisc.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Containers/Ticker.h"

DEFINE_LOG_CATEGORY(LogJamDesigner);

const FName UJamDesignerSubsystem::DirectorsCallEvent(TEXT("directors_call"));
const FString UJamDesignerSubsystem::MyDirectorsCutName(TEXT("My Director's Cut"));

namespace JamDesignerImpl
{
	using FJ = FJamJson;
	using FJPtr = TSharedPtr<FJamJson>;

	static const TCHAR* ConfigSection = TEXT("/Script/JamDesigner.JamDesignerSubsystem");
	static const TCHAR* ConfigKey = TEXT("ApiKey");
	static const TCHAR* ApiUrl = TEXT("https://api.anthropic.com/v1/messages");
	static const TCHAR* ApiVersion = TEXT("2023-06-01");

	static FString Slug(const FString& Name)
	{
		FString Out;
		bool bDash = false;
		for (TCHAR C : Name.ToLower())
		{
			if (FChar::IsAlnum(C) && C < 128) { Out.AppendChar(C); bDash = false; }
			else if (!bDash && !Out.IsEmpty()) { Out.AppendChar('-'); bDash = true; }
		}
		while (Out.EndsWith(TEXT("-"))) Out.LeftChopInline(1);
		return Out.IsEmpty() ? FString(TEXT("cut")) : Out;
	}

	static bool IsStop(const FString& W)
	{
		static const TSet<FString> Stop = {
			TEXT("a"), TEXT("an"), TEXT("the"), TEXT("it"), TEXT("make"), TEXT("of"), TEXT("and"), TEXT("for"), TEXT("my"), TEXT("to"), TEXT("on"),
			TEXT("in"), TEXT("is"), TEXT("be"), TEXT("with"), TEXT("where"), TEXT("everything"), TEXT("version"), TEXT("game"), TEXT("please"),
			TEXT("me"), TEXT("more"), TEXT("much"), TEXT("very"), TEXT("i"), TEXT("want"), TEXT("can"), TEXT("you"), TEXT("its"), TEXT("this"),
			TEXT("that"), TEXT("some"), TEXT("way"), TEXT("lot"), TEXT("bit"), TEXT("like"), TEXT("give"), TEXT("let"), TEXT("lets"), TEXT("us"),
			TEXT("so"), TEXT("but"), TEXT("all"), TEXT("one"), TEXT("cut"), TEXT("really"), TEXT("too"), TEXT("add"),
		};
		return Stop.Contains(W);
	}

	static FString Stem(FString W)
	{
		static const TCHAR* Suffixes[] = { TEXT("est"), TEXT("er"), TEXT("ing"), TEXT("ly"), TEXT("es"), TEXT("s") };
		for (const TCHAR* S : Suffixes)
		{
			const int32 L = FCString::Strlen(S);
			if (W.Len() - L >= 3 && W.EndsWith(S)) { W.LeftChopInline(L); break; }
		}
		return W;
	}

	static TSet<FString> Words(const FString& Text)
	{
		TSet<FString> Out;
		FString Cur;
		auto Flush = [&]
		{
			if (!Cur.IsEmpty() && !IsStop(Cur)) Out.Add(Stem(Cur));
			Cur.Reset();
		};
		for (TCHAR C : Text.ToLower())
		{
			if (FChar::IsAlnum(C)) Cur.AppendChar(C);
			else if (C != '\'') Flush();
		}
		Flush();
		return Out;
	}

	struct FCachedCut { FString Name; FString Sentence; TArray<FString> Keywords; FString Json; };

	static const TArray<FCachedCut>& CachedCuts()
	{
		static TArray<FCachedCut> Cuts = []
		{
			TArray<FCachedCut> Out;
			// The file on disk wins (a designer can tune it without a rebuild); the embedded copy is the packaged fallback.
			FString Text;
			const FString Path = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Designer"), TEXT("cached-cuts.json"));
			if (!FFileHelper::LoadFileToString(Text, *Path)) Text = FJamDesignerPrompt::GetEmbeddedFile(TEXT("cached-cuts.json"));
			FString Err;
			FJPtr Root = FJ::Parse(Text, Err);
			FJPtr List = Root ? Root->Get(TEXT("cuts")) : FJPtr();
			if (!List || !List->IsArray())
			{
				UE_LOG(LogJamDesigner, Error, TEXT("JAMDESIGNER cached-cuts.json unreadable: %s"), *Err);
				return Out;
			}
			for (const FJPtr& E : List->Array)
			{
				if (!E || !E->IsObject()) continue;
				FCachedCut C;
				if (FJPtr N = E->Get(TEXT("name")); N && N->IsString()) C.Name = N->String;
				if (FJPtr S = E->Get(TEXT("sentence")); S && S->IsString()) C.Sentence = S->String;
				if (FJPtr K = E->Get(TEXT("keywords")); K && K->IsArray()) for (const FJPtr& W : K->Array) if (W && W->IsString()) C.Keywords.Add(W->String);
				if (FJPtr Cart = E->Get(TEXT("cartridge")); Cart && Cart->IsObject()) C.Json = Cart->ToCompact();
				Out.Add(MoveTemp(C));
			}
			return Out;
		}();
		return Cuts;
	}

	static FString ThemeName(const FString& Json)
	{
		FString Err;
		FJPtr Root = FJ::Parse(Json, Err);
		FJPtr Theme = Root ? Root->Get(TEXT("theme")) : FJPtr();
		FJPtr Name = Theme ? Theme->Get(TEXT("name")) : FJPtr();
		return Name && Name->IsString() ? Name->String : FString();
	}

	static FString Label(FName Key)
	{
		static const TMap<FName, FString> Labels = {
			{ TEXT("skater_speed"), TEXT("skater speed") }, { TEXT("skater_turn_rate"), TEXT("turning") }, { TEXT("shot_speed"), TEXT("shot power") },
			{ TEXT("puck_max_speed"), TEXT("puck top speed") }, { TEXT("ice_friction"), TEXT("ice friction") }, { TEXT("board_restitution"), TEXT("board bounce") },
			{ TEXT("ai_speed_scale"), TEXT("bot speed") }, { TEXT("ai_red_skill"), TEXT("opponent skill") }, { TEXT("goal_pause_frames"), TEXT("goal pause") },
		};
		const FString* L = Labels.Find(Key);
		return L ? *L : Key.ToString().Replace(TEXT("_"), TEXT(" "));
	}

	static FString Announce(FName Key, double Old, double New, double Min, double Max)
	{
		if (Key == TEXT("ice_friction") && New <= Min + (Max - Min) * 0.05) return TEXT("Director's Call: ice is now glass");
		if (Key == TEXT("board_restitution") && New >= 0.8) return TEXT("Director's Call: the boards are rubber");
		const TCHAR* Dir = New > Old ? TEXT("up") : (New < Old ? TEXT("down") : TEXT("unchanged"));
		return FString::Printf(TEXT("Director's Call: %s %s (%s -> %s)"), *Label(Key), Dir, *FJ::NumberToString(Old), *FJ::NumberToString(New));
	}
}

using namespace JamDesignerImpl;

// ---------------------------------------------------------------------------------------------------------------------
// The live attempt chain (tools/designer-eval/loop.mjs): haiku fresh -> haiku repair -> sonnet repair.
// ---------------------------------------------------------------------------------------------------------------------
struct FJamDesignerJob : public TSharedFromThis<FJamDesignerJob>
{
	TWeakObjectPtr<UJamDesignerSubsystem> Owner;
	TSharedPtr<IJamDesignerTransport> Transport;
	TFunction<void(const FJamDesignerResult&)> Done;
	FString Sentence;
	FString BaseJson;
	FString Key;
	float TimeoutSec = 60.f;
	float RetryDelaySec = 1.5f;
	int32 Step = 0;
	bool bHasPrevious = false;
	FString PreviousOutput;
	TArray<FString> PreviousErrors;
	FJamDesignerResult Result;
	bool bTransportFailure = false;

	const TCHAR* ModelFor(int32 i) const { return i == 0 ? JamDesignerModels::First : (i == 1 ? JamDesignerModels::Repair : JamDesignerModels::Retry); }

	void Start() { RunStep(); }

	void RunStep()
	{
		if (Step >= 3) { Result.Outcome = EJamDesignerOutcome::Failed; Result.Message = TEXT("The Designer could not produce a valid cut. Nothing was changed."); Complete(); return; }
		FString Body, Err;
		if (!FJamDesignerPrompt::AssembleBody(Sentence, BaseJson, ModelFor(Step), bHasPrevious ? &PreviousOutput : nullptr, PreviousErrors, Body, Err))
		{
			Result.Outcome = EJamDesignerOutcome::Failed;
			Result.Errors.Add(Err);
			Result.Message = Err;
			Complete();
			return;
		}
		Send(Body, /*bIsRetry*/ false, 0.0);
	}

	void Send(const FString& Body, bool bIsRetry, double PriorMs)
	{
		TArray<TPair<FString, FString>> Headers;
		Headers.Add({ TEXT("content-type"), TEXT("application/json") });
		Headers.Add({ TEXT("x-api-key"), Key });
		Headers.Add({ TEXT("anthropic-version"), ApiVersion });
		TSharedRef<FJamDesignerJob> Self = AsShared();
		Transport->Post(ApiUrl, Headers, Body, TimeoutSec, [Self, Body, bIsRetry, PriorMs](const FJamDesignerHttpReply& R) { Self->OnReply(Body, R, bIsRetry, PriorMs); });
	}

	void OnReply(const FString& Body, const FJamDesignerHttpReply& R, bool bIsRetry, double PriorMs)
	{
		const TCHAR* Model = ModelFor(Step);
		const bool bRetryable = R.bConnected && (R.Status == 429 || R.Status == 500 || R.Status == 502 || R.Status == 503 || R.Status == 504 || R.Status == 529);
		if (bRetryable && !bIsRetry)
		{
			UE_LOG(LogJamDesigner, Log, TEXT("JAMDESIGNER request attempt=%d model=%s status=%d ms=%.0f result=retry_after_%.1fs"), Step + 1, Model, R.Status, R.Ms, RetryDelaySec);
			TSharedRef<FJamDesignerJob> Self = AsShared();
			const double Ms = R.Ms;
			if (RetryDelaySec <= 0.f) { Send(Body, true, Ms); return; }
			FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([Self, Body, Ms](float) { Self->Send(Body, true, Ms); return false; }), RetryDelaySec);
			return;
		}

		const double Ms = R.Ms + PriorMs;
		Result.Ms += Ms;
		Result.Attempts = Step + 1;
		if (!R.bConnected)
		{
			UE_LOG(LogJamDesigner, Warning, TEXT("JAMDESIGNER request attempt=%d model=%s status=0 tokens_in=0 tokens_out=0 ms=%.0f result=network_error"), Step + 1, Model, Ms);
			Result.Errors.Add(TEXT("the model call failed: network error or timeout"));
			bTransportFailure = true;
			bHasPrevious = true;
			PreviousOutput = TEXT("(no reply)");
			PreviousErrors = { TEXT("at /: the model call produced no reply. Reply with the JSON object.") };
			++Step;
			RunStep();
			return;
		}
		const FJamDesignerApiReply Api = FJamDesignerPrompt::ParseApiResponse(R.Body);
		if (R.Status < 200 || R.Status >= 300)
		{
			const bool bFatal = R.Status == 400 || R.Status == 401 || R.Status == 403 || R.Status == 404;
			UE_LOG(LogJamDesigner, Warning, TEXT("JAMDESIGNER request attempt=%d model=%s status=%d tokens_in=0 tokens_out=0 ms=%.0f result=http_error%s"),
				Step + 1, Model, R.Status, Ms, bFatal ? TEXT("_fatal") : TEXT(""));
			Result.Errors.Add(FString::Printf(TEXT("the model call failed: HTTP %d: %s"), R.Status, Api.ErrorMessage.IsEmpty() ? TEXT("no error message") : *Api.ErrorMessage));
			bTransportFailure = true;
			if (bFatal) { Result.Outcome = EJamDesignerOutcome::Failed; Complete(); return; }
			bHasPrevious = true;
			PreviousOutput = TEXT("(no reply)");
			PreviousErrors = { TEXT("at /: the model call produced no reply. Reply with the JSON object.") };
			++Step;
			RunStep();
			return;
		}

		Result.TokensIn += Api.InputTokens;
		Result.TokensOut += Api.OutputTokens;
		if (Api.StopReason == TEXT("refusal"))
		{
			UE_LOG(LogJamDesigner, Log, TEXT("JAMDESIGNER request attempt=%d model=%s status=%d tokens_in=%lld tokens_out=%lld ms=%.0f result=platform_refusal"), Step + 1, Model, R.Status, Api.InputTokens, Api.OutputTokens, Ms);
			Result.Outcome = EJamDesignerOutcome::Refused;
			Result.Refusal = TEXT("The request was declined.");
			Complete();
			return;
		}
		const FJamDesignerParsedReply P = FJamDesignerPrompt::ParseReply(Api.Text, BaseJson);
		const TCHAR* Kind = P.Kind == EJamDesignerReplyKind::Cartridge ? TEXT("cartridge") : (P.Kind == EJamDesignerReplyKind::Refusal ? TEXT("refusal") : TEXT("invalid"));
		UE_LOG(LogJamDesigner, Log, TEXT("JAMDESIGNER request attempt=%d model=%s status=%d tokens_in=%lld tokens_out=%lld ms=%.0f result=%s errors=%d"),
			Step + 1, Model, R.Status, Api.InputTokens, Api.OutputTokens, Ms, Kind, P.Errors.Num());
		if (P.Kind == EJamDesignerReplyKind::Cartridge)
		{
			TArray<FString> LoadErrors;
			UJamCartridge* C = UJamCartridge::LoadFromString(P.CartridgeJson, LoadErrors, Owner.IsValid() ? Owner.Get() : nullptr);
			if (C)
			{
				Result.Outcome = Step == 0 ? EJamDesignerOutcome::FirstTry : EJamDesignerOutcome::Repaired;
				Result.Cartridge = C;
				Result.CartridgeJson = P.CartridgeJson;
				Result.CutName = C->ThemeName;
				Result.Errors.Reset();
				Result.Message = FString::Printf(TEXT("New cut ready: %s"), *C->ThemeName);
				Complete();
				return;
			}
			// Cannot happen when ParseReply passed (same validator); treat as invalid so the chain repairs it.
			PreviousErrors = LoadErrors;
		}
		else if (P.Kind == EJamDesignerReplyKind::Refusal)
		{
			Result.Outcome = EJamDesignerOutcome::Refused;
			Result.Refusal = P.Refusal;
			Result.Errors.Reset();
			Result.Message = P.Refusal;
			Complete();
			return;
		}
		else
		{
			PreviousErrors = P.Errors;
		}
		Result.Errors = PreviousErrors;
		bHasPrevious = true;
		PreviousOutput = Api.Text;
		++Step;
		RunStep();
	}

	void Complete()
	{
		UE_LOG(LogJamDesigner, Log, TEXT("JAMDESIGNER done outcome=%s attempts=%d tokens_in=%lld tokens_out=%lld ms=%.0f"),
			*StaticEnum<EJamDesignerOutcome>()->GetNameStringByValue(static_cast<int64>(Result.Outcome)), Result.Attempts, Result.TokensIn, Result.TokensOut, Result.Ms);
		UJamDesignerSubsystem* S = Owner.Get();
		if (!S) return;
		// The live Designer could not be reached at all: fall back to the cache so the player still gets a cut.
		if (Result.Outcome == EJamDesignerOutcome::Failed && bTransportFailure && Result.TokensOut == 0)
		{
			FJamDesignerResult Off = S->ResolveOffline(Sentence);
			if (Off.HasCartridge())
			{
				Off.Errors = Result.Errors;
				Off.Attempts = Result.Attempts;
				Off.Ms = Result.Ms;
				Off.Message = FString::Printf(TEXT("The live Designer could not be reached; using the cached cut \"%s\"."), *Off.CutName);
				S->Finish(Off, Done);
				return;
			}
		}
		if (Result.Message.IsEmpty() && Result.Errors.Num()) Result.Message = Result.Errors[0];
		S->Finish(Result, Done);
	}
};

// ---------------------------------------------------------------------------------------------------------------------

UJamDesignerSubsystem::UJamDesignerSubsystem()
{
	Transport = MakeShared<FJamDesignerHttpTransport>();
}

void UJamDesignerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Collection.InitializeDependency(UJamEventBus::StaticClass());
	UE_LOG(LogJamDesigner, Log, TEXT("JAMDESIGNER ready live=%s cached_cuts=%d"), HasApiKey() ? TEXT("yes") : TEXT("no"), CachedCuts().Num());
}

void UJamDesignerSubsystem::SetTransport(TSharedPtr<IJamDesignerTransport> InTransport)
{
	Transport = InTransport ? InTransport : MakeShared<FJamDesignerHttpTransport>();
}

FString UJamDesignerSubsystem::GetSavedCutsDir() const
{
	return FPaths::ConvertRelativePathToFull(SavedCutsDirOverride.IsEmpty() ? FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Cuts")) : SavedCutsDirOverride);
}

FString UJamDesignerSubsystem::ResolveKey() const
{
	if (bKeyOverride) return KeyOverride;
	if (!SessionKey.IsEmpty()) return SessionKey;
	FString K;
	if (GConfig && GConfig->GetString(ConfigSection, ConfigKey, K, GGameUserSettingsIni))
	{
		K.TrimStartAndEndInline();
		if (!K.IsEmpty()) return K;
	}
	K = FPlatformMisc::GetEnvironmentVariable(TEXT("ANTHROPIC_API_KEY"));
	K.TrimStartAndEndInline();
	return K;
}

FString UJamDesignerSubsystem::ResolveCliExe() const
{
	if (bCliOverride)
	{
		if (!bCliOverrideEnabled) return FString();
		return CliExeOverride.IsEmpty() ? FJamCliTransport::FindClaudeOnPath() : CliExeOverride;
	}
	bool bAllowed = !UE_BUILD_SHIPPING;
	bool bConfig = false;
	if (GConfig && GConfig->GetBool(ConfigSection, TEXT("bAllowCliTransport"), bConfig, GGameIni) && bConfig) bAllowed = true;
	return bAllowed ? FJamCliTransport::FindClaudeOnPath() : FString();
}

void UJamDesignerSubsystem::SetApiKey(const FString& Key)
{
	SessionKey = Key.TrimStartAndEnd();
	if (GConfig)
	{
		if (SessionKey.IsEmpty()) GConfig->RemoveKey(ConfigSection, ConfigKey, GGameUserSettingsIni);
		else GConfig->SetString(ConfigSection, ConfigKey, *SessionKey, GGameUserSettingsIni);
		GConfig->Flush(false, GGameUserSettingsIni);
	}
	// Never the key, never its length.
	UE_LOG(LogJamDesigner, Log, TEXT("JAMDESIGNER key %s"), SessionKey.IsEmpty() ? TEXT("cleared") : TEXT("stored in Saved/Config"));
}

UJamEventBus* UJamDesignerSubsystem::GetBus() const
{
	if (BusOverride) return BusOverride;
	UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<UJamEventBus>() : nullptr;
}

bool UJamDesignerSubsystem::CallGameMode(const TCHAR* Function, const FString& StringArg, UObject* ObjectArg) const
{
	// Reflection keeps JamDesigner free of a JamGame dependency (JamGame may depend on the Designer for its UI).
	UGameInstance* GI = GetGameInstance();
	UWorld* World = GI ? GI->GetWorld() : nullptr;
	AGameModeBase* GM = World ? World->GetAuthGameMode() : nullptr;
	UFunction* F = GM ? GM->FindFunction(Function) : nullptr;
	if (!F) return false;
	uint8* Params = static_cast<uint8*>(FMemory_Alloca(F->ParmsSize));
	FMemory::Memzero(Params, F->ParmsSize);
	for (TFieldIterator<FProperty> It(F); It && It->HasAnyPropertyFlags(CPF_Parm); ++It) It->InitializeValue_InContainer(Params);
	bool bRet = true;
	for (TFieldIterator<FProperty> It(F); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
	{
		if (It->HasAnyPropertyFlags(CPF_ReturnParm)) continue;
		if (FStrProperty* S = CastField<FStrProperty>(*It)) S->SetPropertyValue_InContainer(Params, StringArg);
		else if (FObjectPropertyBase* O = CastField<FObjectPropertyBase>(*It)) O->SetObjectPropertyValue_InContainer(Params, ObjectArg);
	}
	GM->ProcessEvent(F, Params);
	if (FBoolProperty* Ret = CastField<FBoolProperty>(F->GetReturnProperty())) bRet = Ret->GetPropertyValue_InContainer(Params);
	for (TFieldIterator<FProperty> It(F); It && It->HasAnyPropertyFlags(CPF_Parm); ++It) It->DestroyValue_InContainer(Params);
	return bRet;
}

// ---- D0 ----

TArray<FJamCutInfo> UJamDesignerSubsystem::ListCuts() const
{
	TArray<FJamCutInfo> Out;
	TArray<FString> Files;
	const FString Shipped = UJamCartridge::GetCartridgeDir();
	IFileManager::Get().FindFiles(Files, *FPaths::Combine(Shipped, TEXT("*.cartridge.json")), true, false);
	Files.Sort();
	for (const FString& F : Files)
	{
		const FString Name = F.LeftChop(FString(TEXT(".cartridge.json")).Len());
		if (Name.Contains(TEXT(".test."))) continue;
		FJamCutInfo I;
		I.Name = Name;
		I.Path = FPaths::Combine(Shipped, F);
		Out.Add(I);
	}
	Files.Reset();
	const FString Saved = GetSavedCutsDir();
	IFileManager::Get().FindFiles(Files, *FPaths::Combine(Saved, TEXT("*.cartridge.json")), true, false);
	Files.Sort();
	for (const FString& F : Files)
	{
		const FString SlugName = F.LeftChop(FString(TEXT(".cartridge.json")).Len());
		FJamCutInfo I;
		I.Name = SlugName;
		I.Path = FPaths::Combine(Saved, F);
		I.bSaved = true;
		FString Meta, Err;
		if (FFileHelper::LoadFileToString(Meta, *FPaths::Combine(Saved, SlugName + TEXT(".meta.json"))))
		{
			if (FJPtr M = FJ::Parse(Meta, Err))
			{
				if (FJPtr N = M->Get(TEXT("name")); N && N->IsString()) I.Name = N->String;
				if (FJPtr S = M->Get(TEXT("sentence")); S && S->IsString()) I.Sentence = S->String;
			}
		}
		Out.Add(I);
	}
	return Out;
}

bool UJamDesignerSubsystem::LoadCut(const FString& Name, FString& OutError)
{
	FString Path;
	if (Name.EndsWith(TEXT(".json"))) Path = Name;
	else
	{
		for (const FJamCutInfo& I : ListCuts())
		{
			if (I.Name == Name || (I.bSaved && FPaths::GetCleanFilename(I.Path) == Slug(Name) + TEXT(".cartridge.json"))) { Path = I.Path; break; }
		}
		if (Path.IsEmpty() && FPaths::FileExists(UJamCartridge::ResolveCartridgePath(Name))) Path = UJamCartridge::ResolveCartridgePath(Name);
	}
	if (Path.IsEmpty()) { OutError = FString::Printf(TEXT("no cut named \"%s\""), *Name); return false; }
	TArray<FString> Errors;
	UJamCartridge* C = UJamCartridge::LoadFromFile(Path, Errors, this);
	if (!C)
	{
		OutError = Errors.Num() ? Errors[0] : TEXT("the cut was rejected");
		for (const FString& E : Errors) UE_LOG(LogJamDesigner, Warning, TEXT("JAMDESIGNER cut %s rejected: %s"), *Name, *E);
		return false;
	}
	Current = C;
	UE_LOG(LogJamDesigner, Log, TEXT("JAMDESIGNER cut loaded %s (%s)"), *Name, *C->ThemeName);
	OnCutChanged.Broadcast(C);
	if (!CallGameMode(TEXT("LoadCut"), Path, nullptr))
	{
		UE_LOG(LogJamDesigner, Log, TEXT("JAMDESIGNER no running game mode took the cut (menu or test); it is current and applies at the next match"));
	}
	return true;
}

FString UJamDesignerSubsystem::SaveCut(const FString& Name, const FString& Json, const FString& Sentence, FString& OutError)
{
	TArray<FString> Errors;
	if (!UJamCartridge::LoadFromString(Json, Errors, this))
	{
		OutError = Errors.Num() ? Errors[0] : TEXT("the cut was rejected");
		return FString();
	}
	FString Err;
	FJPtr Root = FJ::Parse(Json, Err);
	const FString Dir = GetSavedCutsDir();
	IFileManager::Get().MakeDirectory(*Dir, true);
	const FString SlugName = Slug(Name.IsEmpty() ? MyDirectorsCutName : Name);
	const FString Path = FPaths::Combine(Dir, SlugName + TEXT(".cartridge.json"));
	FJPtr Meta = MakeShared<FJ>();
	Meta->Type = FJ::EType::Object;
	auto Str = [](const FString& S) { FJPtr N = MakeShared<FJ>(); N->Type = FJ::EType::String; N->String = S; return N; };
	Meta->Object.Add({ TEXT("name"), Str(Name.IsEmpty() ? MyDirectorsCutName : Name) });
	if (!Sentence.IsEmpty()) Meta->Object.Add({ TEXT("sentence"), Str(Sentence) });
	if (!FFileHelper::SaveStringToFile(FJamDesignerPrompt::Pretty(*Root) + TEXT("\n"), *Path, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM)
		|| !FFileHelper::SaveStringToFile(FJamDesignerPrompt::Pretty(*Meta) + TEXT("\n"), *FPaths::Combine(Dir, SlugName + TEXT(".meta.json")), FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
	{
		OutError = FString::Printf(TEXT("cannot write %s"), *Path);
		return FString();
	}
	UE_LOG(LogJamDesigner, Log, TEXT("JAMDESIGNER cut saved \"%s\" -> %s"), Name.IsEmpty() ? *MyDirectorsCutName : *Name, *Path);
	return Path;
}

bool UJamDesignerSubsystem::ApplyResult(const FJamDesignerResult& Result, FString& OutError)
{
	if (Result.CartridgeJson.IsEmpty()) { OutError = TEXT("the result has no cut"); return false; }
	const FString Path = SaveCut(MyDirectorsCutName, Result.CartridgeJson, Result.Sentence, OutError);
	return !Path.IsEmpty() && LoadCut(Path, OutError);
}

// ---- the Director's Call ----

void UJamDesignerSubsystem::NotifyPeriodStarted(int32 Period)
{
	bCallUsedThisPeriod = false;
	UE_LOG(LogJamDesigner, Log, TEXT("JAMDESIGNER period %d: Director's Call available"), Period);
}

FJamDirectorsCall UJamDesignerSubsystem::DirectorsCall(FName Key, double Value)
{
	FJamDirectorsCall Call;
	Call.Key = Key;
	Call.NewValue = Value;
	auto Refuse = [&](const FString& Why)
	{
		Call.Reason = Why;
		UE_LOG(LogJamDesigner, Log, TEXT("JAMDESIGNER directors_call refused key=%s value=%s: %s"), *Key.ToString(), *FJ::NumberToString(Value), *Why);
		return Call;
	};
	if (bCallUsedThisPeriod) return Refuse(TEXT("one Director's Call per period; this period's call is used"));
	if (!Current)
	{
		TArray<FString> E;
		Current = UJamCartridge::LoadFromFile(UJamCartridge::ResolveCartridgePath(TEXT("hockey.regular")), E, this);
		if (!Current) return Refuse(TEXT("no cut is loaded"));
	}
	FString Err;
	FJPtr Root = FJ::Parse(Current->RawJson, Err);
	if (!Root || !Root->IsObject()) return Refuse(TEXT("the current cut has no source JSON"));
	FJPtr Tunables = Root->Get(TEXT("tunables"));
	FJPtr Tunable;
	if (Tunables && Tunables->IsArray())
	{
		for (const FJPtr& T : Tunables->Array)
		{
			FJPtr K = T ? T->Get(TEXT("key")) : FJPtr();
			if (K && K->IsString() && K->String == Key.ToString()) { Tunable = T; break; }
		}
	}
	if (!Tunable) return Refuse(FString::Printf(TEXT("\"%s\" is not a tunable of this cut"), *Key.ToString()));
	FJPtr Type = Tunable->Get(TEXT("type")), Min = Tunable->Get(TEXT("min")), Max = Tunable->Get(TEXT("max")), Def = Tunable->Get(TEXT("default"));
	const FString TypeName = Type && Type->IsString() ? Type->String : FString();
	if (TypeName != TEXT("number") && TypeName != TEXT("integer")) return Refuse(TEXT("only number tunables can be called"));
	if (!FMath::IsFinite(Value)) return Refuse(TEXT("the value is not a number"));
	if (TypeName == TEXT("integer") && FMath::FloorToDouble(Value) != Value) return Refuse(TEXT("this tunable takes a whole number"));
	const double Lo = Min && Min->IsNumber() ? Min->Number : -DBL_MAX;
	const double Hi = Max && Max->IsNumber() ? Max->Number : DBL_MAX;
	if (Value < Lo || Value > Hi) return Refuse(FString::Printf(TEXT("%s is outside %s..%s"), *FJ::NumberToString(Value), *FJ::NumberToString(Lo), *FJ::NumberToString(Hi)));

	FJPtr Values = Root->Get(TEXT("values"));
	if (!Values || !Values->IsObject())
	{
		Values = MakeShared<FJ>();
		Values->Type = FJ::EType::Object;
		Root->Object.Add({ TEXT("values"), Values });
	}
	FJPtr Old = Values->Get(Key.ToString());
	Call.OldValue = Old && Old->IsNumber() ? Old->Number : (Def && Def->IsNumber() ? Def->Number : 0.0);
	FJPtr NewNum = MakeShared<FJ>();
	NewNum->Type = FJ::EType::Number;
	NewNum->Number = Value;
	Values->Object.RemoveAll([&](const TPair<FString, FJPtr>& P) { return P.Key == Key.ToString(); });
	Values->Object.Add({ Key.ToString(), NewNum });

	TArray<FString> Errors;
	UJamCartridge* C = UJamCartridge::LoadFromString(Root->ToCompact(), Errors, this);
	if (!C) return Refuse(Errors.Num() ? Errors[0] : TEXT("the cut was rejected"));
	C->SourcePath = Current->SourcePath;
	Current = C;
	bCallUsedThisPeriod = true;
	Call.bApplied = true;
	Call.Cartridge = C;
	Call.Announcement = Announce(Key, Call.OldValue, Value, Lo, Hi);
	UE_LOG(LogJamDesigner, Log, TEXT("JAMDESIGNER directors_call applied key=%s old=%s new=%s \"%s\""), *Key.ToString(), *FJ::NumberToString(Call.OldValue), *FJ::NumberToString(Value), *Call.Announcement);

	if (UJamEventBus* Bus = GetBus())
	{
		if (UJamEventBus::IsKnownEvent(DirectorsCallEvent))
		{
			FJamEventPayload P;
			P.Kind = Key;
			P.Strength = Hi > Lo ? static_cast<float>((Value - Lo) / (Hi - Lo)) : 0.f;
			Bus->Broadcast(DirectorsCallEvent, P);
		}
		else
		{
			// Seam: lane A1 adds "directors_call" to JamEvents (contracts/events.md note in the A7 report). Until then only OnDirectorsCall fires.
			UE_LOG(LogJamDesigner, Log, TEXT("JAMDESIGNER seam: the event bus does not know \"directors_call\" yet; delivered through OnDirectorsCall only"));
		}
	}
	OnDirectorsCall.Broadcast(Call);
	if (!CallGameMode(TEXT("ApplyCartridgeLive"), FString(), C))
	{
		UE_LOG(LogJamDesigner, Log, TEXT("JAMDESIGNER seam: no game mode ApplyCartridgeLive(UJamCartridge*); the call is current and applies at the next match"));
	}
	return Call;
}

// ---- D1 ----

FString UJamDesignerSubsystem::CurrentJsonOrDefault() const
{
	if (Current && !Current->RawJson.IsEmpty()) return Current->RawJson;
	FString Text;
	FFileHelper::LoadFileToString(Text, *UJamCartridge::ResolveCartridgePath(TEXT("hockey.regular")));
	return Text;
}

TArray<FJamCutInfo> UJamDesignerSubsystem::GetCachedCuts() const
{
	TArray<FJamCutInfo> Out;
	for (const FCachedCut& C : CachedCuts())
	{
		FJamCutInfo I;
		I.Name = C.Name;
		I.Sentence = C.Sentence;
		Out.Add(I);
	}
	return Out;
}

TArray<FString> UJamDesignerSubsystem::GetCachedSentences() const
{
	TArray<FString> Out;
	for (const FCachedCut& C : CachedCuts()) Out.Add(C.Sentence);
	return Out;
}

FJamDesignerResult UJamDesignerSubsystem::ResolveOffline(const FString& Sentence) const
{
	FJamDesignerResult R;
	R.Sentence = Sentence;
	const TSet<FString> Query = Words(Sentence);
	int32 BestScore = 0;
	FString BestJson, BestName;
	auto Consider = [&](const FString& Name, const FString& CutSentence, const TArray<FString>& Keywords, const FString& Json)
	{
		// A word of the cut's own sentence scores 2, a keyword 1; the first best cut wins a tie.
		const TSet<FString> Own = Words(CutSentence);
		TSet<FString> Extra;
		for (const FString& K : Keywords) Extra.Append(Words(K));
		int32 Score = 0;
		for (const FString& W : Query) Score += Own.Contains(W) ? 2 : (Extra.Contains(W) ? 1 : 0);
		if (Score > BestScore) { BestScore = Score; BestJson = Json; BestName = Name; }
	};
	for (const FCachedCut& C : CachedCuts()) Consider(C.Name, C.Sentence, C.Keywords, C.Json);
	for (const FJamCutInfo& I : ListCuts())
	{
		if (!I.bSaved || I.Sentence.IsEmpty()) continue;
		FString Json;
		if (FFileHelper::LoadFileToString(Json, *I.Path)) Consider(I.Name, I.Sentence, {}, Json);
	}
	if (BestScore == 0)
	{
		R.Outcome = EJamDesignerOutcome::OfflineNoMatch;
		R.Message = TEXT("No key, so the Designer answers from its cached cuts. Try: ") + FString::Join(GetCachedSentences(), TEXT(" / "));
		return R;
	}
	TArray<FString> Errors;
	UJamCartridge* C = UJamCartridge::LoadFromString(BestJson, Errors, const_cast<UJamDesignerSubsystem*>(this));
	if (!C)
	{
		R.Outcome = EJamDesignerOutcome::OfflineNoMatch;
		R.Errors = Errors;
		R.Message = FString::Printf(TEXT("The cached cut \"%s\" was rejected by the loader."), *BestName);
		return R;
	}
	R.Outcome = EJamDesignerOutcome::Offline;
	R.Cartridge = C;
	R.CartridgeJson = BestJson;
	R.CutName = BestName;
	R.Message = FString::Printf(TEXT("Offline Designer: \"%s\""), *BestName);
	return R;
}

void UJamDesignerSubsystem::Finish(const FJamDesignerResult& Result, const TFunction<void(const FJamDesignerResult&)>& Done)
{
	bBusy = false;
	if (Done) Done(Result);
	OnDesignerResult.Broadcast(Result);
}

void UJamDesignerSubsystem::RequestCut(const FString& Sentence, const FString& CurrentCartridgeJson, TFunction<void(const FJamDesignerResult&)> Done)
{
	if (bBusy)
	{
		FJamDesignerResult Busy;
		Busy.Outcome = EJamDesignerOutcome::Busy;
		Busy.Sentence = Sentence;
		Busy.Message = TEXT("The Designer is still working on the last sentence.");
		if (Done) Done(Busy);
		return;
	}
	bBusy = true;
	// Route (R-0004): API key -> HTTP; else the claude CLI (not Shipping, or bAllowCliTransport) -> CLI; else offline.
	const FString Key = ResolveKey();
	const FString CliExe = Key.IsEmpty() ? ResolveCliExe() : FString();
	if (Key.IsEmpty() && CliExe.IsEmpty())
	{
		// No key and no CLI: no network, ever. Resolve from the cache in the same frame.
		FJamDesignerResult R = ResolveOffline(Sentence);
		UE_LOG(LogJamDesigner, Log, TEXT("JAMDESIGNER offline outcome=%s cut=\"%s\""), *StaticEnum<EJamDesignerOutcome>()->GetNameStringByValue(static_cast<int64>(R.Outcome)), *R.CutName);
		Finish(R, Done);
		return;
	}
	TSharedRef<FJamDesignerJob> Job = MakeShared<FJamDesignerJob>();
	Job->Owner = this;
	Job->Transport = CliExe.IsEmpty() ? Transport : TSharedPtr<IJamDesignerTransport>(MakeShared<FJamCliTransport>(CliExe));
	Job->Done = MoveTemp(Done);
	Job->Sentence = Sentence;
	Job->BaseJson = CurrentCartridgeJson.IsEmpty() ? CurrentJsonOrDefault() : CurrentCartridgeJson;
	Job->Key = Key;
	Job->TimeoutSec = CliExe.IsEmpty() ? TimeoutSec : CliTimeoutSec;
	UE_LOG(LogJamDesigner, Log, TEXT("JAMDESIGNER route=%s"), CliExe.IsEmpty() ? TEXT("http") : TEXT("cli"));
	Job->RetryDelaySec = RetryDelaySec;
	Job->Result.Sentence = Sentence;
	Job->Start();
}

void UJamDesignerSubsystem::RequestCutBP(const FString& Sentence, const FString& CurrentCartridgeJson, FJamDesignerCallback Done)
{
	RequestCut(Sentence, CurrentCartridgeJson, [Done](const FJamDesignerResult& R) { Done.ExecuteIfBound(R); });
}
