#include "Progression.h"
#include "UObject/Package.h"   // GetTransientPackage() outside the unity blob (lane A18, R-0021 pass)
#include "MenuFlow.h"
#include "InputMapping.h"   // lane A195: the restart key in the season's key lines
#include "JamJson.h"
#include "JamCartridge.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/IConsoleManager.h"
#include "HokejGameMode.h"

DEFINE_LOG_CATEGORY(LogJamProgression);

namespace JamProgressionLocal
{
	using FJ = FJamJson;
	using FJPtr = TSharedPtr<FJamJson>;

	const TCHAR* LadderSchema = TEXT("tj.ladder/1");
	const TCHAR* ProgressSchema = TEXT("tj.progress/1");
	constexpr int32 MaxTakes = 12;

	bool IsKebab(const FString& S)
	{
		if (S.IsEmpty() || S.Len() > 48 || !FChar::IsLower(S[0])) return false;
		bool bPrevDash = false;
		for (TCHAR C : S)
		{
			const bool bOk = (C >= 'a' && C <= 'z') || (C >= '0' && C <= '9') || C == '-';
			if (!bOk || (C == '-' && bPrevDash)) return false;
			bPrevDash = C == '-';
		}
		return !bPrevDash;
	}

	// Unknown keys first (file order), then required keys: the same order schema/validate.mjs walks.
	void CheckKeys(const FJ& Obj, const FString& Ptr, const TArray<FString>& Allowed, const TArray<FString>& Required, TArray<FString>& Errors)
	{
		for (const FString& R : Required)
		{
			if (!Obj.Has(R)) Errors.Add(FString::Printf(TEXT("at %s: required key \"%s\" is missing."), *FJ::PointerChild(Ptr, R), *R));
		}
		for (const TPair<FString, FJPtr>& P : Obj.Object)
		{
			if (!Allowed.Contains(P.Key))
			{
				Errors.Add(FString::Printf(TEXT("at %s: unknown key \"%s\" is not allowed here; remove it. Allowed keys: %s."), *FJ::PointerChild(Ptr, P.Key), *P.Key, *FString::Join(Allowed, TEXT(", "))));
			}
		}
	}

	bool ReadString(const FJ& Obj, const FString& Ptr, const FString& Key, int32 MinLen, int32 MaxLen, FString& Out, TArray<FString>& Errors)
	{
		const FJPtr V = Obj.Get(Key);
		if (!V) return false;
		const FString At = FJ::PointerChild(Ptr, Key);
		if (!V->IsString()) { Errors.Add(FString::Printf(TEXT("at %s: expected a string."), *At)); return false; }
		if (V->String.Len() < MinLen || V->String.Len() > MaxLen)
		{
			Errors.Add(FString::Printf(TEXT("at %s: the string is %d characters long; it must be %d to %d."), *At, V->String.Len(), MinLen, MaxLen));
			return false;
		}
		Out = V->String;
		return true;
	}

	bool ReadCount(const FJ& Obj, const FString& Ptr, const FString& Key, int32& Out, TArray<FString>& Errors)
	{
		const FJPtr V = Obj.Get(Key);
		if (!V) return false;
		if (!V->IsInteger() || V->Number < 0 || V->Number > 1000000)
		{
			Errors.Add(FString::Printf(TEXT("at %s: expected a whole number from 0."), *FJ::PointerChild(Ptr, Key)));
			return false;
		}
		Out = static_cast<int32>(V->Number);
		return true;
	}

	FString Quote(const FString& S)
	{
		FString O = S.Replace(TEXT("\\"), TEXT("\\\\")).Replace(TEXT("\""), TEXT("\\\""));
		return TEXT("\"") + O + TEXT("\"");
	}
}

using namespace JamProgressionLocal;

// ---------------------------------------------------------------- pure rules

bool UJamProgression::ParseLadder(const FString& Text, FJamLadder& Out, TArray<FString>& OutErrors)
{
	Out = FJamLadder();
	FString Err;
	const FJPtr Root = FJ::Parse(Text, Err);
	if (!Root) { OutErrors.Add(Err); return false; }
	if (!Root->IsObject()) { OutErrors.Add(TEXT("at /: expected an object.")); return false; }
	const int32 Before = OutErrors.Num();
	CheckKeys(*Root, FString(), { TEXT("schema"), TEXT("ladder") }, { TEXT("schema"), TEXT("ladder") }, OutErrors);
	if (const FJPtr S = Root->Get(TEXT("schema")); S && !(S->IsString() && S->String == LadderSchema))
	{
		OutErrors.Add(FString::Printf(TEXT("at /schema: must be exactly \"%s\"."), LadderSchema));
	}
	const FJPtr L = Root->Get(TEXT("ladder"));
	if (L && !L->IsObject()) OutErrors.Add(TEXT("at /ladder: expected an object."));
	if (L && L->IsObject())
	{
		CheckKeys(*L, TEXT("/ladder"), { TEXT("id"), TEXT("takes") }, { TEXT("id"), TEXT("takes") }, OutErrors);
		if (ReadString(*L, TEXT("/ladder"), TEXT("id"), 1, 48, Out.Id, OutErrors) && !IsKebab(Out.Id))
		{
			OutErrors.Add(TEXT("at /ladder/id: must be a kebab-case id such as season-one."));
		}
		const FJPtr T = L->Get(TEXT("takes"));
		if (T && !T->IsArray()) OutErrors.Add(TEXT("at /ladder/takes: expected an array."));
		if (T && T->IsArray())
		{
			if (T->Array.Num() < 1) OutErrors.Add(TEXT("at /ladder/takes: the list has 0 items; it needs at least 1."));
			if (T->Array.Num() > MaxTakes) OutErrors.Add(FString::Printf(TEXT("at /ladder/takes: the list has %d items; the limit is %d."), T->Array.Num(), MaxTakes));
			for (int32 i = 0; i < T->Array.Num(); ++i)
			{
				const FString P = FJ::PointerChild(TEXT("/ladder/takes"), i);
				const FJPtr E = T->Array[i];
				if (!E || !E->IsObject()) { OutErrors.Add(FString::Printf(TEXT("at %s: expected an object."), *P)); continue; }
				CheckKeys(*E, P, { TEXT("title"), TEXT("cartridge"), TEXT("twist_text"), TEXT("unlock"), TEXT("fallback") }, { TEXT("title"), TEXT("cartridge"), TEXT("twist_text"), TEXT("unlock") }, OutErrors);
				FJamLadderTake Take;
				ReadString(*E, P, TEXT("title"), 1, 32, Take.Title, OutErrors);
				if (ReadString(*E, P, TEXT("cartridge"), 1, 48, Take.Cartridge, OutErrors) && !IsKebab(Take.Cartridge))
				{
					OutErrors.Add(FString::Printf(TEXT("at %s/cartridge: must be a preset id (kebab-case)."), *P));
				}
				ReadString(*E, P, TEXT("twist_text"), 1, 90, Take.TwistText, OutErrors);
				if (ReadString(*E, P, TEXT("fallback"), 1, 48, Take.Fallback, OutErrors) && !IsKebab(Take.Fallback))
				{
					OutErrors.Add(FString::Printf(TEXT("at %s/fallback: must be a preset id (kebab-case)."), *P));
				}
				FString Unlock;
				if (ReadString(*E, P, TEXT("unlock"), 1, 32, Unlock, OutErrors))
				{
					if (Unlock == TEXT("always")) Take.bUnlockAlways = true;
					else if (Unlock != TEXT("previous_won")) OutErrors.Add(FString::Printf(TEXT("at %s/unlock: \"%s\" is not allowed; use one of \"always\", \"previous_won\"."), *P, *Unlock));
				}
				if (i == 0 && Take.Cartridge.Len() && !Take.bUnlockAlways && Unlock == TEXT("previous_won"))
				{
					OutErrors.Add(TEXT("at /ladder/takes/0/unlock: the first take must unlock \"always\", or the season can never start."));
				}
				// Every preset id (cartridge and fallback) belongs to one take only.
				auto Claim = [&](const FString& Id, const TCHAR* Key, bool bSameTakeToo)
				{
					if (Id.IsEmpty()) return;
					for (int32 j = 0; j < Out.Takes.Num(); ++j)
					{
						if (Out.Takes[j].Cartridge == Id || Out.Takes[j].Fallback == Id)
						{
							OutErrors.Add(FString::Printf(TEXT("at %s/%s: \"%s\" is already used by take %d; each take is its own cut."), *P, Key, *Id, j + 1));
							return;
						}
					}
					if (bSameTakeToo && Id == Take.Cartridge) OutErrors.Add(FString::Printf(TEXT("at %s/%s: \"%s\" is already used by take %d; each take is its own cut."), *P, Key, *Id, i + 1));
				};
				Claim(Take.Cartridge, TEXT("cartridge"), false);
				Claim(Take.Fallback, TEXT("fallback"), true);
				Out.Takes.Add(Take);
			}
		}
	}
	return OutErrors.Num() == Before;
}

bool UJamProgression::ParseProgress(const FString& Text, FJamSeasonProgress& Out, TArray<FString>& OutErrors)
{
	Out = FJamSeasonProgress();
	FString Err;
	const FJPtr Root = FJ::Parse(Text, Err);
	if (!Root) { OutErrors.Add(Err); return false; }
	if (!Root->IsObject()) { OutErrors.Add(TEXT("at /: expected an object.")); return false; }
	const int32 Before = OutErrors.Num();
	CheckKeys(*Root, FString(), { TEXT("schema"), TEXT("ladder"), TEXT("takes") }, { TEXT("schema"), TEXT("ladder") }, OutErrors);
	if (const FJPtr S = Root->Get(TEXT("schema")); S && !(S->IsString() && S->String == ProgressSchema))
	{
		OutErrors.Add(FString::Printf(TEXT("at /schema: must be exactly \"%s\"."), ProgressSchema));
	}
	ReadString(*Root, FString(), TEXT("ladder"), 1, 48, Out.LadderId, OutErrors);
	if (const FJPtr T = Root->Get(TEXT("takes")))
	{
		if (!T->IsArray()) OutErrors.Add(TEXT("at /takes: expected an array."));
		else
		{
			for (int32 i = 0; i < T->Array.Num(); ++i)
			{
				const FString P = FJ::PointerChild(TEXT("/takes"), i);
				const FJPtr E = T->Array[i];
				if (!E || !E->IsObject()) { OutErrors.Add(FString::Printf(TEXT("at %s: expected an object."), *P)); continue; }
				CheckKeys(*E, P, { TEXT("cartridge"), TEXT("attempts"), TEXT("wins") }, { TEXT("cartridge"), TEXT("attempts") }, OutErrors);
				FJamTakeRecord R;
				ReadString(*E, P, TEXT("cartridge"), 1, 48, R.Cartridge, OutErrors);
				ReadCount(*E, P, TEXT("attempts"), R.Attempts, OutErrors);
				ReadCount(*E, P, TEXT("wins"), R.Wins, OutErrors);
				if (R.Wins > R.Attempts) OutErrors.Add(FString::Printf(TEXT("at %s/wins: more wins than attempts."), *P));
				if (Out.Find(R.Cartridge)) OutErrors.Add(FString::Printf(TEXT("at %s/cartridge: \"%s\" is listed twice."), *P, *R.Cartridge));
				Out.Takes.Add(R);
			}
		}
	}
	return OutErrors.Num() == Before;
}

FString UJamProgression::ProgressToJson(const FJamSeasonProgress& P)
{
	FString S = FString::Printf(TEXT("{\n  \"schema\": %s,\n  \"ladder\": %s"), *Quote(ProgressSchema), *Quote(P.LadderId));
	if (P.Takes.Num())
	{
		S += TEXT(",\n  \"takes\": [");
		for (int32 i = 0; i < P.Takes.Num(); ++i)
		{
			const FJamTakeRecord& R = P.Takes[i];
			// Absent = omit: wins is written only when there is one.
			S += FString::Printf(TEXT("%s\n    { \"cartridge\": %s, \"attempts\": %d"), i ? TEXT(",") : TEXT(""), *Quote(R.Cartridge), R.Attempts);
			if (R.Wins > 0) S += FString::Printf(TEXT(", \"wins\": %d"), R.Wins);
			S += TEXT(" }");
		}
		S += TEXT("\n  ]");
	}
	S += TEXT("\n}\n");
	return S;
}

bool UJamProgression::IsUnlocked(const FJamLadder& L, const FJamSeasonProgress& P, int32 Index)
{
	if (!L.Takes.IsValidIndex(Index)) return false;
	if (L.Takes[Index].bUnlockAlways) return true;
	// A take played before (an older ladder, or a cut the player already reached) stays open.
	if (P.Find(L.Takes[Index].Cartridge)) return true;
	return Index > 0 && P.Won(L.Takes[Index - 1].Cartridge);
}

int32 UJamProgression::CurrentTakeIndex(const FJamLadder& L, const FJamSeasonProgress& P)
{
	for (int32 i = 0; i < L.Takes.Num(); ++i)
	{
		if (IsUnlocked(L, P, i) && !P.Won(L.Takes[i].Cartridge)) return i;
	}
	return FMath::Max(0, L.Takes.Num() - 1);
}

void UJamProgression::ApplyResult(FJamSeasonProgress& P, const FString& Cartridge, bool bWon)
{
	FJamTakeRecord* R = P.Takes.FindByPredicate([&](const FJamTakeRecord& X) { return X.Cartridge == Cartridge; });
	if (!R)
	{
		R = &P.Takes.AddDefaulted_GetRef();
		R->Cartridge = Cartridge;
	}
	++R->Attempts;
	if (bWon) ++R->Wins;
}

FJamSeasonState UJamProgression::BuildState(const FJamLadder& L, const FJamSeasonProgress& P, int32 InActive)
{
	FJamSeasonState S;
	S.bLoaded = L.Takes.Num() > 0;
	S.TakeCount = L.Takes.Num();
	S.ActiveTake = L.Takes.IsValidIndex(InActive) ? InActive : -1;
	for (int32 i = 0; i < L.Takes.Num(); ++i)
	{
		FJamSeasonTakeRow Row;
		Row.Title = L.Takes[i].Title;
		Row.Note = L.Takes[i].TwistText;
		Row.bUnlocked = IsUnlocked(L, P, i);
		Row.bWon = P.Won(L.Takes[i].Cartridge);
		if (const FJamTakeRecord* R = P.Find(L.Takes[i].Cartridge)) Row.Attempts = R->Attempts;
		S.TakesWon += Row.bWon ? 1 : 0;
		S.Takes.Add(Row);
	}
	S.bComplete = S.TakeCount > 0 && S.TakesWon == S.TakeCount;
	S.CurrentTake = CurrentTakeIndex(L, P);
	if (!S.bLoaded) S.MenuLine = TEXT("No season on this build.");
	else if (S.bComplete) S.MenuLine = FString::Printf(TEXT("Season wrapped: %d of %d takes. Replay the final take."), S.TakesWon, S.TakeCount);
	else S.MenuLine = FString::Printf(TEXT("Take %d of %d: %s"), S.CurrentTake + 1, S.TakeCount, *L.Takes[S.CurrentTake].Title);
	return S;
}

// ---------------------------------------------------------------- the subsystem

UJamProgression* UJamProgression::Get(const UObject* WorldContext)
{
	const UWorld* W = WorldContext && GEngine ? GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull) : nullptr;
	const UGameInstance* GI = W ? W->GetGameInstance() : nullptr;
	return GI ? GI->GetSubsystem<UJamProgression>() : nullptr;
}

void UJamProgression::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Reload();
}

FString UJamProgression::DefaultLadderPath()
{
	return FPaths::Combine(UJamCartridge::GetCartridgeDir(), TEXT("ladder"), TEXT("season.ladder.json"));
}

FString UJamProgression::DefaultProgressPath()
{
	return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Progress.json"));
}

void UJamProgression::SetPathsForTest(const FString& InLadderPath, const FString& InProgressPath)
{
	LadderPath = InLadderPath;
	ProgressPath = InProgressPath;
	ActiveTake = -1;
	LastResult = EJamTakeResult::None;
	Reload();
}

bool UJamProgression::Reload()
{
	const FString LP = LadderPath.IsEmpty() ? DefaultLadderPath() : LadderPath;
	const FString PP = ProgressPath.IsEmpty() ? DefaultProgressPath() : ProgressPath;
	Ladder = FJamLadder();
	Progress = FJamSeasonProgress();
	LoadError.Reset();
	bLadderLoaded = false;
	FString Text;
	TArray<FString> Errors;
	if (!FFileHelper::LoadFileToString(Text, *LP)) LoadError = FString::Printf(TEXT("cannot read %s"), *LP);
	else if (!ParseLadder(Text, Ladder, Errors)) LoadError = Errors.Num() ? Errors[0] : TEXT("the ladder was rejected");
	else bLadderLoaded = true;
	for (const FString& E : Errors) UE_LOG(LogJamProgression, Warning, TEXT("JAMSEASON ladder %s rejected: %s"), *LP, *E);
	if (!bLadderLoaded)
	{
		Ladder = FJamLadder();
		UE_LOG(LogJamProgression, Warning, TEXT("JAMSEASON no season: %s"), *LoadError);
		return false;
	}
	Progress.LadderId = Ladder.Id;
	if (FFileHelper::LoadFileToString(Text, *PP))
	{
		FJamSeasonProgress P;
		TArray<FString> PErrors;
		if (!ParseProgress(Text, P, PErrors))
		{
			for (const FString& E : PErrors) UE_LOG(LogJamProgression, Warning, TEXT("JAMSEASON %s rejected (the season starts fresh): %s"), *PP, *E);
		}
		else if (P.LadderId != Ladder.Id)
		{
			UE_LOG(LogJamProgression, Warning, TEXT("JAMSEASON %s belongs to ladder %s, not %s: the season starts fresh"), *PP, *P.LadderId, *Ladder.Id);
		}
		else Progress = P;
	}
	UE_LOG(LogJamProgression, Log, TEXT("JAMSEASON ladder %s, %d takes, %s"), *Ladder.Id, Ladder.Takes.Num(), *GetSeasonState().MenuLine);
	return true;
}

FJamSeasonState UJamProgression::GetSeasonState() const
{
	FJamSeasonState S = BuildState(Ladder, Progress, ActiveTake);
	if (!bLadderLoaded) S.Error = LoadError;
	return S;
}

FString UJamProgression::TakeFilePath(int32 Index, bool bFallback) const
{
	if (!Ladder.Takes.IsValidIndex(Index)) return FString();
	const FString& Id = bFallback ? Ladder.Takes[Index].Fallback : Ladder.Takes[Index].Cartridge;
	if (Id.IsEmpty()) return FString();
	const FString LP = LadderPath.IsEmpty() ? DefaultLadderPath() : LadderPath;
	return FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::GetPath(LP), Id + TEXT(".cartridge.json")));
}

FString UJamProgression::TakePath(int32 Index, bool* bOutFallback) const
{
	if (bOutFallback) *bOutFallback = false;
	const FString Designed = TakeFilePath(Index, false);
	if (Designed.IsEmpty() || Ladder.Takes[Index].Fallback.IsEmpty()) return Designed;
	// R-0014: the designed cut plays when the loader accepts it; otherwise the fallback.
	TArray<FString> Errors;
	if (UJamCartridge::LoadFromFile(Designed, Errors, GetTransientPackage())) return Designed;
	UE_LOG(LogJamProgression, Log, TEXT("JAMSEASON take %d plays its fallback %s: %s"), Index + 1, *Ladder.Takes[Index].Fallback, Errors.Num() ? *Errors[0] : TEXT("rejected"));
	if (bOutFallback) *bOutFallback = true;
	return TakeFilePath(Index, true);
}

void UJamProgression::BeginTake(int32 Index)
{
	ActiveTake = Ladder.Takes.IsValidIndex(Index) ? Index : -1;
	ActivePath = ActiveTake >= 0 ? TakePath(ActiveTake) : FString();
	LastResult = EJamTakeResult::None;
}

bool UJamProgression::StartTake(UObject* WorldContext, int32 Index)
{
	if (!IsUnlocked(Ladder, Progress, Index))
	{
		UE_LOG(LogJamProgression, Warning, TEXT("JAMSEASON take %d is locked or does not exist"), Index + 1);
		return false;
	}
	BeginTake(Index);
	UE_LOG(LogJamProgression, Log, TEXT("JAMSEASON start take %d of %d: %s (%s)"), Index + 1, Ladder.Takes.Num(), *Ladder.Takes[Index].Title, *FPaths::GetCleanFilename(ActivePath));
	UJamMenuFlow::StartMatch(WorldContext, ActivePath);
	return true;
}

bool UJamProgression::StartSeason(UObject* WorldContext)
{
	if (!bLadderLoaded && !Reload()) return false;
	return StartTake(WorldContext, CurrentTakeIndex(Ladder, Progress));
}

void UJamProgression::EndSeasonRun()
{
	if (ActiveTake >= 0) UE_LOG(LogJamProgression, Log, TEXT("JAMSEASON season run ends at take %d"), ActiveTake + 1);
	ActiveTake = -1;
	ActivePath.Reset();
	LastResult = EJamTakeResult::None;
}

bool UJamProgression::IsSeasonMatch(const FString& CartridgePath) const
{
	return ActiveTake >= 0 && !CartridgePath.IsEmpty() && !ActivePath.IsEmpty() && FPaths::IsSamePath(FPaths::ConvertRelativePathToFull(CartridgePath), ActivePath);
}

void UJamProgression::RecordResult(const FString& CartridgePath, bool bWon)
{
	if (!IsSeasonMatch(CartridgePath)) return;
	const FJamLadderTake& T = Ladder.Takes[ActiveTake];
	ApplyResult(Progress, T.Cartridge, bWon);
	LastResult = bWon ? EJamTakeResult::Won : EJamTakeResult::Lost;
	const bool bSaved = SaveProgress();
	UE_LOG(LogJamProgression, Log, TEXT("JAMSEASON take %d %s %s; %s%s"), ActiveTake + 1, *T.Title, bWon ? TEXT("WON") : TEXT("LOST"),
		*GetSeasonState().MenuLine, bSaved ? TEXT("") : TEXT(" (progress NOT saved)"));
}

bool UJamProgression::HandleRestart(UObject* WorldContext)
{
	if (ActiveTake < 0 || LastResult != EJamTakeResult::Won) return false;
	const int32 Next = ActiveTake + 1;
	if (Ladder.Takes.IsValidIndex(Next) && IsUnlocked(Ladder, Progress, Next)) return StartTake(WorldContext, Next);
	UE_LOG(LogJamProgression, Log, TEXT("JAMSEASON season wrapped"));
	EndSeasonRun();
	UJamMenuFlow::ReturnToMenu(WorldContext);
	return true;
}

FString UJamProgression::EndHint(const FString& CartridgePath) const
{
	if (!IsSeasonMatch(CartridgePath) || LastResult == EJamTakeResult::None) return FString();
	// Lane A195 (evolution 7, the Director's word): the restart key as the player has it bound (A183), not a fixed R.
	const FJamVerbBinding& Bi = UJamInputMap::Get()->GetBindings()[EJamVerb::Restart];
	const FString R = Bi.Keys.Num() ? UJamInputMap::KeyName(Bi.Keys[0]) : FString(TEXT("R"));
	if (LastResult == EJamTakeResult::Lost) return FString::Printf(TEXT("%s  REMATCH      ESC  MENU"), *R);
	if (!Ladder.Takes.IsValidIndex(ActiveTake + 1)) return FString::Printf(TEXT("SEASON COMPLETE      %s  MENU"), *R);
	return FString::Printf(TEXT("%s  NEXT MATCH: %s      ESC  MENU"), *R, *Ladder.Takes[ActiveTake + 1].Title.ToUpper());
}

FString UJamProgression::ActiveNote(const FString& CartridgePath) const
{
	return IsSeasonMatch(CartridgePath) ? Ladder.Takes[ActiveTake].TwistText : FString();
}

FString UJamProgression::ActiveTitle(const FString& CartridgePath) const
{
	return IsSeasonMatch(CartridgePath) ? Ladder.Takes[ActiveTake].Title : FString();
}

TArray<FJamCutInfo> UJamProgression::AppendPlayedTakes(const TArray<FJamCutInfo>& Cuts) const
{
	TArray<FJamCutInfo> Out = Cuts;
	for (int32 i = 0; i < Ladder.Takes.Num(); ++i)
	{
		if (!Progress.Find(Ladder.Takes[i].Cartridge)) continue;
		FJamCutInfo I;
		I.Name = FString::Printf(TEXT("Take %d: %s"), i + 1, *Ladder.Takes[i].Title);
		I.Path = TakePath(i);
		// Listed like a saved cut: by path, under its own name.
		I.bSaved = true;
		Out.Add(I);
	}
	return Out;
}

bool UJamProgression::SaveProgress() const
{
	const FString PP = ProgressPath.IsEmpty() ? DefaultProgressPath() : ProgressPath;
	IFileManager::Get().MakeDirectory(*FPaths::GetPath(PP), true);
	const bool bOk = FFileHelper::SaveStringToFile(ProgressToJson(Progress), *PP, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	if (!bOk) UE_LOG(LogJamProgression, Warning, TEXT("JAMSEASON cannot write %s"), *PP);
	return bOk;
}

// ---------------------------------------------------------------- console (QA and the MCP proof)

namespace JamProgressionConsole
{
	UWorld* GameWorld(UWorld* W)
	{
		if (W && W->IsGameWorld()) return W;
		if (GEngine) for (const FWorldContext& C : GEngine->GetWorldContexts()) if ((C.WorldType == EWorldType::PIE || C.WorldType == EWorldType::Game) && C.World()) return C.World();
		return nullptr;
	}

	static FAutoConsoleCommandWithWorldAndArgs StartCmd(TEXT("jam.Season.Start"), TEXT("Starts or resumes the season (same as PLAY THE SEASON)."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>&, UWorld* W)
		{
			W = GameWorld(W);
			if (UJamProgression* P = UJamProgression::Get(W)) P->StartSeason(W);
		}));

	static FAutoConsoleCommandWithWorldAndArgs RestartCmd(TEXT("jam.Season.Restart"), TEXT("The R key at the end of a season take: NEXT TAKE after a win, else RETAKE."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>&, UWorld* W)
		{
			W = GameWorld(W);
			UJamProgression* P = UJamProgression::Get(W);
			if (P && P->HandleRestart(W)) return;
			if (AHokejGameMode* GM = W ? W->GetAuthGameMode<AHokejGameMode>() : nullptr) GM->RestartMatch();
		}));

	static FAutoConsoleCommandWithWorldAndArgs StatusCmd(TEXT("jam.Season.Status"), TEXT("Logs the season state (JAMSEASON lines)."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>&, UWorld* W)
		{
			if (UJamProgression* P = UJamProgression::Get(GameWorld(W)))
			{
				const FJamSeasonState S = P->GetSeasonState();
				UE_LOG(LogJamProgression, Log, TEXT("JAMSEASON status: %s; won %d of %d; active take %d; last result %d"), *S.MenuLine, S.TakesWon, S.TakeCount, S.ActiveTake + 1, static_cast<int32>(P->GetLastResult()));
			}
		}));

	static FAutoConsoleCommandWithWorldAndArgs ResetCmd(TEXT("jam.Season.Reset"), TEXT("Deletes Saved/Progress.json and starts the season over."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>&, UWorld* W)
		{
			IFileManager::Get().Delete(*UJamProgression::DefaultProgressPath(), false, true, true);
			if (UJamProgression* P = UJamProgression::Get(GameWorld(W))) { P->EndSeasonRun(); P->Reload(); }
			UE_LOG(LogJamProgression, Log, TEXT("JAMSEASON progress reset"));
		}));
}
