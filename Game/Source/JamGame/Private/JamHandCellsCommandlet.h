// Lane A217 (track C0, the curve; the Director's ruling of 15 Nov 03:00: "the ladder is read through the person's path"). MEASUREMENT ONLY.
//
// The headless cell runner of the person's path: the game mode's match (AHokejGameMode::BuildMatch / GatherInputs / StepOnce, without the actors)
// with the human's skater played by lane A211's jam.ScriptHuman 2 (ScriptHuman2.h: the playtester decides, lane A209's hit hand presses the hits,
// LMB / RMB go through the button resolver with the shipped bindings, taps and holds by the thresholds, the pass's aim held through the release).
// Thousands of matches a process, not real time; nothing here runs in a package's path (a commandlet, -run=JamHandCells only).
//
//   UnrealEditor-Cmd ThirdJam.uproject -run=JamHandCells -cartridge=hockey.regular -difficulty=rookie -seeds=1-100 [-simseed=N] [-handseed=N]
//       [-maxframes=36000] [-buttons=0|1] [-hand=0|1] [-sprint=0|1] [-kind=-1|0|1|2] [-hashes] -nullrhi -Multiprocess -unattended -stdout
//
// Seeds: match s plays the sim on seed s (or -simseed, as the game plays the cartridge's own seed) and the hand on seed s (or -handseed); so
// "-simseed=1 -seeds=1-7" is the critic's seven-seed table (the hand seed 1 = the cartridge's seed 1 = jam.ScriptHumanSeed 0).
// One {"tool":"thirdjam.hand/1",...} line per match (LogJamHand); with -hashes a JAMHAND line every 600 frames with the state hash, which is
// the game's own JAMSIM hash (UJamSim::StateHashHex) for the same frame. Returns 0 ran, 2 bad arguments.
#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
// The end state hash of the last match the commandlet played (Jam.Curve.PersonPath reads it).
namespace JamHandCells { uint64 LastHash(); }
#include "JamHandCellsCommandlet.generated.h"


UCLASS()
class UJamHandCellsCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UJamHandCellsCommandlet();
	virtual int32 Main(const FString& Params) override;
};
