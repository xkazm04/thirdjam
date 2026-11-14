// Menu flow and cut text (lane A8). The one place that opens the match map with a chosen cut and returns to the menu, plus the
// pure helpers that turn the Designer's ListCuts() rows and cartridges into the film-slate's words (the Jam.Menu tests call them).
// Flow: Menu.umap (AJamMenuGameMode) -> StartMatch -> Rink.umap (AHokejGameMode reads the jam.Cut console variable) -> ReturnToMenu.
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "JamDesignerSubsystem.h"
#include "InputCoreTypes.h"
#include "MenuFlow.generated.h"

class UJamCartridge;
struct FJamSettings;

// One row of "Choose your cut".
USTRUCT(BlueprintType)
struct JAMGAME_API FJamMenuCutRow
{
	GENERATED_BODY()
	// The ListCuts() name: "hockey.regular" for a shipped cut, the saved name ("My Director's Cut") for a saved one.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Menu") FString Name;
	// "Regular Cut", "Large Rink Cut", "My Director's Cut".
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Menu") FString Display;
	// One line: rink, rules, what is special.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Menu") FString Description;
	// The key tunables: "speed 8 m/s   shot 18 m/s   ice 0.015   bounce 0.4".
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Menu") FString Tunables;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Menu") FString Path;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Menu") bool bSaved = false;
	// The sentence a saved cut was directed from (empty for shipped cuts).
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Menu") FString Sentence;
};

// Lane A67 (R-0036): one Director's choice, a mode cartridge in Content/Cartridges/modes/ chosen like a cut. The pitch and the three facts that
// differ from the Regular Cut are the menu's words, read from the cartridge's game.meta since lane A81 (the name is the envelope title);
// docs/experiments/MODES.md and MODES-2.md have the numbers behind them.
struct JAMGAME_API FJamModeCard
{
	// The cut name LoadCut / StartMatch take: "modes/hockey.mode.pond".
	FString Name;
	// The cartridge id: "hockey-mode-pond".
	FString Id;
	// "Pond".
	FString Display;
	// One line.
	FString Pitch;
	// Three facts against the Regular Cut.
	TArray<FString> Facts;
};

// Lane A144 (track U2): one row of the CONTROLS screen. Bound lists the keys of UJamInputMap's context the row speaks for: Jam.Menu.ControlsComplete
// holds the rows against the bindings in both directions (a key bound and not listed fails, a key listed and not bound fails). A row with no Bound
// keys is read outside that context (the team signals).
struct JAMGAME_API FJamControlRow
{
	FString Verb;
	// Mouse and keys.
	FString Keys;
	FString Pad;
	// What it does, one short line.
	FString Note;
	TArray<FKey> Bound;
};

UCLASS()
class JAMGAME_API UJamMenuFlow : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Lane A144: the first screen. FirstCut is what PLAY NOW starts (a first match, no choices); the two lines say what the game is.
	static const TCHAR* FirstCut;   // "hockey.regular"
	static FString WhatItIsLine();
	// Evolution 2: the brand line follows the first cut, read at run time: "Hockey with the whistle taken out." when its rules.penalties is false
	// (R-0054 point 5), the law itself while the cut still has a whistle. BrandLineFor is the rule; BrandLine reads the cut.
	static FString BrandLine();
	static FString BrandLineFor(bool bFirstCutHasWhistle);
	// Evolution 2: the level PLAY NOW plays: the player's own once picked in the settings, rookie on a first run.
	static FString FirstMatchLevel(const FJamSettings& Settings);
	// "ONE MATCH, ROOKIE.  ENTER / A, OR CLICK."
	static FString FirstMatchLine(const FString& Level);
	// The level a cartridge plays as the game's loader and JamAI::ResolveDifficulty read it: "rookie" | "pro" | "all_star".
	static FString CutLevel(const UJamCartridge& C);
	// The one line under a cut card's name: a shipped cut's first four facts ("120 x 60 m, pickups on the ice, first to 3, body checks"), a saved
	// cut's sentence in quotes. The card shows two lines of it.
	static FString CutCardLine(const FJamMenuCutRow& Row);
	// The CONTROLS screen's rows, in the order a hand learns them.
	static const TArray<FJamControlRow>& ControlRows();

	static const TCHAR* MenuMap;   // "/Game/Maps/Menu"
	static const TCHAR* RinkMap;   // "/Game/Maps/Rink"

	// Sets the jam.Cut console variable (AHokejGameMode reads it at InitGame when no -cut= argument is given) and opens the rink.
	// CutNameOrPath: a shipped name ("hockey.raw") or the absolute path of a saved cut's .cartridge.json.
	UFUNCTION(BlueprintCallable, Category = "Jam|Menu", meta = (WorldContext = "WorldContext"))
	static void StartMatch(UObject* WorldContext, const FString& CutNameOrPath);
	// Lane A144 (the level wire): the same, with the player's level ("rookie" | "pro" | "all_star") as the option ?level=<x> of the rink's open:
	// AHokejGameMode gives it to the brain in place of the cartridge's own. Empty Level = StartMatch (the cartridge's own level plays).
	static void StartMatchAtLevel(UObject* WorldContext, const FString& CutNameOrPath, const FString& Level);
	// THE RULE (Director, 14 Nov): the player's level wins over the cartridge's for the regular cuts: the shipped cuts of Content/Cartridges, which
	// is what PLAY NOW and the cut cards of CHOOSE YOUR CUT start. The cartridge's own level stays (this returns empty) for a Director's mode
	// (modes/...), a season take, a played take listed as a cut, and a saved Director's cut: their level is their design.
	static FString PlayerLevelFor(const FString& CutName, bool bSavedOrTake, const FJamSettings& Settings);

	// Unpauses and opens the menu map.
	UFUNCTION(BlueprintCallable, Category = "Jam|Menu", meta = (WorldContext = "WorldContext"))
	static void ReturnToMenu(UObject* WorldContext);

	// "hockey.regular" -> "Regular Cut", "hockey.arena.large" -> "Large Rink Cut" (R-0010); a saved cut keeps its own name.
	UFUNCTION(BlueprintPure, Category = "Jam|Menu")
	static FString DisplayNameFor(const FJamCutInfo& Cut);

	// One line: "Night Rink: 120 x 60 m, first to 3, body checks, 3 pickup kinds". Lane A144: bWithName false leaves the theme name out (a shipped
	// cut's row is headed by its own name; "Night Rink" over a cut that plays on clay told the player wrongly).
	static FString DescribeCut(const UJamCartridge& C, bool bWithName = true);
	// "speed 8 m/s   shot 18 m/s   ice 0.015   bounce 0.4"
	static FString KeyTunables(const UJamCartridge& C);
	// Loads every cut once to describe it; a cut that fails to load is listed with the error as its description.
	static TArray<FJamMenuCutRow> BuildCutRows(const TArray<FJamCutInfo>& Cuts);

	// The N biggest changes between two cartridge texts, largest relative change first, as "skater speed 8 -> 12".
	// Compares the numeric, boolean and string leaves under "game" plus theme.name.
	static TArray<FString> BiggestChanges(const FString& BaseJson, const FString& NewJson, int32 N = 3);

	// Tunable key or dotted game path -> words: "skater_speed" -> "skater speed", "ai_red_skill" -> "opponents' skill".
	static FString LabelFor(const FString& KeyOrPath);

	// Writes a cartridge text to Saved/Preview/directors-preview.cartridge.json (outside Saved/Cuts, so it is not listed as a
	// saved cut) and returns the path, or empty when it could not be written. "Play" without "Save" goes through here.
	static FString WritePreviewCut(const FString& Json);

	// Lane A67 / A81: the Director's choices in menu order, one per mode file that loads and carries game.meta (read once, then cached).
	static const TArray<FJamModeCard>& ModeCards();
	// The Director's choices whose cartridge file exists under Content/Cartridges/modes/.
	static TArray<FJamModeCard> AvailableModeCards();
	// The mode's display name for a cartridge id ("hockey-mode-pond" -> "Pond"); empty when the cartridge is not a mode.
	static FString ModeDisplayForId(const FString& CartridgeId);

	// Current cartridge JSON for the Designer's diff: the Designer's current cut, else the shipped regular cut.
	static FString BaseCartridgeJson(const UJamDesignerSubsystem* Designer);
};
