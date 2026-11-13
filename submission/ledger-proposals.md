# Ledger proposals for the Director (lane A96, 2026-11-13): nothing below was deleted

## 1. P4 under `Game/Content/P4` (device A's files): remove files and rows (R-0036: P4 descoped, P3 is the shipped look)
- 149 ledger rows start with `Game/Content/P4/`; every one has `shipped` = `n`; the folder is 100 MB on disk (`Anim` 24 clips plus `Proof`, `Rigs`, `Src`, `Manny`, `Maps/P4_Lab.umap`, `Meshy`, `Rodin`, `Rodin2`, `Tripo`, `Tripo3`, `Props`).
- Nothing in `Game/Source/**` or `Game/Config/**` names `/Game/P4`, so removing the folder cannot break a C++ reference. (Not proven for Blueprints inside JamAssets: B's `BP_SkaterVisual_*P4*` assets may reference `/Game/P4` skeletons: the editor's reference viewer decides, and that needs an editor.)
- Removing the folder also removes the nine `sourced` rows (Epic template Manny clips retargeted) that were the ledger's only sourced rows outside the shipped DLSS dll. Until then the nine now carry the licence and url, so the gate is green either way.
- Proposed command for the lane that owns it, after the reference check: delete `Game/Content/P4/` and the 149 rows (`grep -n "^| Game/Content/P4/" ASSETS.md`); gate stays green.
- Not done here: this lane has no editor and runs no git, so the reference check and the removal are the Director's.

## 2. Device B's rows (list for B, not touched)
- Six size mismatches, all `Game/Plugins/JamAssets/Content/Skaters/`: `BP_SkaterVisual_TB2` (1.6 listed, 2.6 on disk), `_TB2_P4` (1.6 / 2.4), `_TB2_P3` (1.6 / 2.7), `_TB2_old` (1.6 / 2.4), `_TB2_P3_old` (1.6 / 2.4), `BP_SkaterVisual_P4Lab` (1.84 / 2.7). Warnings only.
- 272 B rows mention P4 (`shipped` = `n`): same descope question for B's P4 skater files.
- The `_old` pair looks like leftovers, not shipped content: B decides.

## 3. Gate finding (fixed here, tell whoever owns the gate)
`tools/gate/check-assets-ledger.mjs` stops reading at the first non-table line. A paragraph in the middle of the table (the Director's ruling of 2026-11-12) ended it at row 1,401, so 253 rows (all of device A's `Game/Content/**` tail) were never validated. The paragraph now sits above the `## Ledger` heading; the gate reads 1,654 rows. It found one bad origin and nine sourced rows without a url (both fixed). Any future prose inside the table repeats the hole: the gate should fail when a table row follows a non-table line.

## 4. Information
- Device A's `vfx/font/other` shipped total is 56.2 MB, all of it `nvngx_dlss.dll` (sourced, NVIDIA licence); the gate's WARN against the 40 MB slice is that one dll, not our content.
