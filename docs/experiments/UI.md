# Experiment: the UI art and typography pass (lane A18, 2026-11-12, from H7.3)

The operator's note (R-0013): "HUD elements and typography are ugly to look and read". This pass puts one style (`FJamUiStyle`, `Game/Source/JamGame/Public/MenuStyle.h`)
under every C++-drawn UMG screen and a Codex-generated slate kit (R-0008, `tools/images/ui-jobs.json`) behind it. Logic, feed structs and tests are untouched; every maker
falls back to a rounded-box brush of the same colour when a texture is missing, so no screen ever loses its text.

## 1. Type scale (Slate units = px at 1080p; the project's DPI curve draws 720p at 2/3)

| Role | Face (engine Roboto) | Size | Tracking (1/1000 em) | Where |
|---|---|---|---|---|
| Caption | Medium | 14 | 20 | captions under settings headings, key hints, own-net marker, ready pill |
| Body | Regular | 18 | 0 | descriptions, status lines, bark bubbles (Bold), cut rows |
| Label | Bold | 24 | 40 | buttons, section titles, far-net distance, scoreboard line (22) |
| Title | Black | 36 | 60 | card titles (the call card uses 28 to clear its megaphone doodle) |
| Display | Black | 64 | 60 | win/lose cards, CUT!; HOKEJ at 104, the countdown number at 96, the TAKE number at 110 |
| Score | BoldCondensed | 56 / 34 | 0 | the two digit plates / the timer |

Anything drawn over ice carries a 2 px ink outline (`FFontOutlineSettings(2, black 0.9)`) instead of a shadow: markers, the countdown, the TAKE card number, the controls strip.
Panel text keeps a 1 px shadow at 0.6.

Minimum sizes measured in the proof shots: primary HUD text is 18 px (controls strip) and 24 px (far-net distance) at 1080p, 12 and 16 px at 720p. The 14 px captions are secondary
(they sit on dark panels only) and go to 9 px at 720p; nothing the player must read in play is smaller than 18 px at 1080p.

## 2. Colours and contrast (WCAG relative luminance from the sRGB values)

| Text | Over | Ratio | Verdict |
|---|---|---|---|
| Paper #F4F4F0 | Ink #0E1014 (pills, strips) | 17.3 : 1 | AAA |
| Paper | Slate #1C1F26 (panels, cards) | 15.0 : 1 | AAA |
| Take-yellow #FFD966 | Slate | 12.1 : 1 | AAA |
| Muted #9AA0A8 | Slate | 6.2 : 1 | AA (captions) |
| Paper | Blue #1D4ED8 (home plate, hover plate) | 6.1 : 1 | AA |
| Paper | Orange #E8651A (away plate, pressed plate) | 3.0 : 1 | large text only: the digits are 56 px bold and outlined in ink, the pressed state is momentary |
| Ink | Paper (bark bubble) | 17.3 : 1 | AAA |
| Blue-ink #122E7A / orange-ink #7A330C | Paper (team-tinted bark text) | 11.3 : 1 / 8.3 : 1 | AAA |
| Paper + 2 px ink outline | white ice (about #DDDDDD) | outline vs ice 14 : 1 | the outline, not the fill, carries the edge; without it chalk on ice is 1.2 : 1 |

The darkest ice in the night rig is the black void above the boards (#000); everything over it is paper or yellow at 15 : 1 or better. The brightest is the spot pool (#E8E8E8);
only outlined text sits there.

## 3. The kit (Codex image generation, invented text only, no brand; `tools/images/ui-jobs.json`, all under 1024 px, BC7, no mips, UI group)

| Texture | Use | Job | Shipped |
|---|---|---|---|
| T_UI_Panel | 9-slice dark panel (scoreboard centre, radar frame, cards, menu card) | panel-9slice-v2 (v1 came back with a transparent centre) | y |
| T_UI_CallCard | Director's Call and pause card frame | call-card-frame, cropped and corner-masked with ffmpeg (the model drew a grey backdrop) | y |
| T_UI_CutFrame | film-strip frame around each cut row (tiled edges) | cut-card-frame | y |
| T_UI_Button_Normal / _Hover / _Pressed | button states (9-slice) | button-* | y |
| T_UI_Ring | faceoff countdown ring | countdown-ring | y |
| T_UI_TitleHokej | the slate: HOKEJ, no subtitle (R-0013 point 5) | title-hokej | y |
| T_UI_Icon_* (17) | cuts (6), pickups and effects (3), signals (4), settings, voice, barks, upscale | icon-* | y |
| score plate | two takes both came back with a hole where the digit goes; the code plate (team colour, ink edge) ships instead | score-plate, score-plate-v2 | n |
| bubble with tail | a good 2:1 image, but a tail in a corner tile cannot 9-slice at one-line text size (the band swallows the body); the code bubble (chalk box, team edge, 45-degree tail on the near side) ships | bubble-left | n |

Generation: 29 images in 32 minutes, one Codex run per image, 0.6 to 1.4 MB PNG each; downscaled with ffmpeg before import (icons 256, plates 512, frames 896 to 1024) so the
whole UI folder stays under 14 MB on disk (budget slice 40 MB). The A8 title slate (subtitle baked in) and the three unsliced icon sheets are removed and ledgered `n`.

## 4. What the HUD and the screens do now

- Scoreboard: two digit plates in the team colours with a dark panel between them (TAKE line in yellow over the timer), top centre; the Director's Call line is a pill under it;
  lane A14's active effects are a slot row (icon + "effect 4.2s") under that.
- Markers: take-yellow diamond with an ink edge and an outlined distance; the own net muted and small. Radar on a framed panel bottom right.
- Controls hint: a full-width bottom strip for 10 s, fading over the last 2 s. Faceoff countdown: the ring with the seconds, "DROP" in yellow for 30 frames at zero
  (reads lane A16's `FaceoffCountdownFrames` when the feed has it; `jam.ui.faceoff <frames>` previews it; `jam.ui.effects icon:label,...` previews the slots).
- Win/lose: a slate plate with a clapper stripe, THAT'S A WRAP in yellow or RETAKE? in paper, the restart hint under it.
- Barks: chalk bubbles with the team colour on the edge and the tail on the side nearest the screen centre, text tinted towards the team; the strip is a dark pill.
- Pause and call cards on the call-card frame; sliders restyled (muted bar, chalk thumb, yellow when focused).
- Menu: HOKEJ only; cut rows framed by the film strip with their icon; Settings in two columns with icons; Credits without the subtitle.

## 5. Errors and open points

- The first build after the edits broke the shared tree for about 14 minutes (13:28 to 13:42): `Pad` and `Tint` parameters shadowed `FJamUiStyle` members under warnings-as-errors,
  and the HUD's private members had not landed (a failed edit). Lane A16 patched the forwards to keep its build going; both are fixed and the 13:42 build is green.
- Links only succeed in the :00 / :30 windows while another lane's editor holds the DLLs (R-0009).
- The 9-slice panel must be drawn at a quarter of its pixels (FJamUiStyle::Panel sets ImageSize / 4): at full size its 113 px corner tiles turned a 60 px panel into a lump and clipped the TAKE line; the film-strip frame tiles at about 45 px per sprocket band whatever ImageSize says (Border draw type), so a framed cut row is 184 px with 54 px of padding.
- Live Coding through MCP failed at 14:05 while another lane's LookTierSwitcher.cpp did not compile; my units were verified with UBT -SingleFile and linked in the 14:30 window.
- The lobby (A9) does not read `FJamUiStyle`; the season screen (A17) is not a screen yet; both are styled when they call the makers.

## 6. R-0021 point 3: in-match copy is hockey (pass of 14:46 to 14:52)

Director's Cut lives in the menu and the Designer; on the ice the words are hockey. Changed (the widgets, timing and tests keep their shape):

| Where | Was | Is |
|---|---|---|
| Scoreboard line, `FormatTake` | TAKE n: first to m / TAKE n | PERIOD n: first to m / PERIOD n |
| End cards, `CardFor` | THAT'S A WRAP / RETAKE? | YOU WIN / REMATCH? |
| End hint, A17 `EndHint` | R  RETAKE / R  NEXT TAKE: x / SEASON WRAPPED | R  REMATCH / R  NEXT MATCH: x / SEASON COMPLETE |
| Pause card | CUT! / TAKE n / one change per take / this cut | TIME OUT / PERIOD n / one change per period / this match |
| Call card title and status | DIRECTOR'S CALL - TAKE n, used this take | DIRECTOR'S CALL - PERIOD n, used this period (the feature keeps its name) |
| Opening card (overlay) | TAKE n on the TAKE clapperboard, ACTION, DIRECTOR'S NOTE | MATCH n on the MATCH clapperboard (T_UI_MatchCard, job match-card), small type `OF m - PERIOD n - FACEOFF` in a season (count from `UJamProgression::GetSeasonState`, one call) or `PERIOD n - FACEOFF`, COACH'S NOTE for a season note |
| `GetTakeCardText` (MenuTests) | TAKE 1 | MATCH 1 |

Unchanged on purpose: the `period` event kind `take_n` (data contract C-0003), the Designer's "Director's Call: skater speed up (11 -> 12)" (already hockey), the menu, the season screen and the Designer copy (film framing stays), lane A12's bark lines (character speech, A12's data).
Tests updated with their owners' consent: CameraTests (FormatTake, CardFor), MenuTests (MATCH 1 card). `Progression.cpp` gained `#include "UObject/Package.h"`: once edited it leaves the unity blob and `GetTransientPackage()` needs the full type.
