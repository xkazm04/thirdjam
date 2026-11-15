// Lane A211 (track E1, the eye; the Director's ruling of 15 Nov 01:12): jam.ScriptHuman 2, the measured hand in the rendered game. CAPTURE ONLY.
//
// The headless playtester (JamAI::FJamPlaytester, JamCore/AI/JamPlaytester.h: the hand every lane's numbers are measured with) decides the human's
// frame, lane A209's person-like hand (JamAI::FJamHitHand, JamHitHand.h) presses his hits, and the two-role presses (shoot / cross-check on LMB,
// pass / check on RMB) go through the same resolver a person's keys do (FJamButtonResolver with the shipped bindings and JamControls::ButtonContext):
// a press means what the possession of that frame makes it mean, a key held across a change of possession goes dead, a hold is a hold only after
// the hand's thresholds. So a film of jam.ScriptHuman 2 shows what the measured hand plays, through the buttons the operator has.
//
// Nothing in a package's default path reads it: AHokejGameMode::ScriptedHumanInput() calls Step() only while jam.ScriptHuman is 2 (values 0 and 1
// run the code they ran before, to the bit). The sim sees one FJamInput, as from any hand. Repeatable: no wall clock, no engine randomness; the
// playtester and the hand are seeded by jam.ScriptHumanSeed (0 = the cartridge's seed), so one build, cut, level and seed film the same match.
//
//   jam.ScriptHuman 2          this hand (1 = the old script, 0 = the keys)
//   jam.ScriptHumanKind N      -1 (default) by the level: rookie = the newcomer, pro = the regular player, all_star = the good player (the pairing
//                              the ladder is set with); 0 newcomer, 1 regular, 2 good
//   jam.ScriptHumanHand 0|1    1 (default) = lane A209's person hand presses the hits (reaction 12 to 18 frames, an aim error, early or late);
//                              0 = the playtester's own presses
//   jam.ScriptHumanButtons 0|1 1 (default) = the presses go through the button resolver; 0 = the playtester's verbs straight into the input
//                              (the headless match's path, for comparing the two)
//   jam.ScriptHumanSprint 0|1  1 (default) = the regular and the good player sprint (the playtester's -playersprint)
//   jam.ScriptHumanSeed N      0 (default) = the cartridge's seed
// JAMSCRIPT lines: one at the start of a match (kind, hand, seed), one every 600 frames (what the hand did since the start).
#pragma once

#include "CoreMinimal.h"
#include "JamSim.h"

namespace JamScriptHuman2
{
	// One sim frame's input for the human's skater. Sim = the state before this frame's step; Level = the brain's difficulty name; CutSeed = the
	// cartridge's seed. Call once per sim frame.
	FJamInput Step(const UJamSim& Sim, const FName& Level, uint64 CutSeed);
}

// The same, as a plain function: AHokejGameMode::ScriptedHumanInput() declares it at block scope (no include line in HokejGameMode.cpp).
FJamInput JamScriptHuman2Step(const UJamSim& Sim, const FName& Level, uint64 CutSeed);
