# Review and repair (track Q2): the adversary's read of today's fast landings

Lane A105, evolution 1 (Fri 13 Nov, R-0047). Family: the ability rule in `JamSim.cpp` (lane A84's functions), the brain (`JamAICore.*`, `JamSimAdapter.cpp`),
the commentary picker (`CommentarySystem.*`), the menu flow (`MenuFlow.*`, `MenuWidget.*`) and the cartridge loader (`JamCartridge.cpp`). Each fault that was
fixed has a test written before the fix, run red on the unfixed build and green on the fixed one.

Severity: **1** crash, hang, desync or NaN; **2** a wrong outcome a player meets in a normal match or mode; **3** an edge a player rarely meets (or only a Designer
cut can reach); **4** cosmetic or latent.

## Fixed (severity 2)

| # | fault | where | test (red before, green after) |
|---|---|---|---|
| F1 | **Slow puck retaken**: the shooter who skated after its own slow puck and carried it again had it thrown off the blade at the wake-up (speed divided by the magnitude: twice the carrier's speed, out of the carry assist's 4 m/s). Its next plain shot or pass inside the window was doubled in mid-flight. | `StepSlowPuck`, `ApplySlowPuck` | `Jam.Core.Abilities.SlowPuckRetaken` |
| F2 | **Commentary silent after a retake**: `RestartMatch` (R, the Director's Call restart) puts the sim at frame 0 with the same cartridge object, so the picker was never reset; the booth's end frame, the gate, every cooldown and the knockdown ring sat in the new match's future (a line that ended at frame 20,180 held the booth for 5.6 minutes of the retake). | `FJamCommentaryPicker::NoteFrame` (new), called by `Observe`, `ObserveBark`, `Resolve`, `Force` | `Jam.Commentary.Rewind` |
| F3 | **The brain did not see the penalty box**: `BuildWorld` mapped only the knockdown into `bDown`, so a boxed skater (pinned at `BoxPos`, on the ice by the bottom boards) counted as a presser, a cover, a pass outlet, the loose puck's owner, and as pressure on a carrier passing the box. hockey.regular plays with penalties. | `JamSimAdapter.cpp` `BuildWorld` | `Jam.Bot.Hockey.BoxedIsOffIce` |
| F4 | **Hip check with body checks off** (a Designer cut, no shipped mode): F armed it and spent the charge; it could never land (both landing sites sit behind `check_enabled`), so the slot stayed held and a pickup slot never emptied: no further pickup ability for the match. Now F refuses it there and a pickup draw passes it over. | `TryAbility`, `NotifyPickupTaken` | `Jam.Core.Abilities.HipCheckNoChecks` |

## Reviewed and sound

- An ability while stumbling, down or in the box: `TryAbility` sits behind `CanAct()`. During a faceoff or a goal pause: only the playing step reads `bAbility`.
- Shotgun and slow puck at a faceoff: `FaceoffReset` clears both. The shield's clock runs only while the puck is live.
- Hip check against a braced skater: an armed hip check is a knockdown whatever the stability (ABILITIES.md: "lands from 0 m/s"); by design.
- A pickup at 5 v 5: granted per human, only with an empty slot; a live change of `game.abilities` waits for the next match.
- The brain: the lift cooldown and every history across a faceoff and a restart (`ResetBrain`); `jam.livecut` (`ApplyLiveConfig` keeps the skaters); teams of
  0 or 1 skaters; no NaN path (every normalisation and division guarded). A roster over 10 cannot load (schema: 2 teams of at most 5).
- The loader: `game.meta` limits (pitch 48 characters, 1 to 3 facts of 20) and unknown keys through the schema walk; one `mode` entry; a pickup needs pickups;
  magnitude 0.1..4. The menu: a file named in the order table and missing is skipped; cards bind by name, never by index.

## Listed for the next evolution (not fixed)

| sev | finding | where |
|---|---|---|
| 2 | A knocked-down human still makes its teammates yield the loose puck to it (the yield reads the human's distance, not `bDown`) | `JamAICore.cpp` `Decide`, `bYield` |
| 3 | The loose puck's owner is the skater nearest the puck now, not where it will be: on a 15 m pass the passer owns it for half the flight and chases its own pass | `JamAICore.cpp` `AssignJobs`, Puck job |
| 3 | A slow pass by a carrier skating its way is taken straight back by the carry assist (half speed is within 4 m/s of the carrier) | `TryPass` + carry assist (`StepPuck`) |
| 3 | The slow puck halves only the horizontal speed: a lifted slow shot or a slow saucer lands at about half its distance (VZ untouched) | `ApplySlowPuck` |
| 3 | An armed hip check on a carrier cannot be re-opened (the check press is refused for the carrier); it waits until the puck is lost | `TryAbility` / the check press |
| 3 | `jam.livecut` and a Designer live apply reset the whole commentary picker mid-match (history lost, a playing voice overlapped) | `CommentarySystem.cpp` `RefreshSettings` |
| 3 | An unreadable mode file drops its card with no log line; the list is cached per process | `MenuFlow.cpp` (the mode list) |
| 3 | Brain: carrier and holder ignore the puck's height and the release (3 to 6 frames of "still carrying" after a pass) | `JamSimAdapter.cpp` `BuildWorld` |
| 3 | A 3 v 3 cut with its chaser boxed has no replacement presser (now that the box is seen) | `JamAICore.cpp` roles |
| 4 | `PassHold[]` is not cleared on the frozen and down path | `JamAICore.cpp` |
| 4 | `ValidateTree` returns `OutErrors.Num() == 0` (a pre-filled list reads as failure) | `JamCartridge.cpp` |
| 4 | Slow puck magnitude over 1 speeds the puck up and its wake-up slows it; shotgun magnitude under 1 ends at once under 6 m/s | ABILITIES.md ranges |

## Proof

Build 1 (the four tests in, no fix) and build 2 (the fixes), editor Development, headless (`-nullrhi`), logs under `Game/logs/a105/` (not landed).

| test | before (no fix) | after |
|---|---|---|
| `Jam.Core.Abilities.SlowPuckRetaken` | Fail: the retaken puck not carried throughout, worst puck-to-carrier speed difference 10.12 m/s; the second plain shot's worst frame-to-frame speed ratio 1.98 (doubled in flight) | Success |
| `Jam.Commentary.Rewind` | Fail: at frame 30 of the retake the shot line is skipped as "playing" (booth end 20,180); the old match's ring still holds 2 knockdowns | Success |
| `Jam.Bot.Hockey.BoxedIsOffIce` | Fail: the checker boxed for 300 frames, the brain's `bDown` 0 | Success |
| `Jam.Core.Abilities.HipCheckNoChecks` | Fail: armed, charges 2 -> 1, uses 1 | Success |
| the full `Jam` filter | (main's premerge run at 18:46: 156 of 156) | 160 of 160 |

Hashes: of the 37 match lines with a hash in the `Jam` run, 27 are identical to main's run (every `JAMCONTACT` line, `Jam.Core.MovementNeutral`'s pinned
`874f67830b20dc4f` and `59762ff73c73f6e3`, boost seeds 150 and 251, regular all_star 387). The 10 that changed are bot matches on cuts with penalties on, which
F3 changes on purpose (a boxed skater left the brain's ice): for example hockey.regular rookie seed 349, 18,345 frames 2:3 before, 25,818 frames 3:2 after.
F1, F2 and F4 reach no cartridge without abilities and no sim state outside a slow puck, so they change no hash of a shipped non-ability cut.

The ladder (R-0046, R-0047 point 4) with F3 in: `tools/perf/results/a95/curve.mjs`, hockey.regular, 600 matches each, the profiles as built: novice at rookie
**84.5 %** (81.4..87.2; R-0046 85.3), regular at pro **47.2 %** (43.2..51.2; 48.3), good at all_star **25.7 %** (22.3..29.3; 26.3). Every cell inside 1.1 points.

Revert paths: F3 is the one line in `JamSimAdapter.cpp` `BuildWorld` (`O.bDown = K.IsDown();`); F1, F2 and F4 are the blocks marked "Lane A105". F1 and F4
act only in a cartridge with `game.abilities`; F2 is presentation only.
