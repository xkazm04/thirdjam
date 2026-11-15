# Lane A224, track F3 (what a hit looks like), Sunday: a miss is a take too; three small leftovers on the screen (device A, Opus)

You are lane A224 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md`, then `briefs/NIGHT-COMMON.md`: their rules are yours in full.**
Then **R-0069 and R-0070** at the end of `contracts/RULINGS.md` (at a hit the figure's animation is the whole message; nothing is drawn on
or in front of a skater), `status/A219.md` (the cross-check's calls as A sends them: `JamPawns.cpp` `AJamSkater::ApplySimFrame`,
`jam.vis.HitTakeGuard`, `HoldEarly`, `HoldOverBrake`; its film tools `tools/qa/eye2/`, and `tools/qa/eye3/` of lane A223),
device B's `status/B.md` H59.5 and `status/E4-report.md` (what its figure answers now), `handoff/A-to-B-0039.md` with B's response,
`status/A222.md` (the controls panel and the clamp of the puck arrow), `status/A223.md` ("A's own" findings).

Your worktree and the commit it stands on are named in the Director's message that starts you. Status file `status/A224.md`. Two heavy
processes at most. Off screen only; no window, no cursor. Nothing of yours runs from the main checkout. Submission is tonight at 22:00.

## 1. The miss (the main job)
Device B's figure now has `PlayCrossWhiff()` and `PlayWhiff(int32 Kind)` (0 = body check, 1 = cross-check) with their own takes, and B asks
A to "take your bridge dip off the figure": A still fakes a whiff from its side of the calls. Read what A sends today on a body check and a
cross-check that meet nobody (the human's and a bot's), then: send B's whiff take on the frame the sim knows the hit met nobody, and stop
A's own bridge. The take must not be cut by another call of A's (the collision reaction, the brake, `PlayLift`): the same care as lane A219's
guard for the hit's take. Behind `jam.vis.WhiffTake` (1; 0 = today's calls). Presentation only: the sim's hash equal.
Film it as A219 did (the game camera, every frame, a Development package of your tree if time allows, the editor binary otherwise and say
which): a tapped cross-check into air, a body check into air, a held cross-check released without a hit, each beside today's picture.
Judge it as a person: does a miss now LOOK like a miss (he swings and finds nothing) without any mark? One `SEEN` line each.

## 2. Three leftovers, each a few lines (do them only after 1 is filmed)
a. **The boxed man's ring:** while a skater sits in the penalty box his ring stays on the ice with no figure in it (lane A223 saw it on
   Glow Ice under the controls panel for about ten seconds). A ring belongs under a skater on the ice: hide it while he is boxed, his or a
   bot's, on every look.
b. **The net chevrons and the controls panel:** the far-net and own-net chevrons can slide to the left edge down onto the panel's box
   (lane A222 clamped only the puck arrow in `UpdatePuckMarker`): the same clamp for them while the panel shows.
c. **The "Preparing Shaders" / engine message lines:** lane A223 saw a yellow engine warning under the score on Pond in a Development
   package. Check by the code and the config, not by starting it, that a SHIPPING build draws no on-screen engine messages at all
   (`EnableAllScreenMessages`, `bEnableOnScreenDebugMessages`, the `DisableAllScreenMessages` command): if there is any doubt, switch them
   off at start for packaged builds of every configuration and say what you did. He records the trailer from a package at about 18:00.

## Prove
The strips of 1 (before / after, the same frames); a still each for 2a and 2b before / after; tests (`Jam.Hit.*` for the whiff calls and
that nothing cuts them; `Jam.Hud.*` for the ring and the chevrons); the full `Jam` filter in a cleared environment; Shipping compiles; the
sim's hash equal on a 60-second scripted match.

`SEEN` lines as soon as the first strip exists (the Director looks at it himself); `MISS READY` with the files by name, the strips and the
gate **by 16:45 by the machine clock**. Keep the files closed after the line. If 2 does not fit, land 1 and name what is open.
