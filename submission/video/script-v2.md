# Video script v2 (lane A96, 2026-11-13 17:40): 48 s, for the capture lane on Saturday, needs H26

Rules: 1920x1080 or higher, 30 to 60 s, theme (Director's Cut) visible in the first seconds, no logos or trademark words (captions are ours; engine and vendor names appear nowhere on screen).
Voice for captions: `docs/experiments/VOICE.md`, taglines `submission/tagline.md`. Machine facts from A88: posted keys and mouse buttons reach the game window, the cursor cannot be posted
in a Shipping build, recording is PrintWindow-based (`tools/media/take.ps1`), no audio device on this machine (see the question at the end).

## What is staged how
- **Shipping H26** (the build we submit): no console, no log. The cut is chosen with `-cut=<absolute path of the cartridge>` (A88 loaded Bruisers this way); menus by posted keys; the match plays by itself when the human is idle (the bots are real).
- **Development H26** (a package of the same tree): `jam.ctrl.Drill N` stages a scene and logs `JAMEVENT`/`JAMCUE`/`JAMDRILL` lines (frame numbers); `tools/qa/ctrl/a73-prove.ps1 -Exe <Development ThirdJam.exe> -Drill N` (A73, tackling) and `a84-prove.ps1` (abilities) drive them with posted messages. The look is the same cooked content, but A88 measured the older Development ice darker than Shipping: **film the Development takes only where no Shipping route exists, and say so in the take log.**
- **Same seed, same frames, same state** (the determinism law): a moment found in a Development log by its frame can be re-filmed in Shipping by replaying the same cut with the same (idle) input and cutting at that time. This is the route for every "the bots do X" shot.

## Shots (48 s)

| # | s | what the viewer sees | caption | how it is staged | depends on |
|---|---|---|---|---|---|
| 1 | 0-3 | the film-slate menu, the HOKEJ clapper | HOKEJ. Clay skaters. Real hits. | Shipping, `menus` take as A88's (`take.ps1`, 2 s after launch) | nothing |
| 2 | 3-8 | Choose Your Cut: the twelve Director's choices as cards, a posted-key move across the rows | TWELVE CUTS. ONE DIRECTOR. | Shipping, posted Down/Right/Return as A88's `menus-H24` take; cards now Pond, Cage, Sniper, Marathon, Power Play, Shotgun / Sudden Death, Outnumbered, Glass House, Arsenal, Bruisers, Fog of War | the menu's SCENE 3 v 3 line fixed (A88 finding) |
| 3 | 8-13 | five blue and five orange at the faceoff, the drop, the first three seconds | FIVE A SIDE. NOBODY IS POLITE. | Shipping, `-cut=<Game>\Content\Cartridges\modes\hockey.mode.bruisers.cartridge.json` (5 v 5 on 80 x 40), idle human | the package really spawns five a side (H24 spawned three: A88 finding; re-check on H26) |
| 4 | 13-19 | a pass play by the bots: carrier skates, a teammate opens, the pass lands, a shot | THEY PASS NOW. | Shipping, `hockey.regular` or Bruisers, idle human, 90 s take, **cut the window**; find the window first in a Development run of the same cut and seed (`JAMEVENT pass` lines, `contracts/events.md`) and cut Shipping at the same time | A94 (landed on main bb18439) |
| 5 | 19-24 | **same direction**: two skaters glide the same way and touch: a bump, both keep their feet; split frame or cut straight to the next | SAME WAY: A BUMP. | Development, `jam.ctrl.Drill 5` (A73/A80: the human checks a carrier from the side at 3-6 m/s) via `a73-prove.ps1 -Drill 5`; after A92 this is a bump (impulse under 0.4) | **A92 landed in H26**; today a drill-5 hit still knocks down |
| 6 | 24-28 | **head-on**: the human carries at full speed into a standing pro bot, gets flattened, the burst of clay chips, the bot's "Crunching check!"-style line | HEAD-ON: FLATTENED. | Development, `jam.ctrl.Drill 6` via `a73-prove.ps1 -Drill 6` (A73: the human carries at the nearest standing pro bot at full speed and gets checked; 32 knockdowns in 140 s); keep hit-stop 3 | A92 (head-on is the biggest hit; the carrier cannot check) |
| 7 | 28-34 | **stop and go**: the human carries at a defender, S brakes hard (spray), the defender overshoots, the human bursts away | S TO STOP. THEN GO. | **No drill stages this yet.** Best route: the acceptance run's new brake step (A93 adds one to drill 20, `tools/qa/accept`) shows the brake alone; a defender overshooting needs a scripted pro bot chase: **write drill 21 (A93's lane or the capture lane): human at 9 m/s toward a standing pro bot at 12 m, brake at 5 m, burst at 3 m** (the same SetSkater/SetPuck staging as drill 6). Fallback: the operator plays 90 s while recorded and the best 6 s is cut | A93; a drill 21 that does not exist |
| 8 | 34-39 | **an ability**: the human fires F in the Shotgun cut: a shot that bowls over everyone in the lane, the bark "One blast and nobody is left standing." | F: SHOTGUN. | Development, `a84-prove.ps1` (drill 10 on `modes\hockey.mode.shotgun`, F posted on every JAMCUE shot); Shipping route: `-cut=...hockey.mode.shotgun.cartridge.json` plus a posted F and left-click while the human stands in front of the net (the shot goes where the cursor sits, not aimed: only if the Development take shows the same flight) | A84 landed; the NS_ability burst from device B is missing today (the binder logs one miss: ask B) |
| 9 | 39-43 | **the menu / the Director**: Esc, TIME OUT, the Director's Call sliders over the paused match, the rules changing | ONCE A PERIOD, THE DIRECTOR CHANGES THE RULES. | Shipping, posted Esc in a match (A88's `menus-H24` at 36.0 s) | the hint line clipped by the slate border (A88 finding) |
| 10 | 43-46 | a goal by the human, the horn, "Blue scores!" | BLUE SCORES. | Development `a73`-style drill 10 (`a91-prove.ps1 -Drill 10`, shots from 10 m, all three charges scored in the acceptance run) | A91 landed |
| 11 | 46-48 | end card: HOKEJ, the tagline | HOCKEY WITH THE WHISTLE TAKEN OUT. | generated (`tools/media/cut.sh` end card), B's logo if delivered | the logo and banner from device B (A-to-B-0019) |

Total 48 s; shots 5 to 7 are the film's point (momentum and the carrier's weapon) and are the ones that need the H26 sim, so they are filmed last on Saturday after the sim freeze (12:00) and the A95 curve pass.

## What to cut first if time runs out (in this order)
Shot 10 (goal), shot 2 (cards), shot 4 (pass play), shot 9 (Director). Keep 1, 3, 5, 6, 8, 11 (about 30 s).

## Edit
`tools/media/cut.sh` takes a table of `take, in-point, length, caption` rows (A88's v1 table is the template). Output `submission/video/hokej-v2.mp4`, 1920x1080, 30 fps, H.264 + AAC. v1 stays as the fallback until v2 is checked: no logo or trademark word on screen, theme visible in the first seconds (the caption of shot 2 and the slate say Director).

## Audio, and the question for device B
The take is silent: this machine has no capture device (dshow lists only a virtual camera). The game's own audio (music, the commentator, crowd, hits) is the half of the video that sells it.
**Question for device B (A-to-B-0020 point 5, still open):** can B capture the final 30 to 60 s of play with sound (the packaged game, system audio, 1080p at 30 fps) on Saturday evening or Sunday morning, from the H26 package? A sends this table and the staging commands; B only needs the package and the capture tool. If B cannot: fall back to A's silent cut with the commentator and music muxed in from the shipped wave files (they are ledger rows `ai-generated`, the voice service in `credits.md`; the audio is the game's own, so the claim "audio from the game" stays true if they are the cooked assets used in the take).
