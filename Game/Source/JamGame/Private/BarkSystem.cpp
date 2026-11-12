#include "BarkSystem.h"
#include "BarkWidget.h"
#include "BarkData.inl"
#include "HokejGameMode.h"
#include "HUDFeed.h"
#include "JamCartridge.h"
#include "JamJson.h"
#include "JamPawns.h"
#include "JamSim.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "Subsystems/EngineSubsystem.h"
#include "UObject/UnrealType.h"

DEFINE_LOG_CATEGORY(LogJamBarks);

namespace JamBarks
{
	const FName GoalFor(TEXT("goal_for"));
	const FName GoalAgainst(TEXT("goal_against"));
	const FName CheckLanded(TEXT("check_landed"));
	const FName CheckReceived(TEXT("check_received"));
	const FName Steal(TEXT("steal"));
	const FName Stall(TEXT("stall"));
	const FName NearMiss(TEXT("near_miss"));
	const FName Faceoff(TEXT("faceoff"));
	const FName Win(TEXT("win"));
	const FName Lose(TEXT("lose"));
	const FName DirectorsCall(TEXT("directors_call"));
	const FName SignalAckDefend(TEXT("signal_ack_defend"));
	const FName SignalAckSpace(TEXT("signal_ack_space"));
	const FName SignalAckPass(TEXT("signal_ack_pass"));
	const FName PraiseReply(TEXT("praise_reply"));
	const FName SpeakerOpponent(TEXT("opponent"));
	const FName SpeakerTeammate(TEXT("teammate"));
	const FName ToneMock(TEXT("mock"));
	const FName ToneSavage(TEXT("savage"));
	const FName ToneComplain(TEXT("complain"));
	const FName ToneBravado(TEXT("bravado"));
	const FName ToneEncourage(TEXT("encourage"));
	const FName TonePraise(TEXT("praise"));
	const FName IntensityQuiet(TEXT("quiet"));
	const FName IntensityNormal(TEXT("normal"));
	const FName IntensityLoud(TEXT("loud"));
	const FName BusEvent(TEXT("bark"));
}

static TAutoConsoleVariable<int32> CVarJamBarks(TEXT("jam.Barks"), -1,
	TEXT("Barks override: -1 = the cartridge's game.barks.enabled (default), 0 = off, 1 = on."));
static TAutoConsoleVariable<int32> CVarJamBarkVoice(TEXT("jam.BarkVoice"), 0,
	TEXT("1: voice every bark through the engine's TextToSpeech plugin (Flite on Windows) when that plugin is enabled; 0: text only."));

// ---------------------------------------------------------------- bank ----------------------------------------------------------------

bool FJamBarkBank::Parse(const FString& Json, TArray<FString>& OutErrors)
{
	Lines.Reset();
	Index.Reset();
	FString Err;
	TSharedPtr<FJamJson> Root = FJamJson::Parse(Json, Err);
	if (!Root) { OutErrors.Add(Err); return false; }
	if (!Root->IsObject()) { OutErrors.Add(TEXT("at /: the bank must be an object")); return false; }
	static const TCHAR* Known[] = { TEXT("schema"), TEXT("note"), TEXT("generated"), TEXT("model"), TEXT("counts"), TEXT("lines") };
	for (const TPair<FString, TSharedPtr<FJamJson>>& KV : Root->Object)
	{
		bool bKnown = false;
		for (const TCHAR* K : Known) bKnown |= KV.Key == K;
		if (!bKnown) OutErrors.Add(FString::Printf(TEXT("at /%s: unknown key"), *KV.Key));
	}
	TSharedPtr<FJamJson> Schema = Root->Get(TEXT("schema"));
	if (!Schema || !Schema->IsString() || Schema->String != TEXT("hokej.barks/1")) OutErrors.Add(TEXT("at /schema: must be \"hokej.barks/1\""));
	TSharedPtr<FJamJson> Arr = Root->Get(TEXT("lines"));
	if (!Arr || !Arr->IsArray()) { OutErrors.Add(TEXT("at /lines: must be an array")); return false; }
	TSet<FName> Ids;
	TSet<FString> Texts;
	for (int32 i = 0; i < Arr->Array.Num(); ++i)
	{
		const FString Ptr = FString::Printf(TEXT("/lines/%d"), i);
		const TSharedPtr<FJamJson>& L = Arr->Array[i];
		if (!L || !L->IsObject()) { OutErrors.Add(FString::Printf(TEXT("at %s: must be an object"), *Ptr)); continue; }
		FJamBarkLine Line;
		bool bOk = true;
		for (const TPair<FString, TSharedPtr<FJamJson>>& KV : L->Object)
		{
			const bool bStr = KV.Value.IsValid() && KV.Value->IsString();
			if (KV.Key == TEXT("id") && bStr) Line.Id = FName(*KV.Value->String);
			else if (KV.Key == TEXT("situation") && bStr) Line.Situation = FName(*KV.Value->String);
			else if (KV.Key == TEXT("speaker") && bStr) Line.Speaker = FName(*KV.Value->String);
			else if (KV.Key == TEXT("tone") && bStr) Line.Tone = FName(*KV.Value->String);
			else if (KV.Key == TEXT("text") && bStr) Line.Text = KV.Value->String;
			else { OutErrors.Add(FString::Printf(TEXT("at %s/%s: unknown key or not a string"), *Ptr, *KV.Key)); bOk = false; }
		}
		if (Line.Id.IsNone() || Line.Situation.IsNone() || Line.Speaker.IsNone() || Line.Tone.IsNone() || Line.Text.IsEmpty())
		{
			OutErrors.Add(FString::Printf(TEXT("at %s: id, situation, speaker, tone and text are required"), *Ptr)); bOk = false;
		}
		if (Line.Text.Len() > 60) { OutErrors.Add(FString::Printf(TEXT("at %s/text: longer than 60 characters"), *Ptr)); bOk = false; }
		for (TCHAR C : Line.Text) if (C < 0x20 || C > 0x7E) { OutErrors.Add(FString::Printf(TEXT("at %s/text: non-ASCII character"), *Ptr)); bOk = false; break; }
		if (Ids.Contains(Line.Id)) { OutErrors.Add(FString::Printf(TEXT("at %s/id: duplicate id %s"), *Ptr, *Line.Id.ToString())); bOk = false; }
		const FString Norm = Line.Text.ToLower();
		if (Texts.Contains(Norm)) { OutErrors.Add(FString::Printf(TEXT("at %s/text: duplicate line"), *Ptr)); bOk = false; }
		if (!bOk) continue;
		Ids.Add(Line.Id);
		Texts.Add(Norm);
		Index.FindOrAdd(Key(Line.Situation, Line.Speaker, Line.Tone)).Add(Lines.Num());
		Lines.Add(MoveTemp(Line));
	}
	return OutErrors.Num() == 0;
}

const TArray<int32>& FJamBarkBank::Find(FName Situation, FName Speaker, FName Tone) const
{
	static const TArray<int32> Empty;
	const TArray<int32>* Found = Index.Find(Key(Situation, Speaker, Tone));
	return Found ? *Found : Empty;
}

FString FJamBarkBank::EmbeddedJson()
{
	return FString(UTF8_TO_TCHAR(reinterpret_cast<const char*>(JamBarkBankJson)));
}

const FJamBarkBank& FJamBarkBank::Embedded()
{
	static FJamBarkBank Bank = []
	{
		FJamBarkBank B;
		TArray<FString> Errors;
		if (!B.Parse(EmbeddedJson(), Errors))
		{
			for (const FString& E : Errors) UE_LOG(LogJamBarks, Error, TEXT("JAMBARK bank %s"), *E);
		}
		UE_LOG(LogJamBarks, Log, TEXT("JAMBARK bank loaded: %d lines, %d errors"), B.Lines.Num(), Errors.Num());
		return B;
	}();
	return Bank;
}

// ---------------------------------------------------------------- settings ----------------------------------------------------------------

FJamBarkSettings FJamBarkSettings::FromCartridgeJson(const FString& RawJson)
{
	FJamBarkSettings S;
	FString Err;
	TSharedPtr<FJamJson> Root = FJamJson::Parse(RawJson, Err);
	if (!Root || !Root->IsObject()) return S;
	TSharedPtr<FJamJson> Game = Root->Get(TEXT("game"));
	TSharedPtr<FJamJson> Barks = Game && Game->IsObject() ? Game->Get(TEXT("barks")) : nullptr;
	if (!Barks || !Barks->IsObject()) return S;
	if (TSharedPtr<FJamJson> E = Barks->Get(TEXT("enabled"))) if (E->IsBool()) S.bEnabled = E->Bool;
	if (TSharedPtr<FJamJson> I = Barks->Get(TEXT("intensity"))) if (I->IsString()) S.Intensity = FName(*I->String);
	return S;
}

float FJamBarkSettings::CooldownScale() const
{
	if (Intensity == JamBarks::IntensityQuiet) return 2.f;
	if (Intensity == JamBarks::IntensityLoud) return 0.5f;
	return 1.f;
}

// ---------------------------------------------------------------- world ----------------------------------------------------------------

int32 FJamBarkWorld::Nearest(const FVector2D& Where, bool bOpponents) const
{
	int32 Best = -1;
	double BestD = 1e30;
	for (int32 i = 0; i < Teams.Num(); ++i)
	{
		if (i == HumanIndex) continue;
		if (bOpponents ? !IsOpponent(i) : !IsTeammate(i)) continue;
		const double D = Positions.IsValidIndex(i) ? FVector2D::DistSquared(Positions[i], Where) : 0.0;
		if (D < BestD) { BestD = D; Best = i; }
	}
	return Best;
}

// ---------------------------------------------------------------- picker ----------------------------------------------------------------

void FJamBarkPicker::Reset(uint64 Seed)
{
	Rng.Seed(Seed, 77u);
	LastGlobalFrame = -1000000;
	LastSpeakerFrame.Reset();
	History.Reset();
	PickCount = 0;
}

int32 FJamBarkPicker::SpeakerFor(FName Situation, EJamBarkSpeaker Side, const FVector2D& WhereM, const FJamBarkWorld& W)
{
	return W.Nearest(WhereM, Side == EJamBarkSpeaker::Opponent);
}

bool FJamBarkPicker::Resolve(FName Event, const FJamEventPayload& Payload, const FVector2D& WhereM, const FJamBarkWorld& W, FJamBarkResolved& Out)
{
	Out = FJamBarkResolved();
	Out.Where = WhereM;
	const int32 Human = W.HumanIndex;
	const int32 HumanTeam = W.HumanTeam();
	auto Finish = [&](FName Situation, EJamBarkSpeaker Side, int32 Preferred)
	{
		Out.Situation = Situation;
		Out.Side = Side;
		const bool bWantOpp = Side == EJamBarkSpeaker::Opponent;
		Out.Speaker = (Preferred >= 0 && (bWantOpp ? W.IsOpponent(Preferred) : W.IsTeammate(Preferred))) ? Preferred : W.Nearest(WhereM, bWantOpp);
		return Out.Speaker >= 0;
	};
	if (Event == JamEvents::Goal)
	{
		// Team 0 attacks +x: a goal at x > 0 is team 0's.
		const int32 ScoringTeam = WhereM.X > 0 ? 0 : 1;
		if (ScoringTeam == HumanTeam)
		{
			// The human scored: a teammate praises; a bot teammate scored: the opponents complain.
			if (Payload.Who == Human) return Finish(JamBarks::GoalFor, EJamBarkSpeaker::Teammate, -1);
			return Finish(JamBarks::GoalFor, EJamBarkSpeaker::Opponent, -1);
		}
		return Finish(JamBarks::GoalAgainst, EJamBarkSpeaker::Opponent, Payload.Who);
	}
	if (Event == JamEvents::Check)
	{
		if (Payload.Who == Human) return Finish(JamBarks::CheckLanded, EJamBarkSpeaker::Opponent, -1);
		return false;
	}
	if (Event == JamEvents::Knockdown)
	{
		if (Payload.Who == Human) return Finish(JamBarks::CheckReceived, EJamBarkSpeaker::Opponent, -1);
		return false;
	}
	if (Event == JamEvents::Faceoff) return Finish(JamBarks::Faceoff, EJamBarkSpeaker::Opponent, -1);
	if (Event == JamEvents::Win) return Finish(JamBarks::Win, EJamBarkSpeaker::Teammate, -1);
	if (Event == JamEvents::Lose) return Finish(JamBarks::Lose, EJamBarkSpeaker::Opponent, -1);
	if (Event == JamEvents::DirectorsCall)
	{
		const FVector2D HumanPos = W.Positions.IsValidIndex(Human) ? W.Positions[Human] : FVector2D::ZeroVector;
		Out.Where = HumanPos;
		Out.Situation = JamBarks::DirectorsCall;
		Out.Side = EJamBarkSpeaker::Opponent;
		Out.Speaker = W.Nearest(HumanPos, true);
		return Out.Speaker >= 0;
	}
	return false;
}

FName FJamBarkPicker::ChooseTone(FName Situation, EJamBarkSpeaker Side)
{
	const int32 Roll = Rng.NextInt(100);
	const bool bQuiet = Settings.Intensity == JamBarks::IntensityQuiet;
	const bool bLoud = Settings.Intensity == JamBarks::IntensityLoud;
	if (Side == EJamBarkSpeaker::Opponent)
	{
		// R-0015: quiet = complain only; normal = mock and the rest (never savage); loud = savage allowed.
		if (bQuiet) return JamBarks::ToneComplain;
		if (bLoud) return Roll < 45 ? JamBarks::ToneSavage : Roll < 75 ? JamBarks::ToneMock : Roll < 90 ? JamBarks::ToneBravado : JamBarks::ToneComplain;
		return Roll < 50 ? JamBarks::ToneMock : Roll < 80 ? JamBarks::ToneComplain : JamBarks::ToneBravado;
	}
	// Teammates: signal acknowledgements and praise replies are encouragement first.
	if (Situation == JamBarks::SignalAckDefend || Situation == JamBarks::SignalAckSpace || Situation == JamBarks::SignalAckPass || Situation == JamBarks::PraiseReply)
	{
		return Roll < 70 ? JamBarks::ToneEncourage : JamBarks::TonePraise;
	}
	if (bQuiet) return Roll < 60 ? JamBarks::ToneEncourage : JamBarks::TonePraise;
	if (bLoud) return Roll < 40 ? JamBarks::ToneComplain : Roll < 70 ? JamBarks::ToneEncourage : JamBarks::TonePraise;
	return Roll < 40 ? JamBarks::ToneEncourage : Roll < 75 ? JamBarks::TonePraise : JamBarks::ToneComplain;
}

bool FJamBarkPicker::IsOnCooldown(int32 SpeakerIndex, int64 Frame) const
{
	const float Scale = Settings.CooldownScale();
	if (Frame - LastGlobalFrame < static_cast<int64>(JamBarks::GlobalCooldownFrames * Scale)) return true;
	if (const int64* Last = LastSpeakerFrame.Find(SpeakerIndex))
	{
		if (Frame - *Last < static_cast<int64>(JamBarks::SpeakerCooldownFrames * Scale)) return true;
	}
	return false;
}

bool FJamBarkPicker::Pick(FName Situation, EJamBarkSpeaker Side, int32 SpeakerIndex, int64 Frame, FJamBarkPick& Out)
{
	if (!Settings.bEnabled || SpeakerIndex < 0 || IsOnCooldown(SpeakerIndex, Frame)) return false;
	const FName SpeakerName = Side == EJamBarkSpeaker::Opponent ? JamBarks::SpeakerOpponent : JamBarks::SpeakerTeammate;
	FName Tone = ChooseTone(Situation, Side);
	if (Bank.Count(Situation, SpeakerName, Tone) == 0)
	{
		// Fall back to any tone of this side that has lines.
		static const FName OppTones[] = { JamBarks::ToneMock, JamBarks::ToneComplain, JamBarks::ToneBravado };
		static const FName OppTonesQuiet[] = { JamBarks::ToneComplain };
		static const FName MateTones[] = { JamBarks::ToneEncourage, JamBarks::TonePraise, JamBarks::ToneComplain };
		Tone = NAME_None;
		const bool bQuietOpp = Side == EJamBarkSpeaker::Opponent && Settings.Intensity == JamBarks::IntensityQuiet;
		const FName* Begin = bQuietOpp ? OppTonesQuiet : (Side == EJamBarkSpeaker::Opponent ? OppTones : MateTones);
		const int32 Num = bQuietOpp ? 1 : 3;
		for (int32 i = 0; i < Num; ++i)
		{
			if (Bank.Count(Situation, SpeakerName, Begin[i]) > 0) { Tone = Begin[i]; break; }
		}
		if (Tone.IsNone()) return false;
	}
	return PickWithTone(Situation, Side, Tone, SpeakerIndex, Frame, Out);
}

bool FJamBarkPicker::PickWithTone(FName Situation, EJamBarkSpeaker Side, FName Tone, int32 SpeakerIndex, int64 Frame, FJamBarkPick& Out, bool bIgnoreCooldown)
{
	if (!Settings.bEnabled || SpeakerIndex < 0 || (!bIgnoreCooldown && IsOnCooldown(SpeakerIndex, Frame))) return false;
	const FName SpeakerName = Side == EJamBarkSpeaker::Opponent ? JamBarks::SpeakerOpponent : JamBarks::SpeakerTeammate;
	const TArray<int32>& All = Bank.Find(Situation, SpeakerName, Tone);
	if (All.Num() == 0) return false;
	TArray<int32> Fresh;
	for (int32 I : All) if (!History.Contains(Bank.Lines[I].Id)) Fresh.Add(I);
	const TArray<int32>& Pool = Fresh.Num() ? Fresh : All;
	const FJamBarkLine& L = Bank.Lines[Pool[Rng.NextInt(Pool.Num())]];
	Out.Speaker = SpeakerIndex;
	Out.Situation = Situation;
	Out.Tone = Tone;
	Out.LineId = L.Id;
	Out.Text = L.Text;
	Out.Frame = Frame;
	LastGlobalFrame = Frame;
	LastSpeakerFrame.Add(SpeakerIndex, Frame);
	History.Add(L.Id);
	if (History.Num() > JamBarks::NoRepeatWindow) History.RemoveAt(0);
	++PickCount;
	return true;
}

// ---------------------------------------------------------------- subsystem ----------------------------------------------------------------

UJamBarkSystem* UJamBarkSystem::Get(const UObject* WorldContext)
{
	const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	return World ? World->GetSubsystem<UJamBarkSystem>() : nullptr;
}

bool UJamBarkSystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer)) return false;
	const UWorld* World = Cast<UWorld>(Outer);
	return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UJamBarkSystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	Picker = MakeUnique<FJamBarkPicker>(FJamBarkBank::Embedded(), 1);
	if (UGameInstance* GI = InWorld.GetGameInstance()) Bus = GI->GetSubsystem<UJamEventBus>();
	if (Bus) Bus->OnJamEvent.AddDynamic(this, &UJamBarkSystem::HandleBusEvent);
	if (UJamPresentation* Pres = UJamPresentation::Get(&InWorld)) FrameHandle = Pres->OnSimFrame.AddUObject(this, &UJamBarkSystem::HandleSimFrame);
	UE_LOG(LogJamBarks, Log, TEXT("JAMBARK system up: bus=%d bank=%d lines"), Bus ? 1 : 0, FJamBarkBank::Embedded().Lines.Num());
}

void UJamBarkSystem::Deinitialize()
{
	if (Bus) Bus->OnJamEvent.RemoveDynamic(this, &UJamBarkSystem::HandleBusEvent);
	if (UWorld* World = GetWorld()) if (UJamPresentation* Pres = UJamPresentation::Get(World)) Pres->OnSimFrame.Remove(FrameHandle);
	Picker.Reset();
	Super::Deinitialize();
}

AHokejGameMode* UJamBarkSystem::GameMode() const
{
	const UWorld* World = GetWorld();
	return World ? Cast<AHokejGameMode>(World->GetAuthGameMode()) : nullptr;
}

void UJamBarkSystem::SetSettings(const FJamBarkSettings& S)
{
	Settings = S;
	if (Picker) Picker->SetSettings(S);
}

void UJamBarkSystem::RefreshSettings()
{
	AHokejGameMode* GM = GameMode();
	const UJamCartridge* C = GM ? GM->GetCartridge() : nullptr;
	if (C && C != SettingsSource)
	{
		SettingsSource = C;
		FJamBarkSettings S = FJamBarkSettings::FromCartridgeJson(C->RawJson);
		const int32 Override = CVarJamBarks.GetValueOnGameThread();
		if (Override == 0) S.bEnabled = false;
		else if (Override == 1) S.bEnabled = true;
		SetSettings(S);
		if (Picker) Picker->Reset(static_cast<uint64>(C->Seed) ^ 0xBA2Cull);
		UE_LOG(LogJamBarks, Log, TEXT("JAMBARK settings from %s: enabled=%d intensity=%s"), *C->Id, S.bEnabled ? 1 : 0, *S.Intensity.ToString());
	}
}

void UJamBarkSystem::BuildWorld(FJamBarkWorld& Out) const
{
	Out = FJamBarkWorld();
	AHokejGameMode* GM = GameMode();
	const UJamSim* Sim = GM ? GM->GetSim() : nullptr;
	if (!Sim) return;
	Out.HumanIndex = Sim->GetHumanIndex();
	for (const FJamSkaterState& S : Sim->GetSkaters()) { Out.Teams.Add(S.Team); Out.Positions.Add(S.Pos); }
	Out.PuckLastTouch = Sim->GetPuck().LastTouch;
}

void UJamBarkSystem::HandleBusEvent(FName Event, const FJamEventPayload& Payload)
{
	if (!Picker) return;
	RefreshSettings();
	if (!Settings.bEnabled) return;
	FJamBarkWorld W;
	BuildWorld(W);
	if (W.Teams.Num() == 0) return;
	const FVector2D WhereM(Payload.Where.X / 100.0, Payload.Where.Y / 100.0);
	const AHokejGameMode* GM = GameMode();
	const UJamSim* Sim = GM ? GM->GetSim() : nullptr;
	const int64 Now = Sim ? Sim->GetFrame() : Frame;

	// Near miss: a shot, then the puck hits the boards next to a net within the window and no goal came.
	if (Event == JamEvents::Shot)
	{
		LastShotFrame = Now;
		LastShotTeam = W.Teams.IsValidIndex(Payload.Who) ? W.Teams[Payload.Who] : -1;
		return;
	}
	if (Event == JamEvents::Board && Sim && Now - LastShotFrame <= JamBarks::NearMissWindow && LastShotTeam >= 0)
	{
		const FJamGameConfig& G = Sim->GetConfig();
		const bool bNearNet = FMath::Abs(WhereM.X) > G.Rink.GoalLineX() - 1.5 && FMath::Abs(WhereM.Y) < G.Rink.GoalWidthM * 0.5 + 3.0;
		if (bNearNet)
		{
			LastShotFrame = -1000000;
			const bool bHumanTeamShot = LastShotTeam == W.HumanTeam();
			const int32 Speaker = W.Nearest(WhereM, true);
			FJamBarkPick P;
			if (Speaker >= 0 && (bHumanTeamShot ? Picker->Pick(JamBarks::NearMiss, EJamBarkSpeaker::Opponent, Speaker, Now, P)
				: Picker->PickWithTone(JamBarks::NearMiss, EJamBarkSpeaker::Opponent, JamBarks::ToneComplain, Speaker, Now, P))) Show(P);
		}
		return;
	}
	FJamBarkResolved R;
	if (!FJamBarkPicker::Resolve(Event, Payload, WhereM, W, R)) return;
	FJamBarkPick P;
	if (Picker->Pick(R.Situation, R.Side, R.Speaker, Now, P)) Show(P);
}

void UJamBarkSystem::HandleSimFrame(const FJamPresentationFrame& F)
{
	Frame = F.SimFrame;
	if (!Picker) return;
	RefreshSettings();
	if (Widget) Widget->TickFrames(Frame);
	if (!Settings.bEnabled) return;
	const AHokejGameMode* GM = GameMode();
	const UJamSim* Sim = GM ? GM->GetSim() : nullptr;
	if (!Sim || Sim->GetPhase() != EJamSimPhase::Playing) { LastTouchTeam = -1; LastTouchFrame = Frame; return; }
	FJamBarkWorld W;
	BuildWorld(W);
	const int32 Touch = W.PuckLastTouch;
	const int32 TouchTeam = W.Teams.IsValidIndex(Touch) ? W.Teams[Touch] : -1;
	if (TouchTeam >= 0 && TouchTeam != LastTouchTeam)
	{
		if (LastTouchTeam >= 0)
		{
			// The puck changed hands. Taken by an opponent: that opponent mocks. Taken by the human: an opponent complains; by a bot teammate: nothing.
			FJamBarkPick P;
			if (TouchTeam != W.HumanTeam()) { if (Picker->Pick(JamBarks::Steal, EJamBarkSpeaker::Opponent, Touch, Frame, P)) Show(P); }
			else if (Touch == W.HumanIndex)
			{
				const int32 Speaker = W.Nearest(W.Positions[Touch], true);
				if (Picker->PickWithTone(JamBarks::Steal, EJamBarkSpeaker::Opponent, JamBarks::ToneComplain, Speaker, Frame, P)) Show(P);
			}
		}
		LastTouchTeam = TouchTeam;
	}
	if (Touch >= 0 && Touch != -1) { if (Sim->GetSkaters().IsValidIndex(Touch) && Sim->GetSkaters()[Touch].bTouchingPuck) LastTouchFrame = Frame; }
	// Stall: a slow puck nobody has touched for StallFrames.
	if (Frame - LastTouchFrame > JamBarks::StallFrames && Sim->GetPuck().Vel.Size() < 2.0 && Frame - LastStallFrame > 600)
	{
		LastStallFrame = Frame;
		const int32 Speaker = W.Nearest(Sim->GetPuck().Pos, true);
		FJamBarkPick P;
		if (Picker->Pick(JamBarks::Stall, EJamBarkSpeaker::Opponent, Speaker, Frame, P)) Show(P);
	}
}

bool UJamBarkSystem::Trigger(FName Situation, EJamBarkSpeaker Side, int32 PreferredSpeaker, bool bForce)
{
	if (!Picker) return false;
	RefreshSettings();
	FJamBarkWorld W;
	BuildWorld(W);
	const AHokejGameMode* GM = GameMode();
	const UJamSim* Sim = GM ? GM->GetSim() : nullptr;
	const int64 Now = Sim ? Sim->GetFrame() : Frame;
	int32 Speaker = PreferredSpeaker;
	const bool bOpp = Side == EJamBarkSpeaker::Opponent;
	if (Speaker < 0 || !(bOpp ? W.IsOpponent(Speaker) : W.IsTeammate(Speaker)))
	{
		const FVector2D HumanPos = W.Positions.IsValidIndex(W.HumanIndex) ? W.Positions[W.HumanIndex] : FVector2D::ZeroVector;
		Speaker = W.Nearest(HumanPos, bOpp);
	}
	FJamBarkPick P;
	if (bForce)
	{
		const FName Tone = bOpp ? JamBarks::ToneMock : (Situation == JamBarks::PraiseReply ? JamBarks::TonePraise : JamBarks::ToneEncourage);
		if (!Picker->PickWithTone(Situation, Side, Tone, Speaker, Now, P, true)) return false;
	}
	else if (!Picker->Pick(Situation, Side, Speaker, Now, P)) return false;
	Show(P);
	return true;
}

void UJamBarkSystem::Show(const FJamBarkPick& P)
{
	LastBark = P;
	++BarkCount;
	UE_LOG(LogJamBarks, Log, TEXT("JAMBARK who=%d situation=%s tone=%s id=%s frame=%lld text=\"%s\""), P.Speaker, *P.Situation.ToString(), *P.Tone.ToString(),
		*P.LineId.ToString(), P.Frame, *P.Text);
	UWorld* World = GetWorld();
	APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	if (!Widget && PC)
	{
		Widget = CreateWidget<UJamBarkWidget>(PC, UJamBarkWidget::StaticClass());
		if (Widget) Widget->AddToViewport(50);
	}
	AJamSkater* Actor = nullptr;
	if (World) for (TActorIterator<AJamSkater> It(World); It; ++It) if (It->GetSimIndex() == P.Speaker) { Actor = *It; break; }
	FVector SpeakerWorld = FVector::ZeroVector;
	int32 Team = 0;
	if (const AHokejGameMode* GM = GameMode()) if (const UJamSim* Sim = GM->GetSim()) if (Sim->GetSkaters().IsValidIndex(P.Speaker))
	{
		Team = Sim->GetSkaters()[P.Speaker].Team;
		SpeakerWorld = UJamSim::ToWorld(Sim->GetSkaters()[P.Speaker].Pos);
	}
	if (Widget) Widget->ShowBark(P, Actor, Team);
	if (Bus && UJamEventBus::IsKnownEvent(JamBarks::BusEvent))
	{
		// contracts/events.md "bark": Who = speaker, Where = speaker, Strength 0, Kind = situation.
		FJamEventPayload Pl;
		Pl.Who = P.Speaker;
		Pl.Where = SpeakerWorld;
		Pl.Kind = P.Situation;
		Bus->Broadcast(JamBarks::BusEvent, Pl);
	}
	OnBark.Broadcast(P);
	if (CVarJamBarkVoice.GetValueOnGameThread() != 0)
	{
		const bool bSpoke = SpeakWithEngineTts(P.Text, 0.55f, 0.8f);
		if (!bVoiceProbed) { bVoiceProbed = true; bVoiceAvailable = bSpoke; UE_LOG(LogJamBarks, Log, TEXT("JAMBARK voice %s"), bSpoke ? TEXT("on (engine TextToSpeech)") : TEXT("unavailable: TextToSpeech plugin not enabled")); }
	}
}

// ---------------------------------------------------------------- voice by reflection ----------------------------------------------------------------

namespace
{
	// Calls Func(Args...) on Obj with the arguments written by name into the parameter struct. Supports FName, FString and float parameters.
	struct FReflectedArg { FName Name; FName NameValue; FString StringValue; float FloatValue = 0.f; uint8 Kind = 0; }; // 0 name, 1 string, 2 float

	bool CallReflected(UObject* Obj, const TCHAR* FuncName, const TArray<FReflectedArg>& Args)
	{
		UFunction* F = Obj ? Obj->FindFunction(FuncName) : nullptr;
		if (!F) return false;
		uint8* Params = static_cast<uint8*>(FMemory_Alloca(FMath::Max<int32>(F->ParmsSize, 1)));
		FMemory::Memzero(Params, F->ParmsSize);
		for (TFieldIterator<FProperty> It(F); It && It->HasAnyPropertyFlags(CPF_Parm); ++It) It->InitializeValue_InContainer(Params);
		for (const FReflectedArg& A : Args)
		{
			FProperty* P = F->FindPropertyByName(A.Name);
			if (!P) continue;
			if (A.Kind == 0) { if (FNameProperty* NP = CastField<FNameProperty>(P)) NP->SetPropertyValue_InContainer(Params, A.NameValue); }
			else if (A.Kind == 1) { if (FStrProperty* SP = CastField<FStrProperty>(P)) SP->SetPropertyValue_InContainer(Params, A.StringValue); }
			else { if (FFloatProperty* FP = CastField<FFloatProperty>(P)) FP->SetPropertyValue_InContainer(Params, A.FloatValue); }
		}
		Obj->ProcessEvent(F, Params);
		for (TFieldIterator<FProperty> It(F); It && It->HasAnyPropertyFlags(CPF_Parm); ++It) It->DestroyValue_InContainer(Params);
		return true;
	}
	FReflectedArg NameArg(const TCHAR* N, FName V) { FReflectedArg A; A.Name = N; A.NameValue = V; A.Kind = 0; return A; }
	FReflectedArg StrArg(const TCHAR* N, const FString& V) { FReflectedArg A; A.Name = N; A.StringValue = V; A.Kind = 1; return A; }
	FReflectedArg FloatArg(const TCHAR* N, float V) { FReflectedArg A; A.Name = N; A.FloatValue = V; A.Kind = 2; return A; }
}

bool UJamBarkSystem::SpeakWithEngineTts(const FString& Text, float Rate, float Volume)
{
	if (!GEngine) return false;
	UClass* Cls = FindObject<UClass>(nullptr, TEXT("/Script/TextToSpeech.TextToSpeechEngineSubsystem"));
	if (!Cls) return false;
	UEngineSubsystem* Sub = GEngine->GetEngineSubsystemBase(Cls);
	if (!Sub) return false;
	static const FName Channel(TEXT("HokejBarks"));
	static bool bChannelReady = false;
	if (!bChannelReady)
	{
		CallReflected(Sub, TEXT("AddDefaultChannel"), { NameArg(TEXT("InNewChannelId"), Channel) });
		CallReflected(Sub, TEXT("ActivateChannel"), { NameArg(TEXT("InChannelId"), Channel) });
		CallReflected(Sub, TEXT("SetRateOnChannel"), { NameArg(TEXT("InChannelId"), Channel), FloatArg(TEXT("InRate"), Rate) });
		CallReflected(Sub, TEXT("SetVolumeOnChannel"), { NameArg(TEXT("InChannelId"), Channel), FloatArg(TEXT("InVolume"), Volume) });
		bChannelReady = true;
	}
	return CallReflected(Sub, TEXT("SpeakOnChannel"), { NameArg(TEXT("InChannelId"), Channel), StrArg(TEXT("InStringToSpeak"), Text) });
}

// ---------------------------------------------------------------- console ----------------------------------------------------------------

static FAutoConsoleCommandWithWorldAndArgs CmdJamBark(TEXT("jam.bark"),
	TEXT("jam.bark <situation> [opponent|teammate]: force a bark for a PIE proof (goal_for, goal_against, check_landed, check_received, steal, stall, near_miss, faceoff, win, lose, directors_call, signal_ack_defend, signal_ack_space, signal_ack_pass, praise_reply)."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		UJamBarkSystem* S = World ? World->GetSubsystem<UJamBarkSystem>() : nullptr;
		if (!S || Args.Num() == 0) { UE_LOG(LogJamBarks, Warning, TEXT("jam.bark: no bark system in this world or no situation given")); return; }
		const FName Situation(*Args[0]);
		EJamBarkSpeaker Side = (Args.Num() > 1 && Args[1] == TEXT("teammate")) || Situation.ToString().StartsWith(TEXT("signal_ack")) || Situation == JamBarks::PraiseReply
			? EJamBarkSpeaker::Teammate : EJamBarkSpeaker::Opponent;
		// A forced bark ignores the cooldown by resetting the picker's clock through a fresh settings push.
		const bool bOk = S->Trigger(Situation, Side);
		UE_LOG(LogJamBarks, Log, TEXT("jam.bark %s: %s"), *Args[0], bOk ? *S->GetLastBark().Text : TEXT("nothing (cooldown, disabled or no lines)"));
	}));
