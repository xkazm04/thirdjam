// Lane A32 (B-to-A-0026): the commentator's line bank. B's manifest Game/Plugins/JamAssets/Content/Audio/VO/commentary.json (hokej.commentary/1,
// rules in briefs/commentary/README.md) is embedded at build time as CommentaryData.inl (node tools/commentary/embed.mjs), the way the bark bank
// is, so a packaged game carries it without a staging rule. This file is the strict loader: unknown keys, missing fields and wrong types are
// errors ("at <pointer>: ..."); the picker and the player live in CommentarySystem.h.
#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPath.h"
#include "JamCommentaryData.generated.h"

JAMGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogJamCommentary, Log, All);

// One line of the bank. Times are sim frames (60 Hz) computed from the manifest's seconds: nothing downstream reads a clock.
USTRUCT(BlueprintType)
struct JAMGAME_API FJamCommentaryLine
{
	GENERATED_BODY()
	// "goal.blue.01"; the situation is the id without its last number ("goal.blue"), the event its first segment ("goal").
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Commentary") FName Id;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Commentary") FName Situation;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Commentary") FName Event;
	// The payload Kind the line needs; NAME_None (an empty string in the manifest) matches any Kind.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Commentary") FName Kind;
	// "blue" | "orange" | "any".
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Commentary") FName Team;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Commentary") int32 Priority = 1;
	// cooldown_s * 60, rounded.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Commentary") int32 CooldownFrames = 0;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Commentary") FString When;
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Commentary") FString Text;
	// "/JamAssets/Audio/VO/VO_Commentator_goal_blue_01.VO_Commentator_goal_blue_01"
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Commentary") FSoftObjectPath Sound;
	// seconds * 60, rounded up: how long the line holds the booth.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Commentary") int32 DurationFrames = 60;
	// A stoppage line (events period, period_end, season_take, win, lose and the goal.pause situation): plays only while the sim is not in Playing.
	UPROPERTY(BlueprintReadOnly, Category = "Jam|Commentary") bool bStoppage = false;
};

namespace JamCommentary
{
	JAMGAME_API extern const FName TeamBlue;    // "blue"
	JAMGAME_API extern const FName TeamOrange;  // "orange"
	JAMGAME_API extern const FName TeamAny;     // "any"
	JAMGAME_API extern const FName Schema;      // "hokej.commentary/1"

	// The id without its last ".<nn>" (one situation = the alternates of one another).
	JAMGAME_API FName SituationOf(FName Id);
	// The first segment of the id.
	JAMGAME_API FName EventOf(FName Id);
	// period, period_end, season_take, win, lose (B's README section 3; goal.pause is a stoppage situation of a live event).
	JAMGAME_API bool IsStoppageEvent(FName Event);
}

// The parsed bank with an index by event and by id.
struct JAMGAME_API FJamCommentaryBank
{
	TArray<FJamCommentaryLine> Lines;

	// Strict parse of commentary.json. Errors are sentences "at <pointer>: ...". Lines with an error are dropped, the rest kept.
	bool Parse(const FString& Json, TArray<FString>& OutErrors);

	// Indices of the lines of one event (empty when none) and the index of one id (-1 when none).
	const TArray<int32>& ForEvent(FName Event) const;
	int32 Find(FName Id) const;
	// Distinct situations.
	int32 SituationCount() const { return Situations.Num(); }

	// The embedded copy (CommentaryData.inl), parsed once; and its raw text (tests compare it with the file on disk).
	static const FJamCommentaryBank& Embedded();
	static FString EmbeddedJson();
	// Lane A84 (R-0041 point 3): A's ability supplement (CommentaryAbilityData.inl from Game/Content/Commentary/abilities.commentary.json); Embedded() merges it.
	static FString AbilityJson();

private:
	TMap<FName, TArray<int32>> ByEvent;
	TMap<FName, int32> ById;
	TSet<FName> Situations;
};
