# Movement 6: Space is worth pressing (lane A152, track F2, evolution 5, 2026-11-14)

REVIEW-2 item 3 (R-0059 point 4) read, on the ladder's commit: a poke takes the puck 2.6 / 4.1 / 4.0 % of the time at rookie / pro / all_star and the human's
6.3 stumbles a match are his own missed pokes. This lane traced every press before changing anything, then built two rules behind keys and one switch on the
instrument, and measured each alone on 600 matches a cell. Status with timestamps: `status/A152.md`. Raw logs, sweep rows and edit scripts: `Game/logs/a152/`
(not landed); cells: `tools/perf/results/a152/` (`cells.jsonl` lands, `raw/` does not).

Code: `Game/Source/JamCore/Private/JamSim.cpp` (`PokeReaches`, `ResolvePokes`, two lines of the stick's part of `Step`, `FaceoffReset`, `StateHash`: every block
marked A152), `Public/JamSim.h` (one state field, one function), `Public/JamCartridge.h` + `Private/JamCartridge.cpp` (two keys, one instrument switch, one test
seam), `Public/JamBaseline.h` (the neutrals, one line), `AI/JamPlaytester.cpp/.h` (the scripted human's poke decision), tests
`Private/Tests/JamPokeTests.cpp` (new), `JamCarrierSkillTests.cpp` (two sweep keys), `schema/`.

## 1. How a poke works, and where the 24 in 25 go

A press opens a window of 8 frames (`stick.poke_window_frames`). It takes the puck on the first of those frames on which the poker's **blade** is within
0.47 m of the puck (`PokeTakeM` 0.35 + the puck's 0.12), the poker is in control, the puck is on the ice and `IsPokeShielded` does not refuse (the spin, the
body between, the take grace, the sweep's shield). A window that runs out next to an opposing carrier (within 3.0 m at the press) is a miss: 20 frames without
control (`stick.stumble_frames`), unless the carrier is in its take grace at that moment. The blade sits where the aim puts it: the cursor on the ice in the
game (`HokejGameMode.cpp:909`), and for the scripted human the direction to the puck **as it was `ReactionFrames` ago** (regular 6, good 3, novice 12): its
whole input, aim and press, reaches the sim that late.

`Bar.Movement.PokeTrace` (new, outside the gate) follows every press of the human in real matches and files it by what the eight frames allowed. A frame is
*takeable* when the carrier is not shielded against him and the puck is inside the stick's full reach of his body (2.07 m: what a perfectly aimed blade takes
from). Pro, the regular player, 100 matches (seeds 1 to 100), the tree as it came:

| What became of the press | Presses | Of all | Of the misses on an opposing carrier | Stumbled |
|---|---|---|---|---|
| took it from an opponent | 51 | 2.1 % | | |
| took a loose puck (the blade met a puck nobody carried) | 452 | 18.5 % | | |
| the faceoff's press (nothing near) | 413 | 16.9 % | | 1 |
| nobody carried any more (the carrier let go while the hand was on its way) | 350 | 14.3 % | | 58 |
| **never takeable: the take grace** stood through the window | 588 | 24.0 % | **49.8 %** | 66 |
| never takeable: the body between | 204 | 8.3 % | 17.3 % | 107 |
| **takeable, the blade was not on it** (the aim) | 175 | 7.2 % | **14.8 %** | 157 |
| never takeable: the puck left inside the window (a pass, a shot) | 109 | 4.5 % | 9.2 % | 77 |
| never takeable: the sweep's shield | 83 | 3.4 % | 7.0 % | 82 |
| never takeable: beyond the stick | 13 | 0.5 % | 1.1 % | 13 |
| the poker was hit inside the window | 6 | 0.2 % | 0.5 % | 1 |
| edge (the blade on an open puck on the trace's frame, no take) | 2 | 0.1 % | 0.2 % | 1 |

2,446 presses, 24.5 a match; the sim's poke events (what REVIEW-2 counted: clean takes and armed misses) 16.9 a match, 0.51 of them clean (3.0 %).
Of the 1,180 misses on an opposing carrier:

- **85 % were never takeable by any aim**: no frame of the window had an open puck inside reach.
- **About 60 % no player could have avoided** (the take grace 49.8 %, the puck passed or shot away 9.2 %, the poker hit 0.5 %): nobody sees the grace, and
  the pro carrier moves the puck under pressure inside the eight frames. The human arrives as the opponent wins the puck, so half his presses fall in the
  half second after the win.
- 24 % (the body between, the sweep) are the carrier's own defence: feet answer them, the stick cannot.
- **15 % were takeable and missed by the aim**: the closest the blade came was 1.05 m on average (the take is inside 0.47). The puck was within 30 / 60 / 90
  degrees of the aimed blade in 48 / 121 / 160 of the 175.
- His stumbles: 5.61 a match from his own missed pokes, 0.76 from anything else.

The brief's bar read on this trace (the carrier **not shielding and not in its grace as the player pressed**): 35 of 228 presses took the puck, **15.4 %**;
on every press at an opposing carrier 51 of 1,431, 3.6 %.

Sprinting carriers: pro bots sprint in 20 % of their carry frames, in 5 % of the frames within 3 m of the human, and **not one press in 100 matches met a
sprinting carrier** (0 of 1,431). The 25 % bar cannot be read in matches at pro; section 3 reads it in a staged pass-by.

## 2. What landed

| Item | What | Default | Neutral / off |
|---|---|---|---|
| 2 | **The poke reaches**: a person's open poke takes a carried puck inside its stick's reach and within 60 degrees of the aimed blade | `game.stick.poke_assist_deg` 60 | 0 (the blade rule for everybody) |
| 3 | **A miss costs a recovery, not a fall**: a person's missed poke locks its stick for 20 frames (no poke, lift or check); it skates on | `game.stick.miss_recovery_frames` 20 | 0 (the stumble of `stumble_frames`) |
| 4 | The scripted human pokes when a poke can land (regular and good: the aim leads its own reaction; no press into a grace or a body) | off: `stick.bPlaytesterPokeRead` false (the instrument, no key) | env `JAM_PLAYTESTER_POKE=read` for an arm |

State field added: `FJamSkaterState::StickLockLeft` (frames left of the recovery; a rule reads it, so it is hashed in `StateHash()` while above 0, behind
the tag 0x152A; `Jam.Net.HashCoverage` is green). The poke's and the lift's ready frames carry the lock too, so the brain's `bCanPoke` / `bCanLift` read it
with no change to the adapter.

**The poke reaches** (`UJamSim::PokeReaches`, called from `ResolvePokes`). For the skater a person steers (`bHuman`: the host's and a guest's), inside the open
window, a puck an opponent carries is taken when it is within the stick's full reach of the poker's body (0.4 + 1.2 + 0.12 + 0.35 = 2.07 m: exactly as far
as a perfectly aimed blade takes from, never further) and within `poke_assist_deg` of the direction the blade is aimed in. Everything else stands: the
poker in control, the puck on the ice, and `IsPokeShielded` (the body between, the sweep, the spin, the take grace) refuses it as it refuses the blade. On
such a take the blade is put on the puck (as far as the stick goes) so the figure's stick and the take agree. A loose puck is met by the blade as before.
**Bots keep the blade rule**: their aim's lag (`stick.sweep.bot_react_frames` by level) is the ladder's lever and the sweep's whole point, and law 3's drill
(a bot defends, the human carries) is untouched by construction.

**The recovery** (`ResolvePokes`, the stick's two lines in `Step`). A person's armed miss sets `StickLockLeft` to `miss_recovery_frames` where it set
`StumbleFrames`; while it runs no poke and no lift opens and a check pressed or still open is dropped; legs and steering are his. A miss the take grace
refused costs nothing (lane A121's rule); with `stumble_frames 0` (`hockey.raw`) a miss costs nothing of either kind. A bot's miss stays the stumble. The sim
does not tell a poke that hit a braced body or the boards from one that met air (a miss is a window running out), so the brief's "keep the stumble there"
has nothing to hang on: next evolutions.

Why the bots keep the stumble: with the recovery for every skater the good player at all_star lost 5.5 points (600 matches): the bots poke often there, and a
missed poke that trips the bot is what the carrier's sweep and stop earn.

## 3. The bars

| Bar (the brief) | Before | After (what ships) | Where |
|---|---|---|---|
| a human-class poke takes the puck in at least 15 % of presses at pro, the carrier not shielding and not in its grace | 35 of 228, 15.4 % | **131 of 228, 57.5 %** | `Bar.Movement.PokeTrace`, 100 matches |
| ... the same at rookie (novice) and all_star (good) | not traced at 100 | 95 of 251, 37.8 % / 72 of 102, 70.6 % | the same, 100 matches each |
| at least 25 % against a sprinting carrier | no press met one in 100 pro matches | **99 of the 99 passes he pressed in** (key 0: 0 of 99) | `Bar.Movement.PokeDrill`, the staged pass-by, 200 seeds |
| law 3's gate: a poke must not beat a clean stop begun at 8 m | `Jam.Core.Movement.StopAndGo` green | green, the rows unchanged (pro, the stop at 9.5 m: 194 of 200, as lane A147 read) | the full `Jam` filter |
| the human's own-poke stumbles under 1 a match at pro | 5.61 | **0.00** (4.59 recoveries); every stumble of his, all causes: 6.64 -> 0.81 on 600 | the trace; the cells' `stumbles` |
| poke events that take the puck (REVIEW-2's number: 2.6 / 4.1 / 4.0 %) | 2.4 / 3.0 / 3.4 % | **8.5 / 10.7 / 7.4 %** | the cells, 600 matches each |

The pass-by drill: a bot seat carries down the x axis at 10.1 m/s (skating, the puck 1.25 m out) or 10.9 m/s (sprinting, 1.65 m out), the human stands 0.8
to 2.4 m beside the lane and pokes as a person does, the aim and the press decided on what it sees and arriving late. In 99 of 200 passes the puck comes
inside the stick's reach and he presses.

| The hand | key 0 (the blade rule) | 45 | 60 | 90 |
|---|---|---|---|---|
| 6 frames late (the regular player), the carrier skating | 0 of 99 | 60 | **99** | 99 |
| 6 frames late, sprinting | 0 of 99 | 77 | **99** | 99 |
| 3 frames late (the good player), either | 0 of 99 | 99 | 99 | 99 |
| no lag, either | 99 of 99 | 99 | 99 | 99 |

The blade rule punished a tenth of a second of lag with a miss every time, and nothing else: the same hand with no lag takes every one. Law 4 in the gate
(`Jam.Core.Movement.PokeReach`, the last scene): head on, the sprinting carrier's puck rides 1.65 m from its body and is taken with the bodies 3.64 m apart;
the skating carrier's rides 1.25 m out and is taken at 3.21 m.

What the trace reads on what ships (pro, 100 matches): 2,505 presses; 195 took the puck from an opponent (was 51); the takeable misses fell from 175 to 47;
the grace still refuses about 590 presses a hundred matches: that is now the wall, and it is invisible (section 6, the first of the next evolutions).

## 4. The cells (600 a cell, seeds 1 to 600, paired; novice at rookie / regular at pro / good at all_star)

`tools/perf/results/a152/cells.mjs <arm> 600 <set|-> [ENV=v]`, three processes, `-Multiprocess`; rows in `cells.jsonl` by `pair.mjs base <arm>`.

| Arm | rookie | pro | all_star | Against the base | Ships |
|---|---|---|---|---|---|
| base (both keys 0, the old press) | 84.0 | 49.5 | 34.7 | | |
| `poke_assist_deg 60` alone | 83.0 | 51.5 | 34.3 | -1.0 / +2.0 / -0.4 | **on** |
| `poke_assist_deg 90` alone | 85.7 | 57.0 | 34.7 | +1.7 / **+7.5** / 0 | no: 60 |
| `miss_recovery_frames 20` alone, the person's only | 84.0 | 52.0 | 33.3 | 0 / +2.5 / -1.4 | **on** |
| the recovery for every skater (code switch) | 83.8 | 48.7 | 29.2 | -0.2 / -0.8 / **-5.5** | no |
| the scripted human's poke read alone | 84.0 | 51.3 | 31.2 | 0 / +1.8 / -3.5 | off (the instrument) |
| **what ships: 60 + 20** | **84.8** | **52.7** | **33.7** | **+0.8 / +3.2 / -1.0** | |
| what ships + the poke read | 84.8 | 51.5 | 31.7 | +0.8 / +2.0 / -3.0 | |
| 60 + the recovery for every skater | 85.2 | 50.7 | 28.0 | +1.2 / +1.2 / -6.7 | |
| the three with the recovery for every skater | 85.2 | 53.5 | 30.3 | +1.2 / +4.0 / -4.4 | |

Each rule alone is inside 4 points in every cell and so is the pair. Pro reads +3.2 with both: the curve track should know it is near the line. (This
commit's base reads 34.7 at all_star on seeds 1 to 600; the ladder's 30 was read on its own seeds.)

**The poke read stays off.** It is inside 4 points alone and on top of what ships, but it is the instrument every lane measures with: it takes pro's poke
events from 17 a match to 5.7 with 56.5 % of them clean (all_star 3.0 a match, 50.3 %), moves all_star by -2 to -3.5, and leaves the novice exactly as it
was (600 of 600 identical matches: the novice does not read). Switching it on is one line (`JamCartridge.h`, `bPlaytesterPokeRead`), for the curve track
when it next re-sets the ladder. What it shows without shipping: of REVIEW-2's "24 in 25", most was the script, not the game: on the rules as they came,
the read alone takes the puck in 44 % of its poke events at pro.

## 5. Neutral

- `Jam.Core.Movement.PokeNeutral` (in the gate): pro seeds 3 and 4, 7,200 frames, played three times in one run. The keys at their neutral values and
  the keys on with the lane's code closed by the test seam `stick.bPokeRulesOff` give the same hash (`e71f312a97a6ff65`, `77eba03e6deca846` on this tree;
  logged, not pinned); the keys on is another match. No constant in the file.
- At scale: the arm `n0` (both keys 0, on the final sim) is the base arm (run before the defaults moved) **hash for hash in 600 of 600 matches of each
  cell** (`same.mjs base n0`), and 12 pro matches of the tree before the rules were written equal the base's.
- `JamBaseline::MakeNeutral` carries the two keys and the switch (one line); `Jam.Core.MovementNeutral` is green with it. `Jam.Core.ContactNeutral` writes
  its neutrals by hand and needed this lane's line (finding, section 7).
- In a cartridge: `"stick": { "poke_assist_deg": 0, "miss_recovery_frames": 0 }`.

## 6. Tests

In the gate (`Jam` filter: 225 pass, 0 fail on the final tree; 222 before):
- `Jam.Core.Movement.PokeReach`: the defaults; aim 40 degrees off: key 0 misses, 60 takes on the press's frame; 100 degrees off: no; from 3.6 m: no; the
  body between refuses, and without the body shield the same poke takes; a bot's trailing aim misses; law 4's distances.
- `Jam.Core.Movement.PokeRecovery`: a bot's miss is still the stumble; key 0: the stumble of 20; key 20: no stumble, in control, lock 20, no poke, lift or
  check opens inside it, 6.0 m/s by its end, and the frame after it the poke and the check open.
- `Jam.Core.Movement.PokeNeutral`: section 5.
- `Jam.Core.Stick`: its miss block keeps the blade rule and the stumble by setting the two keys to 0.

Outside the gate (measurements, no assertion on a share): `Bar.Movement.PokeTrace` (env `JAM_POKE_TRACE="level=pro;player=regular;seeds=100;rows=0;set=stick.poke_assist_deg:0,..."`),
`Bar.Movement.PokeDrill`; `Bar.Movement.StopAndGoSweep` gained the keys `miss_recovery` and `poke_assist`.

## 7. Findings outside the family

- `Game/Source/JamCore/Private/Tests/JamContactTests.cpp:1789` (`Jam.Core.ContactNeutral`): pins two shipped-cut matches and writes every lane's neutral by
  hand (lines 1804 to 1809) instead of `JamBaseline::MakeNeutral`; it failed on this lane's defaults and got one line. It will fail on the next rule too.
- `Game/Source/JamCore/AI/JamPlaytester.cpp:211` to `:216`: the draw's press. 413 of the scripted human's 2,446 presses in 100 pro matches are the faceoff's,
  and the puck is already 7 m away when they land (the pro centre has won the draw before a ten-frame hand arrives): harmless (unarmed), but one in six
  of every count of his presses, and the draw is lost every time.
- `Game/Source/JamCore/Public/JamSkaterVisual.h:93`: `PlayPoke(false)` is documented as "a stumble follows": for a person it is now a recovery; the figure
  gets no `PlayStumble` (no `StumbleFrames`), and nothing shows the locked stick. A cue is device B's and the HUD's (the blade lowered or greyed for 20 frames).
- `Game/Source/JamCore/AI/JamPlaytester.h:154`: `stumbles` still mixes hits with missed pokes; after this lane it reads hits only for a person (0.18 / 0.81 /
  0.77 a match), which is what REVIEW-2 asked to split.
- `docs/PLAY-ME.md:53` says Space rarely takes the puck and a miss trips you: both sentences are out of date on this tree (the Director's file).
- Pro bots sprint with the puck in 20 % of their carry frames but in 5 % of the frames within 3 m of the human: law 4's "a sprinting carrier's puck is
  easier to take" never meets the human's stick in a match (0 presses of 1,431). The brain's (track Q2).

## 8. Next evolutions (ranked)

1. **The grace the player can see, or a press that waits for it.** Half of the misses on a carrier (about 590 presses in 100 pro matches) are the take
   grace, which nobody sees. Either a cue while it runs (U1 / device B: the new carrier's puck glows for the half second) or the rule: a person's poke
   pressed into a grace holds its window until the grace ends. Proof: the share of his presses the grace refuses from 24 % to under 8 % at pro; clean pokes a
   match from 1.8 to 3 or more; the three cells inside 4 (the grace-0 variant of REVIEW-2 read pro -14, so the cue first).
2. **The recovery the player can read, and the miss that hit something.** A read-out of the lock for the HUD and the figure (`StickLockLeft` is there), and
   the stumble kept where the poke met a braced body or the boards: the sim must first tell them apart (the blade's path against bodies and boards in
   `ResolvePokes`). Proof: an acceptance row for the locked stick; own-poke stumbles still under 1 a match; the cells inside 4.
3. **The instrument reads the poke** (the switch of item 4 on, with the faceoff's press removed), landed with the curve track's next re-set. Proof: pro's
   poke events 17 -> under 7 a match with over 50 % clean; the three cells re-set to 85 / 49 / 30 on 600 with it on.
