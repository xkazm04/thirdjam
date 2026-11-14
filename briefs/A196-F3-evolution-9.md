# Lane A196, track F3 (what is seen on the ice), evolution 9 (device A, Opus)

You are lane A196 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then **R-0064** at the end of `contracts/RULINGS.md` (the operator's words after playing; points 5, 6 and 9), the track's `status/A188.md` and
`docs/experiments/BOOSTS.md` (boosts' beacons, the ring on the skater, the GOAL banner: on main), the contact lane's `status/A192.md` and
`docs/experiments/CONTACT-11.md` (the wind-ups: 4 frames before a body check's window, 4 before a cross-check's shove; the held cross-check; the frames
from call to contact), `handoff/A-to-B-0027.md` points 1, 2 and 6 with device B's answers, `contracts/events.md` (`check`, `cross_check`, `knockdown`,
`ability`, `goal`), `contracts/skater-visual.md`, and `EffectsBinder.cpp`.

Your worktree and the commit it stands on are named in the Director's message that starts you. Status file `status/A196.md`. Port 4796. Family:
the hits' and boosts' presentation in `EffectsBinder.cpp` and the game mode, `JamPickupActor.*`, `HUDGoalBanner.cpp`, ONE new file for the hits'
presentation if you need one (`HitRead.cpp/.h`), `docs/experiments/HITS.md`. Not yours: `HUDBrace.cpp` (the contact lane's), `CameraFollow.*` (ask the
Director for a camera impulse hook if you need one: say the one line). Never `Game/Plugins/JamAssets` (device B's). Presentation only: no sim state, the
default match's hash untouched. Off screen with the capture driver.

## Evolution 9: a hit is read by an eye that was looking elsewhere
His words: "Animations of crosscheck and body check are short and not clear, may need today to evolve". Device B makes the animation takes (wind-up,
contact, follow-through, the held pose) for the 18:00 merge; the sim now gives them time (the wind-ups). A's part is everything around the figure.
1. **Measure what a hit shows today** (20 minutes): at pro, off screen, from the game camera: a body check that lands, one that misses, a pressed
   cross-check, a held cross-check run through a man: frame strips (every frame from the press to 30 frames after): what changes on screen and for how
   many frames: the figure's pose, an effect, a sound line, a HUD word. One table. Which of the four an eye cannot tell from skating into a man?
2. **The tell before, the event at contact, the mark after** (build with what the engine has: the binder's existing effect and sound paths, no
   hand-made materials beyond the rink's own toon set):
   - before: during the wind-up frames the checker reads as loading (call device B's `PlayCheckWindup(Frames)` / `PlayCrossCheck(Frames)` /
     `SetCrossCheckHold(bool)` by name behind an exists-check: they arrive with the 18:00 merge; until then a stand-in an eye sees: a short lean driven
     through the figure's existing calls, a streak on the stick for the cross-check, a ground arc in front of a held brace);
   - at contact: an impact burst at the contact point sized by the closing speed (ice spray, a ring), one camera kick of a few pixels if the human is
     in it (through a hook: ask), the hit's sound by weight (light / heavy / braced: the audio table's existing lines; say what is missing for B);
   - after: the downed man's read (he is down for N frames: a marker that counts down or dims: check the screen's down rule holds), the stripped puck's
     flash when a cross-check takes it.
   A miss must read as a miss (the lunge, no burst). Teen rating: no blood, no injury.
3. **Boosts with device B's props**: `/JamAssets/Props/SM_Boost_<kind>` and `SetBoostEquipped(FName Kind, bool bOn)` arrive with the 18:00 merge (stand-ins
   declared earlier): when the Director says they are on main, confirm each by a still (on the ice, on the skater) and remove nothing: the fallback stays.
   The GOAL banner takes `/JamAssets/UI/T_UI_Banner_Goal` by name if it exists. `PlayCelebrate` on the scorer and the nearest mate: confirm by a strip.
4. Cvars for each new element (`jam.hit.*`), default on; `Jam.Hit.*` tests of the rules (burst size from speed, a miss gives none, nothing over a
   downed figure's face); one `JAMHIT` log line a hit for the acceptance run.

Write `HITS READY` with the file list and frame strips (before and after, the four cases) when items 1, 2 and 4 are closed, built, the full `Jam` filter
green, Shipping compiles: **wanted by 17:00 for the 18:30 package**; `PROPS READY` after item 3; `SOURCE OPEN` before any later source change.
`-Multiprocess`; stop only processes whose ids you recorded. Time box 200 minutes.
