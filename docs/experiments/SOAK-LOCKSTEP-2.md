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
