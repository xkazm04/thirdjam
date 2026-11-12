// Lane A37 (R-0028 point 3): every shipped cut through the real sim with the systems the game mode wires (rules, zones, effects),
// the real brain at the cut's level and the scripted human. Driven by tools/cuts/sweep.mjs.
//   UnrealEditor-Cmd ThirdJam.uproject -run=JamCutSweep -cartridge=<path or preset id> -seeds=1-5 -maxframes=36000 [-difficulty=pro] [-mode=playerbot] -nullrhi -stdout
// Prints one thirdjam.tournament/1 JSON line per match (LogJamCutSweep) with a "sweep" object appended: level, hooks and fires, end_match /
// end_period actions, pickups spawned and taken (and by the human), effects started, zone enters, periods, announces, shots blocked,
// checks, knockdowns, fouls, rink revision, end cause (score | timer | rule | none), effect-system errors. A cartridge the loader rejects
// prints one line with "load_error" and the commandlet still returns 0 (the sweep's verdict is "loader rejects").
// Lane A41: the wiring moved into FJamSimAdapter (JamSimAdapter.h, FJamSystems); JamTournament, the playtester and the Jam.Bot tests run
// the same bundle now. This commandlet only adds the counters.
#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "JamCutSweepCommandlet.generated.h"

UCLASS()
class JAMCORE_API UJamCutSweepCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UJamCutSweepCommandlet();
	virtual int32 Main(const FString& Params) override;
};
