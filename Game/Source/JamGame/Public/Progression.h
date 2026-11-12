// The season of takes (lane A17, C-0005 section 1; design: docs/experiments/LADDER.md). A game-instance subsystem that loads the
// ladder (Content/Cartridges/ladder/season.ladder.json, tj.ladder/1, mirror of schema/ladder.mjs), keeps the player's results in
// Saved/Progress.json (tj.progress/1: unknown keys are errors; a missing or rejected file is the start of the season), decides what
// is unlocked, and starts takes through UJamMenuFlow::StartMatch.
// Seams: the menu's "PLAY THE SEASON" entry calls StartSeason; AHokejGameMode reports each finished match (RecordResult) and asks
// HandleRestart before its own restart (R = NEXT TAKE after a season win, RETAKE otherwise); A5's end card reads EndHint; A8's TAKE
// card reads ActiveNote; "Choose your cut" lists the played takes through AppendPlayedTakes. ReturnToMenu ends the season run.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "JamDesignerSubsystem.h"
#include "Progression.generated.h"

JAMGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogJamProgression, Log, All);

// One entry of game.ladder.takes (C-0005 section 4).
USTRUCT(BlueprintType)
struct JAMGAME_API FJamLadderTake
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Progression") FString Title;
	// Preset id: the file ladder/<Cartridge>.cartridge.json.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Progression") FString Cartridge;
	// The director's note on the TAKE card.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Progression") FString TwistText;
	// "always" (true) or "previous_won" (false).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Progression") bool bUnlockAlways = false;
};

USTRUCT(BlueprintType)
struct JAMGAME_API FJamLadder
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Progression") FString Id;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Progression") TArray<FJamLadderTake> Takes;
};

// What Saved/Progress.json keeps per played take.
USTRUCT(BlueprintType)
struct JAMGAME_API FJamTakeRecord
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Progression") FString Cartridge;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Progression") int32 Attempts = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Progression") int32 Wins = 0;
};

USTRUCT(BlueprintType)
struct JAMGAME_API FJamSeasonProgress
{
	GENERATED_BODY()
	// The ladder these results belong to; results of another ladder id are the start of a season.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Progression") FString LadderId;
	// Played takes only, in the order first played.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Progression") TArray<FJamTakeRecord> Takes;

	const FJamTakeRecord* Find(const FString& Cartridge) const { return Takes.FindByPredicate([&](const FJamTakeRecord& R) { return R.Cartridge == Cartridge; }); }
	bool Won(const FString& Cartridge) const { const FJamTakeRecord* R = Find(Cartridge); return R && R->Wins > 0; }
};

// One row per take, for the menu and the HUD.
USTRUCT(BlueprintType)
struct JAMGAME_API FJamSeasonTakeRow
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Progression") FString Title;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Progression") FString Note;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Progression") bool bUnlocked = false;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Progression") bool bWon = false;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Progression") int32 Attempts = 0;
};

USTRUCT(BlueprintType)
struct JAMGAME_API FJamSeasonState
{
	GENERATED_BODY()
	// False when the ladder file is missing or rejected (the menu then hides nothing but says why).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Progression") bool bLoaded = false;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Progression") FString Error;
	// 0-based index of the take "Play the season" starts.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Progression") int32 CurrentTake = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Progression") int32 TakeCount = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Progression") int32 TakesWon = 0;
	// Every take won.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Progression") bool bComplete = false;
	// 0-based take running now through the season (-1: no season match).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Progression") int32 ActiveTake = -1;
	// The menu's subline: "Take 3 of 8: Rough Cut", "Season wrapped: 8 of 8".
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Progression") FString MenuLine;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Progression") TArray<FJamSeasonTakeRow> Takes;
};

UENUM(BlueprintType)
enum class EJamTakeResult : uint8 { None, Won, Lost };

UCLASS()
class JAMGAME_API UJamProgression : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UJamProgression* Get(const UObject* WorldContext);

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// ---- pure rules (tests call them) ----
	// Parses a tj.ladder/1 file. Errors are sentences "at <pointer>: ...", the same pointers as schema/ladder.mjs.
	static bool ParseLadder(const FString& Text, FJamLadder& Out, TArray<FString>& OutErrors);
	// Parses tj.progress/1. Unknown keys, nulls and wrong types are errors (the caller then starts the season fresh).
	static bool ParseProgress(const FString& Text, FJamSeasonProgress& Out, TArray<FString>& OutErrors);
	static FString ProgressToJson(const FJamSeasonProgress& P);
	static bool IsUnlocked(const FJamLadder& L, const FJamSeasonProgress& P, int32 Index);
	// The first unlocked take not yet won; the last take when every take is won.
	static int32 CurrentTakeIndex(const FJamLadder& L, const FJamSeasonProgress& P);
	static void ApplyResult(FJamSeasonProgress& P, const FString& Cartridge, bool bWon);
	static FJamSeasonState BuildState(const FJamLadder& L, const FJamSeasonProgress& P, int32 ActiveTake);

	// ---- the season ----
	// (Re)reads the ladder and Saved/Progress.json. False when the ladder is missing or rejected.
	UFUNCTION(BlueprintCallable, Category = "Jam|Progression") bool Reload();
	UFUNCTION(BlueprintPure, Category = "Jam|Progression") FJamSeasonState GetSeasonState() const;
	UFUNCTION(BlueprintPure, Category = "Jam|Progression") const FJamLadder& GetLadder() const { return Ladder; }
	UFUNCTION(BlueprintPure, Category = "Jam|Progression") const FJamSeasonProgress& GetProgress() const { return Progress; }
	// Absolute path of take Index's cartridge (empty when out of range).
	FString TakePath(int32 Index) const;

	// "Play the season": starts the current take (resume), or the last one when the season is wrapped.
	UFUNCTION(BlueprintCallable, Category = "Jam|Progression", meta = (WorldContext = "WorldContext"))
	bool StartSeason(UObject* WorldContext);
	// Starts take Index when it is unlocked; opens the rink through UJamMenuFlow::StartMatch.
	UFUNCTION(BlueprintCallable, Category = "Jam|Progression", meta = (WorldContext = "WorldContext"))
	bool StartTake(UObject* WorldContext, int32 Index);
	// Marks take Index as the running season take without opening a map (tests, and StartTake).
	void BeginTake(int32 Index);
	// Leaves the season run (the menu, a cut chosen from the list). Results stay.
	UFUNCTION(BlueprintCallable, Category = "Jam|Progression") void EndSeasonRun();

	// True when CartridgePath is the running season take.
	bool IsSeasonMatch(const FString& CartridgePath) const;
	// The game mode reports a finished match once. Ignored unless CartridgePath is the running season take. Saves Progress.json.
	void RecordResult(const FString& CartridgePath, bool bWon);
	UFUNCTION(BlueprintPure, Category = "Jam|Progression") EJamTakeResult GetLastResult() const { return LastResult; }

	// The restart key at the end of a season match: after a win opens the next take (or the menu after the last), returns true;
	// otherwise returns false and the caller restarts the same cut (RETAKE).
	bool HandleRestart(UObject* WorldContext);
	// The end card's hint line for the running match ("R  NEXT TAKE      ESC  PAUSE"), empty when no season match is running.
	FString EndHint(const FString& CartridgePath) const;
	// The director's note of the running season take for this cartridge, empty otherwise.
	FString ActiveNote(const FString& CartridgePath) const;
	FString ActiveTitle(const FString& CartridgePath) const;

	// "Choose your cut": the played takes after the listed cuts, as saved-style rows ("Take 3: Rough Cut", path).
	TArray<FJamCutInfo> AppendPlayedTakes(const TArray<FJamCutInfo>& Cuts) const;

	// Tests: point the subsystem at other files (empty = the shipped defaults) and reload.
	void SetPathsForTest(const FString& InLadderPath, const FString& InProgressPath);
	static FString DefaultLadderPath();
	static FString DefaultProgressPath();

private:
	bool SaveProgress() const;

	FJamLadder Ladder;
	FJamSeasonProgress Progress;
	FString LadderPath;
	FString ProgressPath;
	FString LoadError;
	bool bLadderLoaded = false;
	int32 ActiveTake = -1;
	EJamTakeResult LastResult = EJamTakeResult::None;
};
