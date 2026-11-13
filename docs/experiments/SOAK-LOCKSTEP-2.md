# The lockstep soak, second pass (lane A104, track Q1, 2026-11-13, protocol 8)

Lane A56's method (`SOAK-LOCKSTEP.md`) on the tree that carries the vision (contact by inertia, the movement laws, the hockey brain, abilities on F),
after an audit of every field added since protocol 5. Status with timestamps: `status/A104.md`.

## 1. The audit: what is hashed, what travels

### Inputs (the wire)
Protocol 7 already carried all 13 `FJamInput` fields (`Jam.Net.InputCoverage`): sprint 256 (level), spin 512 (edge), held pass 1024 (level), ability 2048 (edge).
What did not reach the wire was upstream of it:

| where | what | effect online | fixed |
|---|---|---|---|
| `ALobbyGameMode::ReadLocalInput` | the pass sent as the `bPass` edge, not the `bPassHeld` level (A77) | a human online could never saucer; every pass flat | yes: `bPassHeld = IsPassHeld()` as `AHokejGameMode::ReadHumanInput` |
| `ALobbyGameMode::ReadLocalInput` | no A93 back-pull latch | a held S read from the live camera circles instead of braking | yes: the same latch (`bBackYawHeld`, `BackYawRad`) |
| `ALobbyGameMode::ReadLocalInput` | no cursor-on-ice stick (A49 `CursorAimRink`) | online the stick follows the mouse delta, not the cursor | no: `CursorAimRink` is private to `AHokejGameMode` (JamGame); finding |
| `ALobbyGameMode::GatherBotInputs` | `bShootHeld` forced false; `bPassHeld`, `bSpin`, `bAbility` dropped | online bots never charged a shot (A31) nor saucered (A77) | yes: every field the brain sends, as `FJamSimAdapter::Step` |
| `AHokejGameMode::GatherInputs` (single player) | `bPassHeld` dropped (`HokejGameMode.cpp:912-922`) | bots never saucer in the game; headless runs (the curve) do | no: JamGame; a one-line finding |
| the soak harness (A56) | the playtesters' `bPassHeld`, `bSprint`, `bAbility` never copied; `Tester.bSprint` off; the bots' sprint and holds not copied | the soak never pressed sprint, the saucer hold or F | yes |

### State (the hash)
`UJamSim::StateHash()` covers the timers lanes A77 to A95 added, each behind its default (pass charge, sweep, spin, stamina, burst, abilities, shield,
the puck's height, the shotgun and slow puck). It leaves out six pieces of cross-frame state the rules read:

| field | read by | why it can let two peers differ unseen |
|---|---|---|
| `LastCarryFrame` | the foul grace, the lift's target, A92's carrier-cannot-hit | persists; two states with equal hashes can decide a foul differently |
| `bSprinting` | `TryLift`'s cross-check (`StabilityOf` on the victim) runs before `StepSkater` sets it: last frame's value | the stability of the next hit |
| `bStickAimed` | `TryDeke` runs before `StepStick`: last frame's value | the deke's side |
| `bPokeArmed` | the miss stumble when the poke window runs out | the stumble |
| `MoveHist[10]`, `MoveHistAt` | A93's first-step burst (the cut detector) | not even reflected: A56's field-by-field compare never saw them |
| `BotDrawFrame[2]` | the bot centres' draw while the draw is open | the draw's winner |

Read-outs stay out (never read by a rule): `YawRate`, `bBraking`, `bReverseBraking`, `BoardFrames` / `bBoardPinned`, `LastBumpFrame`, `KnockDir`, the
`Contact*` results and counters, the A32 commentary state (`Emit` never reaches a rule hook).

**Decision: `StateHash()` does not change.** Two tests pin whole-match `StateHash()` values of an older main (A92's and A93's neutral proofs,
`JamContactTests.cpp:579`, `JamMovementTests.cpp:631`, against bb18439); a changed `StateHash()` breaks those proofs and nobody can re-derive bb18439's
numbers. Instead `UJamSim::LockstepStateHash()` folds the six into `StateHash()`, and the checkpoint the peers exchange is `JamNetLockstepHash` (the sim's
lockstep hash, the effects', the rules' and the zones' hashes, FNV-1a over the four in a fixed order; protocol 7 xored the sim's and the effects' only, and
left the rules and zones out, A56's finding). The meaning of the checkpoint changed, so the protocol is **8**.

### Restart
`UJamSim::Restart()` (the in-game rematch) did not reset `Stamina`, `BurstLeft`, `MoveHist`, `PassChargeFrames`, `bSprinting`, `bStickAimed` or the A92 /
A93 counters: a rematch after a sprint was not the seed's match. Fixed; `Jam.Net.RestartIsInit` proves it.

## 2. Proof

**Gate** (`Automation RunTests Jam`, headless, the premerge invocation): 159 of 159 pass, exit 0. `Jam.Core.Determinism` before (main's premerge, 18:48)
and after: hockey.regular `d192973ac7c97102`, hockey.raw `b02f67a2da0314b4`, the seam test's `f3613b5fcd33ce32`: **unchanged**, as designed. A92's and
A93's neutral pins (`Jam.Core.ContactNeutral`, the movement neutral test) pass. New tests: `Jam.Net.LockstepHashCovers` (each of the six fields alone:
`StateHash()` blind, `LockstepStateHash()` and the checkpoint see it, put back = hash back; a read-out stays out; two sims on busy inputs agree 900 frames),
`Jam.Net.RestartIsInit` (500 frames of sprinting and holds, `Restart()`, 600 frames hash for hash against a fresh sim), `Jam.Net.RefuseMismatch`.

**Soak** (A56's harness, protocol 8, 3600 frames per match, both playtesters now sprint (A72 / A95), hold the pass for the saucer, brake by reversing
(A95's BrakeIntent), and the host presses F as `RunPlaytestMatch` does; the bots get every field the brain sends):

| run | matches | desync | silent drift | lost | edge presses lost | sprint frames | pass-held frames | F applied / abilities used | reverse brakes |
|---|---|---|---|---|---|---|---|---|---|
| clean, hockey.regular, seeds 1..1000 | 1000 | 0 | 0 | 0 | 0 of 220 k (spin 0/12 260, pass 0/5978, check 0/7423) | 634 351 | 10 152 | - | 405 975 |
| loss10 | 200 | 0 | 0 | 0 | 0 | 123 945 | 1932 | - | 81 243 |
| loss30 | 200 | 0 | 0 | 0 | 0 | 117 464 | 2170 | - | 82 772 |
| loss10 + jitter 0..6 + dup10 | 200 | 0 | 0 | 0 | 0 | 118 038 | 1570 | - | 82 786 |
| loss30 + jitter 0..6 + dup10 | 200 | 0 | 0 | 0 | 0 | 119 282 | 1242 | - | 82 177 |
| slow peer 30 Hz + loss10 + jitter | 200 | 0 | 0 | 0 | 0 | 123 775 | 1561 | - | 82 390 |
| blackout 2 s | 200 | 0 | 0 | 0 | 0 | 129 232 | 2204 | - | 82 332 |
| clean, modes/hockey.mode.shotgun | 300 | 0 | 0 | 0 | 0 (ability 0/1673) | 214 594 | 324 | 1673 / 532 | 50 443 |
| loss30 + jitter + dup10, shotgun | 300 | 0 | 0 | 0 | 0 (ability 0/1961) | 218 960 | 280 | 1961 / 597 | 50 449 |

Speeds equal A56's (0.999 clean, 0.877 at 30 % loss, 0.624 at the worst mix, 0.498 for the slow peer). "Silent drift" = a field differed while the
checkpoint did not: 0 everywhere. Cost on the clean link: Pump 1.96 us mean (p99 4.0), TryStep 0.14 us; the sim frame beside it (brain + step + zones +
effects) is now **9.6 us mean, p99 16.7** (A56: 5.2 us): the vision's brain and contact model, still 0.06 % of a frame (track Q3's number).
Only the host's human holds the mode's ability in this harness (the guest's seat is a bot slot in the sim, as in the lobby), so F is proven one way.

## 3. The REFUSE message (A56's proposal, built)
A HELLO with our magic and another version byte now gets a 6-byte `REFUSE` (magic, the host's version, type 6) from the host
(`UJamLobbySubsystem::TickLobby`, Hosting). Any build from protocol 8 on reads it whatever its own version (`JamNetReadRefuse` ignores the version byte;
`JamNetPeekType` never returns it) and fails at once with "the host runs a different build (protocol X, you have Y): update both". A protocol-7 or older
guest cannot read it and still waits 60 s: the message helps from the next mismatch on. Proven by `Jam.Net.RefuseMismatch` (bytes); not yet by two
processes (`tools/net/twoproc.ps1` with a patched version byte is the next proof).

## 4. Raw
`Game/Saved/SoakLockstep/a104/*.jsonl` (not landed); logs `Game/logs/a104/`, `Game/logs/soak-*`.

# Evolution 2: the online guest plays the whole game (lane A110, track Q1, protocol 8, wire unchanged)

Status with timestamps: `status/A110.md`. Tests: `Game/Source/JamNet/Private/Tests/JamNetGuestTests.cpp`.

## 1. The guest's seat is a human (`UJamSim::SeatGuest`)
The guest drives a skater the cartridge made a bot slot (`bHuman` false), and `bHuman` decides more than the ability:

| rule (JamSim.cpp) | the guest before | seated |
|---|---|---|
| the mode's ability at the drop (`Restart`) | none: F did nothing in Shotgun | granted, as the host's |
| a pickup's ability on a take (`NotifyPickupTaken`) | none | granted |
| the check window (`CheckWindow` 6 / 12) and the human's check-speed floor | the bot's | the human's |
| the move clamp (1.5 / 1.0) | the bot's 1.5 | 1.0 |
| the faceoff draw (`BotDrawFrame`) | a seeded bot press for the guest centre: the guest "won" draws it never played | its own press only |

`SeatGuest` marks the seat human (`Restart` never resets `bHuman`, so a rematch keeps it), grants the mode's ability if it holds none, and withdraws
the opening draw's bot press when the guest is the centre. `ALobbyGameMode::StartPlay` calls it on both peers at frame 0 with WELCOME's guest index;
single player never calls it, so `StateHash()` and the Determinism baseline do not move. `LockstepStateHash()` now folds `bHuman` (bit 8 of the flag
word): two peers that seated differently disagree at the first checkpoint. The wire is unchanged (the guest index already travels in WELCOME), so the
protocol stays **8**; protocol 8 is in no package yet (A104 and A110 land together; if A104 ships alone first, this needs 9).

The lobby's HUD never got the stamina bar or the ability slot (`AHokejGameMode::PushPresentation` fills them; `ALobbyGameMode::PushLockstepPresentation`
did not): online neither player saw F's slot. Filled for the local seat.

## 2. The guest's stick follows the cursor on the ice
`AHokejGameMode::CursorAimRink` (A49) and `EnsureMatchInputMode` are protected now, and the ice math is the public static `CursorAimFromRay`.
`ALobbyGameMode::ReadLocalInput` sets the match's cursor mode and reads the stick in single player's order (right stick, the cursor on the ice, the
mouse delta with its dead zone) through `ComposeLocalStick`. `Jam.Net.CursorAimParity` puts a cursor on the screen point of a known spot on the ice
(the engine's `FSceneView` projection of a third-person camera), and the lobby's stick equals single player's in five cases (full blade, part blade,
the dead zone, the hysteresis, the right stick winning), to the bit and on the wire (the quantised aim), pointing at the spot under the cursor.

## 3. The hash-coverage lint (`Jam.Net.HashCoverage`)
On a live, seated Shotgun match (abilities held, the burst ring live) every reflected field of `FJamSkaterState` and `FJamPuckState` is perturbed alone
(several values per type, behind its guard where `StateHash()` hashes it only while another field left its default) and must move
`LockstepStateHash()`, or be listed in the test's read-out table with its reason. A listed name that is no field any more fails too.
`MoveHist`, `MoveHistAt` and `bReverseBrakeLast` became `UPROPERTY`s, so every member of the two structs is reflected; a layout check fails on bytes
that belong to no `UPROPERTY` (an unreflected member). `UJamSim`'s own state is not reflected: its size is pinned (1736 bytes).
Today: 59 fields seen by the hash, 64 listed read-outs, **0 untagged**. Proven red in a scratch build (build 1): an unhashed `UPROPERTY` int32 was
reported untagged, an unreflected int64 showed as "11 bytes between bAbilityThisFrame and A110ScratchReflected belong to no UPROPERTY", and the
unset pin failed; both scratch fields were removed for build 2.
Limit: an unreflected member small enough to sit in padding the alignment leaves anyway (a bool after a bool) is not seen; the convention is that every
member of the two structs is a `UPROPERTY`.

## 4. Proof

**Gate** (`Automation RunTests Jam`, headless, `-nullrhi -Multiprocess`, build 3): **162 of 162** pass (A104's 159 plus the three new). The
Determinism hashes are unchanged: hockey.regular `d192973ac7c97102`, hockey.raw `b02f67a2da0314b4`, the seam test's `f3613b5fcd33ce32`, all equal to
main's before-hash. `Jam.Net.GuestSeat`: the seated guest holds the shotgun from the drop and uses F; unseated it holds nothing and F does nothing. A
pickup's ability goes to the seated guest's take in arsenal. Two seated sims agree for 1200 frames, and `Restart` keeps the seat and the grant.
`Jam.Net.CursorAimParity`: in 5 of 5 cases the lobby's stick equals single player's to the bit and on the wire, and it points exactly at the ray's
hit (the engine's round trip lands 0.9 to 1.5 cm from the spot the cursor was put on). `Jam.Net.HashCoverage`: 0 untagged.

**Soak** (A56's harness, protocol 8, 3600 frames per match, both peers seat the guest (`-SoakSeatGuest=0` gives back A104's bot seat), the guest
playtester presses F as the host's does). All twelve modes, clean and at 30 % loss + jitter 0..6 + 10 % dup, 200 seeds each (shotgun 300):
**5000 matches, 0 desync, 0 silent drift, 0 lost, 0 missed checkpoints, 0 edge presses lost.** Guest abilities used: **1428** (A104: 0, the
guest could not hold one). Table by `node tools/soak/lockstep/modes-table.mjs Game/Saved/SoakLockstep/a110/*.jsonl`:

| mode | profile | matches | desync | silent drift | lost | missed | edge presses lost | speed | F applied | abilities used (guest) |
|---|---|---|---|---|---|---|---|---|---|---|
| arsenal | clean | 200 | 0 | 0 | 0 | 0 | 0 | 0.9994 | 373 | 110 (65) |
| arsenal | loss30-jitter0-6-dup10 | 200 | 0 | 0 | 0 | 0 | 0 | 0.6244 | 492 | 98 (54) |
| bruisers | clean | 200 | 0 | 0 | 0 | 0 | 0 | 0.9994 | 0 | 0 (0) |
| bruisers | loss30-jitter0-6-dup10 | 200 | 0 | 0 | 0 | 0 | 0 | 0.6244 | 0 | 0 (0) |
| cage | clean | 200 | 0 | 0 | 0 | 0 | 0 | 0.9994 | 0 | 0 (0) |
| cage | loss30-jitter0-6-dup10 | 200 | 0 | 0 | 0 | 0 | 0 | 0.6244 | 0 | 0 (0) |
| fog-of-war | clean | 200 | 0 | 0 | 0 | 0 | 0 | 0.9994 | 0 | 0 (0) |
| fog-of-war | loss30-jitter0-6-dup10 | 200 | 0 | 0 | 0 | 0 | 0 | 0.6244 | 0 | 0 (0) |
| glass-house | clean | 200 | 0 | 0 | 0 | 0 | 0 | 0.9994 | 0 | 0 (0) |
| glass-house | loss30-jitter0-6-dup10 | 200 | 0 | 0 | 0 | 0 | 0 | 0.6244 | 0 | 0 (0) |
| marathon | clean | 200 | 0 | 0 | 0 | 0 | 0 | 0.9994 | 0 | 0 (0) |
| marathon | loss30-jitter0-6-dup10 | 200 | 0 | 0 | 0 | 0 | 0 | 0.6244 | 0 | 0 (0) |
| outnumbered | clean | 200 | 0 | 0 | 0 | 0 | 0 | 0.9994 | 0 | 0 (0) |
| outnumbered | loss30-jitter0-6-dup10 | 200 | 0 | 0 | 0 | 0 | 0 | 0.6244 | 0 | 0 (0) |
| pond | clean | 200 | 0 | 0 | 0 | 0 | 0 | 0.9994 | 0 | 0 (0) |
| pond | loss30-jitter0-6-dup10 | 200 | 0 | 0 | 0 | 0 | 0 | 0.6244 | 0 | 0 (0) |
| power-play | clean | 200 | 0 | 0 | 0 | 0 | 0 | 0.9994 | 0 | 0 (0) |
| power-play | loss30-jitter0-6-dup10 | 200 | 0 | 0 | 0 | 0 | 0 | 0.6244 | 0 | 0 (0) |
| shotgun | clean | 300 | 0 | 0 | 0 | 0 | 0 | 0.9994 | 3724 | 1169 (652) |
| shotgun | loss30-jitter0-6-dup10 | 300 | 0 | 0 | 0 | 0 | 0 | 0.6244 | 3828 | 1205 (657) |
| sniper | clean | 200 | 0 | 0 | 0 | 0 | 0 | 0.9994 | 0 | 0 (0) |
| sniper | loss30-jitter0-6-dup10 | 200 | 0 | 0 | 0 | 0 | 0 | 0.6244 | 0 | 0 (0) |
| sudden-death | clean | 200 | 0 | 0 | 0 | 0 | 0 | 0.9994 | 0 | 0 (0) |
| sudden-death | loss30-jitter0-6-dup10 | 200 | 0 | 0 | 0 | 0 | 0 | 0.6244 | 0 | 0 (0) |

The ten modes without an ability show "F applied 0": their cartridges grant none, so the harness never presses F. Speed is A56's and A104's, 0.9994
clean and 0.6244 at the worst mix. Raw output: `Game/Saved/SoakLockstep/a110/*.jsonl` (not landed).
