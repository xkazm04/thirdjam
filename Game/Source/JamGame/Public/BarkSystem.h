// Lane A12 (ruling R-0007 point 2): situational barks. Opponents mock and complain, teammates encourage, praise and complain at the
// human; the human's own figure stays silent. Lines come only from the reviewed bank (Game/Content/Barks/barks.json, embedded as
// BarkData.inl): no model text reaches the player at runtime. The picker is engine-free logic (tests run it without a world); the world
// subsystem wires it to the event bus, the sim and the HUD, and counts frames, never seconds.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "JamEventBus.h"
#include "JamRandom.h"
#include "BarkSystem.generated.h"

class UJamSim;

class UJamBarkWidget;
class AHokejGameMode;
class AJamSkater;
struct FJamPresentationFrame;
class UJamCartridge;

JAMGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogJamBarks, Log, All);

// Situation names (the bank's "situation" field, seen from the human player).
namespace JamBarks
{
	JAMGAME_API extern const FName GoalFor;         // "goal_for"
	JAMGAME_API extern const FName GoalAgainst;     // "goal_against"
	JAMGAME_API extern const FName CheckLanded;     // "check_landed"
	JAMGAME_API extern const FName CheckReceived;   // "check_received"
	JAMGAME_API extern const FName Steal;           // "steal"
	JAMGAME_API extern const FName Stall;           // "stall"
	JAMGAME_API extern const FName NearMiss;        // "near_miss"
	JAMGAME_API extern const FName Faceoff;         // "faceoff"
	JAMGAME_API extern const FName Win;             // "win"
	JAMGAME_API extern const FName Lose;            // "lose"
	JAMGAME_API extern const FName DirectorsCall;   // "directors_call"
	JAMGAME_API extern const FName SignalAckDefend; // "signal_ack_defend"
	JAMGAME_API extern const FName SignalAckSpace;  // "signal_ack_space"
	JAMGAME_API extern const FName SignalAckPass;   // "signal_ack_pass"
	JAMGAME_API extern const FName PraiseReply;     // "praise_reply"

	JAMGAME_API extern const FName SpeakerOpponent; // "opponent"
	JAMGAME_API extern const FName SpeakerTeammate; // "teammate"

	JAMGAME_API extern const FName ToneMock;        // "mock"
	JAMGAME_API extern const FName ToneSavage;      // "savage" (R-0015: loud only)
	JAMGAME_API extern const FName ToneComplain;    // "complain"
	JAMGAME_API extern const FName ToneBravado;     // "bravado"
	JAMGAME_API extern const FName ToneEncourage;   // "encourage"
	JAMGAME_API extern const FName TonePraise;      // "praise"

	JAMGAME_API extern const FName IntensityQuiet;  // "quiet"
	JAMGAME_API extern const FName IntensityNormal; // "normal"
	JAMGAME_API extern const FName IntensityLoud;   // "loud"

	// The bus event this system would broadcast (proposed for contracts/events.md): Who = speaker, Kind = situation, Strength 0,
	// the text in UJamBarkSystem::GetLastBark(). Broadcast only once the bus knows the name (IsKnownEvent), never as an unknown.
	JAMGAME_API extern const FName BusEvent;        // "bark"

	// Timing in sim frames (60 Hz) at normal intensity; quiet doubles the cooldowns, loud halves them.
	constexpr int32 GlobalCooldownFrames = 240;   // 4 s
	constexpr int32 SpeakerCooldownFrames = 480;  // 8 s
	constexpr int32 NoRepeatWindow = 10;          // picks
	constexpr int32 BubbleHoldFrames = 150;       // 2.5 s fully visible
	constexpr int32 BubbleFadeFrames = 90;        // then 1.5 s fade
	constexpr int32 StallFrames = 300;            // 5 s without a puck touch and a slow puck
	constexpr int32 NearMissWindow = 45;          // frames between a shot and a board hit near the net
}

UENUM(BlueprintType)
enum class EJamBarkSpeaker : uint8 { Opponent, Teammate };

USTRUCT(BlueprintType)
struct JAMGAME_API FJamBarkLine
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Barks") FName Id;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Barks") FName Situation;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Barks") FName Speaker;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Barks") FName Tone;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Barks") FString Text;
};

// The parsed bank with an index by (situation, speaker, tone).
struct JAMGAME_API FJamBarkBank
{
	TArray<FJamBarkLine> Lines;

	// Strict parse of barks.json (hokej.barks/1): unknown keys, missing fields, a non-string text and a text over 60 characters are
	// errors (one sentence each, "at <pointer>: ...").
	bool Parse(const FString& Json, TArray<FString>& OutErrors);

	// Indices of the lines for one combination (empty array when none).
	const TArray<int32>& Find(FName Situation, FName Speaker, FName Tone) const;
	int32 Count(FName Situation, FName Speaker, FName Tone) const { return Find(Situation, Speaker, Tone).Num(); }

	// The embedded copy (BarkData.inl), parsed once; and its raw text (tests compare it with the file on disk).
	static const FJamBarkBank& Embedded();
	static FString EmbeddedJson();

private:
	TMap<FString, TArray<int32>> Index;
	static FString Key(FName S, FName Sp, FName T) { return S.ToString() + TEXT("|") + Sp.ToString() + TEXT("|") + T.ToString(); }
};

// game.barks of the cartridge (schema/game.schema.json): absent = enabled, normal.
USTRUCT(BlueprintType)
struct JAMGAME_API FJamBarkSettings
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Barks") bool bEnabled = true;
	// "quiet" | "normal" | "loud"
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Barks") FName Intensity = TEXT("normal");

	// Reads game.barks from a cartridge's raw JSON (the loader's FJamGameConfig has no Barks field; the schema validated the keys).
	static FJamBarkSettings FromCartridgeJson(const FString& RawJson);
	// Cooldown multiplier: quiet 2, normal 1, loud 0.5.
	float CooldownScale() const;
};

// The facts the picker needs about the match, filled from the sim by the subsystem or by hand in tests.
struct JAMGAME_API FJamBarkWorld
{
	int32 HumanIndex = 0;
	TArray<int32> Teams;          // team per skater index
	TArray<FVector2D> Positions;  // rink metres per skater
	int32 PuckLastTouch = -1;
	int32 HumanTeam() const { return Teams.IsValidIndex(HumanIndex) ? Teams[HumanIndex] : 0; }
	bool IsTeammate(int32 I) const { return I != HumanIndex && Teams.IsValidIndex(I) && Teams[I] == HumanTeam(); }
	bool IsOpponent(int32 I) const { return Teams.IsValidIndex(I) && Teams[I] != HumanTeam(); }
	// Nearest skater of the given side to a rink position (-1 when none).
	int32 Nearest(const FVector2D& Where, bool bOpponents) const;
};

USTRUCT(BlueprintType)
struct JAMGAME_API FJamBarkPick
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Barks") int32 Speaker = -1;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Barks") FName Situation;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Barks") FName Tone;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Barks") FName LineId;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Barks") FString Text;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Barks") int64 Frame = 0;
};

// What a bus event means for the barks: the situation and who speaks. Engine-free, deterministic given the world.
struct JAMGAME_API FJamBarkResolved
{
	FName Situation;
	EJamBarkSpeaker Side = EJamBarkSpeaker::Opponent;
	int32 Speaker = -1;   // skater index
	FVector2D Where = FVector2D::ZeroVector;
};

// Chooses who says what. Owns the cooldowns, the no-repeat history and its own random stream (cosmetic: never the sim's).
class JAMGAME_API FJamBarkPicker
{
public:
	explicit FJamBarkPicker(const FJamBarkBank& InBank, uint64 Seed = 1) : Bank(InBank) { Reset(Seed); }
	void Reset(uint64 Seed);
	void SetSettings(const FJamBarkSettings& S) { Settings = S; }
	const FJamBarkSettings& GetSettings() const { return Settings; }

	// Maps a bus event (contracts/events.md) plus the world to a bark situation and a speaker. False when the event says nothing
	// (shot, board, pickups, period) or nobody is there to speak. Where in rink metres.
	static bool Resolve(FName Event, const FJamEventPayload& Payload, const FVector2D& WhereM, const FJamBarkWorld& W, FJamBarkResolved& Out);
	// Picks the speaker for a situation that has no event position (stall, directors_call, signal acks): the nearest opponent to the
	// human for mocks, the nearest teammate otherwise.
	static int32 SpeakerFor(FName Situation, EJamBarkSpeaker Side, const FVector2D& WhereM, const FJamBarkWorld& W);

	// The tone for a side at the current intensity (seeded weights): quiet never mocks, loud mostly mocks.
	FName ChooseTone(FName Situation, EJamBarkSpeaker Side);

	// Picks a line: false when disabled, on cooldown (global or this speaker) or when the bank has nothing for the combination.
	bool Pick(FName Situation, EJamBarkSpeaker Side, int32 SpeakerIndex, int64 Frame, FJamBarkPick& Out);
	// Pick with a forced tone (tests, signal acks). bIgnoreCooldown: an acknowledgement must always answer (it still sets the cooldowns).
	bool PickWithTone(FName Situation, EJamBarkSpeaker Side, FName Tone, int32 SpeakerIndex, int64 Frame, FJamBarkPick& Out, bool bIgnoreCooldown = false);

	bool IsOnCooldown(int32 SpeakerIndex, int64 Frame) const;
	int32 GetPickCount() const { return PickCount; }
	const TArray<FName>& GetHistory() const { return History; }

private:
	const FJamBarkBank& Bank;
	FJamBarkSettings Settings;
	FJamRandom Rng;
	int64 LastGlobalFrame = -1000000;
	TMap<int32, int64> LastSpeakerFrame;
	TArray<FName> History;
	int32 PickCount = 0;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FJamBarkDelegate, const FJamBarkPick&);

// Per-match world subsystem: listens to UJamEventBus and UJamPresentation::OnSimFrame, reads the sim through AHokejGameMode,
// shows bubbles and the HUD strip through UJamBarkWidget, optionally voices the line through the engine's TextToSpeech plugin.
UCLASS()
class JAMGAME_API UJamBarkSystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	static UJamBarkSystem* Get(const UObject* WorldContext);

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

	// A situation from outside the bus (Phase 2: UJamSignalSystem's acknowledgements; the console). Side picks the speaker group.
	// bForce skips the cooldown (signal acknowledgements) and uses the encourage tone for teammates.
	UFUNCTION(BlueprintCallable, Category = "Jam|Barks")
	bool Trigger(FName Situation, EJamBarkSpeaker Side, int32 PreferredSpeaker = -1, bool bForce = false);

	UFUNCTION(BlueprintPure, Category = "Jam|Barks") const FJamBarkPick& GetLastBark() const { return LastBark; }
	UFUNCTION(BlueprintPure, Category = "Jam|Barks") int32 GetBarkCount() const { return BarkCount; }
	UFUNCTION(BlueprintPure, Category = "Jam|Barks") FJamBarkSettings GetSettings() const { return Settings; }
	UFUNCTION(BlueprintCallable, Category = "Jam|Barks") void SetSettings(const FJamBarkSettings& S);
	UFUNCTION(BlueprintPure, Category = "Jam|Barks") bool IsVoiceAvailable() const { return bVoiceAvailable; }

	// Fires after every shown bark (the Designer panel, tests, Phase 2).
	FJamBarkDelegate OnBark;

	// Voice through the engine's TextToSpeech plugin (Flite on Windows, no download) when the plugin is enabled and jam.BarkVoice is 1.
	// Reached by reflection so JamGame has no dependency on an experimental plugin that may be off. Returns false when unavailable.
	static bool SpeakWithEngineTts(const FString& Text, float Rate, float Volume);

	UJamBarkWidget* GetWidget() const { return Widget; }

protected:
	UFUNCTION() void HandleBusEvent(FName Event, const FJamEventPayload& Payload);
	void HandleSimFrame(const FJamPresentationFrame& Frame);

private:
	void RefreshSettings();
	void BuildWorld(FJamBarkWorld& Out) const;
public:
	// Lane A120 (track Q1): the bark world of a sim seen from Seat (the machine's own seat: the guest's online). Presentation only: reads the sim, writes nothing.
	static void FillWorld(const UJamSim& Sim, int32 Seat, FJamBarkWorld& Out);
private:
	void Show(const FJamBarkPick& P);
	AHokejGameMode* GameMode() const;

	UPROPERTY(Transient) TObjectPtr<UJamEventBus> Bus;
	UPROPERTY(Transient) TObjectPtr<UJamBarkWidget> Widget;
	UPROPERTY(Transient) TObjectPtr<const UJamCartridge> SettingsSource;
	FDelegateHandle FrameHandle;
	TUniquePtr<FJamBarkPicker> Picker;
	FJamBarkSettings Settings;
	FJamBarkPick LastBark;
	int32 BarkCount = 0;
	int64 Frame = 0;
	bool bVoiceAvailable = false;
	bool bVoiceProbed = false;
	// Situation detectors that need more than one event.
	int32 LastTouchTeam = -1;
	int64 LastTouchFrame = 0;
	int64 LastStallFrame = -1000000;
	int64 LastShotFrame = -1000000;
	int32 LastShotTeam = -1;
	int64 LastHumanTouchFrame = 0;
};
