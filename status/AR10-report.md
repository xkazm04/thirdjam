# AR10 report: jobs 2 and 3 seen on the GPU, and the barn's scale tried (device B)

Lane AR10, device B, machine clock 05:16 to 05:50 (2026-11-14). Tree `C:\Users\kazda\jam\wt\b-arena`. As you asked I brought it up to date myself: `git merge --ff-only dev-b` went through over the uncommitted job 2 state (now at a5c4ba47), then one rebuild (exit 0). No other git, no commit, no push, no cook. Two GPU editor processes of mine on port 8008: pid 28428 with job 2's rig (05:20 to 05:29:19) and pid 26336 with job 3's rig and the barn experiment (05:31 to 05:43:38). Both left through `ExitEditor`, nothing unsaved either time, both confirmed gone. **No asset was saved in this lane.** Console variables restored. In both logs: 0 "usage flag" lines, 0 warnings naming B's arena, 0 Blueprint runtime errors, 0 asserts.

The tree is left in **job 2's state** (the rig file equals my copy of job 2 byte for byte); job 3's rig is still beside it in `Game/Saved/AR8/LR_P3_Clay.job3.umap`.

## Verdicts

| Job | Verdict | The reason |
|---|---|---|
| 2: lamp arms, far end | **Merge after 10:00 as is** | Readability inside the line (ice 0.00 %, puck within 0.6, jerseys within 0.7); nothing doubled, floating or through a wall in 18 stills; on the regular rink and Pond it sits right. One flaw on the big rinks, which is the barn's scale and not this job's alone: see 3 |
| 3: one owner | **Do not merge as is; merge with one change** | It works on screen on five rink lengths, the switch and the cheer reach it, the rig is 363,270 bytes against 1,607,256 and nothing is destroyed at load. **But the rig's blocking load is 2 to 4 times longer** (93 to 178 ms against 39 to 52 ms): it builds 480 to 930 fans in Blueprint at BeginPlay. The change: build in batches, or keep the baked rig and use the builder only for lengths without a layout |
| The barn at honest scale | **Not built; shown as an experiment; yours and the operator's to decide** | The shell can be pulled in from the rig alone, and it looks better on the wide arena. The lamps cannot follow without the other device's light scaling, and then sit half a metre under the roof |

## 1. Job 2 on screen (`ar10-job2-regular`, `ar10-job2-pond-wide`)

**The lamp arms.**
- From 3.5 m (second tile): the rod runs up from the shade through a plate under a timber arm that lies on the truss: **the rods end in timber now.**
- From the low camera, the far end and the wide still the arms are small dark timber against a dark roof, 13.6 to 14.7 m up: they do not draw the eye and nothing hangs loose. I looked closely at one arm of eighteen.
- The 4 cm between arm and squeezed chord (my arithmetic in AR8) cannot be seen.

**The far end.**
- **From a game-camera pose** toward the -x end: the sliding door shows as a dark shape over the end stand, left of the net. The firewood and the bench do not show: they are 1.8 m high behind a 4 m stand.
- **From 1.7 m** behind the net: the door's frame over the terrace.
- **From the aisle** behind the stand: the bench with its hung skates reads at once; the door and the firewood further along are dark.
- **Pond:** the aisle is 1.5 m wide; the bench fits against the wall; the pennants show from the low camera.
- **Wide:** the wall pieces are at the barn's scale, 1.556: **the bench and its skates are giants beside the real-size fans** (last tile). The same holds for AR5's gate, props and ice machine at that end. On the regular rink they are life size.

**Readability**, frozen pairs at the game camera on the regular rink, everything of B's arena hidden and shown with `ke * ArenaCrowd 0 / 2`:

| Moment | Ice mean luma | Puck against the ice | Jerseys against the ice |
|---|---|---|---|
| 2.3 s | 164.37 → 164.37 (0.00 %) | -150.6 → -150.8 | blue -93.8 → -94.2; orange -83.8 → -83.6, -88.7 → -89.4 |
| 6.0 s | 166.59 → 166.58 (0.00 %) | -154.9 → -154.9 | blue -117.3 → -117.3; orange -89.0 → -88.8 |
| 2.3 s, job 2's pieces alone hidden and shown | 164.45 → 164.47 (+0.01 %) | -151.4 → -150.8 | blue -94.2 → -94.2; orange -83.3 → -83.6, -89.2 → -89.4 |

Not looked at: the large arena; the door's hoist beam close; each arm one by one.

## 2. Job 3 on screen (`ar10-job3-regular`, `ar10-job3-rinks`)

| Rink | Owner in play | Fans built | Stand pieces | On screen |
|---|---|---:|---:|---|
| Regular 90 m | `Crowd_P3_Clay`, 1 | 642 | 56 | stands and fans as in the baked rig; front row empty; corners filled |
| Pond 60 m | 1 | 476 | 36 | as AR7's Pond |
| Large 120 m | 1 | 834 | 74 | as AR7's |
| Wide 140 m | 1 | 932 | 86 | as AR7's |
| 100 x 50 (no baked layout) | 1 | 722 | 62 | **a full ring of stands and fans where the merged rig has none** |

- **Fans sit on the benches** on all five; I saw none floating. The seats taken and the figures differ from the baked rig (its own random numbers); I did not spot two alike side by side, and did not measure it.
- **The switch reaches the one owner:** `ke * ArenaCrowd 1`: 0 of 12 fan components visible, stands stay; `0`: owner hidden, and with `Dress_P3_Clay` the roof bays, so the roof has no trusses at 0 (the old ones were deleted in AR5; the same on the merged rig); `2`: all back.
- **The cheer answers:** the +x half rises 30 frames after a goal; 2.11 % of the frame (the baked rig: 2.10 %).
- **Readability pair** (2.3 s): ice 164.27 → 164.27 (0.00 %), puck -150.6 → -151.7, jerseys within 1.0.

**Size and load, one owner against seven:**

| | Seven owners (merged, and job 2) | One owner (job 3) |
|---|---:|---:|
| Rig file | 1,607,256 bytes | **363,270 bytes** |
| Owners destroyed at load | 6 | **0** |
| Rig's blocking load, first play session of the process | 241 ms | 511 ms |
| Rig's blocking load, later sessions | 52, 39 ms | **93, 178, 156, 137 ms** |

- The blocking load is the other device's own log line (`blocking load N ms`), one per play session, in my two logs. Few samples, different rinks, and a cook running beside: the direction is clear, the factor is rough.
- So job 3 reaches its size target and misses on time: what it saves in the file it spends twice over in Blueprint.

## 3. The barn at honest scale (`ar10-barn-wide`)

**Tried in a play session on the wide arena, nothing saved:** the shell (53 walls, posts and gables, the two roof slabs, the floor, and `Dress_P3_Clay` with roof bays, wall bays, bunting and arms) pulled in by three factors (0.911 along, 0.860 across, 0.643 up). Measured after: **eave 13.56 m** (it was 21.0), walls 49.4 m from the centre line, 6.4 m behind the stands (14.6 m before), ridge 22.2 m. Lamps and lights were not touched.

- **What it looks like:** from the bench side the crowd now has a wall with windows right behind it and the place reads as a barn built for these stands; from 9 m the roof comes down over the rink. At the game camera almost nothing changes (5.2 % of the frame, the band from luma 20 to 26).
- **The ice does not change:** mean 167.69 → 167.63 (-0.03 %), puck -153.5 → -153.4, jerseys within 1.7.
- **What breaks:** the lamps stay at 18.0 and 20.3 m (the other device's switcher sets their light for that height), so the side lamps hang 0.5 m under the roof and their shades reach into it; the lamp arms move with the roof bays and no longer meet the rods; the gate, loft, props, door, firewood and bench stay at the old wall line, now outside the walls.

**Can it be had from the rig alone? The shell, yes; the lamps, no.**
- The shell: a Blueprint step at BeginPlay that sets the transforms of the shell's actors from the rig's scale (they would have to be authored movable). The wall pieces and arms take the same factors. That is B's alone.
- The lamps: their height is tied to the other device's rule (intensity x scale squared, reach x scale). Lowering them makes hot spots on the ice; more lamps per size is a new light rig per rink. So the honest barn keeps today's lamp heights and lives with lamps close under the roof on the wide arena, or waits for a lighting lane.
- I did not build it as an asset: it changes every still of the large and wide arena, and where the roof meets the lamps wants eyes on each size, with time.

## Not verified

- Job 2: the large arena; all arms one by one; Cage, the 64 m and the 80 m rink.
- Job 3: its cost at BeginPlay in milliseconds as its own number (only the blocking-load line); neighbours alike; a cook with `BP_ArenaCrowdOne` (931,028 bytes of graph).
- The barn experiment on the large arena and Pond; any of it in a cooked game.
- The frame rate of anything.

## For the other device

- Nothing to do for the operator's package: none of this is in it.
- If job 3 is wanted: time the rig's load with the baked rig against the one owner on your machine; mine says 2 to 4 times.
- If the barn's shell is to be capped: your lamp scaling stays as it is; say if the lamps may come down with more of them on big rinks, because that is your intensity rule.

## Decisions for the operator

1. **The wide and large arena: today's very large shed, or the shell pulled in** (`ar10-barn-wide`, top row against bottom row). Pulled in reads better from every low view and costs lamps close under the roof.
2. **Wall pieces at real size on big rinks** (`ar10-job2-pond-wide`, last tile, against `ar10-job2-regular`, last tile): the bench and its skates are 1.56 times life size on the wide arena.
3. **One owner or seven** (`ar10-job3-rinks` against AR7's `20261114-041908-B-ar7-wide.png`): the same picture; the choice is file size and any rink length against load time.

## Files of this lane (no asset changed)

- `devlog/shots/20261114-054357-B-ar10-job2-regular.png`, `-job2-pond-wide.png`, `-job3-regular.png`, `-job3-rinks.png`, `-barn-wide.png` (each with a `.txt`)
- `status/AR10-report.md`
- `status/AR8-report.md`: part two appended (jobs 2 and 3 as built, their files listed apart, the ledger rows in 11 columns)

The assets of jobs 2 and 3 are as AR8's part two lists them; this lane changed none of them, so there are no new ledger rows here.
