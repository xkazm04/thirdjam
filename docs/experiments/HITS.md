# A hit is read by an eye that was looking elsewhere (lane A196, track F3 evolution 9, device A, 14 Nov)

The operator after package H31 (R-0064 point 5): "Animations of crosscheck and body check are short and not clear, may need today to evolve". Device B
makes the takes (wind-up, contact, follow-through, the held pose) for the 18:00 merge; the contact lane (A192, `CONTACT-11.md`) gave them time (the
wind-ups: 4 frames before a body check's window, the cross-check's shove 4 frames after its press; the held brace). This lane is everything round the
figure: **the tell before, the event at contact, the mark after**. Presentation only: no sim state, no rule, nothing written back; `Jam.Core.Determinism`
passes untouched (no JamCore file changed).

## 1. What a hit shows today (before: every `jam.hit.*` at 0)

Measured on the strips of `tools/qa/screen/a196-strips.ps1 -Mode before` (build 6, every `jam.hit.*` at 0 = the tree as it arrived; pro bots, clay,
960 x 540, a still every sim frame from the human's press to 30 frames after its end; `tools/qa/screen/out/a196-clay-before`, 26 strips, 1147 stills)
and the log lines of the same frames. Device B's figure on this tree has none of the new takes: `JAMVIS crosscheck ... call=PlayLift`,
`JAMVIS crosshold ... call=none` (no `PlayCheckWindup`, `PlayCrossCheck`, `SetCrossCheckHold`).

| case (strip) | press to contact | the figure's pose | an effect | a sound | a HUD word | held / camera |
|---|---|---|---|---|---|---|
| **body check that lands** (`check-007598`: down, closing 11 m/s) | 7 frames | `PlayCheck` on the press: in the stills the checker skates in unchanged until contact (no lunge an eye catches); the victim's knockdown take from the contact, down 60 to 150 frames | `NS_check`: grey chips at the feet, about 8 frames (+7 to +14) | `MS_check` (Strength, side) | none for a landed check | hit-stop 6 frames (2 on a weak one), camera kick 8 frames: only when the human is in it |
| **body check that misses** (`check-005751`) | window ends +16 | `PlayCheck` on the press; nothing after | none | none | **MISSED** (the human's only, F1's contact line) | none |
| **pressed cross-check** (`cross-003406`: the shove bumps at +4) | 4 frames | `PlayLift` (the stick's lift) on the press | none for a bump (no event); `NS_check` for a stumble or a knockdown | none for a bump | none for the cross's bump (**TOO SOFT** is a pressed body check's) | 1 frame on a stumble |
| **held cross-check** (`brace-001293`: braced 127 frames, the carrier turned away) | the frame the bodies meet | **nothing** for the whole hold (the figure has no `SetCrossCheckHold`); the skater is 10 % slower | none until a hit | none until a hit | **BRACED HIT** / **RUN THROUGH** on a hit (A192) | as a check |

**What an eye that was looking elsewhere cannot tell from skating into a man**: the **held cross-check** (127 frames identical to skating), the
**pressed cross-check that only bumps** (no event, no burst, no sound) and **a bot's missed check** (nothing at all; the human's says MISSED). A body
check that lands reads only by its result: the victim falling, and 8 frames of grey chips at the feet. Nothing on the screen says "he is down for
this long". Stills: `img/a196/*-before-after.jpg` (top row before, bottom row after).

## 2. What it shows now (`HitRead.h`, `HitRead.cpp`; `jam.hit.*`, all on by default)

| when | element | frames | cvar (0 = before) |
|---|---|---|---|
| **before**: a body check's press | a **chevron of light on the ice**, its tip 1.4 m ahead of the checker at the press sliding out to 2.4 m over the wind-up (loading, brightening), white-hot over his team colour through the window (the lunge). Outside the HUD's YOU and carrier rings (~1 m): at 1.05 m the first build's wedge lay on them and was never seen | press to contact or miss (4 + up to 12 for the human, 4 + 6 for a bot) | `jam.hit.Tell` |
| a miss | the chevron goes grey, slides on 35 cm and goes out; **no burst** (the HUD's MISSED word is F1's) | 12 after the window ends | `jam.hit.Tell` |
| a cross-check's press | a **streak across the stick** at chest height (1.4 m, white over the team colour), brightening over the wind-up; on the shove it thrusts 35 cm forward and flashes white; grey on a miss | press + 4, then 8 | `jam.hit.Tell` |
| the held brace | the streak held across the chest and an **arc of 7 bars on the ice 1.35 m ahead**, brighter with speed (full at 8 m/s) | while braced | `jam.hit.Tell` |
| the figure | a **lean** of the drawn figure (the skater actor, never the sim): back to 6 degrees over the wind-up, forward to 12 through the window (3 frames), upright over 8 frames after; 8 forward while braced; x 0.8 for a cross-check. **Stands down** for each take device B's figure declares (`PlayCheckWindup`, `PlayCrossCheck`, `SetCrossCheckHold`): today B's figure has none of them (`JAMVIS crosscheck ... call=PlayLift`) | as the tell | `jam.hit.Lean` (2 = always) |
| **at contact** | a **ring on the ice** at the contact point, growing from 45 % to its full radius **by the closing speed: 1.2 m at a soft touch to 3.0 m at 14 m/s** (x 1.2 braced; a second inner ring braced), white then the checker's colour; **shards**: 6 (10 braced) out from the hit, rising 45 to 80 cm when the victim stays up, **ice spray on the ice when he goes down**, none within 55 degrees of his fall | 14 | `jam.hit.Burst` |
| the camera | **already there**: the kick along the hit (lane A73 / A101 / A132: `jam.hitkick`, 3 to 12 cm, 8 frames) on the human's landed and taken checks, the cross-check's shove included; the hit-stop (`jam.hitstop`, 1 to 6 frames) | | not this lane's |
| the sound | **already there**: the binder's `MS_check` (Strength = closing / 8 m/s, Variant = the side, 4 a stumble) and `MS_knockdown`. **Missing: a braced line** (asked of device B, section 5) | | |
| **after**: a downed skater | an **oval of 12 bars on the ice round his lying body** (1.5 m along it from its middle, 0.95 m across), his team colour (lifted to white it was faint on the clay ice): the bars go out one by one from his head round as his down frames run out (12 at the fall, 1 on the last frame): a countdown an eye reads. On the ice only: nothing over the figure (`Jam.Hit.DownClear`) | the sim's down frames (60 to 150) | `jam.hit.Down` |
| a stripped puck | a gold ring out from the puck a stick lift took (0.45 to 1.0 m) | 18 | `jam.hit.PuckFlash` |

All in A's own `/Game/Placeholders/M_Trim` (TeamColour, Emissive) on `/Engine/BasicShapes/Cube` bars from one pool (`AJamHitRead`, made on the first
sim step); no hand-made material. The ground marks stand from 3 cm: at 0 to 3 cm the look tier's ice covered them (the first strips).
`jam.hit.Scale` (1) sizes the marks for a window's tuning.

**One `JAMHIT` line a hit and a miss** (for the acceptance run): `JAMHIT frame=2967 kind=check result=down checker=0 victim=9 human=1 closing=3.8 burst_m=1.40
windup=4 press_to_contact=9`; `kind` check | cross | braced, `result` bump | stumble | down | miss | strip.

## 3. Proof

The same drills with the marks on (`-Mode after`, build 5, `tools/qa/screen/out/a196-clay-after`: 31 strips, 1380 stills; the human's JAMHIT lines:
10 check presses (2 landed down, press to contact 7 and 14; 8 missed), 10 cross-checks (2 bumps at +4, the rest strips or misses), 5 braces
(none met a body: the carrier turned away), 7 hits taken (5 down at 10.4 to 11 m/s, 2 stumbles)). The sim is untouched: the `JAMSIM` hashes at frames
600 and 1200 are the same with the marks off and on (`dca05cbee139842d`, `e39515e184de3d57`) and the brace drill's strips fall on the same frames
(629, 711, 1293, 1466) in both runs.

| case | what is on screen now, and for how many frames |
|---|---|
| body check that lands | the chevron from the press to the contact (4 to 14 frames), the lean back then forward; at contact the ring (2.6 m at 11 m/s) and the spray for 14 frames over B's chips; the downed man's oval for his 60 to 150 frames, counting down |
| body check that misses | the chevron through the window (16 frames), then grey and out over 12; no burst (the miss reads as the lunge alone) |
| pressed cross-check | the streak across the stick through the 4 wind-up frames; on the shove it thrusts and flashes (8 frames); a bump now shows the soft ring (1.2 m, 14 frames); a stripped puck flashes gold (18) |
| held cross-check | the streak across the chest and the arc ahead for the whole hold (127 frames in `brace-001293`), the figure leaning 8 degrees into it; a braced hit's double ring and 10 shards |

Stills (`img/a196/`, top row before, bottom row after):

| still | what |
|---|---|
| `brace-before-after.jpg` | the same frames of drill 23 (sim frames 1293 to 1420): nothing, then the streak and the arc |
| `check-land-before-after.jpg` | a landed body check: before, B's chips and the fall; after, the chevron, the ring and spray, the victim's oval |
| `check-miss-before-after.jpg` | a missed body check: before, nothing but MISSED; after, the chevron ahead, grey on the miss |
| `cross-before-after.jpg` | a pressed cross-check whose shove bumps at +4: before, nothing; after, the streak, the soft ring |
| `taken-before-after.jpg` | the first hit of the match on the human (sim frame 139, down at 11 m/s): the ring and spray, the oval round him |

- Tests (`HitRead.cpp`): `Jam.Hit.BurstSize` (the ring grows with the closing speed, 1.2 to 3.0 m, braced larger, a landed check through the model at
  6 m/s draws 16 ring bars and shards, gone after 14 frames), `Jam.Hit.MissNoBurst` (a window that runs out on nobody: one miss on the frame it ends,
  no burst, the chevron dim and gone after 12 frames; a cross-check whose stick stripped and whose shove found nobody: a strip, the puck's flash, no
  burst), `Jam.Hit.DownClear` (the countdown 12 / 6 / 1 / 0; for a knockdown in twelve directions of fall, every bar of every frame of the burst and the
  countdown: the countdown on the ice and clear of the lying body by 0.2 m, nothing above 10 cm over the body), `Jam.Hit.Lean` (the lean's phases,
  on by default, a plain object has no take).
- Full `Jam` filter on the final source (build 7, `Game/logs/a196/auto-6.log`): **296 tests, 296 Success, 0 Fail** (runs 1 to 6 logged in `status/A196.md`); `node tools/gate/check-test-guards.mjs`: PASS, 0 faults in 202 files.
- Shipping compile of the Game target on the final source: **PASS** (149 s).
- Seen in the editor binary only: the first hit of a process draws its first frame dark while the engine compiles the bars' shader ("Preparing Shaders"
  on that still, `taken-before-after.jpg` +0); a package has the shader cooked. The pool of 96 bars is made at the first sim step (no hitch at a hit).
- Not proven here (no cursor for this lane): the acceptance run on a package; queued with the Director. A window's eye: is the chevron read at speed,
  is the oval read as "down for this long", is anything too much at 5 v 5 in a pile-up.
- The hash: `Jam.Core.Determinism` passes; no file of the sim was touched (the files are `JamGame` presentation and the game mode's three call lines
  and drill 23's one input line). The neutral value: every `jam.hit.*` at 0 gives the screen of before (the "before" strips are that).

## 4. Tools

- `tools/qa/screen/a196-strips.ps1 -Mode after|before`: one editor-binary game process off screen (`-RenderOffScreen`, no cursor, `-Multiprocess`),
  fixed 60 Hz step, the cut `tools/qa/screen/a196/cut-clay-long.json` (pro, clay, first to 9), drills 23, 2 and 1 in turn switched on the log;
  `jam.hit.Capture N` films the human's presses (check, cross, brace) and the hits he takes, a still every sim frame. Stills in
  `tools/qa/screen/out/a196-clay-<mode>/shots`, the lines in `hit-lines.txt`. Sheets: `tools/qa/screen/a196/sheet.py` (one strip) and `pair.py` (before over after).
- **Drill 23** (`jam.ctrl.Drill 23`, new in `HokejGameMode::DrillInput`): the human skates at the carrier holding the cross-check (released for a frame
  once the brace is spent).

## 5. For device B (A-to-B-0027 point 1)

Frames from call to contact, as shipped (A192's keys `contact.check_windup_frames` 4, `contact.cross_windup_frames` 4, `contact.cross_hold` true):

- **Body check**: `PlayCheck()` and `PlayCheckWindup(4)` on the press frame F. The window opens at F+4 and lasts 12 frames for the human, 6 for a bot;
  measured contacts F+4 to F+14 (`JAMHIT ... press_to_contact`: bots 4 to 7, the drill's human 6 to 14). Wanted from B's take: a load an eye catches in
  F..F+3 (the shoulder dropped, the weight back), the drive F+4 to F+10, the follow-through 8 to 12 frames after the contact. **A miss gets no call**:
  if B wants a whiff take, A adds `PlayCheckWhiff()` by name on the frame the window runs out (one line in the pawn, the contact lane's file).
- **Cross-check (pressed)**: `PlayLift()` and `PlayCrossCheck(4)` on F; the stick's strip on F; **the shove lands at exactly F+4** if the man is still
  in reach.
- **Cross-check (held)**: `SetCrossCheckHold(true)` from the 10th frame held; the hit on the frame the bodies meet; `SetCrossCheckHold(false)` on that
  frame or on release. The pose in and out in 6 frames.
- **A's stand-ins stand down by themselves** for each take B's figure declares (`jam.hit.Lean 1`): the lean stops; the ground marks (chevron, streak,
  arc, burst, oval) stay unless the Director says otherwise (`jam.hit.Tell 0`).
- **Sound, missing**: a braced hit has no line of its own. Asked: `MS_check` **Variant 5 = braced** (stick across into a body: wood on pads, heavier
  than a check), and **Variant 6 = whiff** (a lunge into air) if B wants the miss heard. A sends Variant 5 when the victim's hit came out of a brace
  the day it exists (one line in the binder's table).
- **Teen rating**: no blood, no injury anywhere: rings, spray and a countdown.

## 6. Next

1. **The miss and the hit heard** (binder, `VariantFor`): `MS_check` Variant 5 for a braced hit and a whiff on a miss, the day B's lines exist;
   proving number: one sound per JAMHIT line in a drill run (10 of 10), the braced ones at Variant 5.
2. **B's takes on the figure, A's stand-ins tuned to them** (after the 18:00 merge): the lean off where B's take plays, the chevron's timing set to B's
   drive frames; proving number: in the strips the chevron's tip meets the victim within 2 frames of the contact in 8 of 10 landed checks.
3. **A hit seen from the bench** (bots' hits off the human's screen): a radar blip and an edge arrow for a knockdown off screen; proving number: of the
   knockdowns in a pro match (about 10), every one off screen gets the blip within 1 frame, none drawn over a downed figure (`Jam.Hit.DownClear`'s rule).

# Evolution 10 (14 Nov, 15:50 to 17:20): the ring quieter, the men down off screen, the tell as long as the wind-up, one miss

The Director on the strips of evolution 9: the chevron, the ring and the oval read; keep them. Four things.

## E1. The ring is no longer the loudest thing on the sheet

Evolution 9's ring at a landed check (`check-002958` +9 to +18) was white blocks all round both figures, wider than the two bodies. Now
(`FJamHitReadModel::BurstWeight`, `BurstBarWidthCm`, `BurstEmissive`, `ClearOfBody`):
- **its radius stays by speed** (1.2 to 3.0 m); its **weight** is the closing speed over 14 m/s (a bump 0);
- the bars' **width** is (6 + 20 x weight) cm x (1 - age) + 2 and their **brightness** (3 + 12 x weight) x (1 - age)^2 + 0.5: both fall from the first
  frame (the material is opaque: width and light are its opacity); a light hit (1.4 m/s) starts at 8 cm against evolution 9's 31: **a quarter**;
  10 + 6 x weight bars; shards only from weight 0.25;
- **nothing of it within 0.75 m of either body** (the checker standing, the victim standing or lying along his fall: `ClearOfBody`), so from the low
  camera at the boards no bar stands in front of a figure's legs.
`Jam.Hit.BurstSize` holds the clearance on every bar, the light hit's third and the fall from the first frame.

Stills (`img/a196/`, evolution 9 above, 10 below, **the same sim frames**: drill 1 from frame 0, `-StartDrill 1`; the `JAMSIM` hashes at
600 to 3600 are identical in the two runs): `e10-ring-boards-before-after.jpg` (the boards camera, sim frame 1085: the human felled at 11 m/s against
the boards) and `e10-ring-open-before-after.jpg` (sim frame 2958: his check that fells a man in open ice). Evolution 9's white blocks round both bodies
are gone; the ring reads round them, thin and fading from its first frame.

## E2. A man down off screen

- **The edge arrow** (`UJamHitEdgeWidget`, over the HUD, `jam.hit.OffScreen 1`): for every downed man whose body is outside the frame (or behind the
  camera), an arrow on the screen's edge 56 px in, on the line from the screen's centre towards him, pointing at him, in his team colour over a dark
  stroke: **the human's mates a hollow diamond**, **opponents a solid triangle**. It is on the edge only while he is off screen, so it is never over him.
- **The radar** marks every downed man (`HUDFeed` `DownsM` / `DownsTeam`, `UJamRadarWidget` `DownMark`): mates a hollow square, opponents an X, 15 px,
  in the team colour, over the dots.
- One `JAMHIT offscreen frame=F skater=S down_at=D delay=F-D mate=M` line a knockdown marked.
`Jam.Hit.OffScreen`: on / off screen, the arrow on the inset edge for 24 directions, inside the screen, never over him, pointing at him; the two shapes;
the radar's two marks.

Measured: drill 1 from frame 0 (pro, clay, 4 / 4): 1 knockdown off screen, marked on its own frame (`delay=0`); the staged run (8 / 12): 3
knockdowns off screen, marked on the frame each left the picture (3, 4 and 104 frames after the fall: until then he lay on screen with his oval). The
arrow is decided on the render frame that finds him off screen and drawn on that frame: within 1 frame. Seen on `e10-tell-windup-4-8-12.jpg` row 2
+12 onwards (the orange triangle at the bottom right while the felled opponent lies off screen; in `tools/qa/screen/out/a196-e10-w812/shots`).

## E3. The tell as long as the wind-up

The Director's lengths (16:0x): the bots' body check winds up 8 frames and their cross-check 12, the human 4 and 4 (`contact.bot_*_windup_frames`,
on main about 16:30, not in this tree: staged here by `tools/qa/screen/a196/cut-clay-windup-8-12.json`, every press at 8 / 12). Both the chevron and the
lean read the frames the call carries (`WindupFrames`): the lean loads over them (`CheckLeanDeg`), the streak brightens over them.
**The chevron now points at the man** (the nearest standing opponent within 7 m and 70 degrees of his facing at the press; a cross-check's own target)
and its tip covers the way in proportion to the frames left to the contact as they look now (`TipRemainFrames`: the bodies' gap over the closing speed,
never before the wind-up ends), so it stands on him when the bodies meet. (The first try ran the tip over the wind-up alone: on a man already close at
the press it met him on the press's frame, 4 frames before the contact: 3 of 9 bots' checks within 2 frames.)

| wind-up (where) | landed checks with the chevron on a man | its tip met him within 2 frames of the contact | contact minus meeting (frames) |
|---|---|---|---|
| 4, bots (drill 1 match, build 10) | 10 of 11 | **10 of 10** | 1 (8 times), 2 (2) |
| 4, the human (drill 1) | 5 of 5 | **5 of 5** | 1 (5) |
| 8, bots (staged cut, build 9) | 4 of 6 | **4 of 4** | 1 (4) |
| 8, the human (staged cut) | 4 of 6 | **4 of 4** | 1, 1, 2, 2 |
| 12, cross-check (staged, drill 2) | the streak loads over +0 to +11, the shove lands at **+12 exactly** (a stumble, a bump; 8 misses) | | |

The landed checks without a meeting (2 at 8 frames each side, 1 at 4) hit a man other than the one the chevron pointed at (contacts at 13 and 16
frames: a second opponent came in). Driver: `tools/qa/screen/a196/tipmeet.py <game.log>` (reads `JAMHIT ... press_to_contact tip_met`).
`Jam.Hit.Lean` holds the lean and the meeting at 4, 8 and 12 (men closing to meet on the wind-up's last frame: met within 2). Still:
`e10-tell-windup-4-8-12.jpg` (rows: 4 human check, 8 human check, 12 human cross-check). Rows 2 and 3 were taken before the banner fix below: the faint
GOAL plate on them is that fault.

## E4. One miss

A body check's miss is now the sim's own (`ChecksMissed` grew: the HUD's MISSED reads the same counter), for the bots as for the human: the chevron goes
grey on that frame. Before: of the human's misses 4 of 5, 2 of 3 and 7 of 8 were on the HUD's frame; the others were a window that closed while he
carried (no sim miss: A said miss) and a window the sim counted while he lay down (A had dropped the press). `Jam.Hit.MissNoBurst` holds both.

After (build 10, drill 1 from frame 0): the human's misses 3, the HUD's MISSED 3, **3 on the same frame**; the bots' misses grey the same way (5 in
the run). Found on the way and fixed (A188's banner, this family): **GOAL 0:0** showed on 3 to 4 of the human's hits a match: the sim's `GoalPause` also
holds the play after a foul's whistle (the drills' checks on a man without the puck: "Hit away from the play. Called."). The banner now shows only
when a side's number grew (`GoalBannerScoreGrew`, `Jam.Hud.GoalBanner`); in the build-10 run 4 banners for 4 goals.

## E5. The proof of evolution 10

- Full `Jam` filter on the final source (build 10, `Game/logs/a196/auto-9.log`): **297 tests, 297 Success, 0 Fail**; test-guards PASS (0 faults, 202 files).
- Shipping compile on the final source: **PASS** (418 s, the shared build slots).
- The sim untouched: no JamCore file changed; `JAMSIM` hashes identical with evolution 9's look and evolution 10's at 600 to 4800 (`dca05cbee139842d`,
  `513c22db22f23983`, `d7ceb5bf3fdc7bd2`, ...).
- Neutral values: `jam.hit.OffScreen 0` (no arrow, no radar mark); the ring's quieter look and the predictive tip have no switch of their own
  (`jam.hit.Burst 0` / `jam.hit.Tell 0` take them off).
- The staged lengths: `tools/qa/screen/a196/cut-clay-windup-8-12.json` (every press 8 / 12) until `contact.bot_*_windup_frames` are on this tree.

# PROPS READY (device B's dev-b 1da715c1 in this tree from 16:55)

| B's piece | how A calls it | seen |
|---|---|---|
| `SM_Boost_<kind>` (6 props) | A188's beacon loads it by name | Arsenal, clay: `JAMBOOST beacon ... prop_from_b=1` for every pickup (22); the winged bolt (speed) on A's pedestal and ring (`tools/qa/screen/out/a196-props/shots`, f960) |
| `SetBoostEquipped(FName, bool)` | A188's aura, by name (`figure_call=SetBoost`; A's waist ring then stands down) | the keg on the back (shotgun, `img/a196/props-goal-banner-art.jpg`), the hip bumper (f240 / f480), the round shield (f960); the crest (slow puck) not caught from behind at f1440 |
| `T_UI_Banner_Goal` | `GoalBannerArtBrush` by name (`JAMHUD goal_banner_art=T_UI_Banner_Goal`) | `img/a196/props-goal-banner-art.jpg`. B's art carries the word GOAL: the first still showed it twice (A's word over B's); A's word is no longer drawn over the art, the score stays under it |
| `PlayCelebrate(int32 0-3)` | A188's `CelebrateGoal` by name on the scorer and his nearest mate | both goals of the drill run (frames 1811, 4628): `scorer=7:stick_raise:PlayCelebrate mate=5:knee_glide:PlayCelebrate`. The takes run 58 to 110 frames; the goal's pause is 120: **none is cut by the faceoff** (10 frames to spare at the longest). The camera stays on the human (untouched) |
| `SetCrossCheckHold(bool)` | the pawn, by name (lane A184) | on both figures; A's lean stands down for the brace on any figure that declares it |
| `SetWindupTakes(bool, int32)` | **new: `jam.hit.BTakes`** (`AJamHitRead::TellTakes`), by name on each figure at spawn and when the figure or the cvar changes: `SetWindupTakes(true, 8)` | `JAMHIT btakes ... on=1 call=SetWindupTakes` for the 9 bots, `on=0` for the human |

**Which wind-ups ship on B's takes: the bots' (`jam.hit.BTakes 1`, default).** The human's 4-frame wind-up starts B's take inside its coil (B: "reads as
a dip"); in the strips (`img/a196/props-human-windup-lean-vs-btake.jpg`: the same sim frames, A's lean above, B's 4-frame take below) B's take turns the
figure and swings the stick from +4; at 960 px neither reads as a dip, and B advises 6 as its floor: the human keeps A's lean. `jam.hit.BTakes 2` gives
every figure B's take, 0 none. **A's lean stands down exactly on the figures told** (`TakesOn`): the bots' checks and cross-checks, and every brace.
**The bots' cross-check at 12 frames** calls `PlayCrossCheck(12)`: B plays from frame `10 - 12 = -2`. Asked of B: clamp to frame 0 and hold the first
pose 2 frames (or say what it does), so its contact (frame 10) lands on the sim's (frame 12).
Not filmed: a bot's wind-up take (the capture follows the human's presses); the bots' takes are on by the log lines.
