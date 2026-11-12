# Lane B3: puck readability kit, event effects and sounds (device B)

You are lane B3 on device B of the Third Jam (team Hokej, `GAME.md`). Gameplay on device A broadcasts events; you make what the player sees and
hears when they fire, and the kit that keeps a 12 cm near-black puck findable on dark ice.

Repo `C:/Users/kazda/jam/thirdjam`, branch `lane/b-vfx-audio`, own worktree. Read first: `contracts/events.md` (names and payloads: it wins over this
file), `contracts/assets-naming.md`, `tools/skills/look-painted-cel.md` (1 and 7), `docs/experiments/CAMERA-120M.md` (puck findings),
`tools/mcp/ANIMATION.md` (the Niagara-from-template calls that are proven), `briefs/B0-lead-plan.md`.
You own `Game/Plugins/JamAssets/Content/VFX/**`, `.../Audio/**` and the puck kit meshes under `.../Props/`. Machine rules as in `briefs/B1-look.md`; you drive
the editor only in your turn.

What the contracts and rulings fixed after this brief was first written (they win):
- Folders: `NS_<event>` in `/JamAssets/VFX/`, `MS_<event>` in `/JamAssets/Audio/` (C-0003, C-0004). R-0001 point 4 says `/JamAssets/FX/`: handoff B-to-A-0004 asks
  for one spelling; build in `VFX/` and expect a possible folder move.
- **No `BP_EventFx`**: the binder is A's C++ component `UJamEffectsBinder` in `JamGame`; it finds your assets by name. You deliver only the assets.
- The payload is `{ Who, Where, Strength, Kind }`; `Kind` is the pickup kind on `pickup_spawn` / `pickup_take` and the cause (`check` | `boost`) on `knockdown`.
- Until A answers 0004, expose on every Niagara system the user parameters `User.Strength` (float) and `User.Colour` (linear colour), and on every
  MetaSound a float input `Strength`; test them yourself by spawning the system from Python or a test Blueprint you do not commit.

## Part 1, first: the puck readability kit (due with the H6 row)

| Asset | What | Notes |
|---|---|---|
| `/JamAssets/Props/SM_PuckDisc` + `/JamAssets/Look/MI_Trim_PuckHalo` (B1 makes the instance) | flat glowing disc under the puck: diameter 60 cm, 4 mm thick, stencil 0, no shadow | accent colour that is neither team colour nor trim: start at yellow-green (0.75, 1.0, 0.15), Emissive 10 to 25; a white disc vanishes on lit ice (measured) |
| `NS_puck_trail` | ribbon behind the puck, width 10 cm, life 0.35 s, alpha by speed (off under 4 m/s), same accent colour | user parameter `Speed` (float) and `Colour`; A attaches it to the puck and feeds `Speed`. Deterministic gameplay never reads it |
| `MI_Toon_Puck` rim variant | optional thin band | measured as sub-pixel past 20 m: build it last or not at all |

A spawns and attaches these (the puck is A's actor): the lead files the handoff with the asset paths, the socket or offset, and the `Speed` parameter.
Verify on a stand-in puck in a scratch map: stills at 10, 24 and 40 m, on a lit pool and on the dark ice between pools.

## Part 2: one effect and one sound per event (M1 set by H24, the rest by H48)

`Strength` is the payload float (0 to 1 unless the contract says otherwise); effects scale with it. Sounds are **MetaSounds synthesised in the engine**
(oscillators, noise, envelopes, filters): original, a few kB each, no wave files unless noted. Nothing is downloaded.

| Event | By | `NS_<event>` | `MS_<event>` (how it is synthesised) |
|---|---|---|---|
| `faceoff` | M1 | none (the slate prop is B2's) | **clapperboard**: two wood clicks 40 ms apart (short band-passed noise bursts at 1.8 and 2.6 kHz, 15 ms decay) plus a low body thump; the theme's signature sound |
| `shot` | M1 | ice spray fan at the stick, 12 to 30 sprites by Strength, 0.25 s | stick slap: noise burst high-passed at 2 kHz, 30 ms, plus a 180 Hz thump; pitch and gain by Strength |
| `board` | M1 | small dust puff and a 0.15 s flash on the trim strip at `Where` | hollow knock: 90 Hz sine with 120 ms decay plus a rattle (band noise 400 to 900 Hz); gain by Strength |
| `goal` | M1 | burst in the scoring team's colour from the net, 1.2 s, plus trim strips pulsing | horn-like stack of detuned saws at 220/277/330 Hz with a slow attack, 1.5 s, then a film-reel flutter tail (amplitude-modulated noise at 24 Hz) |
| `win` | M1 | confetti in both trim colours over centre ice, 3 s | rising major arpeggio on a soft square, then the clapper once ("that's a wrap") |
| `lose` | M1 | none; the post volume desaturates (a look-tier parameter, ask the lead) | descending two-note figure, tape-stop pitch drop |
| `check` | M2 | impact star at `Where`, 8 sprites | body thud: 70 Hz sine, 80 ms, plus cloth noise |
| `knockdown` | M2 | ice scrape streak along the fall direction | long scrape: band noise sweeping 3 kHz to 800 Hz over 0.5 s |
| `pickup_spawn` | M3 | upward column of sparks, 0.6 s | soft chime, two sine partials |
| `pickup_take` | M3 | ring expanding from the skater in the pickup's colour | fast upward glissando |

Continuous sounds (not events; ask the lead to hand off the hooks): `MS_skate_loop` (band noise whose gain and filter follow skater speed, a short
scrape on sharp turns) and `MS_arena_room` (very low hum plus sparse lamp buzz, so the dark set is not silent). Music is **not** in this brief: its source
is an operator decision (`briefs/B0-lead-plan.md`).

Sound classes `SC_SFX`, `SC_Ambience`, `SC_Music` and one mix so the menu can carry a volume slider (A's UI); names go into the handoff.

## Acceptance

- [ ] Part 1 assets exist, with the stills; the handoff to A is written by the lead from your report.
- [ ] For every M1 event: `NS_<event>` (where the table has one) and `MS_<event>` exist, play from a test Blueprint that calls them with Strength 0.2 and
      1.0, and a 5-second capture of each sound is described in the report (peak level, length).
- [ ] No effect exceeds 200 live sprites; all event effects together add under 0.3 ms GPU on device B in the worst moment (goal).
- [ ] Every file has an `ASSETS.md` row (`kind` vfx or audio, `origin` original, tool "UE 5.9 Niagara / MetaSounds via MCP"); the audio slice stays under 80 MB.

## Report

PASS/FAIL per row, the MCP or Python calls that worked for MetaSound graph building (this was **not** proven before the jam: if graphs cannot be built
through MCP or Python in 30 minutes, stop and say so; the fallback is short waves rendered by a script, declared as original), numbers, open names.
Time box: part 1 one hour; part 2 M1 set three hours.
