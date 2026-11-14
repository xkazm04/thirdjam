# Lane TRAILER report: the submission video on paper (device B, 14 Nov 01:41 to 02:00)

Writer and editor on paper: no GPU, no Unreal editor, no Blender. Inputs read: `docs/SUBMISSION.md`, `docs/THEME.md`, `docs/VISION.md`, `docs/experiments/VOICE.md`,
`docs/experiments/COMMENTARY.md` (headings), `contracts/RULINGS.md` R-0050 to R-0057, `handoff/A-to-B-0022.md`, `status/A144.md`, `status/AU10-report.md` part B,
`submission/video/script-v2.md`, `submission/video/cutlist.md`, `submission/tagline.md`, the Designer's `Game/Content/Designer/cached-cuts.json`, the review page
`C:\Users\kazda\jam\review\index.html` (headings and image list).

**Stills I looked at with the Read tool** (all real; the shots below are written to what they show):
`devlog/shots/20261113-194533-B-qa3-clay-play-1.png` (game camera on clay: figures about 110 px tall, HUD with score, PERIOD 1, radar, "ESC / START CALL READY"),
`20261114-004300-B-sk24-lying-loop-play.png` (in play: an orange check, the blue human down 114 frames, gets up),
`20261114-011347-B-b41-goal-clay.png` (a goal on clay: the lamp, the net, the pink wash),
`20261113-205819-B-br4-banner-1920.png` (the banner: HOKEJ gouged in the ice, a Blender render, 1920 x 1080),
`docs/experiments/img/a144/after-1080-main.png` and `after-1080-cuts.png` (tonight's menu: PLAY NOW, "The speed you bring is the hit you get.", the twelve cards and six cuts),
`submission/shots/08-directors-call.png` (the Director's Call card: five sliders, APPLY; drawn on the dark tier, an older build),
`devlog/shots/20261113-232904-B-sk23-fall-sizes.png` (three fall sizes, fixed camera), `20261113-194832-B-qa3-clay-play-4.png` (five a side, a scrum at the boards,
the human down, a commentator caption bubble on screen), `20261113-164934-B-sk18-human-knocked-down.jpg` (the hit-stop held frames, slide, get-up, close crop),
`20261112-153300-A-a18-menu-direct-1080p.png` (DIRECT A CUT: sentence box, ROLL, the chips "make it faster" etc.; an older header "SCENE 3 v 3"),
`20261113-171620-B-b34-ten-figures-faceoff.jpg` (ten figures at the faceoff on clay), `20261114-004259-B-sk24-stop-level.png` (the human's hard stop, poses only).
Captions only (not looked at): `20261113-232906-B-sk23-board-fall-wall.png`, `20261114-001015-B-b40-brake-strip.png` (its caption: the spray reads small; SK24's caption: "the
spray does not read in these stills").

What that means for the film: the game camera is high and behind; figures are small (about 110 px of 1080). Close views exist only as crops of stills. **No low camera, no
replay camera and no slow motion exist in a package** (`docs/experiments/CAMERA.md`: "A replay camera ... Not built"). So every shot below is the game camera, and the only
slow-motion beat is an edit of 60 fps footage at half speed. The hit-stop (held frames at contact) is the game's own built-in "slow beat".

## 1. Summary

Done: Evolution 1 (three complete 45 s scenarios with beat sheets, narration with word counts, sound beats, theme placement, cards; ranked) and Evolution 2 (shot list
for the chosen scenario A, storyboard page with the nearest stills and a playable read, the rough read in the commentator's voice measured and laid on the beat sheet with
three overruns fixed, a filming recipe for device B with a take checker tested on two files). Also done from Evolution 3: a 30 s cut of the same film (beat sheet and its own
read), a 45 s animatic (placeholder stills held for each shot's length with the read under them), a second-voice proposal from voices that already exist, the music plan.
**Credits: 145 of the 400 allowed** (16 requests, summed from the response headers in `C:\Users\kazda\jam\trailer-lab\read\ledger.jsonl`). Not done: the five store stills
(proposed below, not chosen against a package: there is no H27 package yet); nothing was filmed (by design); every line of copy is a proposal for the Director.

## 2. Evolutions

### Evolution 1: three scenarios, each a complete 45 s film on paper

Rules held in all three: 45.0 s; the theme on screen inside the first 5 s; narration at most 2.4 words per second of its slot; at least a third of the film without voice;
the title is never spoken (it is on the menu art and the end card); the end card is the banner and the banner line only (R-0054 point 5: "Hockey with the whistle taken out."
if the whistle's removal lands by Saturday 19:00, else "The speed you bring is the hit you get."). The game's own commentator is switched off in the capture (Settings, VOICE)
so that the narration is the only voice; the game's hits, crowd and music stay in.

#### Scenario A: "The Director's Chair" (ranked first)
Pitch: you pick a cut, tell it what you want in one sentence, play it, hit hard, then change the rules in the middle of the match and play the new rule.

| # | time (s) | on screen (all game camera, Shipping) | narration (commentator, proposal) | words / slot | game sound needed |
|---|---|---|---|---|---|
| 1 | 0.0-2.5 | main menu: the clapper card, PROD. HOKEJ / SCENE ROUGH / TAKE A HIT, PLAY NOW | none | | menu music, click |
| 2 | 2.5-5.0 | DIRECTOR'S CHOICES: the twelve cards and six cuts, the cursor crosses them | A1 "Pick your cut." (in at 2.6) | 3 / 2.0 s = 1.5 | clicks |
| 3 | 5.0-9.0 | DIRECT A CUT: "make it faster" typed (or its chip), ROLL, the Fast Cut loads | A2 "Tell it what you want." (6.0) | 5 / 2.5 s = 2.0 | keys |
| 4 | 9.0-11.0 | faceoff: five blue, five orange on clay, the drop | none | | faceoff clapper, crowd bed |
| 5 | 11.0-16.5 | the human charges an oncoming orange carrier: contact at about 12.5, the hit-stop hold, flattened, slide | A3 "Full speed. Full price." (14.0) | 4 / 2.5 s = 1.6 | **the hit, the crowd's roar** |
| 6 | 16.5-21.0 | the figure lies, breathes, gets up; play goes on round it (first 0.5 s of the fall may run at half speed) | none | | crowd settles |
| 7 | 21.0-26.5 | the human carries at a defender, brakes hard, cuts, bursts past | A4 "Stop. Cut. Gone." (22.5) | 3 / 2.5 s = 1.2 | the brake's scrape |
| 8 | 26.5-31.0 | Esc: DIRECTOR'S CALL over the paused match, ICE FRICTION to its minimum, APPLY, the scoreboard names the rule | A5 "Once a period, you change the rules." (27.0) | 7 / 3.0 s = 2.33 | the rule-change sting |
| 9 | 31.0-38.0 | glass ice: long glides, a check sends one sliding into the boards | A6 "Glass ice. Good luck." (34.5) | 4 / 2.5 s = 1.6 | **the board hit**, crowd |
| 10 | 38.0-41.5 | a shot, the red lamp, the net shakes, the score turns | A7 "That's the cut." (39.2) | 3 / 2.0 s = 1.5 | **the goal, the crowd** (no horn claimed) |
| 11 | 41.5-45.0 | end card: the banner still, the banner line under it | none (title unvoiced) | | the music's last bar |

Theme in the first 5 s: the slate menu (0.0) and DIRECTOR'S CHOICES (2.5); the theme is shown three times (shots 1-3 and 8). Narration: 29 words in 17.0 s of slots (1.71 words
a second); 28.0 s of the film's slots carry no voice (62 %); measured on the read, 35.0 s are below -50 dBFS on the voice track.

#### Scenario B: "One hit, three ways"
Pitch: the vision's law 1 in one film: the same check told three times, by direction and speed, and the carrier's answer.

| # | time (s) | on screen | narration (proposal) | words / slot | game sound |
|---|---|---|---|---|---|
| 1 | 0.0-3.0 | DIRECTOR'S CHOICES, the cursor lands on BRUISERS ("Five a side, no whistle. Hit everything."), Return | "Pick a cut." (0.5-2.5) | 3 / 2.0 = 1.5 | clicks |
| 2 | 3.0-5.0 | Bruisers faceoff, ten figures | none | | clapper |
| 3 | 5.0-10.0 | way one: two skaters gliding the same way at the same speed touch; both keep their feet | "Same way. A nudge." (7.0-9.0) | 4 / 2.0 = 2.0 | a bump |
| 4 | 10.0-16.0 | way two: a check from the side; the victim stumbles, keeps its feet, loses the puck | "From the side. A stumble." (13.0-15.5) | 5 / 2.5 = 2.0 | the stumble's sound |
| 5 | 16.0-26.0 | way three: head-on at full speed; the hold, flattened, slide, lie, get up (first 0.6 s at half speed) | "Head-on. Flattened." (21.0-23.0) | 2 / 2.0 = 1.0 | **the big hit, the roar** |
| 6 | 26.0-33.0 | the carrier cannot hit: it stops, cuts and goes past a defender | "Carrier can't hit. Carrier can stop." (29.0-32.0) | 6 / 3.0 = 2.0 | the scrape |
| 7 | 33.0-39.0 | Director's Call: SKATER SPEED up, APPLY | "Then you change the rules." (35.0-37.5) | 5 / 2.5 = 2.0 | the sting |
| 8 | 39.0-41.5 | a goal | none | | the goal, crowd |
| 9 | 41.5-45.0 | end card | none | | |

Theme in the first 5 s: the cards. 25 words in 14.0 s of slots; 31 s without voice. Filmability: **way one has no drill** (R-0054 point 3 owes it to the contact track) and
needs the operator to match a bot's speed by hand; drills 5 and 6 stage ways two and three only in a Development package, which R-0054 point 4 forbids next to Shipping
footage, so the three-way rhythm would collapse into one split-frame shot.

#### Scenario C: "A match in forty seconds"
Pitch: one real match of the season's first take, played by hand, cut down in match order, the booth calling it.

| # | time (s) | on screen | narration (proposal) | words / slot | game sound |
|---|---|---|---|---|---|
| 1 | 0.0-3.0 | main menu, PLAY THE SEASON: TAKE 1 OF 8, Return | "Take one." (0.5-2.0) | 2 / 1.5 = 1.3 | click |
| 2 | 3.0-5.0 | MATCH 1 card, faceoff | none | | clapper |
| 3 | 5.0-10.0 | the bots pass: carrier, outlet, pass lands | "They pass now." (6.0-8.0) | 3 / 2.0 = 1.5 | sticks, puck |
| 4 | 10.0-15.0 | the human flattens a carrier | "Leveled." (12.0-13.5) | 1 / 1.5 = 0.7 | **the hit** |
| 5 | 15.0-21.0 | a bot flattens the human; it gets up | "Up you get." (18.0-20.0) | 3 / 2.0 = 1.5 | the hit, the wince |
| 6 | 21.0-24.0 | stop and go | none | | scrape |
| 7 | 24.0-29.0 | Director's Call between plays | "Change one rule." (25.0-27.0) | 3 / 2.0 = 1.5 | sting |
| 8 | 29.0-36.0 | the push and the winning goal | "That's a take." (33.0-35.0) | 3 / 2.0 = 1.5 | **goal, crowd** |
| 9 | 36.0-41.5 | the win card | none | | crowd's win |
| 10 | 41.5-45.0 | end card | none | | |

15 words in 11.0 s of slots; 34 s without voice. Theme in the first 5 s: only the slate and the word TAKE; weakest on theme. Most filmable: one session.

#### Ranking (1 to 5 per jam criterion; my judgment from the stills and the docs, not a measurement)

| | theme | gameplay | look | sound | ambition | total | filmable in Shipping by the operator |
|---|---|---|---|---|---|---|---|
| A The Director's Chair | 5 | 4 | 4 | 4 | 5 | **22** | yes, one continuous session; shots 5 and 7 need practice |
| B One hit, three ways | 3 | 5 | 4 | 4 | 4 | 20 | way one only by hand, many takes; Development drills unusable beside Shipping |
| C A match in forty seconds | 2 | 4 | 4 | 5 | 3 | 18 | yes, easiest |

**I would film A.** It is the only one that shows the theme as something the player does (pick, say a sentence, change a dial mid-match and play the result), it still carries
the key feature (the head-on hit, the fall and the get-up, shots 5 and 6), and every shot comes from one Shipping session, so R-0054 point 4 never bites. C is the fallback
film if shots 3, 5 or 8 fail: most of its shots come out of the same session.

### Evolution 2: scenario A made filmable

#### Shot list (package: the H27 Shipping package the operator plays at 10:00, R-0056; nothing from a Development package)

The whole film is **one continuous recording of one session** (menu, Direct, the Fast Cut match, then a second match if needed), 4 to 8 minutes, cut afterwards. That keeps
the sound continuous and keeps every frame Shipping. Who films: **the operator, on the package, by hand** (the human must hit, brake and press Esc; posted keys cannot
aim the stick in Shipping, script-v2). Device B in an editor session: unproven and not tonight (the GPU belongs to the engine lane); an editor session is also not Shipping.

| # | length | what must happen | how to get it | likely takes | fallback |
|---|---|---|---|---|---|
| 1 | 2.5 s | the slate menu, still, then the cursor to CHOOSE YOUR CUT | launch the package with no `-cut=`; record from the first frame of the menu | 1 | none needed |
| 2 | 2.5 s | the twelve cards and six cuts read; the cursor crosses BRUISERS and SHOTGUN | CHOOSE YOUR CUT, move the mouse slowly over two rows, BACK | 2 | hold the first frame and push in 5 % in the edit |
| 3 | 4.0 s | "make it faster" in the sentence box, ROLL, the Fast Cut comes back (clay, 5 v 5, no whistle, speed 14.5) | DIRECT A CUT, type the sentence, ROLL. With no key the screen offers the same sentence as a chip ("OR PICK A SENTENCE THE DESIGNER ALREADY SHOT") | 2 | click the chip "make it faster"; if the Direct screen fails, PLAY NOW (the regular cut) and drop A2 |
| 4 | 2.0 s | ten figures on clay, the drop | the first faceoff of the match | 1 (free) | any later faceoff |
| 5 | 5.5 s | the human flattens an oncoming carrier: closing speed head-on, right mouse button (CHECK) at contact; the hold; the victim slides | play the Fast Cut; skate at the orange carrier as it comes up the ice; check at the last moment. A knockdown needs impulse over stability (VISION law 1): head-on at full speed is the biggest hit | 3 to 6 tries over one or two matches | a bot flattening the human (happens unprompted at pro: `20261113-164934-B-sk18-human-knocked-down.jpg`); A3 still fits |
| 6 | 4.5 s | the figure lies, breathes and gets up; play goes on | the same take as 5 (a knockdown is 60 to 150 frames down, then the get-up) | free with 5 | any knockdown in the session |
| 7 | 5.5 s | the human carries at a defender, brakes hard, cuts, bursts away | with the puck, skate at the nearest defender, press against the travel (S against W) at about 8 m, cut 60 degrees, accelerate. The spray reads small (B40, SK24): the shot sells the pose and the change of speed, not snow | 3 to 5 | swap to the bots' pass play with the fallback line F4 "They pass now." (recorded) |
| 8 | 4.5 s | Esc, DIRECTOR'S CALL, ICE FRICTION dragged to its minimum, APPLY; the scoreboard names the new rule | once per period only: do it early in period 1 so period 2 is the spare | 1 to 2 (one try per period) | SKATER SPEED to its maximum, with fallback line F6 "Top speed. Good luck." (recorded) |
| 9 | 7.0 s | on glass ice: long glides, a check sends one into the boards | keep playing after APPLY; check near the boards | 2 to 3 chances in the period | any check on glass ice; a glide alone with A6 |
| 10 | 3.5 s | a blue goal: the shot, the lamp, the net, the score turns | the operator scores, or a blue bot does | over the session | a goal by orange is NOT a substitute (A7 brags) |
| 11 | 3.5 s | the banner and the banner line | generated in the edit from `devlog/shots/20261113-205819-B-br4-banner-1920.png` (1920 x 1080; also `C:\Users\kazda\jam\brand-lab\out\final\banner.png`, same size) | 0 | none |

Capture settings for that session: 1920 x 1080 window or fullscreen, 60 fps, the game's commentator OFF (Settings, VOICE), music and effects at their defaults (the
operator's mix of 13 Nov). Record a second pass with the commentator ON: its lines are a fallback narration and its captions are on screen either way (see Not verified).

#### Storyboard page
`C:\Users\kazda\jam\trailer-lab\storyboard.html`: eleven rows, each with the nearest existing still (file links into `devlog\shots`, two from `docs\experiments\img\a144` and one
from `submission\shots` because they are newer than any menu still in `devlog\shots`; the older `devlog\shots` menu stills are linked beside them), what must happen, the
sound, the narration line with its slot and measured length, and a running timecode. The rough read plays at the top and lights the current row; a click on a row jumps
there. Plain HTML, no outside resources. The same rows as a sheet: `C:\Users\kazda\jam\thirdjam\devlog\shots\20261114-015039-B-trailer-storyboard-A.png` (caption beside it).
An animatic of the same: `C:\Users\kazda\jam\trailer-lab\read\animatic-A.mp4` (45.0 s, 1280 x 720, 30 fps, stills held, the read under them, every tile marked PLACEHOLDER STILL).

#### The rough read
How: the existing pipeline, unchanged: `audio-lab/vox2/lib.mjs` `tts()` calling `eleven-tools/eleven.mjs`, which reads the key itself; voice C1 from
`audio-lab/commentators/saved.json`; my driver `C:\Users\kazda\jam\trailer-lab\read\read.mjs` (a 380-credit stop, a refusal of any line holding the title, a team or a place,
the bank's processing from `post.mjs`: mono 48 kHz, trim at 0.001, 2 / 10 ms fades, -18 LUFS capped at -1.5 dBTP). Two takes a line: "game" = the bank's own settings (AU10 B1:
stability 0.18, style 0.85, speed 1.15, closing "!"), "dry" = stability 0.40, style 0.45, speed 1.05, closing ".". Texts sent: exactly the seven lines below and the two
fallbacks; nothing else.

Measured (seconds of trimmed audio; the "game" take is the one used):

| line | text | game s | dry s | first slot | overran? | fixed slot |
|---|---|---|---|---|---|---|
| A1 | Pick your cut. | 0.81 | 0.77 | 2.5-4.5 | no | 2.6-4.6 |
| A2 | Tell it what you want. | 1.40 | 1.48 | 6.0-8.5 | no | same |
| A3 | Full speed. Full price. | 2.12 | 1.86 | 14.0-16.0 | **yes (game)** | 14.0-16.5 |
| A4 | Stop. Cut. Gone. | 2.00 | 2.29 | 22.5-25.0 | no | same |
| A5 | Once a period, you change the rules. | 2.01 | 2.15 | 27.0-30.0 | no | same |
| A6 | Glass ice. Good luck. | 2.25 | 1.49 | 34.5-36.5 | **yes (game)** | 34.5-37.0 |
| A7 | That's the cut. | 1.90 | 1.72 | 39.5-41.0 | **yes (both)** | 39.2-41.2 |
| F4 (fallback for 7) | They pass now. | 1.42 | | 22.5-25.0 | no | |
| F6 (fallback for 9) | Top speed. Good luck. | 1.95 | | 34.5-37.0 | no | |

The three overruns were fixed by widening the slot into air that was already there (no line was cut); every line now ends inside its slot and every slot holds at most 2.33
words a second. The full read: `C:\Users\kazda\jam\trailer-lab\read\rough-read-A.wav` (45.000 s, -19.8 LUFS integrated, true peak -2.1 dBFS): the seven "game" takes at their
times, the game's match music (`audio-lab/ready/MUS_MatchLoop_01.wav`, the source of `MUS_MatchLoop_01`) ducked under the voice, and **stand-ins** for the game's sounds
(clicks, clapper, body check, carve, rule-change sting, slap shot, goal crowd from `audio-lab/ready` and `audio-lab/crowd`: they mark where the capture's own sound must land;
in the film they come from the capture). Voice only: `rough-read-A-voice.wav`. Page: `C:\Users\kazda\jam\trailer-lab\read\listen.html` (every take, both reads, the animatic).
The WAVs: `C:\Users\kazda\jam\trailer-lab\read\wav\` (16 files), the raw takes `read\mp3\`, the request ledger `read\ledger.jsonl` (16 rows, 145 credits by header, no key in it).

#### Filming recipe for device B (written, not run)
What is installed (looked up, nothing recorded): ffmpeg 9.0.2 full build (winget, `...\Gyan.FFmpeg...\ffmpeg-9.0.2-full_build\bin\ffmpeg.exe`) with `gdigrab`, `ddagrab` and
`gfxcapture` (window capture) and the encoders libx264 (CPU), h264_mf, h264_nvenc and hevc_nvenc; the GPU is an NVIDIA RTX Spark N1X, so A's NVENC route may work here
(not tried: the GPU is the engine lane's tonight). **Audio is the gap: ffmpeg's DirectShow list on this machine has only the microphone ("Microphone Array on SoundWire
Device (Cirrus Logic XU)") and the webcam, and this build has no WASAPI loopback input, so ffmpeg alone cannot record the game's sound.** No OBS or ShareX is installed.
Installed and able to: Xbox Game Bar (`Microsoft.XboxGamingOverlay` 7.326.11042.0; `GameDVR_Enabled` = 1) and the Snipping Tool (11.2608) both record a window or screen
with system audio. `tools/media/take.ps1` (A88) records video only (PrintWindow into NVENC) and posts keys; it cannot carry sound.

Recipe (the operator at device B, on the H27 Shipping package):
1. Settings, Gaming, Captures: "Video frame rate" 60 fps, "Video quality" High, "Capture audio when recording a game" on, game audio only (not the microphone); "Record
   what happened" off. Files land in `%USERPROFILE%\Videos\Captures` (Settings shows the folder).
2. Windows sound output: the speakers or headphones the game plays on, at a fixed volume; no other sound playing (a notification lands in the take). Focus assist on.
3. Start the package at 1920 x 1080 (fullscreen, or windowed at exactly 1920 x 1080 client area); Settings in the game: VOICE off for pass 1.
4. On the main menu press Win+Alt+R (start), play the session in the shot list's order, press Win+Alt+R again (stop). One file per session. Then pass 2 with VOICE on, if time.
5. Copy each file to `C:\Users\kazda\jam\trailer-lab\takes\<yyyymmdd-hhmm>-<session>.mp4` (outside the repository: about 100 MB a minute) and write one line per
   take in `C:\Users\kazda\jam\trailer-lab\takes\takes.log` (time, cut, what happened at which minute).
6. Check every take: `node C:\Users\kazda\jam\trailer-lab\check-take.mjs <file>`: it fails a take below 1920 x 1080, not at 60 fps, with no audio or a silent one (under
   -50 LUFS), with a gap over 50 ms between frames, or with more than 2 % repeated frames (a stalled capture). Read the repeated-frame figure only on play: menus and the
   hit-stop repeat frames on purpose. Then look at three stills of each kept moment: `ffmpeg -ss <s> -i <file> -frames:v 1 <out>.png`.
7. If Game Bar refuses (it can on some apps) or its file fails the check: Snipping Tool, Record, select the game window, sound on. If both fail, the fallback is A's silent
   NVENC take plus the game's cooked music and sounds laid in the edit (script-v2's own fallback), and the claim "sound from the game" is only true of those cooked files.
Tested: the checker on two files: the animatic (FAIL: 1280 x 720, 30 fps, 95.6 % repeated, gaps 66.7 ms: as it should) and `submission/video/hokej-v1.mp4` with `--fps30`
(FAIL: audio silent at -70 LUFS, 40.4 % repeated frames over the whole film, menus and end card included).

### Evolution 3 (partial, done before the hand-back)
- **A 30 s cut of the same film**: shots 1 (0-2), 2 (2-4.5, A1 at 2.2), 5 (4.5-9.5, A3 at 7.0, the hit at 5.5), 6 (9.5-12), 8 (12-16, A5 at 12.5), 9 (16-21, A6 at 18.0),
  10 (21-26, A7 at 23.5), 11 (26-30). Theme at 0.0. 21 words in 12 s of slots; 22.6 s of the voice track under -50 dBFS (75 %). Read: `C:\Users\kazda\jam\trailer-lab\read\rough-read-A30.wav`
  (30.000 s) and `rough-read-A30-voice.wav`; plan `read\plan-A30.json`. It drops the sentence (shot 3) and the stop (shot 7): the theme then rests on the cards and the Call.
- **A second voice (proposal, nothing generated)**: the lab already holds a director's voice, two readers ("Bill" and "Liam"): `audio-lab/ready/VO_Director_*_Action.wav`,
  `_Cut.wav`, `_CutPrintIt.wav`, `_DirectorsCall.wav`, `_ThatsAWrap.wav`, `_Retake.wav` and six more. They are not in the game (no ledger row). Proposal: one word each at three
  places: "Action!" on the drop (9.0), "Cut!" on APPLY (30.2), "Cut, print it." over the end card (42.0). That gives the film a director's voice beside the commentator at no
  cost, but claims a voice the game does not have: the Director decides (section 6).
- **Music plan**: the game has one music asset, `Game/Plugins/JamAssets/Content/Audio/Music/MUS_MatchLoop_01` (ledger: ai-generated, ElevenLabs music, lane N3 loop edit),
  played by `MS_music_match`, ducked to 0.45 under the commentator by `SCM_DuckMusic`. The film uses it from the capture; for the edit's clean bed the source is
  `C:\Users\kazda\jam\audio-lab\ready\MUS_MatchLoop_01.wav` (fade in 0.5 s, out over the end card). `MUS_MenuTheme_01.wav` exists in `audio-lab/ready` but has no ledger row:
  it is not the game's music and stays out unless it ships.

## 3. Not verified
- I listened to nothing: every judgment of the read is a measured length, word count and loudness. Whether the "game" or the "dry" takes sound better is the operator's ear.
  A7 "That's the cut!" game take is 1.90 s for 3 words while only 1.1 s of it is above -50 dBFS: a long tail or pause I did not hear.
- That the offline Direct screen accepts a typed "make it faster" without a key (the chip is the safe route); that the Fast Cut loads on clay in the package (its cartridge says
  `p3_clay` and `penalties: false`).
- That the Director's Call card still looks like `submission/shots/08-directors-call.png` on clay in H27 (that still is an older dark-tier build), and that the scoreboard
  names the rule after APPLY (the card says it does).
- That VOICE off also hides the commentator's caption bubbles (seen on screen in `qa3-clay-play-4`): if not, the narration and an unrelated caption share the screen.
- That Game Bar records this package at 60 fps with its sound on this machine; that the GPU holds 1920 x 1080 at 60 with a recorder running (A-to-B-0022 already said: unproven).
- That a head-on knockdown by the human is reachable in a few tries in the Fast Cut at pro (VISION law 1 says it is the biggest hit; nobody has filmed one in Shipping).
- The ranking of the three scenarios is my judgment, not a measurement.

## 4. For the engine lane
Nothing needed for this film: it uses only takes that already fire (knockdown, lie loop, get-up, stop, board fall) and asks the engine for no new take. One wish that would
make shot 7 read: the brake's spray visible at the game camera's size (B40 and SK24 report it reads small or not at all). No take names, lengths or triggers change.

## 5. For the other device
- **A frame recorder or replay camera** (`docs/experiments/CAMERA.md` "Not built") would give the film a low or close view of the hit; not asked for tonight.
- Nothing else is required. Useful, if A's Director agrees: in the Shipping package a way to start a match on a named Designer cut from the command line (`-cut=` with a
  cached cut's slug) so a missed shot 5 can be retaken without the menu; and the Director's Call card retaken on clay for the store stills.

## 6. Decisions for the operator (and the Director, who owns the copy)
1. Which scenario: A (my pick), B, or C.
2. The narration lines A1 to A7 (and F4, F6): proposals in the commentator's voice. VOICE.md rule 7 keeps film words off the ice; the trailer's lines use "cut" twice (A1, A7)
   and A4 "Cut" as the hockey move: allowed for a trailer, or rewrite?
3. "game" takes (the bank's hyped delivery) or "dry" takes (calmer): listen on `C:\Users\kazda\jam\trailer-lab\read\listen.html`.
4. The second voice: use the existing director voice ("Action!", "Cut!", "Cut, print it.") although it is not in the game, or keep the commentator alone?
5. R-0054 point 4: may a split frame of two Development drills appear inside a film whose other shots are Shipping, or is that itself "cut against Shipping"? (Matters only if
   shots 5 or 7 fail in Shipping.)
6. The engine logo on the end card: the rule allows it; nobody has the logo file in the repository. Leave it out unless he wants it.
7. Who films and where: the operator on his machine (if it has an audio device and Game Bar) or at device B with this recipe.

## 7. Next three evolutions (each with its proving number)
1. **Film it**: the operator's first session on H27 with this recipe; proof: `check-take.mjs` PASS (1920 x 1080, 60 fps, audio above -50 LUFS, under 2 % repeated frames on
   play) and a log line per shot 1 to 11 marking found / not found; target 11 of 11 found in two sessions.
2. **The rough cut on the real takes**: lay the read on the real footage in the edit (ffmpeg, CPU); proof: every narration line inside its shot (0 overruns), the film 45.0 s, the
   game's hit and goal sounds at their beats within 2 frames of the picture, -16 to -14 LUFS integrated for the upload.
3. **The five store stills** from the same session (proposal: the slate menu, the head-on hit's held frame, the figure lying with the crowd behind, the Director's Call over
   the clay match, the goal lamp): proof: five PNGs at 1920 x 1080 taken from Shipping frames, each with a caption, each passing the R-0054 point 2 check (clay figures on clay
   ice visible).

Leftovers: nothing running. Files written: `status/TRAILER-report.md` (this), `devlog/shots/20261114-015039-B-trailer-storyboard-A.png` and `.txt`; everything else under
`C:\Users\kazda\jam\trailer-lab\`.


---

# Part two (14 Nov 02:00 to 02:20): the edit as a command, scenario D, scenario C made real

## 1. Summary
All three of the coordinator's evolutions are done. **4:** `C:\Users\kazda\jam\trailer-lab\edit\edit.mjs` cuts a film from an edit list (JSON) on the CPU: picture and game
sound per shot, the narration, the game's music, ducking by set amounts while a line plays, loudness for upload, banner cards, then `check-take.mjs` on the result. Proved on
stand-ins: A at 45 s, A at 30 s, C at 45 s and D at 45 s all **PASS** (1920 x 1080 at 60, exact length, 0 narration overruns, -16.0 to -16.2 LUFS, -1.9 dBTP), and on real
video sources (a test cut from `hokej-v1.mp4` with a half-speed shot: the cut works; the checker rightly fails it for 41.2 % repeated frames because that source is 30 fps).
**5:** scenario D, the editor film, written shot by shot with camera, lens, speed and staging, ranked honestly below A, with three legal hybrids. **6:** scenario C to A's depth,
with two new lines and one line from the game's own bank. `storyboard.html` now has tabs A, C, D. Credits: **155 of 400** in total (two new requests this part, 10 credits).
Not done: nothing of the three; nothing filmed; the GPU estimate for D is an estimate, not a measurement.

## 2. Evolutions

### Evolution 4: the edit as a command
Files: `C:\Users\kazda\jam\trailer-lab\edit\edit.mjs` (the command), `edit\make-edl.mjs` (writes A45/A30 from the storyboard), `C:\Users\kazda\jam\trailer-lab\scenarios-CD.mjs`
(writes C45/D45), the edit lists `edit\A45.json`, `edit\A30.json`, `edit\C45.json`, `edit\D45.json`, the test `edit\T-video.json`, `edit\film-sheet.mjs` (a frame per shot),
`edit\SATURDAY.txt`. Outputs in `C:\Users\kazda\jam\trailer-lab\edit\out\` (`A45.mp4`, `A30.mp4`, `C45.mp4`, `D45.mp4`, `T-video.mp4`, each with `<name>-result.json`).

The edit list, per shot: `src` (a take or a still), `in`/`out` (seconds in the source), `speed` (0.5 = half speed; out - in = len x speed), `at`/`len` (the slot on the film's
timeline), `push` (a still's zoom-in), `text`/`text2` (card lines), `fade_in`/`fade_out`. Per film: `voice` (wav, at, slot_end), `sound` (extra sounds laid on the game track:
tonight's stand-ins), `music`, `duck`, `loudness`, `out`. The command refuses a list whose shots leave a gap, overlap, miss a source or do not add up to the film's length.

What it does: (1) each shot to a 1920 x 1080, 60 fps mezzanine (x264 CRF 17 + PCM stereo) with its own game sound (a take's audio, retimed with the picture; silence for a
still), the still pushed in from a 2x scale so the motion is smooth (push 2.5 % a second, at least 8 %); (2) concatenated; (3) the mix: game track and music each ducked
**game -8 dB, music -10 dB while a line plays, 0.15 s down before the line, 0.40 s back up after it**; the narration bus compressed 4:1 above -24 dBFS (5 / 150 ms); music
bed at -14 dB, 0.5 s fade in, 2.5 s fade out; (4) loudness **-16 LUFS integrated, true-peak ceiling -1.0 dBTP** (gain, then a limiter at -2.0 dBFS to leave room for the AAC
encode, repeated until within 0.3 LU); (5) AAC 320 kb/s mux; (6) EBU R128 measure, `check-take.mjs` at 60 fps, and an overrun check of every line against its slot end and the
film's end. I first aimed at -14 LUFS: the stand-in voice needed about 7 dB of limiting to get there (measured: 13.7 dB of gain for 6.8 dB of loudness), so the target is -16
(the AES streaming range is -16 to -14; set `loudness.I` to -14 for a hotter mix once the real game sound is in).

| film | length | format | loudness | true peak | overruns | check | repeated frames |
|---|---|---|---|---|---|---|---|
| A45 | 45.000 s | 1920x1080 @ 60 | -16.1 LUFS | -1.9 dBTP | 0 of 7 | PASS | under 0.5 % (counter reads -0.4) |
| A30 | 30.000 s | 1920x1080 @ 60 | -16.1 LUFS | -1.9 dBTP | 0 of 5 | PASS | under 0.5 % |
| C45 | 45.000 s | 1920x1080 @ 60 | -16.0 LUFS | -1.9 dBTP | 0 of 6 | PASS | under 0.5 % |
| D45 | 45.000 s | 1920x1080 @ 60 | -16.2 LUFS | -1.9 dBTP | 0 of 6 | PASS | |
| T-video (real 30 fps video, a half-speed shot) | 8.000 s | 1920x1080 @ 60.01 | -16.1 LUFS | -2.0 dBTP | 0 of 1 | FAIL: 41.2 % repeated (correct: 30 fps source) | |

Time on this machine: A30 about 75 s, A45 about 105 s (CPU only). Still: `C:\Users\kazda\jam\thirdjam\devlog\shots\20261114-021528-B-trailer-edit-A45.png` (a frame from the
middle of each shot of A45: the push-in, the end card with the banner line drawn).

**For whoever cuts on Saturday (also in `C:\Users\kazda\jam\trailer-lab\edit\SATURDAY.txt`):**
1. Drop the takes in `C:\Users\kazda\jam\trailer-lab\takes\`; in `C:\Users\kazda\jam\trailer-lab\edit\A45.json` (or `A30.json`, `C45.json`) set each shot's `src` to its take and `in`/`out` to the seconds in that take.
2. Run `node C:\Users\kazda\jam\trailer-lab\edit\edit.mjs C:\Users\kazda\jam\trailer-lab\edit\A45.json`: the film is `edit\out\A45.mp4`, and the last line must read length_ok true, 1920x1080@60, overruns 0, check PASS.

Notes for Saturday: the stand-in `sound` rows must be **removed** from the list once real takes carry their own game sound (they are only there to mark the beats); a 30 fps
take fails the checker by design (Game Bar must be set to 60); the banner line sits bottom-centre over the banner's art and touches the gouged letters: move it with `text`
position when the operator sees it.

### Evolution 5: scenario D, "The editor film"
**Stills looked at for D:** `devlog/shots/20261113-111726-B-b28-hero-low.png` (low camera, caption: x -17, y -7, z 2.2 m, pitch -10, yaw 25: the barn and its lamps read
as a place), `20261113-232906-B-sk23-board-fall-wall.png` (fixed close camera about 1 m high along the boards: the board fall reads at once), `20261114-004259-B-sk24-lying-loop-test.png`
(test figures lying, low side view), `20261114-001015-B-b40-ability-kinds.png` (close behind the human: the ability effects read small even close). Caption only: `20261113-103104-B-b27-hero-low`
(the same camera as B28). In the D sheet I also saw `20261113-221554-B-sk22-stop-human.png` (the stop from above).

Pitch: the key feature filmed the way a film would: the hit at ice level, the contact frames slowed, the figure lying close, the stop's spray into the lens, the boards taking a
body, the lamp from behind the net. **All footage from device B editor sessions (play-in-editor, Development, clay tier), the menu and the Director's Call included, so
nothing in it is Shipping and nothing is mixed.** Distances are metres relative to the subject figure (1.7 m tall at game scale: 110 px at 0.65 px a cm); x along its travel,
y to its left, z above the ice; fov is horizontal (the game camera's is 70).

| # | time (s) | shot | camera | fov | speed | engine lane stages | narration |
|---|---|---|---|---|---|---|---|
| 1 | 0.0-2.5 | the slate menu, cursor to CHOOSE YOUR CUT | the menu's own view, window captured | - | 1.0 | Menu map in play-in-editor; `jam.menuscreen main` then `cuts` | A1 "Pick your cut." (0.3) |
| 2 | 2.5-5.5 | low wide: ten figures at centre ice under the lamps | B28's camera (world x -17, y -7, z 2.2, pitch -10, yaw 25), dolly 1.0 m in over the shot | 50 | 1.0 | hockey.regular on clay, the faceoff, `jam.ScriptHuman 1`; the drop at 5.0 | none |
| 3 | 5.5-9.0 | tracking beside the blue carrier, the defender growing ahead | y -3.0, z 0.6, 15 degrees ahead of square, follows the carrier's position (not its rotation) | 40 | 1.0 | drill 21 with `jam.ctrl.CarryScript 0` (the straight charge into the defender) | none |
| 4 | 9.0-13.0 | the head-on hit: real time in, the contact frames slowed, real time out as the victim slides | fixed, 4.0 m from the contact point, square to the travel, z 0.3, tilted up 5 degrees | 35 | 1.0, then 0.25 for 40 source frames from 6 frames before contact (about 2.7 s on screen), then 1.0 | drill 6 (the human carries into the nearest standing pro bot) or drill 21 CarryScript 0; a knockdown of 130 frames or more (Fall_Heavy, the turn in the air) | none (the hit carries it) |
| 5 | 13.0-17.5 | close on the figure lying, breathing; it springs up into frame | 1.2 m from its chest, z 0.25, level | 30 | 1.0 | `SetKnockedDown(150)` on a test figure (SK24 job 3): Lie_Loop_Front, the 36-frame get-up from frame 115 | A3 "Full speed. Full price." (13.0) |
| 6 | 17.5-21.5 | the stop: skates across toward the lens, the spray at it, then away | 3.0 m ahead on its line, z 0.2, aimed at the skates | 35 | 0.5 for the 15 frames of the stop, 1.0 after | the human's stop by `SetBraking` at 10 m/s (SK24 job 2), NS_brake on | A4 "Stop. Cut. Gone." (18.0) |
| 7 | 21.5-25.5 | a figure driven into the boards, sitting down against them | 2.5 m off the boards looking along them, z 1.0 | 40 | 1.0 (0.5 for the 12 frames of impact) | `SetKnockedDown` + `PlayBoardHit` in one frame 66 cm from the boards (SK23 row 1) | none |
| 8 | 25.5-30.0 | Esc, DIRECTOR'S CALL over the paused clay match, ICE FRICTION to minimum, APPLY | the game camera (the card is UI) | 70 | 1.0 | play-in-editor of hockey.regular; Esc and the slider by hand | A5 "Once a period, you change the rules." (26.0) |
| 9 | 30.0-35.0 | from above: long glides on glass ice, one slides into the boards | crane 8 m above centre ice, pitch -60, a slow 10-degree yaw | 50 | 1.0 | after APPLY, `jam.ScriptHuman 1`, any check near the boards | A6 "Glass ice. Good luck." (31.5) |
| 10 | 35.0-40.0 | from behind the net, low: the puck in, the lamp red over the camera, the net shakes | 2.0 m behind the goal line through the net, z 0.5, aimed at the slot | 45 | 1.0 | drill 10 (shots from 10 m) or `NS_goal` staged on the event bus (B41) | A7 "That's the cut." (37.5) |
| 11 | 40.0-45.0 | the banner and the banner line | - | - | - | generated in the edit | none |

Narration: the A lines re-timed, no new credits: 24 words in 14.5 s of slots (1.66 a second, at most 2.33 in a slot); 36.2 s of the voice track below -50 dBFS. Read:
`C:\Users\kazda\jam\trailer-lab\read\rough-read-D.wav`; stand-in film `edit\out\D45.mp4`; still `C:\Users\kazda\jam\thirdjam\devlog\shots\20261114-021528-B-trailer-scenario-D.png`.

**How to film it, two routes:** (a) real time: play-in-editor at 1920 x 1080 recorded by Game Bar, with `slomo 0.25` / `slomo 0.5` typed for the slow beats (the world slows,
the recording stays 60 fps, no retime needed); the game's sound is in the recording but slowed with the world in the slow beats. (b) frame-stepped: the sim is
deterministic (same cut, seed and frames = same play), so each shot is replayed with a fixed camera and a screenshot is taken at every sim frame from a tick callback (the way
SK23 and the camera lane's harness already capture), then assembled at 60 fps by `edit.mjs` (a numbered image sequence needs one small addition to it: an `image2` source);
no frame can drop however slow the GPU, but there is no live sound: the game's own sound files are laid at the logged frames in the edit, and the page must not claim "audio
captured from the game" for D. Route (b) is the safer on device B's GPU.

**GPU time, an estimate (not measured):** an editor start and the first map load about 10 minutes; per shot about 8 minutes of staging (camera placed, the trigger fired,
the take checked) and 2 to 4 takes; 9 engine shots: about 1 h 45 min to 2 h 15 min in one session, or two sessions of about 70 minutes. Route (b) adds the capture time per
frame, unmeasured on this GPU: if a 1080 screenshot costs 0.3 s, 45 s of film (2,700 frames) with two takes is about 30 minutes more. It needs the one editor slot device B
has (one editor at a time).

**What the engine lane must provide (beyond triggers it already has):** a camera it can place by numbers and make the view (a cine camera actor spawned by script and set as
the player's view target), one following a skater's position (shot 3), and the world's time dilation on demand. The triggers named above (`SetKnockedDown`, `PlayBoardHit`,
`SetBraking`, drills 6 / 10 / 21, `NS_goal` on the event bus) are the ones its lanes used this week.

**Ranked against A for a judge (1 to 5, my judgment):**

| | theme | gameplay | look | sound | ambition | total | risk |
|---|---|---|---|---|---|---|---|
| A (Shipping, the operator) | 5 | 4 | 4 | 4 | 5 | **22** | shots 5 and 7 need practice |
| D (editor) | 3 | 3 | 5 | 3 | 4 | 18 | the GPU (three hangs on 13 Nov); not what the judge's download looks like; sound laid, not captured (route b) |

A wins: the operator asked for a gameplay trailer, and a judge scores what he will play; D's beauty is a camera the game does not have, and a judge who downloads the zip
meets the high game camera. D's best use is the store page and a short separate clip.

**Hybrids that are legal under R-0054 point 4 (development footage never cut against shipped footage):**
1. **One film entirely in the editor:** D as written (the menu and the Call filmed in the same sessions). Legal; but then nothing in the film is the shipped look (R-0054 point 2 asks the page's clay claims to be backed by package stills, so the page needs Shipping stills anyway).
2. **A as filmed, D's shots only as stills** for the store page (the hit at ice level, the figure lying close, the lamp from behind the net), each captioned as an editor capture. Legal and cheap: 3 to 5 stills, about 40 minutes of GPU.
3. **Two separate videos:** A as the submission video, D as a second clip on the page (a "how it moves" reel). Legal if the page lets two videos stand apart; the form takes one video link.
Not legal as I read the ruling: D's shots cut into A, or a split frame of editor and Shipping.

### Evolution 6: scenario C made real, the fallback film
Built so that **any one recorded match** (Shipping, the operator) can become a film. Default cut: season take 1 "The Read-Through" (read from its cartridge: 5 v 5, rookie
bots, clay, first to 2, no whistle), reached from the menu by PLAY THE SEASON; any other match works by editing shot 1's line.

| # | time (s) | length | what must happen | how / likely takes | fallback | narration |
|---|---|---|---|---|---|---|
| 1 | 0.0-3.0 | 3.0 | the menu, PLAY THE SEASON: TAKE 1 OF 8, Return | record from the menu; 1 | PLAY NOW (drop the line or keep it: "Take one" still reads) | C1 "Take one." (0.8; new) |
| 2 | 3.0-5.0 | 2.0 | the MATCH 1 card and the drop, ten figures | free | any faceoff | none |
| 3 | 5.0-10.0 | 5.0 | the bots move the puck: carrier, outlet, a pass that lands | happens in most minutes at rookie; pick the cleanest | any carry with a teammate in frame | F4 "They pass now." (6.5) |
| 4 | 10.0-15.0 | 5.0 | the human flattens a carrier (contact about 11.0) | right mouse button, head-on; 2 to 4 tries in a match | any knockdown by blue | A3 "Full speed. Full price." (12.3) |
| 5 | 15.0-21.0 | 6.0 | a bot flattens the human; it lies and gets up | at rookie bots rarely hit (VOICE glossary): may not happen; at pro it does | a knockdown of anyone, with the line kept | BK1 "Back up, quickly now." (17.5; the game's own bank line `knockdown.human.01`, no credits) |
| 6 | 21.0-24.0 | 3.0 | stop and go past a defender | as A's shot 7; 2 to 4 tries | cut (shot 5 or 7 absorbs the 3 s) | none |
| 7 | 24.0-29.0 | 5.0 | Esc, the Director's Call, one slider, APPLY | once a period | none needed (always reachable) | A5 "Once a period, you change the rules." (25.0) |
| 8 | 29.0-36.0 | 7.0 | the deciding goal: lamp, net, score | the operator scores; first to 2 means the match ends on it | any blue goal | C8 "That's a take." (33.5; new) |
| 9 | 36.0-41.5 | 5.5 | the end card over the rink | free if he wins (A144: the loss card reads only REMATCH?) | the scoreboard after the goal | none |
| 10 | 41.5-45.0 | 3.5 | the banner and the banner line | generated | | none |

New lines (game settings, 10 credits): C1 "Take one!" 1.09 s; C8 "That's a take!" 1.47 s. Narration: 23 words in 14.5 s of slots (1.59 a second, at most 2.33 in a slot);
37.1 s of the voice track below -50 dBFS (82 %). Read `C:\Users\kazda\jam\trailer-lab\read\rough-read-C.wav`; stand-in film `edit\out\C45.mp4` (PASS, -16.0 LUFS, 0 overruns);
still `C:\Users\kazda\jam\thirdjam\devlog\shots\20261114-021528-B-trailer-scenario-C.png`. Season take 1 plays rookie bots: shot 5 (the human flattened) is the one most likely to
be missing; a match at pro (PLAY NOW, the regular cut) has it unprompted. If the operator records one pro match of the regular cut, C needs only shot 1's line changed.

### Storyboard
`C:\Users\kazda\jam\trailer-lab\storyboard.html` now has tabs A, C, D (D's rows add camera, lens, speed and staging); each tab plays its own read with the current shot lit and
links its stand-in film. `C:\Users\kazda\jam\trailer-lab\read\listen.html` has every take, the bank line and all five reads.

## 3. Not verified (this part)
- I watched none of the cut films in a player: their numbers are measured (format, length, loudness, overruns, repeated frames) and I looked at one frame per shot of A45 and D45.
- The duck amounts (-8 game, -10 music) and the -16 LUFS target are chosen, not heard; the real game sound will change the balance.
- Scenario D: every camera number is a proposal from the stills' cameras, not placed in an editor; the GPU time is an estimate; route (b) needs a small image-sequence addition to `edit.mjs` that I have not written or tested.
- Scenario C: that season take 1's rookie bots ever flatten the human in a match.
- The repeated-frames counter reads slightly negative on the stand-in films (-0.3 to -0.4 %): its frame and showinfo counts differ by a few frames; it still caught the 30 fps source.

## 4. For the engine lane
For D only (Saturday, if the operator chooses D or hybrid 2): a scripted cine camera placed by numbers as the view target, a follow mode on one skater's position, time
dilation on demand, and the triggers named in D's table. Nothing for A or C.

## 5. For the other device
Nothing new. If the operator wants hybrid 3 (two videos), the page (`submission/page.md`) needs a place for a second clip: the Director's.

## 6. Decisions for the operator (and the Director)
1. A, C or D as the submission film (I keep A first, C as the fallback).
2. D at all: none, hybrid 2 (3 to 5 store stills from an editor session, about 40 minutes of GPU), or hybrid 3 (a second clip); hybrid 1 (D as the only film) I would not choose.
3. Loudness: -16 LUFS (chosen, less limiting) or -14 (louder, crushes the voice on tonight's stand-ins).
4. Season take 1 (rookie) or the regular cut at pro as C's match.
5. The open decisions of part one stand (lines, delivery, second voice, R-0054 split frame, engine logo, who films).

## 7. Next three evolutions
1. **Film it and cut it the same hour**: the operator's session on H27, then `edit.mjs` on A45.json with real `src`/`in`/`out`; proof: the film PASS (format, 45.000 s), 0 overruns,
   and the game's own hit and goal within 2 frames of their stand-in beats (then the stand-in rows deleted).
2. **Image-sequence sources in `edit.mjs`** (for D's route b and for frame-stepped Shipping replays): proof: a 120-frame PNG sequence cut into a shot at 60 fps with 0 repeated frames.
3. **The five store stills** (part one's evolution 3), from Shipping frames, or hybrid 2's editor stills if he chooses it; proof: five 1920 x 1080 PNGs with captions, each showing clay figures on clay ice.

Leftovers: nothing running that I started. Files written in the repository this part: this section of `status/TRAILER-report.md`; `devlog/shots/20261114-021528-B-trailer-edit-A45.png`,
`-scenario-D.png`, `-scenario-C.png` and their `.txt` captions. Everything else under `C:\Users\kazda\jam\trailer-lab\`.


---

# Part three (14 Nov 02:20 to 02:30): image sequences in the edit, the capture brief for D's stills, the operator's page

## 1. Summary
All three are done. **7:** a shot's `src` in `edit.mjs` may now be a folder of numbered PNG or JPG frames, or an ffmpeg pattern, played at a stated `src_fps`. Proved on
a 120-frame sequence made on the CPU from an existing strip: 120 frames, 2.000 s, 0 repeated frames, check PASS. I also found and fixed the bug behind the checker's
slightly negative repeated-frame reading. **8:** `C:\Users\kazda\jam\trailer-lab\capture-D-stills.md` is a one-page brief an engine lane can run in one session of about 40
minutes: five store stills, each with camera, aim, lens, trigger, capture frame, what must and must not be in frame, and a caption proposal. **9:**
`C:\Users\kazda\jam\trailer-lab\OPERATOR.md` is the operator's two-minute page. No sound-service use this part (155 of 400 credits in total). Nothing was filmed or captured.

## 2. Evolutions

### Evolution 7: image-sequence sources
- `edit.mjs`: `src` may be a folder (the frames must share one prefix, one digit width and one extension; the start number is read from the first file; the command refuses
  a folder whose names do not match) or a pattern such as `.../frame_%04d.png` with an optional `start_number`. A sequence needs `src_fps`. `in`/`out` are seconds of the
  sequence; `speed` works as for video; a sequence shorter than `out` holds its last frame and says so. An empty `voice` list is now allowed.
- Proof: `C:\Users\kazda\jam\trailer-lab\edit\seqtest\frame_0001.png` to `frame_0120.png` (a 16:9 window sliding 36.5 px a frame along
  `devlog/shots/20261114-004300-B-sk24-lying-loop-play.png`, scaled to 1920 x 1080; every frame differs). Cut by `edit\T-seq.json` to `edit\out\T-seq.mp4`:
  **120 frames, 2.000 s, 0 repeated frames (kept 120 of 120), max gap 17 ms, 1920 x 1080, check PASS**. The stream's average rate reads 60.03 because of the MP4 time base's
  rounding over 2 s; every frame gap is 16 or 17 ms. A half-speed sequence shot (60 source frames shown over 120) is also cut correctly, and the checker fails it for
  repeated frames, as it should.
- Still: `C:\Users\kazda\jam\thirdjam\devlog\shots\20261114-022426-B-trailer-image-sequence.png` (frames 0, 24, 48, 72, 96 of the cut).
- **The negative counter, explained and fixed.** `check-take.mjs` counted frames by parsing ffprobe's `pts_time` lines with `Number()`. The first frame of every shot (a
  key frame with side data) is printed with a trailing comma (`2.000000,`), so it parsed as NaN and was dropped. The frame count came out one short per shot, and "kept
  frames / frames" went above 1. Now the frame count is the decoder's own (`-count_frames`, `nb_read_frames`), the kept count reads only showinfo's per-frame lines,
  and the time list uses `parseFloat`. Re-checked: A45 2,700 of 2,700 kept, A30 1,800 of 1,800, C45 2,700 of 2,700, D45 2,700 of 2,700: **0 % repeated**; the 30 fps test cut T-video 281 of 480 kept (41.46 %, FAIL, correct). The max gap now reads 17 ms
  (it read 34 ms when a shot's first frame was missing).

### Evolution 8: the capture brief for D's stills (hybrid 2)
`C:\Users\kazda\jam\trailer-lab\capture-D-stills.md`. In store-page order:
1. **The hit at ice level.** Drill 6; capture at the check event's frame + 1 (inside the hit-stop). Camera 4.0 m square to the travel, 0.3 m up, aimed at the contact point at 0.9 m, fov 35. Proposed caption: *The speed you bring is the hit you get.*
2. **The barn.** Match start; device B's hero camera (x -17, y -7, z 2.2, pitch -10, yaw 25), fov 50. Proposed caption: *Five clay skaters a side, in a timber barn.*
3. **Down, close.** `SetKnockedDown(150)`; capture at + 60. Camera 1.2 m beside the chest, 0.25 m up, fov 30. Proposed caption: *Everybody gets back up.*
4. **The boards.** `SetKnockedDown(110)` and `PlayBoardHit` together, 66 cm off the boards; capture at + 5 and + 24. Camera 2.5 m off the boards, 1.0 m up, fov 40. Proposed caption: *The boards hit back.*
5. **The lamp.** Drill 10 or `NS_goal` staged; capture at + 10. Camera 2.0 m behind the goal line, 0.5 m up, looking out through the net, fov 45. Proposed caption: *Score, and the barn answers.*

Fixed for all five:
- the clay tier;
- `HighResShot` at 1920 x 1080, then 3840 x 2160 of the same paused frame if the session allows (keep only the 1080 one if the poses differ);
- no HUD, no on-screen text, no annotations, no editor icons, and no human ring where a bot can be the subject.

Order and timing: 10 min to start, then 2, 1, 3, 4, 5 at about 4 to 8 min each, then the 4K pass of the best two. That order is also the priority if the session is cut short.
The page also has an optional separate short clip (about 15 s, about 30 more minutes of GPU), never cut into A.

### Evolution 9: the operator's page
`C:\Users\kazda\jam\trailer-lab\OPERATOR.md`. It has:
- the three films in one sentence each, my pick (A) and why;
- what filming A takes on Saturday: about 30 to 45 minutes of his time, Game Bar set once, the commentator's voice off, Win+Alt+R, and the play order with keys and likely tries (head-on hit 3 to 6 tries with the right mouse button, stop and go 3 to 5 with S then W, the Director's Call once per period with Esc, one blue goal);
- what D costs (about 2 h of device B's editor and GPU) and gives, and its cheapest use (five stills, about 40 minutes);
- five questions, each with a suggested answer: A with C as backup; the seven lines yes; the excited delivery (not heard by me); D as five store pictures only; he films A himself, on his machine if it records game sound, otherwise at device B.

## 3. Not verified
- `HighResShot` leaving out the HUD in this project's play-in-editor (standard engine behaviour; the brief asks the lane to check the first still).
- Every camera number in the brief; the 40-minute session (an estimate).
- The operator page's minute counts.
- `edit.mjs` on a real engine frame dump (tested only on frames made from a still).

## 4. For the engine lane
`capture-D-stills.md`, only if the operator picks hybrid 2. It needs a script-placed cine camera as the view target, `pause`, and `HighResShot`. The triggers are the ones device B's lanes used this week.

## 5. For the other device
Nothing. The five caption proposals and the seven lines are the Director's to rule on.

## 6. Decisions for the operator
The five questions on `OPERATOR.md`; part one and two's decisions are folded into them.

## 7. Next three evolutions (when footage exists)
1. Cut A from the operator's real takes. Proof: PASS at 45.000 s, 0 overruns, 0 % repeated on play, the real hit and goal sounds within 2 frames of their beats.
2. The five store stills, from Shipping frames for the page's clay claims, plus the D stills if chosen. Proof: five 1920 x 1080 PNGs with captions, and R-0054 point 2 met.
3. The 30 s cut from the same takes. Proof: PASS at 30.000 s with the theme on screen at 0.0 s.

Leftovers: nothing running that I started. Repository files this part: this section; `devlog/shots/20261114-022426-B-trailer-image-sequence.png` and `.txt`.
