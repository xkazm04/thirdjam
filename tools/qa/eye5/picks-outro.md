## Categories: what was rejected, and what is weak in a picked frame

- **MODES with its preview** (the other choice for "many games in one"): no good frame. The screen itself is clean (a list of twelve names, a preview on
  the right), but the three previews compared with their modes show a game that is not played: Fog of War's is a clay hall with a black post on the puck,
  Glow Ice's a bright blue-white sheet, Pond's the indoor hall (fault F1). Glow Ice's goal is the page's fifth picture instead.
- **A save**: not hunted; the category "a save or a scramble at the net" is filled by the scramble (11).
- **SETTINGS, CONTROLS**: filmed, not picked. SETTINGS carries an API KEY entry field (it shows no key: "KEY: NONE"), which a page picture should not show;
  CONTROLS is the REBIND ANY KEY table, a screen of text.
- **Pro**: filmed (the hand loses 1 : 3), no pick: almost every pro frame carries the sprint bar across his ring (fault F5).
- Weak points in picked frames, said plainly: 04 has grey smoke puffs along the top and the banner stands over the net; 05 has a mate (42) between the
  camera and you, so your ring reads as his; 07 has an orange skater cut by the frame's bottom edge and touching the minimap; 09 has a sliver of a mate
  at the left edge; 12 has the puck's distance label over a downed skater far right; 03 and 12 are the instant after the hit, so the hitter's own ring is
  not drawn (it comes back about 40 frames later).

## Older stills in `submission/shots/` that are now untrue to the game (all twenty; none is deleted by this lane)

| Files | Why untrue against H43 |
|---|---|
| `01-menu.png`, `a155-01-first-screen-pro-lit.png` | the slate clapper menu; H43's first screen is HOME (the PLAY NOW plate, the level switch, SEASON / MODES / PLAY WITH A FRIEND); a155-01 has PRO lit |
| `02-directors-choices.png`, `a155-02-twelve-mode-cards.png` | mode cards; MODES is a list with one preview; 02 names modes that no longer exist under those names (Glass House, Pinball, Blind Side) |
| `a155-03-controls.png` | the fifteen-row CONTROLS table; H43's CONTROLS is the REBIND ANY KEY screen |
| `03-faceoff.png`, `a155-04-faceoff-clay-you-marker.png` | no stands, no crowd; 03 is Thursday's blue rink with the key strip; a155-04 has the black post on the puck and the PRO card |
| `04-body-check.png`, `05-knockdown.png`, `06-clay-and-net.png`, `07-goal.png` | Thursday's blue rink, an empty hall, the pill "ESC / START CALL READY", the burst at a hit, no GOAL banner, no controls panel |
| `08-directors-call.png`, `a155-11-pause-card.png` | the old HUD round the card (the pill, the key lines; a155-11 reads "PRO (THIS CUT'S OWN)"). H43's own pause card and Director's Call were not filmed by this lane |
| `a155-05-stop-and-go-drill21.png`, `a155-06-nudge-drill22.png`, `a155-07-hit-drill22.png` | drill text or the 8-second key strip on screen, an empty hall |
| `a155-08-board-hit-flash.png`, `a155-12-shotgun-F.png` | an empty hall; 12 has the black post and the small pill |
| `a155-09-goal-lamp.png`, `a155-10-end-card-win.png` | an empty hall; the goal has no banner; the end card has no stats |

## Faults seen while looking (what a judge would meet in the first minutes); sheet `docs/experiments/img/a226/faults.jpg`

Raw stills: `tools/qa/eye5/out/<run>/shots/` (`u1-<tier>-f<frame>.png`, or the still's name for `m-front`).

| # | Fault | Still (run, frame) |
|---|---|---|
| F1 | **MODES previews show another game.** Fog of War: a clay hall, clay figures, a black post on the puck (the mode plays on blue night ice with other figures). Glow Ice: a bright blue-white sheet (the mode is a dark sheet of light lines). Pond: the indoor hall with stands (the mode is outdoors in daylight). The other nine previews were not compared | `m-front` `fe-modes-3`, `fe-modes-4`, `fe-modes-7` against `f-fog` 180, `f-glow` 2560, `f-pond` 220 |
| F2 | **The goal's smoke covers the camera.** After a goal scored near the camera a grey wall fills a third of the frame (3 : 0) or all of it for about a second (2 : 0: the scorer is seen through grey) | `f-clay-rookie` 6492 to 6555 (worst 6510), 8348 to 8396 (8364) |
| F3 | **Glow Ice: sitting out shows nothing.** A body check is a foul there; for 8 seconds the camera stands at the box and the screen is a boost beacon and an empty sheet, the boxed figure behind the scoreboard. The hand met it twice in the first 40 seconds | `f-glow` 520 to 1000 (640), 1840 to 2320 |
| F4 | **The camera at the side boards.** It goes straight down and close: he is cut by the bottom edge, his rings are drawn over his own back, no rink in the picture | `f-clay-rookie` 4320 to 4380 (4380) |
| F5 | **The sprint bar reads as a bar under the skater.** An unlabelled grey bar 240 px wide at the bottom centre, while the legs are not full, lies across the lower edge of his ring. At pro it is on most of the time (it is `HUDWidget.cpp`'s stamina bar, not a hit mark) | `f-clay-pro` 6274, 6232 to 6328, 6448 to 6532 |
| F6 | **White streaks on the ice at a contact** on the clay rink too (PLAY-ME names them for the night look): a ring of about twenty white dashes for some nine frames. He asked for nothing drawn at a hit | `f-clay-rookie` 1053, 1872, 2178, 2544 |
| F7 | **The orange edge triangle on the controls panel**: on the BRAKE row and on the E keycap (a known limit; here is how it looks) | `f-clay-rookie` 2568, 2592, 3057 |
| F8 | **Glow Ice: the plate "speed 2.8s"** under the scoreboard, small and lower-case, reads as a debug line | `f-glow` 2440 to 2680 (2560) |
| F9 | A pill "ESC / START  RULE CHANGE READY" stands above the controls panel at the faceoffs of periods 2 and 3. Not a fault if meant; said because the brief counts "the small pill" among things gone | `f-clay-rookie` 684, 6582 |

Seen and not called a fault: the hitter's ring is not drawn from the contact for about 40 frames (03, 12); the GOAL banner stands over the net when the
scorer is far from the camera (04); in the pro match the hand holds the cross-check pose through the opponents' goal banner (`f-clay-pro` 5324 to 5440).

## How to shoot them again (the DLSS plugin, or any later package)

One command from this worktree, the game off screen, no window, no cursor:

```
powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\eye5\reshoot.ps1 -Package <...\Game\Packaged\Accept_HNN> -Tag hNN
```

It copies the Development package (robocopy, no link, without its Saved) to `tools/qa/eye5/pkg-<tag>/`, plays the six runs of `runs.json` only up to
the frames `picks.json` names, and writes `submission/shots/<tag>-NN-<what>.png`, `<tag>-contact.jpg` and `<TAG>-PICKS.md`. Tried on H43's own copy at 16:46: 4 minutes 29 seconds for the twelve (a first copy of a package adds about 10 s), all twelve found.
The same sim gives the same frames: in that trial the figures, the puck, the score and the camera of all twelve were the same as in the first shoot. What is NOT the same from run to run: the booth's line at the bottom, the bark bubble beside the scoreboard and the goal's smoke (they are not on the sim's seed), so a re-shot picture can carry another sentence (`h43r` did in eight of the twelve) and its quoted line must be read again from the frame. A package whose sim, brain or cartridges changed plays other matches: the frames then show other moments, and the
picks must be made again from whole films (`-Full`: about 3 000 stills, 9 GB, 20 minutes; then `sheet.py`, `events.py` and an eye).
A Shipping package cannot be filmed (no console) and is refused.

## Not filmed

The pause card and the Director's Call; SEASON, ALL RINKS, MAKE YOUR OWN, PLAY WITH A FRIEND; all-star; the large rink; nine of the twelve modes
(only Glow Ice, Fog of War and Pond); a pad's glyphs; a bot's cross-check near the camera; the Shipping package; any sound. Nothing was played by a
person: every match is the scripted hand's, one seed a mode. H43 has no DLSS plugin, so no picture shows the game with it.
