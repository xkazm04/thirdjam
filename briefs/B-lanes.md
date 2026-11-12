# Device B lane board (the lead rewrites this file whenever a lane starts or reports)

Updated 2026-11-12 17:15 CET (H11.2). Two editors at most by the operator's decision. Directions in force: the operator to B at 14:55 (**hyped commentary; Blender and the P2, P3, P4
skaters**), and the operator's verdicts through device A at 14:40 (R-0021, `handoff/A-to-B-0007.md`: **props out of the arena; night sky, ice surface, construction, lighting; both hands on a hockey stick**). Reports land in `status/`.

## Running

| Lane | What | Editor | Started | Due | Report (when done) |
|---|---|---|---|---|---|
| SKF3 | Aim grid in Blender (5 angles x 3 reaches, blade on the ice in the blends), the broadcast figure's arms off the number and on the shaft, stride pelvis height; then a fourth retarget and an engine lane on port 8002 | none | 17:10 | 18:10 | `status/B-skater-skf.md` (third pass) |
| B14 | Look tiers on the real rink: clay tier pass 2 (clay at game distance, the clay figure, the arena without props in clay), then a first broadcast rig (`LR_P4_Broadcast`: arena light, no cel pass, the broadcast figure) so tomorrow's tier decision has stills of all three | port 8000, main tree | 17:00 | 19:30 | `status/B14-report.md` |

## Waiting for the operator's ears or eyes (pages on device B)

| Page | What to decide |
|---|---|
| `C:\Users\kazda\jam\audio-lab\ready\picks.html` | the picked effects and music in import-ready form: two boosted takes (clapperboard 05, puck post 01) and three loops to confirm |
| `C:Userskazdajamaudio-labox2listen.html` | the commentator bank (340 lines, voice C1): the first section asks whether the bank's delivery (normal capitals, speed 1.15) is right or the bank is regenerated as the picked v3 exactly; then spot-check, untick to reject |
| decision | **Goalie:** lane SKG built a goalie kit and 14 goalie clips for all three styles (`status/B-skater-skg.md`), but the rules say "no goalies" (`docs/HOCKEY.md`) and R-0013 point 3 says the last man acts as goalie with no goalie figure. The lead's brief was wrong about the rules. Options: (a) clips only on the last man, no kit; (b) the kit appears on the last man while he holds the crease; (c) a dedicated goalie per team (a rule change for device A). Nothing is imported until the operator chooses |
| stills | **Night sky:** open sky with stars (saved) or the dark volume (one-line switch): `devlog/shots/20261112-163901-B-b13-final-over.png` against `-final-variantA-over.png`. **Atmosphere** (haze and shafts, veto until 21:00): `devlog/shots/20261112-163703-B-b13-atmo-variant-wide.png` |
| decision | **Default figure:** the game still ships device A's cylinder figure. Stills of the two candidates in a match: `wt-vfxdevlogshots61112-170016-B-sk2-tb1-match.jpg` (scripted), `...-170013-B-sk2-tb2-carry.jpg` (hybrid). The lead proposes the scripted one as default |
| no page yet | no pick was made for the director's voice, the goal horn, the room tone and the menu theme |

## Queued (needs an editor slot)

| Lane | What | Waits for |
|---|---|---|
| P2b | Atmosphere: haze and faded shafts (lighting, may proceed by R-0021); the backdrop flats wait with the props | the operator's veto window (21:00); lane B13 prepares the stills |
| AUD | Import of the operator's picks (`status/B-audio-picks.md`) as `SFX_*` / `MUS_*`; MetaSounds layered or swapped; the rebuilt `MS_faceoff`; the commentary bank as sound waves with its manifest | the operator's confirmation on the ready page; lane VOX2; an editor slot |

## Done today (newest first; every one has a report under `status/`)

SK2 hockey sticks in two hands, the nine stick calls and the puck on the blade on all four figures; the rebuilt scripted figure is T-B1; numbers and crests on the clay and broadcast figures (`status/SK2-report.md`, handoff 0024) · B13 the arena without props: open night sky (dark-volume variant kept hidden), ice scuff zones, scratches and frost, kick plate, net grid, glass material, towers and stanchions, end-board fills; 218 actors, 12k triangles (`status/B13-report.md`, handoff 0023) · SKH3 third retarget from the fixed clips: skates within 1.3 cm of the ice in kneels, falls and get-ups (was up to 20 cm), stride pop gone, one-frame hand jumps 58 to 80 cm -> 31 to 38 cm; `skh/export3/` replaces the engine animations for all three generated figures (lane SK2 re-imports) · SKF2 solver fixed at the source: limb twist over 45 degrees per frame 93 to 99 times per style -> 0, foot pitch 81 -> 21 degrees, reach unchanged; exports in `skf/export2/` (`status/B-skater-skf.md`, second pass) · SKH2 second retarget from the scripted family: bottom hand closer to the shaft, poke overshoot gone, reach within 5 cm; found the twist flips in the source solver (`status/B-skater-skh.md`, second pass) · SKI the broadcast figure `SK_SkaterTB2_P4` and the clay figure `SK_SkaterTB2_P3` in the engine with 31 takes and a Blueprint each (`status/SKI-report.md`) · VOX3 the 48 hockey-first lines recorded again: bank 340 lines, 612 s, 56.0 MB · SKG goalie kit (nine rigid pieces per style, under 5k triangles) and 14 goalie clips, Blender only, not imported (`status/B-skater-skg.md`) · B12 picked images imported, jersey numbers and crests on both visuals, glyphs on tokens, fast-ice tile (`status/B12-report.md`) · SKF scripted skater family: three styles on one skeleton, 31 takes each, reach 1.000 / 1.600 m measured (`status/B-skater-skf.md`) · SKH generated-mesh skaters: broadcast pick P4B, clay pick P3A (`status/B-skater-skh.md`) · VOX2 commentator bank: 340 lines, 607 s, 55.6 MB of WAV, two lines over 2.5 s accepted (`status/B-audio-vox2.md`, in git when lane B12 frees the main tree) · B11 night rig re-laid for 90 x 45, six arena pieces and six pickup props (handoff 0019) · B9 clay tier pass 1 (handoff 0018) · B10 zone strips, pickup look, blade spray (handoff 0016) ·
COOK cook-only check: 540 packages, 0 errors (handoff 0015) · N1 Codex images batch 1 · S5 hybrid with 31 takes · VOX commentary first batch · N3 audio ready-making ·
N6 Rodin props batch 2 · N5 stick-verb clips · N2 commentary script · N4 dev log notes · B3b effects and sounds for the new events · B8 texture rules and skater tweaks ·
B1c GPU-hang hardening and 90 x 45 pool re-lay · AU1 audio lab · B6s skater polish and real-match preview · B2c stage light and atmosphere test ·
B4 scripted skater as T-B1 and the hybrid as T-B2 · S6 texture cleanup · S4 scripted original skater · B2b Rodin props in the arena · B1b look fixes on the real rink ·
V1 visual-upgrade plan · S3 Rodin route · S2 motion clips · S1 figure styles · B5 Meshy skater path · B3 puck kit, effects, sounds · B2 dressing pass 1 ·
B1 painted-cel look and night rig.

## What device A can sync on

- Assets: everything under `Game/Plugins/JamAssets/Content/` on `dev-b`, ledgered in `ASSETS.md` (389 rows; art 42.8 MB shipped, audio 4.1 MB, vfx/other 8.4 MB). **The night rig on `dev-b` is without props since lane B13's checkpoint (16:05): A-to-B-0007 point 1 is met; the look work on sky, ice, construction and lighting follows in the same lane.**
- **`main` 6a2681e builds on device B from a clean state: PASS in 137 s at 15:49** (closes B-to-A-0017 and 0020 on B's side). For A's UI: crests, glyphs and the digit atlas are under `/JamAssets/UI/` (`status/B12-report.md`, paths and atlas layout); `SetNumber` must be called for a jersey number to show.
- Open requests to A: `handoff/B-to-A-0013` (dev log notes), `0019` (pickup props to attach), `0021` (a skater class per look tier, `SetNumber` at spawn, UI textures), `0024` (what the pawn sent: no charged shots, a shot call with every pass, no goalie stance; the default figure), `0023` (floor lowering, glass material, stanchion scaling, rim strength), **`0022` (lane A22 duplicates B's Blender skater family: take B's, keep A22 on props)**. Answered by A in R-0022: `0012` (Variant table), `0014` (the stick calls, C-0001 v3), `0016`, `0018`; fixed by A: `0015`, `0017`, `0020`.
- Answered by B: `A-to-B-0001` done, `0002` accepted (images generated, the operator's picks in `status/B-image-picks.md`, import in lane B12), `0003` done,
  `0004` done (needs `bFinalUsesRDO=True` in A's config), `0005` parked by R-0020, `0006` accepted, `0007` done.
- Device B cannot package (the linker hangs under x64 emulation); it checks its assets with a cook-only run. Packaging stays device A's.
