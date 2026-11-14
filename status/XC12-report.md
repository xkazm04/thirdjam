# XC12: the cross-check past frame 10 (A-to-B-0031 ask 1), a fault fix after the freeze

Tree `C:\Users\kazda\jam\wt\b-fig` (branch `lane/b-fig`, dev-b 333b965b, rebuilt with `build.ps1` at 20:39, pass). Headless editor pid 21172 (port 8004,
20:42 to 21:12). GPU editor pid 34740 (21:15 to 21:18). Both left through `ExitEditor`, and both processes are confirmed gone. `GPU-FREE.txt` = "free from XC12 at 21:18".
No editor is running. Nothing was committed.

## Verdict: **merge**
- Frames 12 and 14 now reach contact on the call tick + Frames, for both figures, for the cross-check and the body check.
- Frames 4 and 8 are unchanged: the same montage positions on every tick, and 0.000 cm on every joint once the take is at full weight.
- In a seeded clay match with the shipped bot settings: 0 Blueprint errors, and every sim line is identical to the same match on the unchanged figure (75 = 75).
- I looked at it on the GPU from the game camera.

## 1. What it did before (measured headless, not assumed)
The test figures had `SetWindupTakes(true, 8)`, then `PlayCrossCheck(F)` or `PlayCheck` + `PlayCheckWindup(F)` on tick 30. Contact is the tick on which the take reaches frame 10 (montage position 0.167 s). For the body check that is also the forward peak of the hand and stick, as E2 measured it. The cross-check's hand peak is its follow-through at take frame 16.

| F | Cross-check, frame 10 on tick | Body check, peak on tick | Off by |
|---|---|---|---|
| 4 | 34 | 34 | 0 |
| 8 | 38 | 38 | 0 |
| 12 | **40** | **40** | **2 early** |
| 14 | **40** | **40** | **4 early** |

Both events start at `Clamp(10 - F, 0, 10) / 60`, so for F > 10 the take starts at frame 0 and contact lands on call + 10. Clay and night figures behave the same.

## 2. What I changed (both `BP_SkaterVisual_TB1` and `BP_SkaterVisual_TB1_P3`, Blueprint DSL)
- **New variables:**
  - `WindHoldLeft` (float)
  - `WindHoldMon` (AnimMontage reference, added through `BlueprintEditorLibrary.add_member_variable`)
- **`CrossWind`** (run by `PlayCrossCheck` when the wind-ups are on and nobody holds):
  - The play call is unchanged.
  - After it: if the effective frames (`Frames`, or N when Frames is 0) are > 10, `WindHoldMon` = the active montage and `WindHoldLeft` = (F - 10) / 60, with a log line `XC12 wind hold frames <F-10>`.
  - Otherwise `WindHoldLeft` = 0.
- **`PlayCheckWindup`:** the `Montage_SetPosition` is unchanged, followed by the same rule on `Frames`.
- **New event `WindHoldTick`**, run from `ReceiveTick` while `WindHoldLeft` > 0:
  - It subtracts what the take has advanced since the last tick (`Montage_GetPosition`) from `WindHoldLeft`, then puts the take back to frame 0.
  - When `WindHoldLeft` <= half a frame, the hold is spent and the take runs on from frame 0.
  - If the figure is down or getting up, or the active montage is no longer `WindHoldMon` (a knockdown, a new call, the brace's release, a pin, any take), the hold ends and that other take is left alone.
  - It has an unused input `Dt`, left over from the first version (it is called with 0.0).
- **Why it counts on the take's own clock and not on ticks:** it is ordering-proof.
  - I first measured a pause with a tick countdown. In the harness it held one frame too many, then exactly right once corrected.
  - In the match the same countdown would have held only 1 frame: the game calls from the game mode's tick, before the figure's own tick in the same frame (engine frames in the log: call c, resume c+1).
  - Counting the take's actual advance holds exactly F - 10 of the mesh's updates, whatever the order of the call, the mesh and the figure.
- **One visual side effect:** during the hold the drawn pose is the take's frame 1 rather than frame 0. Both are the stance start of the wind-up, and no step was visible on the GPU.
- **Not changed:** `ChkCutWind` (`PlayCheck` with N > 10 through `SetWindupTakes`). A always sends `PlayCheckWindup(F)` right after `PlayCheck`, and that call applies the rule.

## 3. The numbers (headless, 26 test figures per run: TB1 and P3 each, cross-check and body check at 4, 8, 12 and 14, plus 5 robustness figures)

| F | Cross-check frame 10 (TB1 / P3) | Body check peak (TB1 / P3) | Wanted |
|---|---|---|---|
| 4 | 34 / 34 | 34 / 34 | 34 |
| 8 | 38 / 38 | 38 / 38 | 38 |
| 12 | **42 / 42** | **42 / 42** | 42 |
| 14 | **44 / 44** | **44 / 44** | 44 |

- **F = 4 and 8 against before** (run `xc_pre` against `xc_post4`):
  - The montage position is identical on every tick from 30 to 60.
  - The joints differ by **0.000 cm** from tick 36 on (the take at full weight) through tick 174.
  - The only differences are before the call: 0.07 to 2.1 cm, in every figure alike. That is the idle base pose, which differs between any two sessions: two runs of the same Blueprint (`xc_post2` against `xc_post3`) differ by 0.28 / 2.14 cm in the same place.
  - One exception, not mine: the body check picks its side and size at random per call. P3's F = 8 drew `_Light` in one run, so its curves cannot be compared.
- **Robustness, F = 12** (both figures):
  - R1: a knockdown on the next tick. The fall, the lie and the get-up play as before (takes and positions identical to the unchanged run).
  - R2: a new `PlayCrossCheck(4)` on the next tick. It plays from frame 6 as before; the hold is dropped.
  - R3: the brace (`SetCrossCheckHold(true)`) arrives on the next tick. The wind-up still lands on tick 42, and the later `PlayCrossCheck(0)` plays the release as before.
  - R4: a 3-frame hit-stop (time dilation 0.0001) during the hold. Contact on tick **45** (= 42 + 3): the hold freezes with the figure.
  - R5: `PlayCheckWindup(12)` and then a knockdown. Identical to before.
- **The same timing on the GPU:** frame 10 on tick 42 (gamecam run) and tick 102 (side run, call on 90).
- **Seeded clay match** (`clayA`, seed in the cartridge, scripted human, hit-stop 3, headless, to sim 7200; the shipped bot settings 8 / 12):

  | | Before (unchanged figure, same build) | After |
  |---|---|---|
  | Blueprint errors / `Error:` lines | 0 / 0 | 0 / 0 |
  | Knockdowns | 6 | 6 |
  | Get-ups ending on the release (`end_f` = `down_f`) | 5 of 5 (97, 150, 84, 125, 122) | the same |
  | Sim lines (knockdown, getup, stumble, stickhit, pin, check, crosscheck, crosshold + JAMSIM hashes) | 75 | 75, **identical** (frame 7200 `cfc677349d89475b`) |
  | Bots' `PlayCrossCheck(12)` | 9 | 9 holds of 2: 8 spent 2 engine frames after the call; 1 ended by the board-pin take that replaced it (the pin played untouched) |

  - The "before" match ran on the same build with the pre-change `.uasset` reloaded from my backup.
  - The disk then got the changed files back (SHA-256 matches the saved post copies).

## 4. The film (GPU; what I looked at with the Read tool)
- **Game-camera strip:** `C:\Users\kazda\jam\wt\b-fig\devlog\shots\20261114-211800-B-xc12-gamecam-strip.png` (+ `.txt`)
  - Two clay figures: the left one gets `PlayCrossCheck(12)`, the right one `(8)`, on the same tick.
  - **Seen:** the left figure holds its stance while the right one already coils with the stick back. It then runs the same coil and thrust about 4 frames behind.
  - At tick 42 the left figure is at its shove; the right one is past its shove.
  - Panel 2 (tick 31) came back as a blank capture (a screenshot fault, not the figure).
- **Side strip at hip height:** `...-xc12-side-strip.png` (+ `.txt`)
  - **Seen:** the near figure (12 frames) stands for the held frames, then coils and thrusts two-handed at chest height.
  - No pop or snap where the hold ends.
- **The contact frame from the game camera:** `...-xc12-gamecam-contact.png` (+ `.txt`)

## 5. Not verified
- The human's figure: A sends it 4 frames, so it never reaches this code.
- `PlayCheck` alone with `SetWindupTakes(true, N > 10)` (see section 2). A does not send that.
- Sound: not touched.

## 6. For device A
- Nothing to call differently. `PlayCrossCheck(12)` / `PlayCheckWindup(F > 10)` now land the take's contact on the call + F.
- Two new log lines when the figure's log is on: `XC12 wind hold frames N` and `XC12 wind hold spent` / `ended by another take`.
- Merge `lane/b-fig` (the two `.uasset` files below) if you take faults of this size.

## 7. Files changed
- `Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1.uasset` (11656719 → 11838919 bytes)
- `Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1_P3.uasset` (11865740 → 12047934 bytes)
- `status/XC12-report.md`
- `devlog/shots/20261114-211800-B-xc12-*` (3 stills, 3 captions)

## 8. Ledger rows (they replace these files' rows in ASSETS.md)

| path | kind | size_mb | origin | tool | licence | url | date | device | shipped | note |
|---|---|---|---|---|---|---|---|---|---|---|
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1.uasset | other | 11.29 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | as the part-3 E2 row above, and: HoldTick sets the ABP's bHoldIn (first 0.1 s of the hold), bHoldOut, HoldInT, HoldOutT and HoldLoopT ((t-0.1) mod 0.9833 s); test call TestChkCut(Size, bRight) (lane E2, fourth hand-back); and: PlayCrossCheck / PlayCheckWindup with Frames > 10 hold the wind-up take on frame 0 for Frames-10 of its own updates (WindHoldTick from ReceiveTick, variables WindHoldLeft and WindHoldMon), so contact (take frame 10) lands on the call + Frames; Frames <= 10 unchanged (lane XC12, A-to-B-0031 ask 1) (11838919 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1_P3.uasset | other | 11.49 | original | UE 5.9 editor via MCP/Python (Blueprint DSL) | | | 2026-11-14 | B | y | as the part-3 E2 row above, and: HoldTick drives the hold layer's in, loop and out, and TestChkCut, as BP_SkaterVisual_TB1 (lane E2, fourth hand-back); and: the wind-up hold past frame 10 (WindHoldTick, WindHoldLeft, WindHoldMon) as BP_SkaterVisual_TB1 (lane XC12, A-to-B-0031 ask 1) (12047934 bytes) |

## Lab
- Scripts and logs: `C:\Users\kazda\AppData\Local\Temp\claude\6d8eaecb999746c2\1f5c7b1e-557b-4c70-b752-8370d9dbef0b\scratchpad\e2\`
  - `xcwind3.mjs` (the change), `xcgen.py` / `fa_XC.*` (test figures), `xcana.py`, `xccmp.py`, `xccmp2.py`
  - `logs\w6_xc_pre.json`, `w6_xc_post4.json`, `full_xcm_pre.txt`, `full_xcm_post4.txt`
- The DSL and `.uasset` backups before the change: `xcbk\`
