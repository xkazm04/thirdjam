> **Lane A142, 2026-11-13 night: this file is the record of the fallback video `hokej-v1.mp4` (the H24 build, three a side, no goal by the human), not of the game on main.** Its captions say what the take shows and nothing more. The film we want is `script-v2.md`. Truth row VS15 in `submission/TRUTH.md`.

# hokej-v1.mp4: cut list (A88, 2026-11-13 14:42; 39.5 s, 1920x1080, 30 fps, H.264 + silent AAC)

Built by `tools/media/cut.sh` from takes recorded by `tools/media/take.ps1` (PrintWindow capture piped to ffmpeg NVENC: a window in front of the game cannot spoil it).
Raw take .mp4 files are NOT in the repo (about 100 MB each): they sit in `%LOCALAPPDATA%\Temp\a88\takes-raw\` (menus-H24, dev5-check, long1-bruisers); run `TAKES=<that dir> bash tools/media/cut.sh`.
The logs of every take (inputs posted, second by second) are in `submission/video/takes/*/take.log`. No audio: no capture device exists on this machine (dshow lists only a virtual camera), the track is silent.

| # | s | take, in-point | what the viewer sees | caption |
|---|---|---|---|---|
| 1 | 0-2 | menus-H24 2.0 (H24 Shipping) | the film-slate menu, HOKEJ clapper | HOKEJ |
| 2 | 2-6 | menus-H24 5.0 (H24) | CHOOSE YOUR CUT with the twelve Director's choices cards | 12 MODES. YOU PICK THE PAIN. |
| 3 | 6-9.5 | menus-H24 14.0 (H24, Bruisers cut) | MATCH 1 card, faceoff ring DROP, controls strip | FACEOFF |
| 4 | 9.5-16.5 | dev5-check 55.0 (**Development N1015, drill 5**) | the human body-checks a carrier into the boards, burst, "Crunching check!" | BODY CHECK |
| 5 | 16.5-24.5 | dev5-check 90.0 (**Development N1015**) | a clay skater down on the ice, the human skating on | DOWN. GET UP ANGRY. |
| 6 | 24.5-30.5 | long1-bruisers 58.0 (H24) | the clay look at a red net, ink outline, spot pools | CLAY AND INK |
| 7 | 30.5-33.5 | menus-H24 28.5 (H24) | far goal, score 00 to 01, "Blue scores!" | BLUE SCORES |
| 8 | 33.5-37.5 | menus-H24 36.0 (H24) | TIME OUT then Director's Call sliders over the paused match | THE DIRECTOR CHANGES THE RULES |
| 9 | 37.5-39.5 | generated | end card HOKEJ, RAW ICE. CLAY BODIES. | |

## What the video does not have (measured)
- **A knockdown from the submitted build (H24).** Shipping has no console and no log; posted keys move the skater but the mouse cursor cannot be posted, so a blind skater never meets a bot (long1-bruisers, 190 s: no check). Rows 4 and 5 come from `Game/Packaged/Hokej_Hokej_N1015-dev` (a 10:09 Development package, the game's own `jam.ctrl.Drill 5`, hit-stop 3): its ice is darker than H24's look. Redo from the H-final package: needs A87's acceptance run (cursor and buttons) or a Shipping drill switch.
- **A cross-check, a goal by the human, 5 v 5, abilities on F**: not in H24's reach (R-0042 fault, A83 held, A84 not landed). Redo with H25.
- **Bruisers is 3 a side in H24** although the cartridge says 5 v 5 (the take shows three skaters a side).
- **Audio.** Add the game's music and the commentator in the Sunday cut (assets are in the ledger as ai-generated rows).
- Captions use no brand words; engine and plugin names appear nowhere on screen. The firewall dialog is in no take (PrintWindow).

## Shot list for the final cut (Sunday, from H-final)
Faceoff won, a carry, a pass received, a shot at three charges, a goal with the commentator line, a body check landing, a knockdown and the get-up, a cross-check, the Director's choices screen, one mode (Bruisers 5 v 5), an F ability, the result card. Re-run `take.ps1` per take, then edit the table in `tools/media/cut.sh`.
