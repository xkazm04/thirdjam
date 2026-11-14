# Boosts seen, and a goal is an event (lane A188, track F3 evolution 8, device A, 14 Nov)

The operator after package H31 (R-0064 points 6, 7, 9): "Special boosts are not seen on ice, not seen equipped on players"; "Puck's white outer ring for
visibility can be 50% thinner"; nice to have: "short celebration animation set after a goal, HUD middle screen large banner saying 'Goal'".
Presentation only: no sim state, no rule; the default match's hash is the same before and after (section 6).

## 1. Why boosts were not seen (measured on the build of 06:16, before any change)

Arsenal (`hockey.mode.arsenal`, seed 1, 5 v 5, pro bots, first to 3) played by the scripted human off screen (`tools/qa/screen/a188-stills.ps1`, 1280 x 720)
on clay, night and neon; then the same match with the probe (`jam.boost.Probe 60`) and `jam.boost.Beacon 0` (the look of before).

- **Where**: the pickups spawn only in Arsenal's `boost-lane` zone (70 x 14 m down the middle, `visible: false`), up to 4 speed and 4 precision at a time;
  they never despawn (taken only, by bots too). The camera follows the human, who lives along the boards: most pickups are 20 to 45 m off and low.
- **How big**: B's look is a 70 cm base ring with a small prop at 45 cm (film reel = speed, canister = precision). The visible parts' bounding box on screen
  (median at 720p): **60 px at 20 m, 26 px at 40 m, 22 px at 60 m** (x 1.5 at 1080p). The prop alone is about half of that; at 40 m it is a 12 to 15 px
  speck (`img/a188/before-clay-f01920.png`: the yellow reel at 43 m; the base is orange: it reads as the away team's colour, not as a boost).
- **How often missed**: the scripted human passed within 5 m of a pickup without taking it 4 times in 45 s (closest 0.93 m: the take radius is 0.5 m plus
  the skater's); he took none.
- **Held**: nothing on the skater. The effect boost of the pickup (speed 1.5x for 5 s, precision for 8 s) pulses the figure's emissive for its frames; the
  ability the pickup grants (shotgun, hip check, shield, slow puck) has no look at all. The HUD's plate said "F  SHOTGUN  ARMED" from the press until the
  shot fired: an armed shotgun waits for the next shot, so the window lane read "ARMED" after `ability used` as a fault.

## 2. On the ice: the beacon (`AJamPickupActor`, `jam.boost.Beacon 1`)

A turning shape by kind 1.25 m over the ice (0.8 m: **speed a cone pointing up, precision a cube on its corner**; heavy puck a flat disc, big puck a
ball, reverse a cube, custom a ball) on a **1 m pedestal of light** (0.2 m wide) with a **2.2 m ground ring** of 16 segments, all in the kind's colour
(`JamEffects::KindColour`: speed yellow, precision cyan) in A's own `/Game/Placeholders/M_Trim` (TeamColour, Emissive 5 / 1.6). Turns 120 degrees a
second and bobs 8 cm, by sim frame. **Low profile**: inside 1.6 m of any skater or the puck it sinks to 35 % of its height (a metre's blend), so it never
hides a skater or the puck, and a downed figure lying by it is not covered. The radar draws every pickup as a 7 px square in its colour (under the skaters).
Device B's prop by name: `/JamAssets/Props/SM_Boost_<kind>` (fitted to 0.8 m) replaces A's stand-in wherever it exists; `jam.boost.Scale` (1) sizes all.

Measured (same match, same frames, clay / night / neon): **106 px at 20 m, 45 to 51 px at 40 m, 26 px at 60 m** at 720p (x 1.5 at 1080p; bounding
box with the ground ring, against 60 / 26 / 22 for B's look). The scripted human still passed within 5 m without taking 2 to 6 times a match: he does
not chase boosts (a person does); the first ruling of whether the eye now finds them is a window's.

## 3. On the skater (`AJamBoostAura`, `jam.boost.Aura 1`)

While a skater holds an ability (the held kind; an armed kind after its last charge; a running shield) a ring of 12 bars of light turns round his waist
(0.62 m, at 0.95 m) in the ability's colour: shotgun crimson (a plain red read orange on clay), hip check magenta, shield ice white, slow puck lime. Armed or running: it turns three times
faster and breathes; a running shield is a wider ring of taller bars (0.85 m, 24 cm) round the chest. Hidden while he is down or in the box.
Device B's `SetBoost(FName Kind, bool bOn)` (or `SetBoostEquipped`, same inputs) is called by name on each change, signature-checked (the skater-visual
interface's own one-input `SetBoost(FName)` is the effect boost and is left as it was); when the figure takes it, A's ring stands down (`jam.boost.Aura 2`
keeps both). One `JAMBOOST equip` line per change.

The plate (lane A84's, above the radar, now 440 wide): **"F: SHOTGUN  READY  x3"**; the key is the ABILITY row of `UJamInputMap::BindingTableText` (the
controls lane fills it from the rebinding). An armed kind says what fires it: **NEXT SHOT** (shotgun), **NEXT HIT** (hip check), **NEXT PASS** (slow puck); a running shield **ON 4.2s** (it read
"SHIELD  SHIELD 4.2s").

## 4. The puck's ring

`SM_PuckDisc` carries four slots: Halo (`MI_Trim_PuckHalo`: TeamColour, Emissive, ClayGlowMul), Ring (`MI_Fx_PuckRing` on `M_Fx_PuckMark`: FadeStart
only), Beacon, Marker. **No parameter sets the ring's width**: its thickness is geometry, device B's (A-to-B-0027 point 5). A sets nothing (a half from
A on top of B's push would quarter it). A's flattening scale (`jam.puck.Post`, lane A168) is gone with its test `Jam.Hud.PuckPostFlat`: B's disc no longer
carries the upright marker: `JAMPUCK disc scale=(1.00,1.00,1.00) half_extent_cm=(35.0,35.0,1.5)` (it was 22.6 cm tall with the marker).

## 5. GOAL (`HUDGoalBanner.cpp`, `jam.hud.GoalBanner 1`) and the celebration

On a goal a plate 760 x 176 (slate units at 1080p) in the scoring side's colour, "GOAL" in 84 pt Black and the new score under it ("0 : 1", home first as
the scoreboard reads). In over 12 sim frames (scale 125 % to 100 % with the fade), held, out over 12 frames ending 6 frames before the goal's pause ends:
with the default 120-frame pause it shows from frame 0 to 114 and is gone before the faceoff's countdown. A pause under 42 frames shares its frames in
thirds. **Its band**: centre at 30 % of the screen's height, between the scoreboard and the human (the camera holds him in the middle third); when the
scorer's projected figure stands under it, at 20.5 % (just under the scoreboard), or the band he overlaps least. One `JAMHUD goal_banner` line a goal.
`Jam.Hud.GoalBanner` tests the timing, the band, the score and the plate's words.

The celebration: on the `goal` event the game mode calls `PlayCelebrate` by name on the scorer (the last touch when he skates for the scoring team, else
that team's skater nearest the puck) and his nearest mate, inside the pause that already follows. Accepted signatures: no input, one `FName` or `FString`
(the take: `arms_up`, `stick_raise`, `knee_glide`, by the goal count; the mate `stick_raise`, or `knee_glide` when the scorer raises his) or one number
(0..2 the scorer's take, 3 the mate's). Until device B's figures have it, they do what they did. One `JAMFIG celebrate` line a goal.

## 6. Proof

- Hash: `Jam.Core.Determinism` hockey.regular `ef34dd88c7c62b47`, hockey.raw `79d43eb5ea69a468`, regular `4864f6033fe51840`: the same as main's gate
  (premerge 12:26). Arsenal seed 1, every tier: frame 600 `49a31f0876c30c87`, 1200 `03526783fb32c858`, 1800 `c4fec820c90b261c`, 2400
  `dedef771a16a5331` before and after.
- Tests: `Jam.Fx.BoostLook` (silhouettes, colours, the low profile, the by-name calls on an object without them, the scorer and mate), `Jam.Hud.GoalBanner`.
- Neutral values: `jam.boost.Beacon 0`, `jam.boost.Aura 0`, `jam.hud.GoalBanner 0` give the screen of before.
- Tools: `tools/qa/screen/a188-stills.ps1` (u1-stills with any cut), `tools/qa/screen/a188/arsenal-<tier>.json`, `tools/qa/screen/a188/boost-px.py`
  (the probe's pixels by distance); proof cvars `jam.boost.Probe N`, `jam.boost.Preview <kind>[!]` (the human drawn as holding a kind, never in the sim),
  `jam.hud.GoalBannerStill N`.

## 7. Stills (`img/a188/`)

| still | what |
|---|---|
| `before-clay-f01920.png`, `before-neon-f01920.png` | the look of before: the yellow reel at 7 and 43 m, the cyan canister at 23 m, the orange base |
| `after-clay-f01920-beacons-shield.png` | the same frame: the speed cone at 7 m (low: the human is by it) and 43 m, the precision diamond at 23 m; a running shield |
| `after-night-f01920-beacons-slowpuck.png`, `after-neon-f01920-beacons-shield.png` | the same frame on night and neon; the slow puck's lime ring |
| `after-clay-f01440-shotgun-next-shot.png` | the shotgun armed: the crimson ring turning fast, the plate "F: SHOTGUN  NEXT SHOT  x3" |
| `goal-clay-f001068-band20.png` | GOAL for orange: the scorer stood under the middle band, the banner sits just under the scoreboard |
| `goal-neon-f001722-band30.png` | GOAL for blue in the middle band, the scorer under it in the clear |
| `faceoff-night-disc-shotgun-ready.png` | the first faceoff: the puck's disc at its own scale (no post), the shotgun held and ready |

The ring segments first drew the default material (instanced meshes: A's M_Trim is not flagged for them; plain components now), the plate clipped
"x3" at 380 (440 now), the banner's score overflowed its plate (84 / 32 pt now) and the high band touched the scoreboard (0.205 now): all seen in
the build-1 stills (`tools/qa/screen/out/a188-after-*`), fixed in builds 2 and 3.

## 8. What a real window must confirm

The beacon found in a third of a second at 40 m while skating; the ring readable at speed and not mistaken for the YOU ring; the banner's punch felt as an
event, not a block over the play; the plate's width at 1080p; whether the ground ring should pulse on spawn.
