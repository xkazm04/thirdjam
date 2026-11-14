# Lane A188, track F3 (what is seen on the ice), evolution 8 (device A, Opus)

You are lane A188 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then **R-0064** at the end of `contracts/RULINGS.md` (the operator's words after playing package H31; points 6, 7 and 9), `handoff/A-to-B-0027.md` points 2, 5
and 6 (what device B makes today and the names it will give), the abilities' docs `docs/experiments/ABILITIES.md` and `MODES-6.md` / `MODES-7.md` (Arsenal:
the mode of boosts), `contracts/events.md` (`ability`, `goal`), `contracts/skater-visual.md`, the pickup actor (`JamPickupActor`), the effects binder
(`EffectsBinder.cpp`) and the HUD's ability plate and goal handling in `HUDWidget.cpp`.

Your worktree and the commit it stands on are named in the Director's message that starts you. Status file `status/A188.md`. Port 4788. Family:
`JamPickupActor.*`, the pickups' and abilities' presentation in the game mode and the binder, `RinkActor.*` only for a goal's sequence, ONE new HUD file
`HUDGoalBanner.cpp` and one call from `HUDWidget.cpp` (other lanes edit the HUD today: keep to your own function), `docs/experiments/BOOSTS.md`. Never
`Game/Plugins/JamAssets` (device B's). Presentation only: no sim state, the default match's hash untouched. Off screen with the capture driver.

## Evolution 8: boosts are seen, and a goal is an event
His words: "Special boosts are not seen on ice, not seen equipped on players"; "Puck's white outer ring for visibility can be 50% thinner"; "Nice to have:
short celebration animation set after a goal, HUD middle screen large banner saying 'Goal'".
1. **Why they are not seen** (measure first, 20 minutes): in Arsenal and in a cut with pickups: where the pickups are, how big on screen from the game
   camera at 20 / 40 / 60 m (pixels), what they look like on clay, on night, on the neon tier (a still each), how long they live, how often the human
   passes within 5 m of one without taking it; and when a skater HAS one: what shows on him and on the HUD today.
2. **On the ice**: a pickup an eye finds in a third of a second from the game camera on a 90 m rink: a larger turning object on a short pedestal of
   light, a colour and a silhouette by kind (A's stand-ins now: simple distinct shapes from basic meshes with the TL-free toon material the rink uses;
   device B's props by name when they arrive: a soft reference with a fallback), a ground ring in the kind's colour, a marker on the HUD's radar; never
   in a place that hides a skater or the puck; the screen's down rule holds (not over a downed figure).
3. **On the skater**: while he holds a boost: something by kind an eye reads from the game camera: an attached stand-in prop or a coloured aura (B's
   `SetBoost(Kind, bOn)` by name when it exists, behind an exists-check), and the HUD's plate says the boost's NAME and its key from the live bindings
   ("F: SHOTGUN"), and stops saying armed after it is used (the window lane saw "F SHOTGUN ARMED" after `ability used`).
4. **The puck's ring**: device B thins its mesh or material; A's flattening scale on the disc goes when B's push with the marker removed is on your
   tree (it is: main has dev-b at 46bdb717): remove `jam.puck.Post`'s scale and its test, check the disc at the faceoff in a still (no post).
   If the ring's thickness is a material parameter A can set (read the instance's parameters by name: do not edit the asset), set it to half and say so.
5. **GOAL**: a large banner across the middle of the screen on a goal: the word, the scoring team's colour, the new score, in for 12 frames, held, out
   before the faceoff's countdown; never over the scorer (the middle band above the play: say where); `Jam.Hud.*` test; and the celebration: call
   device B's `PlayCelebrate(Kind)` by name on the scorer and the nearest mate inside the pause that already follows a goal (exists-check; until then
   the figures do what they do today). One `JAMHUD goal_banner` line a goal.

Write `BOOSTS READY` with the file list and eight stills when items 2, 3 and 5 are closed, built, the full `Jam` filter green, Shipping compiles: **wanted
by 15:00 for the 15:30 package if clean, else 17:30 for the 18:30 package**; `SOURCE OPEN` before any later source change. `-Multiprocess`; stop only
processes whose ids you recorded. Time box 180 minutes.
