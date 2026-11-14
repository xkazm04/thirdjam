# Lane A210, track F3 (what is seen), the night: a check and a cross-check are animations, not marks (device A, Fable: animation decisions)

You are lane A210 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md`, then `briefs/NIGHT-COMMON.md`: their rules are yours in full.**
Then the track's `docs/experiments/HITS.md` with `status/A196.md` (everything drawn around a hit today: `HitRead.cpp/.h`: the chevron, the streak on
the stick, **the streak across the chest and the ground arc of a held cross-check**, the contact ring, the downed oval, the stand-in lean, the
`jam.hit.*` cvars, `jam.hit.BTakes`), `contracts/skater-visual.md`, the calls A makes on device B's figure (`JamPawns.cpp`: `PlayCheck`,
`PlayCheckWindup(Frames)`, `PlayLift`, `PlayCrossCheck(Frames)`, `SetCrossCheckHold(bool)`, `SetWindupTakes(bool, N)`), device B's reports on its
takes (`status/E2-report.md`, `status/B.md` sections H51.6, H52.5, H53.3: the wind-up takes put contact on their frame 10 and start at `10 - N`;
`CrossCheck_Hold` with its in and out; `CrossCheck_Release`), and `handoff/A-to-B-0029.md` to `0032.md`.

Your worktree and the commit it stands on are named in the Director's message that starts you. Status file `status/A210.md`. Port 4810. Family:
`HitRead.cpp/.h`, the hits' presentation calls in `JamPawns.cpp` and the game mode (your own lines), the capture tools `tools/qa/screen/a196*`,
`docs/experiments/HITS-2.md`. Presentation only: no sim state; the sim's hash untouched (prove it). Never `Game/Plugins/JamAssets`: what must change
in a take or a Blueprint is written for device B, exactly (asset, frame, what is wrong), in your status file under `FOR B`: the Director sends it.

## His words: "crosscheck has bug in animation displaying technical blue rectangle instead of animating player with stick into its position. Tackle is also unrecognizable and animation too quick"
1. **Find the blue rectangle** (first 20 minutes): play the human's pressed and held cross-check off screen from the game camera on clay at rookie
   (strips of every frame, the human in blue): the Director's reading is that it is A's own mark: the team-coloured streak across the chest / on the
   stick and the ground arc of `HitRead` (blue because he is the blue side), drawn where an animation should be, while the human's figure plays only
   a stand-in lean (`jam.hit.BTakes` gives device B's takes to the bots only). Prove or disprove it by switching each `jam.hit.*` element off in turn;
   if it is something else (a missing material, a debug draw, a widget), find its owner by file and line. `SEEN` in the status file by 21:00.
2. **The figure does the hitting**: the human's and the bots' figures play device B's takes for every hit: the body check (wind-up, drive through
   the contact, follow-through held long enough to read: say the frames), the pressed cross-check (the stick comes up across the body and shoves),
   the held brace (`CrossCheck_Hold` with its in and out, the release on contact). The human's wind-up becomes 8 frames tonight (lane A209): until
   it lands, stage it with a cartridge that sets `check_windup_frames` 8 / `cross_windup_frames` 8 and judge the takes there; at 4 frames the take
   is cut: show it. The stand-in lean goes wherever B's take plays. Timing: the take's contact frame lands within one frame of the sim's (strips with
   the sim's frame printed).
3. **Marks become quiet helpers or go**: the chest streak and the stick streak go (they are what he called a bug); the ground chevron stays only if
   it reads as a direction cue and not as a rectangle (thin, short-lived, the ice's own white, not a team-coloured slab: decide by stills at the game
   camera's distance); the contact ring stays small; the downed oval stays. What a person must see is the BODY: say for each mark whether it stays.
4. **A tackle that is recognised**: from the game camera at 20 and 35 m: a strip of the body check landing on a man, the man's fall (B's reaction and
   knockdown takes), the checker's follow-through, the camera's kick, the sound: 5 people-seconds of "that was a hit": the pause frames (hit-stop)
   and the kick by the hit's weight (they exist: `jam.hitstop`, `jam.hitkick`: tune them: a heavy hit holds 3 to 5 frames), the victim thrown along
   the hit's line. Before / after strips at the same sim frames.
5. **FOR B, precisely**: every take that does not read at the game camera's distance or misses its contact frame (asset name, frames, what the eye
   needs: a bigger wind-up silhouette, the stick visibly across the chest, a longer follow-through), the cross-check at 12 frames (`10 - 12 = -2`),
   the sounds asked in A-to-B-0030 / 0031. Written so B can act without asking.

`SEEN` by 21:00; `HITS READY 1` by 23:15 (items 1 to 3 on the human's figure: file list, strips, the hash proof), then waves 2 and 3 as NIGHT-COMMON
says. Two heavy processes at most. Time box: the night.
