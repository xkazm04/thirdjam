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

## 7. HUD pass 2 (lane A42, from H12.1): which skater is yours, how a faceoff is won, one bark at a time, the own-net marker

Lane A27's eyes pass (EYES.md) read all five of A18's HUD stills the same way: nothing says which skater is yours, nothing says how the faceoff is won, the bark is drawn twice
(bubble and strip), the own-net marker sits on the screen edge. The bake-off strips (FIGURE-BAKEOFF.md) added the bubble over the `MATCH 1` banner. All presentation:
`HUDWidget.*`, `BarkWidget.*`, one band helper on the overlay (`DirectorsCallWidget.h`); the feed, the sim and the state hash are untouched.

| Gap | Before | After |
|---|---|---|
| Yours | the human's figure is one blue skater among three | a ring under the human's feet from the kit's chalk ring (T_UI_Ring) tinted the team colour, 1.1 m on the ice projected to a screen ellipse (four points of the circle give the box); from 20 m it fades and a team-colour chevron over the helmet fades in (gone at 24 m); a YOU tag (Label 22 Black, paper on a team plate, ink edge) over the helmet (1.9 m) the first 300 frames of a match and 120 frames after every drop |
| Carrier | the puck glow only | a thin paper ring (0.8 m) under the carrier when it is not you (`UJamSim::CarrierIndex`, read only; loose puck = no ring) |
| Faceoff | the ring counts 3, 2, 1 then DROP | the digit (72 Black) over **HOLD** (Label, yellow, outlined) while it counts; **DROP** at zero as before; then **WON** or **LOST** in the human's team colour for 40 frames (the bus's `faceoff` `won` event: Who's team against the human's); a one-line pill under the ring the first three faceoffs of a session: "press shoot at the drop, not before" (R-0013 point 2 in the player's words: the early mover loses, the first clean press wins) |
| Barks | the bubble over the speaker and the same text on a bottom pill | the bubble only; the bottom-centre strip is the commentator's line (lane A32's JAMVO text from `UJamCommentarySystem::GetLastLine`, Body 18 Bold, wrap at 880 or the width minus the radar and ready pill, two lines at most), held for the line's duration plus 90 frames, faded over 30; it moves up (y -150) while the controls strip shows. The strip lives on the HUD widget (always on screen), not the bark widget (created at the first bark) |
| Bubble over the banner | the bubble's y clamped to [170, VH-60] whatever was there | the overlay tells the band the MATCH banner covers (`BannerBandPx`: 0.4 VH -190 to +236); a bubble crossing it goes above the band when it fits under the scoreboard, else below (`PushOutOfBand`); the whole bubble stays on screen (clamped by its half width, it was cut at the edge before) |
| Markers | own-net caption flush on the right edge (clamp 60 px by the diamond's centre) | both markers clamped 48 px inside every edge plus their half width (`ClampMarker`), bottom room 56 / 64 px so the captions clear the ready pill and the strips; a chevron arrow (two bars, ink edge) appears beside a clamped or behind-us marker pointing at the edge the net lies beyond |

Previews for the proof: `jam.ui.faceoff <frames>` now counts sim frames (a time-dilation freeze holds it), `jam.ui.draw won|lost [frames]` shows the result. Tests:
`Jam.Hud.YoursFaceoffMarkers` (cue words, result words, tag windows, ring fade, clamps, strip opacity) in CameraTests, the band and push assertions in `Jam.Menu.DirectorsCallOncePerPeriod`.

Not changed (seen in the stills, not in this brief): the bubble's top clamp (170) overlaps the far-net caption's top clamp (150) when the net is dead ahead; the `PERIOD n - FACEOFF`
line of the match card touches the ring's top at 0.66 (A18's slots); the yours ring is the countdown ring's art with its tick marks, a reticle rather than a plain ring (a plain
ring texture is a one-line swap in `MakeGroundRing`).

### Stills (`docs/experiments/img/hud2/`, floating PIE window 1920x1080 and 1280x720 through `StartPIE playMode: PlayMode_InEditorFloating`, `CaptureGameViewport`)

| Still | Shows |
|---|---|
| `a42-match-start-{1080p,720p}.jpg` | the YOU tag and the ring under 7, the carrier's glow, the commentary strip above the controls strip, own-net marker left with the arrow |
| `a42-banner-countdown-*.jpg` | a restart at a tenth of the speed: the MATCH 1 card, the ring with 2 over HOLD, the hint under it |
| `a42-faceoff-countdown-*.jpg` | the previewed countdown (4, HOLD, hint) at the boards, both markers on the left edge with arrows |
| `a42-draw-won-*.jpg`, `a42-draw-lost-*.jpg` | WON / LOST in the ring in blue (the human's team) |
| `a42-bark-and-commentary-*.jpg` | one bark in its bubble (whole bubble on screen) and the commentator's line on the strip, nothing twice |
| `a42-own-net-a-*.jpg`, `a42-own-net-b-*.jpg` | the own-net marker 48 px inside the right edge and the bottom-left corner, caption clear of the ready pill |
| `a42-banner-bubble-*.jpg` | the banner up; the bark asked for during it was refused by the bark cooldowns, so the y-slot is proven by the test, not the still |

### Eyes verdicts (`tools/eyes` `hud` preset, gemini-3.8-flash, no thinking, six stills per call; `docs/experiments/eyes/a42-hud-{1080p,720p}.{json,md}`, $0.035 each)

Run on the PNG captures (converted to JPEG q88 afterwards to keep the folder at a third of the size). Legibility 1080p: A 4, B 3, C 3, D 3, E 3, F 2; 720p: A 4, B 3, C 3, D 4, E 4, F 3.
Smallest text read 12-13 px at 1080p, 10 px at 720p (the own-net caption; A18 measured 9 px for the same caption).

What it now sees that A27's pass listed as missing: the `YOU` tag ("Player indicator, above blue skater #7, legible", both resolutions), `HOLD` under the digit and
`LOST` in the ring (transcribed on every countdown and draw still), one copy of each line (the bark in its bubble, the commentary on the "subtitle bar", never the same
text twice), the own-net marker inside the edge ("mid-left edge", "mid-right edge"; the 1080p F still reads it as colliding with the far-net marker's `88 m`, which is
true: both nets lie to the right there and the two markers stack 40 px apart; the 720p F still reads the two as one line "own 18 m 88 m").

Where it is wrong or not ours: "Far net marker ... own 52 m" (it swaps the names: the caption says own); "the reticle overlaps the skater's skates / torso" (the ring is
under the human's feet by design, and the countdown ring sits at 0.66 of the height where the camera puts the human); "Clapperboard banner completely occludes center
ice" (A8's match card, 150 frames); "missing: a match timer" (first-to-3 cuts have none); "missing: puck highlight" (the puck glow is in the frame); "missing: what to
hold for the faceoff" on stills without the hint (the hint shows the first three faceoffs of a session; still C is the fourth of that PIE session and D/E are after the draw).
Fair hits to keep: the own-net caption is "legible: false" four times at 1080p (muted #9AA0A8 at 14 px over ice: A18's secondary-caption choice; a paper caption would
fix it in one line), and the commentary line in F sits behind the ring when the human stands at the bottom of the frame (the strip is at the bottom centre by the brief).

### 7b. A48's storyboard findings 2, 3 and 5 (H17.6, from the Director's message)

| Finding | Was | Is |
|---|---|---|
| 2: YOU over the wrong skater | the tag at 1.9 m over the helmet, projected: at the match camera it landed on the orange centre behind the human | while the ring shows, the tag sits inside the ring's top arc at the human's own feet (`UpdateYours`: alignment 0.5/0, 6 px under the arc); over the helmet only past the 24 m fade, where the projection error is small. The ring is a plain chalk ring with an ink edge (`T_UI_RingPlain`, a code-drawn 512 px PNG imported BC7 / no mips / UI group, ledger row in ASSETS.md; the countdown ring stays the fallback when the asset is missing) |
| 3: MATCH card over the human for 150 frames | 640 x 360 card centred at 0.4 VH, its FACEOFF line on the ring | 448 x 252 (0.7) centred at y 262 in the top band under the scoreboard (`MatchCardCentreY`, `MatchCardW/H` on the overlay), number 77 px, the line under the card; `BannerBandPx` is now that absolute band (132 to 428), the human and the ring at 0.66 stay clear; MenuTests assert the band is under 0.42 VH and above the ring |
| 5 / still 9: far speaker's bubble, bubbles over the pause card | a speaker 40 m away floated its bubble at the top of the frame; bubbles and the commentary strip stayed under TIME OUT | a speaker farther than `MaxBubbleDistanceM` (24) from the camera, or outside the frame, gets no bubble (the voice still plays); every bubble is cleared (`ShownFrame` reset) and the commentary strip hidden while the world is paused (`UGameplayStatics::SetGamePaused` is the pause menu's signal; the feed's `Paused` phase is honoured too); bubbles near the radar or the ready pill rise to VH-180 |

Stills (`docs/experiments/img/hud2/a42b-*-1080p.jpg`, floating PIE 1920x1080): `match-card-top-band` and `you-tag-at-ring` (the card under the scoreboard, YOU inside the
ring on the dot, 2 over HOLD, the hint), `bark-plain-ring` (plain blue ring under 7, the white carrier ring under 9, the commentary strip; the forced bark drew no bubble:
either refused by the cooldown or its speaker outside the frame, the still cannot tell which), `pause-clears-bubbles` (TIME OUT with no bubble and no strip; still 9's
"own 1 m" marker is the own-net caption clamped to the edge), `play-plain-ring`. Eyes (`docs/experiments/eyes/a42b-hud-1080p.{json,md}`, $0.032): legibility 3 / 3 / 4 / 4 / 3,
smallest text 13-14 px; it reads "Player indicator ring tag, lower center, YOU" on both card stills (no longer over the orange centre), "Match clapperboard, center upper
half", and on the pause still lists no bubble and no strip; its collisions are the card over centre ice (it is in the top band now: the far ice behind it, not the human),
the ring over the human's skates (by design) and the minimap flush with the margin (A18's 24 px edge). Not changed: InputMapping, CameraFollow, the pawn, the pause
menu's controls card (R-0032, lane A49).

## 8. HUD pass 3 (lane A76, from H16): the far-net badge, the edge slots, the MATCH card by rink size, and the one table of the HUD

Presentation only (`HUDWidget.*`, `DirectorsCallWidget.*`); the sim hash, the input path and `CameraFollow.cpp` are untouched. `jam.hud.Pass3 0` brings back the A74 layout in the
same package (the before column of the proof); default 1. Proof: `docs/experiments/img/a76/`, `docs/experiments/eyes/a76-*.json`, drivers `tools/qa/eyesgrid/a76grid.mjs`,
`a76eyes.mjs`, `a76score.mjs`, the new eyes preset `tools/eyes/presets/locate.md`.

| Gap | Before (A74) | After |
|---|---|---|
| Far-net badge on the crossbar ("badge covers the crossbar", 3 of 3) | the world point was 3 m up (`GetFarNetWorldCm`), the top clamp 150 px: at 49-66 m the badge was inside the net's pixels, and on the 120 / 140 m rinks it was **hidden under the MATCH card** (still `*-before-card.jpg`: no badge at all) | the point is 1.2 m over the crossbar (`NetCrossbarCm` 61 + `NetBadgeLiftCm` 120), and the badge stands at least `NetBadgeGapPx` (40) over the crossbar's own projection; clamp top 120 (the scoreboard plates end near 104), sides `ClampMarker`. Net off screen: the same element is the edge chevron with the label beside it (`PlaceMarkerArrow`, A74) |
| Edge elements stacking (puck chevron beside "own 46 m" and a skater) | three independent placements | one rule, `ResolveEdgeSlots`: the puck chevron, the far-net and own-net chevrons are groups (marks plus label); placed puck first, far net, own net; two groups whose anchors are closer than `EdgeSlotPx` (48) or whose label rectangles meet slide along their own edge in 24 px steps, away from the blocker first, inside [130, VH-200] on a side edge and [100, VW-100] on a top / bottom edge. The puck never moves. A skater under a label is accepted (labels are over the world), a label over another label is not |
| MATCH card over the far centre on the 120 m and 140 m rinks (A64 row 6, partial) | 50 pt everywhere, centre y 190 | `MatchCardPtFor`: 50 pt up to 90 m, **40 pt over 95 m** (120, 140), centre y 190 unchanged. The move variant (`jam.ui.CardMode 1`: 40 pt lifted to centre y 150) was shot too and **rejected**: it lands on the scoreboard's score plates (`*-cardmove-card.jpg`) |

### The HUD, every element (Slate units; 720p is drawn at 2/3 of these, so a 24 px label reads 16 px)

Order is draw order bottom to top. "Hides" is what takes the element away.

| Element | Anchor (screen point -> element point, offset) | Size at 1080p | Size at 720p | Hides when |
|---|---|---|---|---|
| YOU ring / carrier ring | ground ellipse projected at the skater's feet (1.1 m / 0.8 m radius), canvas 0,0 centre | 64 x 20 minimum, grows with the projection | 2/3 | not Play or GoalPause; the YOU ring fades 20-24 m from the camera |
| Far-net badge (diamond 22 px, distance in Label 24 outlined) | the world point 1.2 m over the far crossbar, clamped to x [92, VW-92], y [120, VH-112]; label to its right (left past VW-200) | 22 px + about 90 px label | 15 px + 60 px | never (the sim always has a far net); off screen it is the edge chevron |
| Far-net chevron (22 px) | beside the clamped badge towards the edge the net lies beyond | 22 | 15 | the net is on screen with the badge on it |
| Own-net marker (diamond 12, "own n m" Caption 14 muted) | the world point 1.5 m over the own goal line, clamped x [88, VW-88], y [150, VH-104]; label 10 px under it | 12 px + about 80 px label | 8 px + 53 px | never; its chevron only while clamped |
| Own-net chevron (16 px) | beside the clamped marker | 16 | 11 | marker on screen |
| Puck chevron (28 px) + distance (Label 24) | on the ray from the screen centre to the puck, where it meets x [64, VW-64], y [150, VH-170]; the radar corner (x > VW-330, y > VH-200) pushed up to VH-200; label 62 / 34 px behind it towards the centre | 28 + about 70 px label | 19 + 47 px | the puck is inside the inner 85 % of the frame; the MATCH card or countdown (first 150 frames), a goal pause, the pause menu, the end cards; `jam.hud.PuckMarker 0`; faded in / out over 6 sim frames |
| YOU chevron / YOU tag | over the human's head (past 24 m); the tag inside the ring's top arc | 26 px chevron; tag Label 22 | 2/3 | not Play or GoalPause, or the ring shows (chevron) |
| Scoreboard (home plate, take over timer, away plate) | top centre, y 14, to y 104 | plates 56 pt digits, timer 34 | 2/3 | never (Play, pause and cards) |
| Effects strip (A14) | top centre, y 158 | slot row, icon + Caption | 2/3 | no effect on the human |
| Director's Call line pill | top centre, y 112 | one line, Label 22 | 2/3 | no rule announced (A15 / A8) |
| MATCH card (banner, "MATCH n" over "PERIOD n - FACEOFF") | top centre, centre y 190 (`MatchCardCentreY`), 50 pt up to 90 m rinks, 40 pt over 95 m | 448 x 130, band y 121 to 295 | 2/3 | after `TakeCardFrames` (150), the end cards, the pause menu |
| Faceoff countdown ring + hint | screen 0.5 / 0.66, 260 px right of the centre (`RingOffsetXPx`), hint 124 px under it | ring 96 pt digit over HOLD | 2/3 | not a faceoff (HOLD / DROP / WON / LOST 40 frames); the hint after the third faceoff of a session |
| Sprint bar | bottom centre, y -168 | 240 x 8 | 2/3 | full stamina is shown, never hidden in play |
| Commentary strip (A32) | bottom centre, y -110 (-150 while the controls strip shows) | Body 18 Bold, wrap 560-880, two lines | 2/3 | no line, or 30 frames after it ends; the pause menu |
| Controls strip | bottom, x 24 to VW-256 | Caption-over-ice 16 pt, one line | 2/3 | after 10 s (fades over the last 2) |
| Radar | bottom right, 24 px margin | 200 x 100 on a 216 x 116 frame | 2/3 | never |
| Ready pill ("ESC / START CALL READY") | bottom left, 24 px margin | Caption 14 on a pill | 2/3 | no Director's Call available |
| Toast (rejected cut) | top left, 24 px margin | Body on a plate, 4 s | 2/3 | after 4 s |
| Bark bubbles | over the speaker, y clamped out of the MATCH band | Body 18 Bold | 2/3 | speaker beyond 24 m or off frame; the pause menu |
| Win / lose card, pause card, Director's Call card | screen 0.5 / 0.4 and centre | Display / Title | 2/3 | not their phase |

The edge-slot rule applies to the three edge rows (puck, far net, own net) only; every other element has a fixed slot above, and the slots do not overlap by construction
(scoreboard y 14-104, line pill 112, effects 158, MATCH band 121-295, commentary and strips from the bottom, radar and ready pill in the corners).
