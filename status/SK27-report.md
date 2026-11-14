# Lane SK27 report (phase A, without a GPU): one match on figures that are not wiped, and what the Blueprints' bytes are

Tree `C:\Users\kazda\jam\wt\b-fig` (`lane/b-fig` c214e8fe), port 8004. One headless editor of mine (pid 30728, started 04:20:31 with `-nullrhi -unattended`; 0 `LogD3D12RHI` lines, line 918 loads `NullDrv`), left through `ExitEditor` at 04:25:53 when the GPU came free at 04:22. **Nothing was written: no asset changed in this phase, so there are no ledger rows.** No commit, no push.

The phase was 5 minutes long. Done: one of A1's three matches, and A4's table. Not done: A1's second clay match and the night match, A2 (the sounds), A3 (the brake effect's strength).

## A1. A match on figures that are not wiped

One clay match, seed 1, scripted human, recorded with the quiet pose-tick set. The log says what is in force: **`JAMMATCH level=pro source=cartridge`**.

**The merged tree plays a different match.** The same seed now ends at sim frame 6890 on the fourth goal (the scripted human loses 1 to 3); in SK25 and SK26 it ran past 12,600. Its knockdown and stumble lines are not SK26's. So the numbers below are not a re-measure of the same events; they are the same checks on a new match.

| Check | Result (to frame 6890, 115 s) |
|---|---|
| Blueprint errors | 0 |
| Knockdowns | 3, all the human |
| Get-up end minus release | 0.0 in 3 of 3 (largest 0.000009 frame) |
| Stumbles shown of sent | 3 of 3 (`PlayStumble`, frames 16, 15, 15), each started once |
| Human's brake lines | 36, of them 18 `on=1`; the figure counted 18 |
| Stops shown, the human | 8 (11 of the 18 started from 7 m/s; 4 of those lasted under 6 frames) |
| Stops shown, each bot | 7, 6, 7, 7, 8, 5, 8, 7, 8 |
| Whips started | 33 (no plain collision among them: 112 `JAMVIS hit`, 25 board hits) |
| Frames the human's figure spent in a whip | 252 of 12,661 recorded |

| Sim frame | Frames down | Fall (size) | Loop | Rule | Start | L | Cut | Clean | Largest travel cm | Get-up | End minus release |
|---|---|---|---|---|---|---|---|---|---|---|---|
| 4950 | 150 | `Fall_Heavy` (heavy) | `Lie_Loop` | B | 39 | 83 | 14 | yes | **12.9** knee | `GetUp` | 0.0 |
| later | 150 | `Fall_Heavy` (heavy) | `Lie_Loop` | A | 93 | 83 | 68 | yes | 15.9 hand | `GetUp` | 0.0 |
| later | 88 | `Knockdown_Left_Light` (light) | `Lie_Loop_Side_Left` | B | 39 | 55 | 94 | yes | 13.7 skate | `GetUp_Quick_Side_Left` | 0.0 |

**Which SK25 and SK26 numbers moved, now that the figures keep their variables:**
- **The stop.** This is the one that was wrong before. With the bars wiped to 0, SK25's and SK26's recorded matches showed a stop pose on every brake call; I did not report stop counts from them, so no published number is false, but nobody had seen the gate work headless. Here it does: the human 8 in 115 s, each bot 5 to 8, against 185 brake spans.
- **The loops, the release, the stumbles: not moved.** 0.0, clean cuts, every stumble shown once, as before.
- **The travel line has a third miss.** `Lie_Loop` on rule B at L 83 shows 12.9 cm at the knee bone (the lab gives 14.0). With the two wall loops accepted in SK25, that is three loops whose second move I measure under 13.5 cm at the bone.
- **A stumble of 15 frames** was sent for the first time (the cross-check's); it played.
- **Whips:** 33 in 115 s, none from a plain collision, as decided.
- **Not re-measured:** the check's closing speed against Strength (6 landed checks here), seed 7920, and night.

## A4. What the figure Blueprints' bytes are

**They are the graph, and nothing else of size.** There is no dead weight from the script edits to cut.

| | Clay figure | Night figure |
|---|---|---|
| Asset | 9,685,732 bytes | 9,476,476 |
| Graphs | 2: the event graph, and a construction script of 1 node | the same |
| Nodes in the event graph | 4,192 | 4,103 |
| Bytes a node | 2,311 | 2,310 |
| The tools' graph text | 116,827 characters, 88 events | 113,613 |
| Calls, sets and branches in that text / operators and casts | 2,849 / 1,248, together 4,097 | about the same |

- The text's own count (4,097) and the graph's (4,192) agree to 2 %. **So there are no orphaned nodes worth the name and no duplicated graph**: every write replaced its events' nodes.
- A Blueprint node costs about 2.3 KB in the asset, whatever it does. The size has followed the node count all night: 5.17 MB at 00:10, 6.08 MB (SK24), 7.47 MB (SK25), 9.69 MB (SK26).
- No embedded arrays, no thumbnails of size: the 133 take references are paths.

**Where the 4,192 nodes are** (clay; shares of the text's calls, sets and branches):

| Group | Share | About |
|---|---|---|
| The knockdown's machinery (`KnockDown`, `BoardFall`, `FallPick`, `PlayGetUp`, `PlayLie`, `PlayUp`, `WallFix`, `ShiftTick`, the tick, their helpers) | 36 % | 3.5 MB |
| The new sets' events (SK26) | 16 % | 1.6 MB |
| The stop (`StopTick`, `SetBraking`) | 7 % | 0.7 MB |
| **The test trigger `SK26Test`** | 6.8 % | 0.65 MB |
| The whip (SK25) | 5.4 % | 0.5 MB |
| Everything else (aim, stick correction, reactions, bus, team colours) | 29 % | 2.8 MB |
| *of all of the above:* **logging** (print, join and to-string nodes, spread through every group) | *8.6 %* | *0.8 MB* |

**The cut I would make after 10:00**, in this order:
1. **The test trigger asks "is this the human's figure?" 22 times over** (22 lookups of the game mode). Asked once, it is about 170 nodes less: 0.4 MB a figure. Better still, it and `SetTestMode` move to a child Blueprint the picture lane summons, and leave the shipped figure.
2. **Logging behind one event** with a switch, in place of 53 print chains: about 200 nodes, 0.5 MB a figure.
3. **The take choices as tables.** `FallPick`, `PlayGetUp`, `PlayLie`, `ChkCut` and `WeaponPick` are if-chains over take paths (about 300 nodes); an array indexed by kind and size is a tenth of that: 0.6 MB.
4. **The sets in their own component Blueprint**, so the base figure stops growing with every set.

**Cut now: nothing.** Nothing in the graph is plainly dead. The one safe saving (the trigger's 22 lookups) needs a write to both Blueprints an hour before the package is cut and the trigger is what phase B films with, so I left it.

This is the editor asset's size, in the repository. The cooked figure carries no graph nodes; I did not look at the cooked size.

## Not done in this phase (they wait for the next run)

- **A1:** the second clay match (seed 7920) and the night match. Phase B plays both on the GPU; its logs give the same checks.
- **A2:** the fourteen sounds. Nothing wired. The places are in `status/SK26-report.md` ("Sounds: where each would go"); six of them need a timer event each (the skid at frame 2, the wipeout at 13, the mallet's swing at 4 and bonk at 15, the cannon at 8), and the dash loop needs a kept component.
- **A3:** `SetVariableFloat(BrakeFx, "User.Strength", ...)` after the two `Activate(BrakeFx)` nodes in `StopTick`. Not written.

## Not verified

- Everything above is one match of 115 s.
- Why the merged tree's seed 1 differs from SK26's: I take it for the other device's changes in the merge; I did not look.

## Ledger rows

None: no asset was written in this phase.
