# AR13 film: the bigger cheer, the wave, the scoreboard and the darker stands, for the GPU

Lane AR13 worked without a renderer. The meshes, the per-fan data, the Blueprint's timing and the scoreboard are in; **the three changes to the two masters are not**, because a master cannot be compiled without a renderer. They are one script. Until it has run, the crowd still cheers (AR12's bigger pose A, a slower rise), without target B, without the bounce, and the stands' new vertex colours do nothing.

## Step 0 (3 minutes): the one script call

In a GPU editor of the tree that holds AR13's assets, run `status\AR13-master.py` once: paste its text into one `execute_python` call, or type in the editor's console

```
py "C:/Users/kazda/jam/wt/b-arena/status/AR13-master.py"
```

It edits `/JamAssets/Look/M_ClayCrowd` (target B, the wave, the bounce) and `/JamAssets/Arena/M_ArenaWood` (base colour and wash times the vertex colour), compiles both and saves the two packages. It is safe to run twice. What it should answer:
- `M_ClayCrowd`: `built; 26 old nodes removed`
- `M_ArenaWood`: `built; base colour and 2 custom input(s) now read base x vertex colour: B26BarnWash.Base, B36FloorSpill.Base`
- two lines of instruction counts (before AR13: 230 vertex / 217 pixel and 147 / 331), and `saved: true`.

Then read the log for `LogMaterial: Error`. I built the same graphs on scratch copies of both masters in my editor (stock nodes only, no Custom node added), so the wiring is proven; **the compile is not.** If `M_ClayCrowd` fails, the likeliest places are the eight texture coordinates on an instanced mesh (the script reads coordinates 2 to 7) and the `Time` node in the world position offset. The masters as they are today are in `scratchpad\ar13\bk` and in git.

New parameters on `M_ClayCrowd` after the script: `WaveK` (2.5), `BounceCm` (15). `PoseK` stays.

## Step 1 (5 minutes): how big the cheer is

A frozen clay match on the regular rink, a game-camera pose (3.5 m up, 17.65 degrees down) looking at the +x end stand from **15, 25 and 40 m** (camera x = 34, 24 and 9 m; the stand's front is at 49.0 m). At each: the frame at rest and the frame with every fan of that end fully risen, and the share of the frame that changes by more than 8 luma.

- The Blender lane measured **9.4, 5.8 and 2.5 %** with AR12's poses, against 4.0, 2.5 and 1.0 % for today's. On the GPU, AR7 measured 2.1 % at 31 m with the old pose.
- To hold a frame: as in AR7 (`scratchpad\ar7\tk.py`, `cheer_at`), but the Blueprint's rise is now 28 frames, not 8: for "fully risen" set the end's scalar to 1.

## Step 2 (5 minutes): what only eyes can pass

- **Risen fans against the row behind:** a fan on its feet has its head level with the seated heads one row up. From 8 m at the side of a stand, say if it reads as a crowd on its feet or as fans inside each other.
- **Arms through neighbours:** hands and scarves reach up to 69 cm to the side on a 58 cm seat pitch (figure E's mug arm 92 cm, read from its mesh). Neighbours now take different targets (97 % of the fans have a nearest neighbour on the other target), which is what should keep them apart. Look along one full row.
- **The wave:** the strip at +4, +8, +14, +20 and +28 frames after a goal from the 25 m pose. The stand should rise from the net outward: by my count 31 fans of 332 have started at frame 2, 131 at 8, 230 at 14, 327 at 20, all at 28; on the way down the far fans drop first.
- **The level switch during a cheer:** at full rise, walk the camera from 26 to 34 m from a stand (the switch is at 30 m; the meshes' bounds grew, so their screen sizes were set again for 30 m).
- **Culling:** a risen fan at the frame's edge must not vanish (bounds raised to 172 cm up and 95 cm to the sides).
- **The bounce:** un-paused only (it runs on time): 2.2 times a second, 15 cm at full cheer. Say if the stand shimmers.
- **Half way through the blend** the hats are in the air: quick it reads as a throw. Watch one rise at normal speed.

## Step 3 (3 minutes): three numbers to set

| What | Asset, parameter | Now | Range |
|---|---|---|---|
| How far the pose goes | `PoseK` on `MI_ClayCrowd_Fan`, `_Skin`, `_Scarf`, `_Tin` (the same value on all four) | 1.0 | 0.6 to 1.0; at 0.6 the fans are still on the bench with their arms up (AR12) |
| How long the wave takes | `WaveK` on the same four | 2.5 (20 frames from the net to the far corner) | 0 (everyone at once) to 2.5; the Blueprint's 28 frames are 8 x (1 + WaveK) at 2.5 |
| The bounce | `BounceCm` on the same four | 15 | 0 to 25 |

And the stands: with their treads darker the Blender lane's crowd band dropped by 8. Measure the band as in AR7 (mean 36 to 45 then) and lift `WashK` on `MI_ClayBarn_Stand` and `_StandDark` (8.5 now, to about 10) if the band should stay.

## Step 4 (2 minutes): the scoreboard

`SM_Barn_ScoreboardHung` hangs under the centre truss on the 80 m, regular, large and wide rink (lowest point 9.2, 10.7, 15.2 and 18.2 m), not on Pond, the 64 m and the 70 m rink. From the low camera at centre ice looking up, and from the far end: the face is the boards' cream (`MI_ClayCrowd_Cream`) until a score is drawn on it; its two lamps are `Arena/MI_Barn_ScoreLamp` (`FillK` 1.5, a low warm glow: tune to luma 40 to 60). It is in the game camera's frame only if the camera looks up; say if it ever is in normal play.

## Step 5 (4 minutes): readability

The frozen pairs of AR7 (2.3 s, 6.0 s, 9.5 s) with B's arena hidden and shown (`ke * ArenaCrowd 0 / 2`), once at rest and once with the +x end fully risen: ice within 1 %, puck and jerseys within 3. The second pair is new: a risen stand is a bigger, lighter band.

## What to send back

The script's answer and the two instruction counts; the three shares at 15, 25 and 40 m; a yes or no on each line of step 2; the values of `PoseK`, `WaveK`, `BounceCm` and `WashK` you leave; the scoreboard's luma; the two pairs.
