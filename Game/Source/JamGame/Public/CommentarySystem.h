// Lane A32 (B-to-A-0026, R-0027 point 4): the commentator in the booth, beside lane A12's bark system. On every bus event the picker takes the
// manifest lines whose event matches, whose kind is empty or equal to the payload's, whose team is any or the event's, and whose `when` holds
// (briefs/commentary/README.md sections 3 and 4); the highest priority wins, ties go to the more specific line, then an alternate at random
// that is not among the situation's last three. One line at a time, at most one every 2.5 s from the start of the last line, a late quick-play
// line is dropped and never queued; a stoppage line plays only in a pause; the booth yields to a bark addressed to the human. Plays through
// /JamAssets/Audio/MS_commentary (wave input Line, which ducks SC_Music by 7 dB). Every count is in sim frames (60 Hz): nothing reads a clock.
// The same subsystem runs B's two loops: MS_music_match (Level 0.35 in play, 0.6 in pauses) and MS_skate_loop on the human's skater (Speed, Turn).
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "JamEventBus.h"
#include "JamRandom.h"
#include "JamCommentaryData.h"
#include "CommentarySystem.generated.h"

class AHokejGameMode;
class AJamSkater;
class UJamCartridge;
class UJamSim;
class UAudioComponent;
class USoundBase;
struct FJamPresentationFrame;
struct FStreamableHandle;

namespace JamCommentary
{
	// Timing in sim frames (60 Hz).
	constexpr int32 GateFrames = 150;           // at most one line per 2.5 s, from the start of the last line (B's rule 5)
	constexpr int32 BarkYieldFrames = 150;      // a bark addressed to the human holds the booth for its bubble hold
	constexpr int32 NoRepeatPerSituation = 3;   // the last three alternates of a situation are skipped
	constexpr int32 SecondBeatFrames = 300;     // a goal's second beat may still follow this long after the goal, in the pause (B's rule 6)
	constexpr int32 LastTenFrames = 600;        // "late", "last ten seconds"
	constexpr int32 StreakWindowFrames = 600;   // shot.human_streak: three shots in ten seconds
	constexpr int32 BankWindowFrames = 60;      // board.bank: a goal within a second of a board hit
	constexpr int32 ProtectWindowFrames = 120;  // carry.protect: a check on the carrier within two seconds
	constexpr int32 LongShotWindowFrames = 180; // goal.long reads the shot that went in
	constexpr float MusicLevelPlay = 0.35f;     // MS_music_match Level in play
	constexpr float MusicLevelPause = 0.6f;     // ... in pauses and at the end
	constexpr double TurnTriggerRadS = 3.0;     // MS_skate_loop Turn when |yaw rate| passes this (B-to-A-0007 point 4)
	constexpr int32 TurnGapFrames = 15;

	JAMGAME_API extern const FName IntensityQuiet;   // "quiet"
	JAMGAME_API extern const FName IntensityNormal;  // "normal"
	JAMGAME_API extern const FName IntensityLoud;    // "loud"
}

// game.commentary of the cartridge (schema/game.schema.json): absent = enabled, normal.
USTRUCT(BlueprintType)
struct JAMGAME_API FJamCommentarySettings
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Commentary") bool bEnabled = true;
	// "quiet" | "normal" | "loud"
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Commentary") FName Intensity = TEXT("normal");

	// Reads game.commentary from a cartridge's raw JSON (the loader's FJamGameConfig has no field for it; the schema validated the keys).
	static FJamCommentarySettings FromCartridgeJson(const FString& RawJson);
	// Situation cooldown multiplier: quiet 2, normal 1, loud 0.5. The 2.5 s gate never scales.
	float CooldownScale() const;
	// Lowest priority that plays: quiet 3, else 1.
	int32 MinPriority() const;
};

// The match facts the picker needs, filled from the sim by the subsystem once per frame, or by hand in tests. Positions in rink metres.
struct JAMGAME_API FJamCommentaryWorld
{
	int64 Frame = 0;
	int32 HumanIndex = 0;
	TArray<int32> Teams;            // team per skater index
	TArray<FVector2D> Positions;    // rink metres per skater
	TArray<bool> DrawEarly;         // the centre moved before the drop (FJamSkaterState::DrawFrame == -2)
	int32 PuckLastTouch = -1;
	FVector2D PuckPos = FVector2D::ZeroVector;
	// The sim is not in Playing (goal pause, faceoff countdown, match over): stoppage lines may play.
	bool bPause = false;
	int32 Score[2] = { 0, 0 };
	int32 Period = 1;
	bool bTimer = false;
	int32 FramesLeftInPeriod = 0;
	bool bFirstTo = true;
	int32 ScoreToWin = 0;
	bool bOvertime = false;
	double RinkLengthM = 90.0;
	double RinkWidthM = 45.0;
	double GoalLineX = 41.0;
	double GoalWidthM = 4.0;
	// +1 when team 0 attacks +x (the default), -1 after SwapNets.
	int32 AttackSign0 = 1;
	// zone id -> zone kind (game.rink.zones), for zone_enter / zone_exit lines whose kind is the zone kind.
	TMap<FName, FName> ZoneKinds;
	// The running season take is the ladder's last (win.season).
	bool bSeasonLastTake = false;

	int32 HumanTeam() const { return Teams.IsValidIndex(HumanIndex) ? Teams[HumanIndex] : 0; }
	int32 TeamOf(int32 Who) const { return Teams.IsValidIndex(Who) ? Teams[Who] : -1; }
	int32 AttackSign(int32 Team) const { return Team == 0 ? AttackSign0 : -AttackSign0; }
	// Team 0 is blue, team 1 orange (B's README section 4); anything else is "any".
	static FName ColourOf(int32 Team);
};

// One bus event as the picker saw it, with the facts of its frame.
struct JAMGAME_API FJamCommentaryEvent
{
	FName Event;
	FName Kind;
	int32 Who = -1;
	FVector2D WhereM = FVector2D::ZeroVector;
	float Strength = 0.f;
	int64 Frame = 0;
	// The event's team: the scoring team for goal, the human's team for win and lose, the team of Who otherwise; -1 when none.
	int32 Team = -1;
	int32 ScoreBefore[2] = { 0, 0 };
	int32 ScoreAfter[2] = { 0, 0 };
	// Lines that may play for it (indices into the bank), decided when it was observed.
	TArray<int32> Candidates;
};

USTRUCT(BlueprintType)
struct JAMGAME_API FJamCommentaryPick
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Commentary") FName LineId;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Commentary") FName Situation;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Commentary") FName Event;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Commentary") FString Text;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Commentary") FSoftObjectPath Sound;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Commentary") int64 Frame = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Commentary") int32 DurationFrames = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Commentary") int32 Priority = 0;
};

// Chooses the line. Engine-free (tests drive it without a world); owns the cooldowns, the history, the gate and its own random stream (cosmetic,
// never the sim's). Protocol per sim frame: Observe() every bus event of the frame (and ObserveBark for barks), then Resolve() once at the end.
class JAMGAME_API FJamCommentaryPicker
{
public:
	explicit FJamCommentaryPicker(const FJamCommentaryBank& InBank, uint64 Seed = 1) : Bank(InBank) { Reset(Seed); }
	void Reset(uint64 Seed);
	void SetSettings(const FJamCommentarySettings& S) { Settings = S; }
	const FJamCommentarySettings& GetSettings() const { return Settings; }

	// One bus event of the current frame. WhereM in rink metres.
	void Observe(FName Event, const FJamEventPayload& Payload, const FVector2D& WhereM, const FJamCommentaryWorld& W);
	// A bark played this frame. When it is addressed to the human the booth yields: no line starts for BarkYieldFrames and a playing line is cut
	// (the subsystem reads TakeCutRequest and fades the voice).
	void ObserveBark(int64 Frame, bool bAtHuman);
	// End of frame: starts at most one line. OutSkip (optional) says why the best candidate was dropped: "gate", "playing", "yield", "pause".
	bool Resolve(const FJamCommentaryWorld& W, FJamCommentaryPick& Out, FString* OutSkip = nullptr);
	// jam.say_line and tests: starts the line by id regardless of the rules; it still takes the booth (gate and duration). False for an unknown id.
	bool Force(FName Id, int64 Frame, FJamCommentaryPick& Out);

	bool IsGateOpen(int64 Frame) const { return Frame - LastStartFrame >= JamCommentary::GateFrames && Frame >= LastEndFrame && Frame >= BarkHoldUntil; }
	bool IsLinePlaying(int64 Frame) const { return Frame < LastEndFrame; }
	bool TakeCutRequest() { const bool b = bCutRequested; bCutRequested = false; return b; }
	int32 GetPickCount() const { return PickCount; }
	const FJamCommentaryPick& GetLast() const { return Last; }
	int64 GetLastEndFrame() const { return LastEndFrame; }

	// The rules, for the tests. Candidates: the event's lines whose kind, team and when hold, at or above the intensity's priority floor, off their
	// situation's cooldown, not among its last three, and (stoppage lines) only in a pause. Specificity: kind + colour + a conditional situation + a human one.
	void Candidates(const FJamCommentaryEvent& E, const FJamCommentaryWorld& W, TArray<int32>& Out) const;
	bool WhenHolds(const FJamCommentaryLine& L, const FJamCommentaryEvent& E, const FJamCommentaryWorld& W) const;
	static int32 Specificity(const FJamCommentaryLine& L);
	static FName Suffix(const FJamCommentaryLine& L);

private:
	bool Start(int32 LineIndex, FName Event, int64 Frame, FJamCommentaryPick& Out);
	void Remember(const FJamCommentaryEvent& E, const FJamCommentaryWorld& W);
	int32 ChooseAmong(const TArray<TPair<int32, int32>>& Options) const;   // (line index, event slot); -1 when empty

	const FJamCommentaryBank& Bank;
	FJamCommentarySettings Settings;
	mutable FJamRandom Rng;
	TArray<FJamCommentaryEvent> Pending;

	int64 LastStartFrame = -1000000;
	int64 LastEndFrame = -1000000;
	int64 BarkHoldUntil = -1000000;
	bool bBarkAtHumanThisFrame = false;
	bool bCutRequested = false;
	TMap<FName, int64> LastSituationFrame;
	TMap<FName, TArray<FName>> SituationHistory;
	FJamCommentaryPick Last;
	int32 PickCount = 0;

	// History the conditions read.
	int32 PrevScore[2] = { 0, 0 };
	bool bPrevScoreValid = false;
	int32 LowestDeficit[2] = { 0, 0 };   // the most a team has been behind this match
	int32 PrevPuckLastTouch = -1;
	int64 LastShotFrame = -1000000;
	FVector2D LastShotWhere = FVector2D::ZeroVector;
	int32 LastShotTeam = -1;
	TArray<int64> HumanShotFrames;
	int64 LastBoardFrame = -1000000;
	int64 LastGoalFrame = -1000000;
	int64 LastFoulFrame = -1000000;
	int64 LastDropFrame = -1000000;
	int64 PrevDropFrame = -1000000;
	int64 LastCheckFrame = -1000000;
	FVector2D LastCheckWhere = FVector2D::ZeroVector;
	int32 PassStreakTeam = -1;
	int32 PassStreak = 0;
	// A goal's second beat (B's rule 6): one score-state or mood line when the gate reopens in the pause; goal.pause only if nothing else fits.
	bool bGoalBeatPending = false;
	FJamCommentaryEvent GoalBeat;
	FName GoalBeatSaid;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FJamCommentaryDelegate, const FJamCommentaryPick&);

// Per-match world subsystem: listens to UJamEventBus and UJamPresentation::OnSimFrame, reads the sim through AHokejGameMode, plays the picked
// line through MS_commentary and runs the music and skate loops. Logs "JAMVO id=<id> event=<e> frame=<f>" per line.
UCLASS()
class JAMGAME_API UJamCommentarySystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static UJamCommentarySystem* Get(const UObject* WorldContext);

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

	// The game mode's one hook (BuildMatch, after the opening events): starts MS_music_match, attaches MS_skate_loop to the human's skater and
	// broadcasts season_take when the cartridge is the running season take.
	void OnMatchBuilt(AHokejGameMode* GM);

	// jam.say_line: plays a line by id for a proof (no rules but the booth itself).
	UFUNCTION(BlueprintCallable, Category = "Jam|Commentary")
	bool SayLine(FName Id);

	UFUNCTION(BlueprintPure, Category = "Jam|Commentary") const FJamCommentaryPick& GetLastLine() const { return Last; }
	UFUNCTION(BlueprintPure, Category = "Jam|Commentary") int32 GetLineCount() const { return PlayCount; }
	UFUNCTION(BlueprintPure, Category = "Jam|Commentary") FJamCommentarySettings GetSettings() const { return Settings; }
	UFUNCTION(BlueprintCallable, Category = "Jam|Commentary") void SetSettings(const FJamCommentarySettings& S);

	// Fires after every line that starts (the Designer panel, tests).
	FJamCommentaryDelegate OnLine;

protected:
	UFUNCTION() void HandleBusEvent(FName Event, const FJamEventPayload& Payload);
	void HandleSimFrame(const FJamPresentationFrame& Frame);

private:
	void EnsureUp();
	void RefreshSettings();
	void BuildWorld(FJamCommentaryWorld& Out) const;
	AHokejGameMode* GameMode() const;
	void Play(const FJamCommentaryPick& P);
	void CutLine();
	void UpdateLoops(const UJamSim& Sim, AHokejGameMode* GM);
	USoundBase* LoadWave(const FSoftObjectPath& Path);
	// A bark is addressed to the human unless it is goal_against, a faceoff taunt, or an opponent's complaint about a bot teammate's goal.
	static bool IsBarkAtHuman(const FJamEventPayload& Payload, const FJamCommentaryWorld& W);

	UPROPERTY(Transient) TObjectPtr<UJamEventBus> Bus;
	UPROPERTY(Transient) TObjectPtr<const UJamCartridge> SettingsSource;
	UPROPERTY(Transient) TObjectPtr<UAudioComponent> Voice;
	UPROPERTY(Transient) TObjectPtr<USoundBase> VoicePlayer;
	UPROPERTY(Transient) TObjectPtr<UAudioComponent> Music;
	UPROPERTY(Transient) TObjectPtr<UAudioComponent> Skate;
	UPROPERTY(Transient) TObjectPtr<AJamSkater> SkateOwner;
	UPROPERTY(Transient) TMap<FName, TObjectPtr<USoundBase>> Waves;
	TSharedPtr<FStreamableHandle> Preload;
	FDelegateHandle FrameHandle;
	TUniquePtr<FJamCommentaryPicker> Picker;
	FJamCommentarySettings Settings;
	FJamCommentaryPick Last;
	int32 PlayCount = 0;
	int64 Frame = 0;
	bool bUp = false;
	bool bVoicePlayerProbed = false;
	float MusicLevel = -1.f;
	int32 TurnCooldown = 0;
	bool bSeasonLastTake = false;
	TSet<FString> MissingLogged;
};
