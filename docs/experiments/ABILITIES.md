# Abilities on F (lane A84, R-0041 point 3)

The operator (R-0041): "Director's mode then can produce via boosts fun aspects like one-shot shotgun, binding special abilities to 'F' key." A cartridge names the
abilities under `game.abilities[]`; the human holds one at a time and uses it with **F** (gamepad **R3**, the right stick click); the pass moved to **Q** (held Q is the
saucer; gamepad X stays the pass); the deke keeps the mouse wheel and gamepad Y. Bots never hold an ability. A cartridge without the key plays and hashes exactly as
before (`Jam.Core.Abilities.None`, the Determinism hash below).

## The shape: `game.abilities[]`

A81's ask A12 (MODES-2.md) plus the brief's `trigger`. 0..4 items, unknown keys are errors, absent value = omit the key.

| key | values | absent |
|---|---|---|
| `kind` | `shotgun` \| `hip_check` \| `shield` \| `slow_puck` (required) | |
| `trigger` | `mode` (held from the first drop, comes back after its cooldown) \| `pickup` (a boost take grants one of the pickup entries) (required) | |
| `charges` | 1..9 uses, then the slot empties | unlimited for `mode`, 1 for `pickup` |
| `cooldown_frames` | 60..3600 frames between uses | 1200 (20 s) |
| `duration_frames` | 1..600 (shield, slow_puck) | 360 shield (6 s), 120 slow_puck (2 s) |
| `magnitude` | 0.1..4, the kind's strength | 1.5 shotgun (shot speed x), 1.5 hip_check (knockdown frames x), 0.5 slow_puck (speed share); shield ignores it |

Cross-field rules (validate.mjs and the C++ loader, parity cases `ability *` in gen-parity.mjs): at most one `mode` entry (F is one key); a `pickup` entry needs
`game.pickups` (else it can never arrive). A pickup grant goes to a human who holds nothing: the entry is picked from the pickup pool by the frame and the taker
(deterministic, off the seeded stream, so a cartridge without abilities draws the same numbers). Live config (Director's Call) holds a changed list back to the next match.
Files: `schema/game.schema.json`, `limits.json`, `DEFAULTS.md`, `README.md`, `validate.mjs`, `hockey.test.mjs`; `JamCartridge.h/.cpp` (`FJamAbilityConfig`, `FJamGameConfig::Abilities`).

## The four rules (JamSim.cpp, "Lane A84")

F (`FJamInput::bAbility`, an edge) is read before the shot in the step. A use emits `ability` (Who = holder, Kind = the kind, Strength 1); a grant emits it with
Strength 0. One effect at a time: F is refused while an armed kind waits or a shield runs, inside the cooldown and with no charges.

- **shotgun**: arms the next shot; with the puck on the blade the shot fires on the same frame. It leaves at `magnitude` x speed (puck.max_speed x the same factor),
  and on every puck substep every standing skater but the shooter within 1 m (`ShotgunPathM`) of the path goes down: the existing knockdown, Strength 1, cause
  `shotgun`. The puck passes through the bodies it dropped (a goalie in the crease too: "goes in through a crowd"). It ends after 120 frames, on another skater's touch,
  under 6 m/s or at a faceoff. Teammates on the path go down too (the operator's words: "every skater it passes within a metre").
- **hip_check**: arms the next check and opens the check window now. It lands from 0 m/s of the checker's own speed, Strength 1, `magnitude` x knockdown frames, and
  pushes the victim 3 m (`HipCheckPushM`: +6 m/s on the knocked-down glide, which halves every 0.5 s). The foul rules stand (a hip check on a skater without the puck is
  a foul when penalties are on).
- **shield**: `duration_frames` during which a check on the holder does not land (the bodies still bump, the bump read-out fires), a stick lift on the holder misses, and a
  shotgun passes the holder standing. Pokes still work.
- **slow_puck**: arms the next pass or shot. The puck leaves at `magnitude` of its speed for `duration_frames`, then wakes up (the speed divided back, capped at
  max_speed): a trick play. Another skater's touch ends it.

The state is per skater (`AbilitySlot`, `AbilityCharges`, `AbilityReadyFrame`, `AbilityArmed`, `ShieldFrames`, `AbilitiesUsed`) and on the puck (`ShotgunFrames/By/Mul`,
`SlowFrames/By/Mul`); `StateHash` reads it only while it left its default.

## Tests (headless, `Jam.Core.Abilities`, JamAbilityTests.cpp)

| test | what | measured |
|---|---|---|
| `None` | regular, raw, boost, Bruisers name no ability; F every third frame changes nothing | frame 600 hash the same with and without F (regular `f114b21f673e9303`) |
| `GrantUseCooldown` | mode grant to the human only; use, cooldown, back after it; charges empty the slot; a pickup take grants from the pool (a bot's grants nothing; nothing while holding; both entries over 40 frames); Restart; the two mode files load | pass |
| `Shotgun` | x = -20 with the puck, opponents on the line at 5, 10, 15 m (0, +0.6, -0.6 m off it) | plain: 17.85 m/s, stopped at x -15.6, nobody down; shotgun: 26.78 m/s, 3 of 3 down (3 `knockdown` cause shotgun), the puck reached x +2.5 |
| `HipCheck` | the human gliding 0.5 m/s into a standing opponent | plain check: no landing; hip check: strength 1, 135 frames down (90 x 1.5), the victim 2.89 m from where it stood |
| `Shield` | a 7 m/s check from the front on the carrier; a lift from 1 m | unshielded: down, lift clean; shielded: no knockdown, 1 bump, lift a miss; ends after 360 frames |
| `SlowPuck` | a shot up the empty ice | release 8.93 vs 17.85 m/s; frame 60 5.41 vs 10.82; wakes at frame 120: 3.32 -> 6.41 m/s |

Determinism: `Jam.Core.Determinism` before (main 8fc69d3) and after: hockey.regular `eb27841dda8fad3f`, hockey.raw `0e5637bb401646ae`, unchanged (no ability in either).
Schema: `node --test schema/*.test.mjs` 99/99 (a new abilities test); the three generators re-run (schema data, parity 87 cases, designer data 36 cuts).

## Input, net, HUD, bark

- `InputMapping.cpp`: `IA_Ability` on F and gamepad R3; the pass on Q (held = saucer) and gamepad X; the deke on the wheel and Y (Q left it). **Gamepad: R3, not the D-pad**:
  the four D-pad directions are the team signals (`SignalSystem`, `SignalTests`) and LT is the sprint; R3 was the only free button. `Jam.Input.Mapping` checks it.
- `HokejGameMode::ReadHumanInput` (and `LobbyGameMode`): the edge into `bAbility`; proof lines `JAMCTRL frame=<f> btn=ability kind=<held|none> (F / R3)` on the press and
  `JAMCTRL frame=<f> ability used kind=<k> charges=<n> ready=<frame>` when the sim used it. The pass line reads `(Q / X)`, the deke `(wheel / Y)`.
- Lockstep: `FJamNetQuantInput` flag `Ability = 2048` (an edge), protocol 7, `Jam.Net.InputCoverage` covers it.
- HUD (UI.md section 8): a plate above the radar frame (first right of the stamina bar: the commentary strip covered its F in the package still), "F  SHOTGUN  READY" / ARMED / SHIELD 4.2s / the cooldown seconds / "x2" charges, a 360 x 4 cooldown fill
  under it (a fill in the HUD's plate style instead of a ring); yellow while armed or a shield runs, grey cooling down, hidden when nothing is held.
- Commentary: `Game/Content/Commentary/abilities.commentary.json`, three lines per kind on a use (teen-rated: rough, not cruel), merged into the embedded bank
  (`node tools/commentary/embed.mjs`); the text shows in the strip, the voice waits for B's waves.
- Controls text: `CONTROLS.md`, the Director's Call card, the settings card, the HUD strip ("PASS  Q"). The proof drivers `tools/qa/ctrl/a77-prove.ps1` and
  `a82-prove.ps1` now press Q for the saucer and the pass; `a49-prove.ps1` still taps Q for its deke (now a pass): a wheel notch there if it is run again.

## The two modes (Game/Content/Cartridges/modes/)

| file | mode | keys changed from regular | level | character (tools/playtest/modes.mjs) |
|---|---|---|---|---|
| `hockey.mode.shotgun.cartridge.json` | Shotgun | `abilities` [shotgun, mode, cooldown 1200]; `look.tier` p3_clay | pro | knockdowns (all skaters) / match |
| `hockey.mode.arsenal.cartridge.json` | Arsenal | `pickups` speed (480 / 300 / 1.5, 2 on the ice), precision (720 / 480 / 2, 2 on the ice); `abilities` the four, all `pickup`; `look.tier` p3_clay | rookie | human abilities used / match |

The playtester presses F where a player would (`PlaytesterAbility` in JamPlaytester.cpp: the shotgun on a shot, the hip check on a check, the shield carrying with an
opponent within 4 m, the slow puck armed while carrying; and the shotgun also when an opponent stands within 1 m of the line ahead, the hip check on an opposing
carrier within 1.2 m); the playtest line gains `knockdowns_all` and `abilities_used`.

### Measured (`node tools/playtest/modes.mjs --modes shotgun,arsenal --seeds 1-20`, editor build, `tools/playtest/out/a84-modes-2.*`)

| mode | level | character | mode | regular | change | F uses / match | win % (regular) | human goals (regular) |
|---|---|---|---|---|---|---|---|---|
| Shotgun | pro | knockdowns (all skaters) / match | **6.95** | 5.05 | +38 % pass | 5.0 | 65 (70) | 1.75 (2.15) |
| Arsenal | rookie | human abilities used / match | **0.90** | 0 | new, pass | 0.9 | 100 (100) | 2.9 (2.9) |

Both 20 of 20 finished. Shotgun: the human is knocked down as often as in regular (1.50 vs 1.55 a match); the extra bodies on the ice are the shotgun's. Arsenal's
number is small because a first-to-3 rookie match lasts 2 minutes and the playtester does not chase boosts (a person would); a first pass with boosts every 600 / 900
frames gave 0.6. The first pass (10 seeds, the playtester's F only on its own shots) gave Shotgun +33 % and 4.1 uses.

**No menu card yet.** `Jam.Menu.DirectorsChoices` holds the Director's choices to twelve (two rows of six) and counts every mode file with `game.meta`; so both files ship
without `meta` and play by `-cut=modes/hockey.mode.shotgun` (or `LoadCut`). The words are ready for when the rows grow or two modes swap out:
Shotgun `{ "pitch": "F fires a shot that drops the crowd.", "facts": ["F: shotgun shot", "back every 20 s", "1.5x, knocks down"] }`,
Arsenal `{ "pitch": "Grab a boost, get a trick on F.", "facts": ["boosts give F tricks", "4 abilities", "rookie bots"] }`.

## In the package (Development, `tools/qa/ctrl/a84-prove.ps1`: drill 10 on the Shotgun cut, F posted to the game window on every JAMCUE shot and every 4 s)

Run `a84-shotgun-6` (`tools/qa/out/ctrl/a84-shotgun-6/game.log`), the shotgun fired with the puck on the blade:

```
JAMEVENT ability who=0 where=(-750.0,0.0,0.0) strength=0.000 frame=0 kind=shotgun                  (the mode's grant at the drop)
JAMCTRL frame=437 btn=ability kind=shotgun (F / R3)                                               (F reached ReadHumanInput)
JAMCTRL frame=438 ability used kind=shotgun charges=-1 ready=1638                                  (the sim used it: 20 s cooldown)
JAMEVENT ability who=0 where=(1446.8,202.7,0.0) strength=1.000 frame=438 kind=shotgun
JAMEVENT shot who=0 where=(1549.5,277.2,4.0) strength=0.900 frame=438 kind=none                   (27 m/s of max 30: 1.5 x 18)
JAMEVENT knockdown who=5 where=(3408.1,121.8,0.0) strength=1.000 frame=486 kind=shotgun           (a body on the path, 19 m on)
JAMCTRL frame=634 btn=ability kind=shotgun (F / R3)                                               (inside the cooldown: refused)
JAMSTILLS start kind=ability frame=2343 ...                                                        (the second use after 20 s)
```

The bark in run `a84-shotgun-2`: `JAMVO id=ability.shotgun.03 event=ability frame=453 dur=108 text="One blast and nobody is left standing."`; the binder:
`JAMFX missing /JamAssets/VFX/NS_ability.NS_ability (logged once, skipped)` (B's ask below). HUD stills (package, 1280 x 720):
`docs/experiments/img/a84/hud-shotgun-fired-f438.png` and `hud-shotgun-cooldown-f458.png` ("F  SHOTGUN  20s" above the radar),
`hud-first-placement-overlap-f452.png` (the first placement, right of the sprint bar, where the commentary line covered the F: moved and widened to 380).

Note: in runs 2 and 4 the F came while the drill held the blade at full reach (`JAMAIM reach=1.60`, outside `shot.reach_m` 1.5: R-0042, lane A85's fix), so the
shotgun armed and waited for a shot the reach allowed (a second F is refused while armed: one effect at a time). A85's single carrier reach removes that wait.

## Asks

- **For B (device B, JamAssets)**: the event `ability` (Who = holder, Kind = the kind, Strength 1 use / 0 grant) has no burst or sound yet (the binder logs one miss):
  `NS_ability` / `MS_ability`, ideally one per kind (`NS_ability_shotgun`: a muzzle-flash ring of clay chips at the blade; `NS_ability_hip_check`: a dust ring at the
  contact; `NS_ability_shield`: a translucent clay dome on the holder for its frames; `NS_ability_slow_puck`: a slow-motion ripple trail on the puck); the knockdown
  cause `shotgun` may want its own stronger `NS_knockdown` variant; the 12 commentary waves of `abilities.commentary.json` (ids `ability.<kind>.0N`).
- **For the Director**: the menu's rows (two cards to add, words above); `contracts/events.md` gains `ability` (proposed for C-0003, the same payload as above);
  A81's four modes (A12a..d: Sudden Death shotgun x1, Outnumbered hip check every 10 s, Pinball slow puck, Bruisers shield x3) are one key each now, left to A81 or
  the Director since those files are A81's and their measured numbers would move.

## Six more ability ideas (same shape, one rule each)

1. **dash**: a 0.5 s burst at 1.6x top speed through the stamina bar (the sprint without the bar), a body bump on the way. 
2. **magnet**: for 2 s a loose puck within 4 m curves to the holder's blade (a reception radius, not a steal from a carrier).
3. **ice patch**: drops a 4 m slow_ice zone behind the holder for 5 s (R-0019's zone rule, owned by the cartridge).
4. **long arm**: 3 s of `stick.reach_m` + 0.6 and `lift_range_m` x 2 (pokes and lifts from further).
5. **decoy**: the brain's defenders read the puck 1.5 m to the deke side for 1 s (the deke's goalie commit, on demand).
6. **rebound wall**: the boards behind the opposing net turn lively (restitution 0.95) for 4 s: a dump-in that comes back to the slot.
