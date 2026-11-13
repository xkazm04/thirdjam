#include "CommentarySystem.h"
#include "EffectsBinder.h"
#include "HokejGameMode.h"
#include "HUDFeed.h"
#include "Progression.h"
#include "JamCartridge.h"
#include "JamJson.h"
#include "JamPawns.h"
#include "JamSim.h"
#include "Components/AudioComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "UObject/UObjectGlobals.h"
#include "Misc/PackageName.h"

namespace JamCommentary
{
	const FName IntensityQuiet(TEXT("quiet"));
	const FName IntensityNormal(TEXT("normal"));
	const FName IntensityLoud(TEXT("loud"));
}

namespace
{
	// Situation suffixes and kinds the conditions read (B's README section 4).
	const FName SfxAny(TEXT("any")), SfxBlue(TEXT("blue")), SfxOrange(TEXT("orange")), SfxHuman(TEXT("human")), SfxFor(TEXT("for")), SfxAgainst(TEXT("against")),
		SfxToHuman(TEXT("to_human")), SfxHard(TEXT("hard")), SfxSoft(TEXT("soft")), SfxLong(TEXT("long")), SfxClose(TEXT("close")), SfxHumanStreak(TEXT("human_streak")),
		SfxFirst(TEXT("first")), SfxTie(TEXT("tie")), SfxLead(TEXT("lead")), SfxComeback(TEXT("comeback")), SfxBlowout(TEXT("blowout")), SfxMatchPoint(TEXT("match_point")),
		SfxOwn(TEXT("own")), SfxLate(TEXT("late")), SfxPause(TEXT("pause")), SfxBig(TEXT("big")), SfxRim(TEXT("rim")), SfxBank(TEXT("bank")), SfxStrip(TEXT("strip")),
		SfxBoards(TEXT("boards")), SfxCoast(TEXT("coast")), SfxProtect(TEXT("protect")), SfxAfterGoal(TEXT("after_goal")), SfxAfterFoul(TEXT("after_foul")),
		SfxEarly(TEXT("early")), SfxDrop(TEXT("drop")), SfxTakeN(TEXT("take_n")), SfxTakeTied(TEXT("take_tied")), SfxTied(TEXT("tied")), SfxLast10(TEXT("last_10")),
		SfxLast10Tied(TEXT("last_10_tied")), SfxLast10Close(TEXT("last_10_close")), SfxSudden(TEXT("sudden")), SfxShutout(TEXT("shutout")), SfxSeason(TEXT("season"));
	const FName KindBoost(TEXT("boost")), KindCheck(TEXT("check"));
	// Lane A90 (R-0041 point 4): A89's rough-voice situations.
	const FName SfxLight(TEXT("light")), SfxSolid(TEXT("solid")), SfxMassacre(TEXT("massacre")), SfxAfterHit(TEXT("after_hit")), SfxCross(TEXT("cross")),
		KindClean(TEXT("clean"));
	const FName SitCheckCross(TEXT("check.cross")), SitGoalAfterHit(TEXT("goal.after_hit"));
	// Lane A47 (B-to-A-0030 fault 4): B tagged board.bank and board.rim with their condition as the line's kind so they could never play on a plain
	// board hit. The payload's kind for board is "skater" or none, so these kinds are conditions for WhenHolds, not a kind to match.
	bool IsConditionKind(FName Kind) { return Kind == SfxBank || Kind == SfxRim; }
	const FName SitGoalBlue(TEXT("goal.blue")), SitGoalOrange(TEXT("goal.orange")), SitGoalPause(TEXT("goal.pause")), SitBoardBank(TEXT("board.bank"));
	const FName BarkGoalFor(TEXT("goal_for")), BarkGoalAgainst(TEXT("goal_against")), BarkFaceoff(TEXT("faceoff"));
	const TCHAR* PathCommentary = TEXT("/JamAssets/Audio/MS_commentary.MS_commentary");
	const TCHAR* PathMusic = TEXT("/JamAssets/Audio/MS_music_match.MS_music_match");
	const TCHAR* PathCrowdBed = TEXT("/JamAssets/Audio/MS_crowd_bed.MS_crowd_bed");   // R-0033 point 4 (lane A50): B's crowd bed, when it exists
	const TCHAR* PathSkate = TEXT("/JamAssets/Audio/MS_skate_loop.MS_skate_loop");

	int32 TakeNumber(FName Kind)
	{
		const FString S = Kind.ToString();
		return S.StartsWith(TEXT("take_")) ? FCString::Atoi(*S.Mid(5)) : 0;
	}
}

static TAutoConsoleVariable<int32> CVarJamCommentary(TEXT("jam.Commentary"), -1,
	TEXT("Commentator override: -1 = the cartridge's game.commentary.enabled (default), 0 = off, 1 = on."));

// ---------------------------------------------------------------- settings ----------------------------------------------------------------

FJamCommentarySettings FJamCommentarySettings::FromCartridgeJson(const FString& RawJson)
{
	FJamCommentarySettings S;
	FString Err;
	TSharedPtr<FJamJson> Root = FJamJson::Parse(RawJson, Err);
	if (!Root || !Root->IsObject()) return S;
	TSharedPtr<FJamJson> Game = Root->Get(TEXT("game"));
	TSharedPtr<FJamJson> C = Game && Game->IsObject() ? Game->Get(TEXT("commentary")) : nullptr;
	if (!C || !C->IsObject()) return S;
	if (TSharedPtr<FJamJson> E = C->Get(TEXT("enabled"))) if (E->IsBool()) S.bEnabled = E->Bool;
	if (TSharedPtr<FJamJson> I = C->Get(TEXT("intensity"))) if (I->IsString()) S.Intensity = FName(*I->String);
	return S;
}

float FJamCommentarySettings::CooldownScale() const
{
	if (Intensity == JamCommentary::IntensityQuiet) return 2.f;
	if (Intensity == JamCommentary::IntensityLoud) return 0.5f;
	return 1.f;
}

int32 FJamCommentarySettings::MinPriority() const
{
	return Intensity == JamCommentary::IntensityQuiet ? 3 : 1;
}

FName FJamCommentaryWorld::ColourOf(int32 Team)
{
	return Team == 0 ? JamCommentary::TeamBlue : Team == 1 ? JamCommentary::TeamOrange : JamCommentary::TeamAny;
}

// ---------------------------------------------------------------- picker ----------------------------------------------------------------

void FJamCommentaryPicker::Reset(uint64 Seed)
{
	Rng.Seed(Seed, 91u);
	Pending.Reset();
	LastStartFrame = LastEndFrame = BarkHoldUntil = LastSeenFrame = -1000000;
	bBarkAtHumanThisFrame = bCutRequested = false;
	LastSituationFrame.Reset();
	SituationHistory.Reset();
	HeardThisMatch.Reset();
	Last = FJamCommentaryPick();
	PickCount = 0;
	PrevScore[0] = PrevScore[1] = 0;
	bPrevScoreValid = false;
	LowestDeficit[0] = LowestDeficit[1] = 0;
	PrevPuckLastTouch = -1;
	LastShotFrame = LastBoardFrame = LastGoalFrame = LastFoulFrame = LastDropFrame = PrevDropFrame = LastCheckFrame = -1000000;
	LastShotWhere = LastCheckWhere = FVector2D::ZeroVector;
	LastShotTeam = -1;
	HumanShotFrames.Reset();
	PassStreakTeam = -1;
	PassStreak = 0;
	KnockdownsBy[0].Reset();
	KnockdownsBy[1].Reset();
	LastTeamCheckFrame[0] = LastTeamCheckFrame[1] = -1000000;
	bGoalAfterHit = false;
	bGoalBeatPending = false;
	GoalBeat = FJamCommentaryEvent();
	GoalBeatSaid = NAME_None;
}

FName FJamCommentaryPicker::Suffix(const FJamCommentaryLine& L)
{
	const FString S = L.Situation.ToString();
	int32 Dot = INDEX_NONE;
	return S.FindChar(TEXT('.'), Dot) ? FName(*S.Mid(Dot + 1)) : NAME_None;
}

int32 FJamCommentaryPicker::Specificity(const FJamCommentaryLine& L)
{
	const FName Sfx = Suffix(L);
	int32 N = 0;
	if (!L.Kind.IsNone()) ++N;
	if (L.Team != JamCommentary::TeamAny) ++N;
	// A conditional situation: not the plain one, not just the kind or the colour spelled out.
	if (!Sfx.IsNone() && Sfx != SfxAny && Sfx != L.Kind && Sfx != SfxBlue && Sfx != SfxOrange) ++N;
	if (Sfx == SfxHuman || Sfx == SfxFor || Sfx == SfxAgainst || Sfx == SfxToHuman) ++N;
	return N;
}

bool FJamCommentaryPicker::WhenHolds(const FJamCommentaryLine& L, const FJamCommentaryEvent& E, const FJamCommentaryWorld& W) const
{
	const FName Sfx = Suffix(L);
	const int32 T = E.Team;
	const int32 O = (T == 0 || T == 1) ? 1 - T : -1;
	const int32 Human = W.HumanTeam();
	const int32 Diff = (T >= 0 && O >= 0) ? E.ScoreAfter[T] - E.ScoreAfter[O] : 0;
	const bool bLate = W.bTimer && !W.bOvertime && W.FramesLeftInPeriod <= JamCommentary::LastTenFrames;
	// pass.blue / pass.orange: a third pass in a row by that team (the colour already matched the passer's team).
	if (E.Event == JamEvents::Pass && (Sfx == SfxBlue || Sfx == SfxOrange)) return PassStreakTeam == T && PassStreak >= 3;
	// directors_call.<key>_up / _down: Strength is the new value's position in its range.
	if (E.Event == JamEvents::DirectorsCall)
	{
		const FString S = Sfx.ToString();
		if (S.EndsWith(TEXT("_up"))) return E.Strength >= 0.5f;
		if (S.EndsWith(TEXT("_down"))) return E.Strength < 0.5f;
		return true;
	}
	// Plain, colour and kind situations hold once event, kind and team matched (a condition kind, B's bank / rim tag, is not a kind: it reads on below).
	if (Sfx.IsNone() || Sfx == SfxAny || Sfx == SfxBlue || Sfx == SfxOrange || (Sfx == L.Kind && !IsConditionKind(L.Kind))) return true;
	// The human.
	if (Sfx == SfxHuman) return E.Who == W.HumanIndex;
	if (Sfx == SfxToHuman) return E.Kind == FName(*FString::Printf(TEXT("to_%d"), W.HumanIndex));
	if (Sfx == SfxFor) return T == Human;
	if (Sfx == SfxAgainst) return T >= 0 && T != Human;
	// Strength thresholds (B's section 4: 0.8 / 0.7 / 0.3).
	if (Sfx == SfxHard) return E.Strength >= (E.Event == JamEvents::Board ? 0.7f : 0.8f);
	if (Sfx == SfxBig) return E.Strength >= 0.8f;
	if (Sfx == SfxSoft) return E.Strength < 0.3f;
	// Lane A90: a check by its Strength (closing speed / skater speed): light < 0.4 <= solid < 0.8 <= big.
	if (Sfx == SfxLight) return E.Strength < JamCommentary::CheckLightBelow;
	if (Sfx == SfxSolid) return E.Strength >= JamCommentary::CheckLightBelow && E.Strength < JamCommentary::CheckSolidBelow;
	// Lane A90: the third knockdown by one team (the victim's opponents) inside 20 s; the ring already holds this knockdown (Observe adds it first).
	if (Sfx == SfxMassacre) return E.Event == JamEvents::Knockdown && T >= 0 && O >= 0 && KnockdownsInWindow(O, E.Frame) >= JamCommentary::MassacreCount;
	// Lane A90: a goal within 5 s after the scoring team's check (Observe decides it at the goal; the line waits for the goal's second beat).
	if (Sfx == SfxAfterHit) return E.Event == JamEvents::Goal && bGoalAfterHit;
	// Lane A90: check.cross is offered on a lift that lands (Observe asks with Event check, Kind cross; the kind already matched).
	if (Sfx == SfxCross) return E.Kind == SfxCross;
	if (Sfx == SfxLong)
	{
		if (E.Event == JamEvents::Pass) return E.Strength >= 0.8f;
		if (E.Event == JamEvents::Carry) return E.Strength * W.RinkLengthM >= 30.0;
		if (E.Event == JamEvents::Goal) return E.Frame - LastShotFrame <= JamCommentary::LongShotWindowFrames && T >= 0 && LastShotWhere.X * W.AttackSign(T) < 0.0;
		return T >= 0 && E.WhereM.X * W.AttackSign(T) < 0.0;   // a shot from the shooter's own half
	}
	// Where against the rink (shot.close: inside the crease area); elsewhere "close" is a one-goal margin (win.close, lose.close).
	if (Sfx == SfxClose)
	{
		if (E.Event == JamEvents::Shot) return T >= 0 && FMath::Abs(E.WhereM.X - W.GoalLineX * W.AttackSign(T)) < 4.5 && FMath::Abs(E.WhereM.Y) < W.GoalWidthM * 0.5 + 2.0;
		return FMath::Abs(E.ScoreAfter[0] - E.ScoreAfter[1]) == 1;
	}
	if (Sfx == SfxRim) return FMath::Abs(E.WhereM.X) > W.GoalLineX;
	if (Sfx == SfxBoards) return FMath::Abs(E.WhereM.X) > W.RinkLengthM * 0.5 - 2.0 || FMath::Abs(E.WhereM.Y) > W.RinkWidthM * 0.5 - 2.0;
	// The score before and after.
	if (Sfx == SfxFirst) return E.ScoreBefore[0] + E.ScoreBefore[1] == 0 && E.ScoreAfter[0] + E.ScoreAfter[1] > 0;
	if (Sfx == SfxTie || Sfx == SfxTied || Sfx == SfxTakeTied || Sfx == SfxLast10Tied)
	{
		const bool bTied = E.ScoreAfter[0] == E.ScoreAfter[1];
		if (Sfx == SfxTakeTied) return bTied && TakeNumber(E.Kind) >= 2;
		return bTied;
	}
	if (Sfx == SfxLead)
	{
		if (E.Event == JamEvents::PeriodEnd) return E.ScoreAfter[0] != E.ScoreAfter[1];   // a take ends with one team ahead
		return T >= 0 && E.ScoreBefore[0] == E.ScoreBefore[1] && Diff > 0;                 // the goal breaks a tie
	}
	if (Sfx == SfxComeback)
	{
		if (E.Event == JamEvents::Goal) return T >= 0 && E.ScoreBefore[O] - E.ScoreBefore[T] >= 2;
		return T >= 0 && LowestDeficit[T] >= 2;   // win.comeback
	}
	if (Sfx == SfxBlowout) return T >= 0 && FMath::Abs(Diff) >= 3;
	if (Sfx == SfxLast10Close) return FMath::Abs(E.ScoreAfter[0] - E.ScoreAfter[1]) == 1;
	if (Sfx == SfxMatchPoint) return W.bFirstTo && T >= 0 && W.ScoreToWin - E.ScoreAfter[T] == 1;
	if (Sfx == SfxOwn) return T >= 0 && W.TeamOf(E.Who) >= 0 && W.TeamOf(E.Who) != T;
	if (Sfx == SfxShutout) return T >= 0 && O >= 0 && E.ScoreAfter[O] == 0;
	if (Sfx == SfxLast10) return true;   // kind last_10s matched
	if (Sfx == SfxSudden) return true;   // kind sudden_death matched
	if (Sfx == SfxTakeN) return TakeNumber(E.Kind) >= 4;
	if (Sfx == SfxSeason) return W.bSeasonLastTake;
	// Time left.
	if (Sfx == SfxLate) return bLate;
	// A short history the picker keeps.
	if (Sfx == SfxHumanStreak)
	{
		int32 N = 0;
		for (int64 F : HumanShotFrames) if (E.Frame - F <= JamCommentary::StreakWindowFrames) ++N;
		return N >= 3;
	}
	// board.bank is a goal-time line (Observe adds it at the goal). Lane A47 (B-to-A-0030 fault 4): on a board event this held whenever the previous
	// board hit was within a second (the puck rattling along the boards), so "Banked it in off the boards!" played on a plain board hit.
	if (Sfx == SfxBank) return E.Event == JamEvents::Goal && E.Frame - LastBoardFrame <= JamCommentary::BankWindowFrames;
	if (Sfx == SfxAfterGoal) return LastGoalFrame > PrevDropFrame;
	if (Sfx == SfxAfterFoul) return LastFoulFrame > PrevDropFrame;
	if (Sfx == SfxEarly) return W.DrawEarly.IsValidIndex(E.Who) && W.DrawEarly[E.Who];
	if (Sfx == SfxStrip)
	{
		const int32 Prev = PrevPuckLastTouch;
		return Prev >= 0 && W.TeamOf(Prev) >= 0 && W.TeamOf(Prev) != T && W.Positions.IsValidIndex(Prev) && FVector2D::Distance(W.Positions[Prev], E.WhereM) < 2.0 && W.PuckLastTouch != Prev;
	}
	if (Sfx == SfxProtect) return E.Frame - LastCheckFrame <= JamCommentary::ProtectWindowFrames && FVector2D::Distance(LastCheckWhere, E.WhereM) < 3.0;
	if (Sfx == SfxCoast) return E.Kind == SfxCoast;
	if (Sfx == SfxPause) return W.bPause;
	// pass.blue / pass.orange: a third pass in a row by that team (the colour check already matched the team).
	if (E.Event == JamEvents::Pass) return PassStreakTeam == T && PassStreak >= 3;
	// An unknown situation suffix (a line B adds later): event, kind and team matched, so it holds.
	return true;
}

void FJamCommentaryPicker::Candidates(const FJamCommentaryEvent& E, const FJamCommentaryWorld& W, TArray<int32>& Out) const
{
	Out.Reset();
	const float Scale = Settings.CooldownScale();
	const int32 MinPriority = Settings.MinPriority();
	const bool bZone = E.Event == JamEvents::ZoneEnter || E.Event == JamEvents::ZoneExit;
	const FName* ZoneKind = bZone ? W.ZoneKinds.Find(E.Kind) : nullptr;
	for (int32 I : Bank.ForEvent(E.Event))
	{
		const FJamCommentaryLine& L = Bank.Lines[I];
		if (L.Priority < MinPriority) continue;
		if (!L.Kind.IsNone() && !IsConditionKind(L.Kind))
		{
			if (bZone) { if (!ZoneKind || *ZoneKind != L.Kind) continue; }
			else if (L.Kind != E.Kind) continue;
		}
		if (L.Team != JamCommentary::TeamAny && L.Team != FJamCommentaryWorld::ColourOf(E.Team)) continue;
		if (L.bStoppage && !W.bPause) continue;
		if (!WhenHolds(L, E, W)) continue;
		if (const int64* LastF = LastSituationFrame.Find(L.Situation))
		{
			if (E.Frame - *LastF < static_cast<int64>(L.CooldownFrames * Scale)) continue;
		}
		Out.Add(I);
	}
	// Never the same line twice running: drop the situation's last three alternates, unless that empties the situation (then only the last one).
	TMap<FName, int32> PerSituation;
	for (int32 I : Out) ++PerSituation.FindOrAdd(Bank.Lines[I].Situation);
	for (int32 i = Out.Num() - 1; i >= 0; --i)
	{
		const FJamCommentaryLine& L = Bank.Lines[Out[i]];
		const TArray<FName>* H = SituationHistory.Find(L.Situation);
		if (!H || H->Num() == 0) continue;
		const bool bRecent = H->Contains(L.Id);
		const bool bLast = H->Last() == L.Id;
		const int32 N = PerSituation[L.Situation];
		if ((bRecent && N > JamCommentary::NoRepeatPerSituation) || (bLast && N > 1)) Out.RemoveAt(i);
	}
}

int32 FJamCommentaryPicker::KnockdownsInWindow(int32 Team, int64 Frame) const
{
	if (Team < 0 || Team > 1) return 0;
	int32 N = 0;
	for (int64 F : KnockdownsBy[Team]) if (F <= Frame && Frame - F < JamCommentary::MassacreWindowFrames) ++N;
	return N;
}

void FJamCommentaryPicker::Remember(const FJamCommentaryEvent& E, const FJamCommentaryWorld& W)
{
	if (E.Event == JamEvents::Shot)
	{
		LastShotFrame = E.Frame;
		LastShotWhere = E.WhereM;
		LastShotTeam = E.Team;
		if (E.Team == W.HumanTeam()) HumanShotFrames.Add(E.Frame);
		HumanShotFrames.RemoveAll([&](int64 F) { return E.Frame - F > JamCommentary::StreakWindowFrames; });
	}
	else if (E.Event == JamEvents::Board) LastBoardFrame = E.Frame;
	else if (E.Event == JamEvents::Goal) LastGoalFrame = E.Frame;
	else if (E.Event == JamEvents::Foul) LastFoulFrame = E.Frame;
	else if (E.Event == JamEvents::Faceoff && E.Kind == SfxDrop) { PrevDropFrame = LastDropFrame; LastDropFrame = E.Frame; }
	else if (E.Event == JamEvents::Check) { LastCheckFrame = E.Frame; LastCheckWhere = E.WhereM; }
	else if (E.Event == JamEvents::Pass)
	{
		if (E.Team == PassStreakTeam) ++PassStreak; else { PassStreakTeam = E.Team; PassStreak = 1; }
	}
}

void FJamCommentaryPicker::NoteFrame(int64 Frame)
{
	// Lane A105: AHokejGameMode::RestartMatch (R after a match, the Director's Call restart) puts the sim back to frame 0 with the same cartridge object, so
	// UJamCommentarySystem::RefreshSettings never resets the picker; the previous match's frame-keyed state (the booth's end frame, the gate, the cooldowns,
	// the knockdown ring) would sit in the new match's future and hold the booth silent for as long as the last match ran. The stream carries on (cosmetic).
	if (Frame < LastSeenFrame)
	{
		const bool bWasPlaying = IsLinePlaying(LastSeenFrame);
		const FJamRandom Keep = Rng;
		Reset(0);
		Rng = Keep;
		if (bWasPlaying) { bCutRequested = true; CutReason = TEXT("rewind"); }
	}
	LastSeenFrame = Frame;
}

void FJamCommentaryPicker::Observe(FName Event, const FJamEventPayload& Payload, const FVector2D& WhereM, const FJamCommentaryWorld& W)
{
	NoteFrame(W.Frame);
	if (!Settings.bEnabled) return;
	if (Event == JamEvents::Ability && Payload.Strength < 0.5f) return;   // lane A84: a grant is not narrated, only a use (Strength 1)
	FJamCommentaryEvent E;
	E.Event = Event;
	E.Kind = Payload.Kind;
	E.Who = Payload.Who;
	E.WhereM = WhereM;
	E.Strength = Payload.Strength;
	E.Frame = W.Frame;
	E.ScoreBefore[0] = bPrevScoreValid ? PrevScore[0] : W.Score[0];
	E.ScoreBefore[1] = bPrevScoreValid ? PrevScore[1] : W.Score[1];
	E.ScoreAfter[0] = W.Score[0];
	E.ScoreAfter[1] = W.Score[1];
	if (Event == JamEvents::Goal)
	{
		// The scoring team: the one whose score grew; failing that (a forced event), the side of the net.
		if (E.ScoreAfter[0] > E.ScoreBefore[0]) E.Team = 0;
		else if (E.ScoreAfter[1] > E.ScoreBefore[1]) E.Team = 1;
		else E.Team = (WhereM.X > 0) == (W.AttackSign0 > 0) ? 0 : 1;
	}
	else if (Event == JamEvents::Win || Event == JamEvents::Lose) E.Team = W.HumanTeam();
	else E.Team = W.TeamOf(Payload.Who);
	// The history before this event for the "after" conditions, then this event into the history, then the streak conditions.
	const bool bPassLike = Event == JamEvents::Pass || Event == JamEvents::Shot;
	if (bPassLike) Remember(E, W);
	// Lane A90: the rough-voice history this event's own conditions read (the knockdown ring includes this knockdown; the goal reads the checks before it).
	if (Event == JamEvents::Knockdown && E.Team >= 0 && E.Team <= 1)
	{
		TArray<int64>& Ring = KnockdownsBy[1 - E.Team];
		Ring.Add(E.Frame);
		Ring.RemoveAll([&](int64 F) { return E.Frame - F >= JamCommentary::MassacreWindowFrames; });
	}
	if (Event == JamEvents::Goal) bGoalAfterHit = E.Team >= 0 && E.Team <= 1 && E.Frame - LastTeamCheckFrame[E.Team] <= JamCommentary::AfterHitWindowFrames;
	Candidates(E, W, E.Candidates);
	if (!bPassLike) Remember(E, W);
	if (Event == JamEvents::Check && E.Team >= 0 && E.Team <= 1) LastTeamCheckFrame[E.Team] = E.Frame;
	// Lane A90: goal.after_hit is a second-beat line: the colour call keeps the goal frame, the hit's payoff follows when the gate reopens.
	if (Event == JamEvents::Goal) E.Candidates.RemoveAll([&](int32 I) { return Bank.Lines[I].Situation == SitGoalAfterHit; });
	// Lane A90 (brief A90: "check.cross: the stick lift that lands"): the sim has no cross-check Kind (A85 owns the sim); a clean lift is the stick across the
	// carrier, so the check.cross lines (event check, kind cross, never matched by a body check's Kind) are offered on it beside lift.clean.
	if (Event == JamEvents::Lift && E.Kind == KindClean)
	{
		FJamCommentaryEvent X = E;
		X.Event = JamEvents::Check;
		X.Kind = SfxCross;
		TArray<int32> C;
		Candidates(X, W, C);
		for (int32 I : C) if (Bank.Lines[I].Situation == SitCheckCross) E.Candidates.Add(I);
	}
	// board.bank is a goal-time line of event "board": a goal within a second of a board hit.
	if (Event == JamEvents::Goal && E.Frame - LastBoardFrame <= JamCommentary::BankWindowFrames && W.bPause)
	{
		for (int32 I : Bank.ForEvent(JamEvents::Board))
		{
			const FJamCommentaryLine& L = Bank.Lines[I];
			if (L.Situation != SitBoardBank || L.Priority < Settings.MinPriority()) continue;
			const int64* LastF = LastSituationFrame.Find(L.Situation);
			if (LastF && E.Frame - *LastF < static_cast<int64>(L.CooldownFrames * Settings.CooldownScale())) continue;
			E.Candidates.Add(I);
		}
	}
	if (Event == JamEvents::Goal)
	{
		// The second beat follows when the gate reopens, whether or not the colour call plays now.
		bGoalBeatPending = true;
		GoalBeat = E;
		GoalBeat.Candidates.Reset();
		GoalBeatSaid = NAME_None;
	}
	Pending.Add(MoveTemp(E));
}

void FJamCommentaryPicker::ObserveBark(int64 Frame, bool bAtHuman)
{
	NoteFrame(Frame);
	if (bAtHuman) bBarkAtHumanThisFrame = true;
}

int32 FJamCommentaryPicker::ChooseAmong(const TArray<TPair<int32, int32>>& Options) const
{
	if (Options.Num() == 0) return -1;
	int32 BestP = -1, BestS = -1;
	for (const TPair<int32, int32>& O : Options)
	{
		const FJamCommentaryLine& L = Bank.Lines[O.Key];
		const int32 S = Specificity(L);
		if (L.Priority > BestP || (L.Priority == BestP && S > BestS)) { BestP = L.Priority; BestS = S; }
	}
	TArray<int32> Top;
	for (int32 i = 0; i < Options.Num(); ++i)
	{
		const FJamCommentaryLine& L = Bank.Lines[Options[i].Key];
		if (L.Priority == BestP && Specificity(L) == BestS) Top.Add(i);
	}
	// R-0033 point 3 (lane A50, the operator's "randomly change the lines to prevent repetition"): a random draw among the top candidates, preferring the
	// lines not yet heard this match; when every one of them has been heard, any of them (Candidates already dropped the situation's last line).
	TArray<int32> Unheard;
	for (int32 i : Top) if (!HeardThisMatch.Contains(Bank.Lines[Options[i].Key].Id)) Unheard.Add(i);
	const TArray<int32>& Pool = Unheard.Num() > 0 ? Unheard : Top;
	return Pool[Rng.NextInt(Pool.Num())];
}

bool FJamCommentaryPicker::Start(int32 LineIndex, FName Event, int64 Frame, FJamCommentaryPick& Out)
{
	if (!Bank.Lines.IsValidIndex(LineIndex)) return false;
	const FJamCommentaryLine& L = Bank.Lines[LineIndex];
	Out.LineId = L.Id;
	Out.Situation = L.Situation;
	Out.Event = Event;
	Out.Text = L.Text;
	Out.Sound = L.Sound;
	Out.Frame = Frame;
	Out.DurationFrames = L.DurationFrames;
	Out.Priority = L.Priority;
	LastStartFrame = Frame;
	LastEndFrame = Frame + L.DurationFrames;
	LastSituationFrame.Add(L.Situation, Frame);
	TArray<FName>& H = SituationHistory.FindOrAdd(L.Situation);
	H.Add(L.Id);
	if (H.Num() > JamCommentary::NoRepeatPerSituation) H.RemoveAt(0);
	HeardThisMatch.Add(L.Id);   // R-0033 point 3 (lane A50)
	Last = Out;
	++PickCount;
	return true;
}

bool FJamCommentaryPicker::Resolve(const FJamCommentaryWorld& W, FJamCommentaryPick& Out, FString* OutSkip)
{
	if (OutSkip) OutSkip->Reset();
	const int64 Frame = W.Frame;
	NoteFrame(Frame);
	// The score and possession of this frame become the "before" of the next.
	auto Close = [&]
	{
		for (int32 T = 0; T < 2; ++T) LowestDeficit[T] = FMath::Max(LowestDeficit[T], W.Score[1 - T] - W.Score[T]);
		PrevScore[0] = W.Score[0];
		PrevScore[1] = W.Score[1];
		bPrevScoreValid = true;
		const int32 TouchTeam = W.TeamOf(W.PuckLastTouch);
		if (TouchTeam >= 0 && TouchTeam != PassStreakTeam) { PassStreakTeam = TouchTeam; PassStreak = 0; }
		PrevPuckLastTouch = W.PuckLastTouch;
		Pending.Reset();
		bBarkAtHumanThisFrame = false;
	};
	if (!Settings.bEnabled) { Close(); return false; }
	// The booth yields to a bark addressed to the human: nothing starts, a playing line is cut.
	// Lane A47: a must-play candidate (priority 5: the goal calls) is never silenced by a bark; in the first PIE proof a bark 61 frames before a goal held
	// the booth through the goal and the colour call fell (B-to-A-0030 fault 3, "a goal always called"). The hold still starts for everything else.
	bool bMustPlayPending = false;
	for (const FJamCommentaryEvent& E : Pending) for (int32 I : E.Candidates) if (Bank.Lines[I].Priority >= JamCommentary::MustPlayPriority) bMustPlayPending = true;
	if (bBarkAtHumanThisFrame)
	{
		BarkHoldUntil = FMath::Max(BarkHoldUntil, Frame + JamCommentary::BarkYieldFrames);
		// Lane A47 (B-to-A-0030 fault 3): a line under 0.5 s old is never cut after a word; it plays on and the hold starts after it. An older line is cut.
		if (IsLinePlaying(Frame) && Frame - LastStartFrame >= JamCommentary::CutFloorFrames) { bCutRequested = true; CutReason = TEXT("bark_at_human"); LastEndFrame = Frame; }
		if (!bMustPlayPending)
		{
			if (OutSkip && Pending.Num()) *OutSkip = TEXT("yield");
			Close();
			return false;
		}
	}
	// The goal's second beat expires when the pause ends.
	if (bGoalBeatPending && (!W.bPause || Frame - GoalBeat.Frame > JamCommentary::SecondBeatFrames)) bGoalBeatPending = false;

	TArray<TPair<int32, int32>> Options;   // (line, pending slot; -1 = the goal beat)
	for (int32 S = 0; S < Pending.Num(); ++S) for (int32 I : Pending[S].Candidates) Options.Emplace(I, S);
	bool bFromBeat = false;
	if (Options.Num() == 0 && bGoalBeatPending && IsGateOpen(Frame))
	{
		// One score-state or mood line (whether or not the colour call made it through the gate); goal.pause only if nothing else fits (B's rule 6).
		GoalBeat.Frame = Frame;
		TArray<int32> C;
		Candidates(GoalBeat, W, C);
		TArray<TPair<int32, int32>> Mood, Pause;
		for (int32 I : C)
		{
			const FName Sit = Bank.Lines[I].Situation;
			if (Sit == SitGoalBlue || Sit == SitGoalOrange || Sit == GoalBeatSaid) continue;
			(Sit == SitGoalPause ? Pause : Mood).Emplace(I, -1);
		}
		Options = Mood.Num() ? Mood : Pause;
		bFromBeat = Options.Num() > 0;
	}
	if (Options.Num() == 0) { Close(); return false; }
	const int32 Pick = ChooseAmong(Options);
	const int32 LineIndex = Options[Pick].Key;
	const int32 Slot = Options[Pick].Value;
	const FName Event = Slot >= 0 ? Pending[Slot].Event : GoalBeat.Event;
	if (!IsGateOpen(Frame))
	{
		// Lane A47 (B-to-A-0030 fault 3, A32's open point 1): a must-play line (priority 5: the goal calls) cuts a playing lower-priority line, skips the
		// 2.5 s gate behind one and plays through a bark hold; it never cuts a playing priority-5 line (then it is dropped as before).
		const bool bMustPlay = !bFromBeat && Bank.Lines[LineIndex].Priority >= JamCommentary::MustPlayPriority && !(IsLinePlaying(Frame) && Last.Priority >= JamCommentary::MustPlayPriority);
		// Lane A90 (R-0041 point 4, "the hit is called"): a check or knockdown line cuts a playing line that is not about a hit, of the same or a lower priority,
		// and skips the 2.5 s gate behind one; never inside a bark hold and never over another hit's line. The first headless pro match dropped all five
		// knockdowns of the human behind the stick chatter that led to them (a clean lift 7 and 10 frames before, a poke 50 before).
		const bool bHitEvent = Event == JamEvents::Check || Event == JamEvents::Knockdown;
		const bool bLastHit = Last.Event == JamEvents::Check || Last.Event == JamEvents::Knockdown;
		const bool bHitCall = !bFromBeat && !bMustPlay && bHitEvent && !bLastHit && Frame >= BarkHoldUntil && Bank.Lines[LineIndex].Priority >= Last.Priority;
		if (!bMustPlay && !bHitCall)
		{
			// A late quick-play line describes a play that is over: dropped, never queued (the goal beat alone waits for the gate).
			if (OutSkip) *OutSkip = Frame < LastEndFrame ? TEXT("playing") : Frame < BarkHoldUntil ? TEXT("yield") : TEXT("gate");
			Close();
			return false;
		}
		if (IsLinePlaying(Frame)) { bCutRequested = true; CutReason = bMustPlay ? TEXT("must_play") : TEXT("hit"); }
		LastEndFrame = Frame;
	}
	const bool bOk = Start(LineIndex, Event, Frame, Out);
	if (bOk)
	{
		if (Event == JamEvents::Goal && !bFromBeat) GoalBeatSaid = Out.Situation;   // the colour call (or whatever won) was said; the beat follows
		if (bFromBeat) bGoalBeatPending = false;
	}
	Close();
	return bOk;
}

bool FJamCommentaryPicker::Force(FName Id, int64 Frame, FJamCommentaryPick& Out)
{
	NoteFrame(Frame);
	const int32 I = Bank.Find(Id);
	if (I == INDEX_NONE) return false;
	return Start(I, Bank.Lines[I].Event, Frame, Out);
}

// ---------------------------------------------------------------- subsystem ----------------------------------------------------------------

// Lane A114 (track Q3): an asset whose package is not in the build is found absent once per process (the preload's list from the menu, else one
// package-store lookup, never a synchronous load) so no line, wave or crowd bed asks the loader for it during play. jam.vo.SkipAbsent=0: the commentary
// before (a line whose wave is absent is picked and played as silence, its caption shown).
static TAutoConsoleVariable<int32> CVarJamVoSkipAbsent(TEXT("jam.vo.SkipAbsent"), 0,
	TEXT("1: a commentary line whose wave is not in the build is skipped when picked (no lookup, no caption). 0: played as silence (the game before lane A114)."));

namespace
{
	bool IsAbsentAsset(const FSoftObjectPath& Path)
	{
		if (Path.IsNull() || Path.ResolveObject()) return false;
		if (UJamPreloadSubsystem::IsKnownMissing(Path)) return true;
		static TMap<FString, bool> Absent;   // game thread only
		const FString Pkg = Path.GetLongPackageName();
		if (const bool* Known = Absent.Find(Pkg)) return *Known;
		return Absent.Add(Pkg, !FPackageName::DoesPackageExist(Pkg));
	}
}

UJamCommentarySystem* UJamCommentarySystem::Get(const UObject* WorldContext)
{
	const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	return World ? World->GetSubsystem<UJamCommentarySystem>() : nullptr;
}

bool UJamCommentarySystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!Super::ShouldCreateSubsystem(Outer)) return false;
	const UWorld* World = Cast<UWorld>(Outer);
	return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UJamCommentarySystem::EnsureUp()
{
	if (bUp) return;
	UWorld* World = GetWorld();
	if (!World) return;
	bUp = true;
	Picker = MakeUnique<FJamCommentaryPicker>(FJamCommentaryBank::Embedded(), 1);
	if (UGameInstance* GI = World->GetGameInstance()) Bus = GI->GetSubsystem<UJamEventBus>();
	if (Bus) Bus->OnJamEvent.AddDynamic(this, &UJamCommentarySystem::HandleBusEvent);
	if (UJamPresentation* Pres = UJamPresentation::Get(World)) FrameHandle = Pres->OnSimFrame.AddUObject(this, &UJamCommentarySystem::HandleSimFrame);
	// The 340 waves are "load on demand" in B's import; ask for them now so the first line does not hitch on a load.
	TArray<FSoftObjectPath> Paths;
	for (const FJamCommentaryLine& L : FJamCommentaryBank::Embedded().Lines) Paths.Add(L.Sound);
	if (Paths.Num() && UAssetManager::IsInitialized()) Preload = UAssetManager::GetStreamableManager().RequestAsyncLoad(Paths, FStreamableDelegateWithHandle(), FStreamableManager::DefaultAsyncLoadPriority, false, false, FString(TEXT("JamCommentary")));
	UE_LOG(LogJamCommentary, Log, TEXT("JAMVO system up: bus=%d bank=%d lines, %d waves requested"), Bus ? 1 : 0, FJamCommentaryBank::Embedded().Lines.Num(), Paths.Num());
}

void UJamCommentarySystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	EnsureUp();
}

void UJamCommentarySystem::Deinitialize()
{
	if (Bus) Bus->OnJamEvent.RemoveDynamic(this, &UJamCommentarySystem::HandleBusEvent);
	if (UWorld* World = GetWorld()) if (UJamPresentation* Pres = UJamPresentation::Get(World)) Pres->OnSimFrame.Remove(FrameHandle);
	if (Preload.IsValid()) { Preload->CancelHandle(); Preload.Reset(); }
	Picker.Reset();
	Super::Deinitialize();
}

AHokejGameMode* UJamCommentarySystem::GameMode() const
{
	const UWorld* World = GetWorld();
	return World ? Cast<AHokejGameMode>(World->GetAuthGameMode()) : nullptr;
}

void UJamCommentarySystem::SetSettings(const FJamCommentarySettings& S)
{
	Settings = S;
	if (Picker) Picker->SetSettings(S);
}

void UJamCommentarySystem::RefreshSettings()
{
	AHokejGameMode* GM = GameMode();
	const UJamCartridge* C = GM ? GM->GetCartridge() : nullptr;
	const int32 Override = CVarJamCommentary.GetValueOnGameThread();
	if (C && C != SettingsSource)
	{
		SettingsSource = C;
		FJamCommentarySettings S = FJamCommentarySettings::FromCartridgeJson(C->RawJson);
		if (Override == 0) S.bEnabled = false;
		else if (Override == 1) S.bEnabled = true;
		SetSettings(S);
		// R-0033 point 3 (lane A50): two matches of the same cut must not open with the same line (B-to-A-0032 acceptance), so the cut's seed is salted
		// with a per-process match counter and the clock. Cosmetic only: the picker's stream is never the sim's, and the tests seed the picker directly.
		static uint64 MatchCount = 0;
		if (Picker) Picker->Reset((static_cast<uint64>(C->Seed) ^ 0xC0DEull) + (++MatchCount) * 0x9E3779B97F4A7C15ull + FPlatformTime::Cycles64());
		UE_LOG(LogJamCommentary, Log, TEXT("JAMVO settings from %s: enabled=%d intensity=%s"), *C->Id, S.bEnabled ? 1 : 0, *S.Intensity.ToString());
	}
	else if (Override >= 0 && Settings.bEnabled != (Override == 1))
	{
		FJamCommentarySettings S = Settings;
		S.bEnabled = Override == 1;
		SetSettings(S);
	}
}

void UJamCommentarySystem::BuildWorld(FJamCommentaryWorld& Out) const
{
	Out = FJamCommentaryWorld();
	AHokejGameMode* GM = GameMode();
	const UJamSim* Sim = GM ? GM->GetSim() : nullptr;
	if (!Sim) return;
	const FJamGameConfig& G = Sim->GetConfig();
	Out.Frame = Sim->GetFrame();
	Out.HumanIndex = GM->GetLocalSeat();   // lane A120 (track Q1): the local seat (the guest's on the guest's machine), was Sim->GetHumanIndex() (the host's)
	for (const FJamSkaterState& S : Sim->GetSkaters()) { Out.Teams.Add(S.Team); Out.Positions.Add(S.Pos); Out.DrawEarly.Add(S.DrawFrame == -2); }
	Out.PuckLastTouch = Sim->GetPuck().LastTouch;
	Out.PuckPos = Sim->GetPuck().Pos;
	Out.bPause = Sim->GetPhase() != EJamSimPhase::Playing;
	Out.Score[0] = Sim->GetScore(0);
	Out.Score[1] = Sim->GetScore(1);
	Out.Period = Sim->GetPeriod();
	Out.bTimer = G.Rules.Mode == FName(TEXT("timer")) && G.Rules.TimerFrames > 0;
	Out.FramesLeftInPeriod = Out.bTimer ? FMath::Max(0, (Out.Period * G.Rules.TimerFrames) / 3 - Sim->GetTimerElapsed()) : 0;
	Out.bFirstTo = !Out.bTimer;
	Out.ScoreToWin = G.Rules.ScoreToWin;
	Out.bOvertime = Sim->IsOvertime();
	Out.RinkLengthM = G.Rink.LengthM;
	Out.RinkWidthM = G.Rink.WidthM;
	Out.GoalLineX = Sim->GoalLineX();
	Out.GoalWidthM = G.Rink.GoalWidthM;
	Out.AttackSign0 = Sim->AttackGoalX(0) > 0 ? 1 : -1;
	for (const FJamZoneConfig& Z : G.Rink.Zones) Out.ZoneKinds.Add(FName(*Z.Id), Z.Kind);
	Out.bSeasonLastTake = bSeasonLastTake;
}

bool UJamCommentarySystem::IsBarkAtHuman(const FJamEventPayload& Payload, const FJamCommentaryWorld& W)
{
	// Kind = the bark situation, Who = the speaker. goal_against is the scorer's boast to the rink, a faceoff taunt is for the dot,
	// an opponent's goal_for is a complaint about a bot teammate's goal; every other bark speaks to the human (R-0007: barks are at the human).
	if (Payload.Kind == BarkGoalAgainst || Payload.Kind == BarkFaceoff) return false;
	if (Payload.Kind == BarkGoalFor) return W.TeamOf(Payload.Who) == W.HumanTeam();
	return true;
}

void UJamCommentarySystem::HandleBusEvent(FName Event, const FJamEventPayload& Payload)
{
	if (!Picker) return;
	RefreshSettings();
	FJamCommentaryWorld W;
	BuildWorld(W);
	if (W.Teams.Num() == 0) return;
	if (Event == JamEvents::Bark) { Picker->ObserveBark(W.Frame, IsBarkAtHuman(Payload, W)); return; }
	if (Event == JamEvents::Signal) return;   // the human's own voice: the booth does not narrate the bench
	Picker->Observe(Event, Payload, FVector2D(Payload.Where.X / 100.0, Payload.Where.Y / 100.0), W);
}

void UJamCommentarySystem::HandleSimFrame(const FJamPresentationFrame& F)
{
	Frame = F.SimFrame;
	if (!Picker) return;
	RefreshSettings();
	AHokejGameMode* GM = GameMode();
	const UJamSim* Sim = GM ? GM->GetSim() : nullptr;
	if (!Sim) return;
	UpdateLoops(*Sim, GM);
	FJamCommentaryWorld W;
	BuildWorld(W);
	FJamCommentaryPick P;
	FString Skip;
	const bool bStart = Picker->Resolve(W, P, &Skip);
	if (Picker->TakeCutRequest()) CutLine();   // before Play: a must-play cut names the line it cut, not the one that starts
	if (bStart && CVarJamVoSkipAbsent.GetValueOnGameThread() != 0 && IsAbsentAsset(P.Sound))
	{
		bool bAlready = false;
		MissingLogged.Add(P.Sound.ToString(), &bAlready);
		if (!bAlready) UE_LOG(LogJamCommentary, Warning, TEXT("JAMVO missing %s (logged once)"), *P.Sound.ToString());
		UE_LOG(LogJamCommentary, Log, TEXT("JAMVO_SKIP frame=%lld reason=no_wave id=%s"), W.Frame, *P.LineId.ToString());
	}
	else if (bStart) Play(P);
	else if (!Skip.IsEmpty()) UE_LOG(LogJamCommentary, Verbose, TEXT("JAMVO_SKIP frame=%lld reason=%s"), W.Frame, *Skip);
}

USoundBase* UJamCommentarySystem::LoadWave(const FSoftObjectPath& Path)
{
	const FName Key(*Path.ToString());
	if (const TObjectPtr<USoundBase>* Found = Waves.Find(Key)) return Found->Get();
	USoundBase* S = Cast<USoundBase>(Path.ResolveObject());
	if (!S && !IsAbsentAsset(Path)) S = Cast<USoundBase>(StaticLoadObject(USoundBase::StaticClass(), nullptr, *Path.ToString(), nullptr, LOAD_NoWarn | LOAD_Quiet));
	if (!S)
	{
		bool bAlready = false;
		MissingLogged.Add(Path.ToString(), &bAlready);
		if (!bAlready) UE_LOG(LogJamCommentary, Warning, TEXT("JAMVO missing %s (logged once)"), *Path.ToString());
		return nullptr;
	}
	Waves.Add(Key, S);
	return S;
}

void UJamCommentarySystem::CutLine()
{
	if (Voice && Voice->IsPlaying())
	{
		Voice->FadeOut(0.15f, 0.f);
		UE_LOG(LogJamCommentary, Log, TEXT("JAMVO_CUT id=%s frame=%lld reason=%s"), *Last.LineId.ToString(), Frame, Picker ? Picker->GetCutReason() : TEXT("bark_at_human"));
	}
}

int64 UJamCommentarySystem::GetLineEndFrame() const
{
	return Picker ? Picker->GetLastEndFrame() : -1000000;
}

void UJamCommentarySystem::Play(const FJamCommentaryPick& P)
{
	Last = P;
	++PlayCount;
	UE_LOG(LogJamCommentary, Log, TEXT("JAMVO id=%s event=%s frame=%lld dur=%d text=\"%s\""), *P.LineId.ToString(), *P.Event.ToString(), P.Frame, P.DurationFrames, *P.Text);
	UWorld* World = GetWorld();
	USoundBase* Wave = World ? LoadWave(P.Sound) : nullptr;
	if (World && Wave)
	{
		if (!bVoicePlayerProbed)
		{
			bVoicePlayerProbed = true;
			VoicePlayer = Cast<USoundBase>(UJamEffectsBinder::LoadOrLogOnce(FSoftObjectPath(PathCommentary)));
		}
		if (VoicePlayer)
		{
			if (!Voice) Voice = UGameplayStatics::CreateSound2D(World, VoicePlayer, 1.f, 1.f, 0.f, nullptr, false, false);
			if (Voice)
			{
				if (Voice->IsPlaying()) Voice->Stop();
				Voice->SetObjectParameter(TEXT("Line"), Wave);
				Voice->SetFloatParameter(TEXT("Volume"), 1.f);
				Voice->Play();
			}
		}
		else
		{
			// No MS_commentary: the wave itself (it sits in SC_VO, so the sliders still reach it; no duck).
			if (Voice && Voice->IsPlaying()) Voice->Stop();
			Voice = UGameplayStatics::SpawnSound2D(World, Wave, 1.f, 1.f, 0.f, nullptr, false, false);
		}
	}
	OnLine.Broadcast(P);
}

bool UJamCommentarySystem::SayLine(FName Id)
{
	EnsureUp();
	if (!Picker) return false;
	FJamCommentaryPick P;
	if (!Picker->Force(Id, Frame, P)) return false;
	if (Picker->TakeCutRequest()) CutLine();
	Play(P);
	return true;
}

void UJamCommentarySystem::OnMatchBuilt(AHokejGameMode* GM)
{
	EnsureUp();
	UWorld* World = GetWorld();
	if (!World || !GM) return;
	RefreshSettings();
	// The season take (B-to-A-0026 ask 3, event season_take): the running ladder take, before its first drop.
	bSeasonLastTake = false;
	if (const UJamCartridge* C = GM->GetCartridge())
	{
		if (UJamProgression* Prog = UJamProgression::Get(this); Prog && Prog->IsSeasonMatch(C->SourcePath))
		{
			const FJamSeasonState S = Prog->GetSeasonState();
			if (S.ActiveTake >= 0)
			{
				bSeasonLastTake = S.ActiveTake == S.TakeCount - 1;
				if (Bus)
				{
					FJamEventPayload Pl;
					Pl.Kind = FName(*FString::Printf(TEXT("level_%d"), S.ActiveTake + 1));
					Bus->Broadcast(JamEvents::SeasonTake, Pl);
				}
			}
		}
	}
	// Music from match start; the Level follows the phase every sim frame (UpdateLoops).
	if (!Music)
	{
		if (USoundBase* M = Cast<USoundBase>(UJamEffectsBinder::LoadOrLogOnce(FSoftObjectPath(PathMusic))))
		{
			Music = UGameplayStatics::CreateSound2D(World, M, 1.f, 1.f, 0.f, nullptr, false, false);
			if (Music) { Music->SetFloatParameter(TEXT("Level"), JamCommentary::MusicLevelPause); Music->Play(); MusicLevel = JamCommentary::MusicLevelPause; }
		}
	}
	// R-0033 point 4 (lane A50): B's crowd bed loop starts with the music when the asset exists (a quiet load, no "missing" line: B says when it lands).
	if (!CrowdBed && !IsAbsentAsset(FSoftObjectPath(PathCrowdBed)))   // lane A114: an absent bed is asked for once per process, not at every match start
	{
		if (USoundBase* Bed = Cast<USoundBase>(StaticLoadObject(USoundBase::StaticClass(), nullptr, PathCrowdBed, nullptr, LOAD_NoWarn | LOAD_Quiet)))
		{
			CrowdBed = UGameplayStatics::CreateSound2D(World, Bed, 1.f, 1.f, 0.f, nullptr, false, false);
			if (CrowdBed) CrowdBed->Play();
		}
	}
	// The skate loop is attached to the human's skater when the first frame runs (the actor may be rebuilt by LoadCut).
	if (Skate) { Skate->Stop(); Skate->DestroyComponent(); Skate = nullptr; SkateOwner = nullptr; }
	UE_LOG(LogJamCommentary, Log, TEXT("JAMVO match built: music=%d crowd_bed=%d season_last=%d"), Music ? 1 : 0, CrowdBed ? 1 : 0, bSeasonLastTake ? 1 : 0);
}

void UJamCommentarySystem::UpdateLoops(const UJamSim& Sim, AHokejGameMode* GM)
{
	// MS_music_match Level: 0.35 in play, 0.6 in pauses and at the end (the sound smooths over 0.5 s; the duck under a line is B's SCM).
	if (Music)
	{
		const float Target = Sim.GetPhase() == EJamSimPhase::Playing ? JamCommentary::MusicLevelPlay : JamCommentary::MusicLevelPause;
		if (Target != MusicLevel) { MusicLevel = Target; Music->SetFloatParameter(TEXT("Level"), Target); }
	}
	// MS_skate_loop on the human's skater: Speed 0..1 every frame, Turn when |yaw rate| passes 3 rad/s (B-to-A-0007 point 4).
	AJamSkater* Human = GM ? GM->GetHumanSkater() : nullptr;
	if (Human && (!Skate || SkateOwner != Human))
	{
		if (Skate) { Skate->Stop(); Skate->DestroyComponent(); Skate = nullptr; }
		if (USoundBase* L = Cast<USoundBase>(UJamEffectsBinder::LoadOrLogOnce(FSoftObjectPath(PathSkate))))
		{
			Skate = UGameplayStatics::SpawnSoundAttached(L, Human->GetRootComponent(), NAME_None, FVector::ZeroVector, EAttachLocation::KeepRelativeOffset, true, 1.f, 1.f, 0.f, nullptr, nullptr, false);
			SkateOwner = Human;
			TurnCooldown = 0;
		}
	}
	if (Skate && Sim.GetSkaters().IsValidIndex(Sim.GetHumanIndex()))
	{
		const FJamSkaterState& S = Sim.GetSkaters()[Sim.GetHumanIndex()];
		const double Top = FMath::Max(Sim.GetConfig().Skater.Speed, 0.1);
		Skate->SetFloatParameter(TEXT("Speed"), static_cast<float>(FMath::Clamp(S.Vel.Size() / Top, 0.0, 1.0)));
		if (TurnCooldown > 0) --TurnCooldown;
		else if (FMath::Abs(S.YawRate) > JamCommentary::TurnTriggerRadS && S.Vel.Size() > 1.0)
		{
			Skate->SetTriggerParameter(TEXT("Turn"));
			TurnCooldown = JamCommentary::TurnGapFrames;
		}
	}
}

// ---------------------------------------------------------------- console ----------------------------------------------------------------

static FAutoConsoleCommandWithWorldAndArgs CmdJamSayLine(TEXT("jam.say_line"),
	TEXT("jam.say_line <id>: play one commentator line by id for a PIE proof (goal.blue.01, save.big.02, ...); no rules but the booth itself."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		UJamCommentarySystem* S = World ? World->GetSubsystem<UJamCommentarySystem>() : nullptr;
		if (!S || Args.Num() == 0) { UE_LOG(LogJamCommentary, Warning, TEXT("jam.say_line: no commentary system in this world or no id given")); return; }
		const bool bOk = S->SayLine(FName(*Args[0]));
		UE_LOG(LogJamCommentary, Log, TEXT("jam.say_line %s: %s"), *Args[0], bOk ? *S->GetLastLine().Text : TEXT("unknown id"));
	}));
