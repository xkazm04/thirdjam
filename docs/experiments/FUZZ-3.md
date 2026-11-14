# FUZZ-3: the release chain runs the window rows itself (lane A179, track S3, evolution 5, 14 Nov 2026)

## Item 1: the rows `release.sh` calls after the acceptance run (all under `tools/qa/accept/`)
Run them where nothing else drives a window (they take the cursor and post input to their own window), as the acceptance run is run.

```
powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\accept\release-rows.ps1 -Package <Development package dir> -Zip <release zip> -Name <tag>
powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\accept\release-menu.ps1 -Package <Development package dir> [-Name x] [-Port 4779] [-Scenarios return,click]
powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\accept\release-ship.ps1 -Zip <release zip> [-Name x] [-PlaySeconds 30] [-Keep]
```
Exit codes (all three): 0 pass; 1 the package is wrong (do not hand it over); 2 could not run; 3 machine (another window or game disturbed the row). `release-rows.ps1` repeats a machine
row once and exits 3 only if it stayed machine; a 1 outranks a 2 outranks a 3. Last stdout line is JSON (`thirdjam.releaserows/1`, `releasemenu/1`, `releaseship/1`); per-row stills
and `rows.txt` under `tools\qa\out\release\<Name>-menu|-ship`.
- **menu row** (Development package, clean profile: the package's `Saved` is moved aside and put back): PLAY NOW by one Return, and by a click at the place `jam.menuscreen where` prints
  for PLAY NOW. Each: exactly one `JAMMATCH` and one `JAMCART loaded`, the last `JAMMATCH` is `level=rookie source=settings`, a `JAMLOOK tier=` line, the still 9 s after the press lit >= 0.90.
- **ship row** (the zip is unpacked into `tools\qa\out\release\<Name>\unz`, removed after): window found, first screen not dark, Return, the match lit >= 0.90 after 9 s, 30 s of skating
  with a still every 5 s all lit >= 0.90, the window's close ends the process with code 0 and no new `Saved\Crashes` folder. A Windows Security / firewall dialog holding the foreground
  (a first start from a new path) is a warning only: posted input does not need the foreground.
- Proof: `Accept_H30` menu row PASS (return lit 0.964, click lit 0.964, one build each); `Accept_H27` menu row FAIL (return: JAMMATCH x2, JAMCART x2, level pro source cartridge, lit 0.011:
  the seeded fault); `Hokej_Hokej_H30.zip` ship row PASS (6 stills, lowest lit 0.954, close code 0); `Hokej_Hokej_H27.zip` ship row FAIL (lit 0.0111). The click row on H27 fails earlier
  (that package has no `where` verb).

## Item 2: the acceptance drill without a mate (`HokejAcceptDrill.cpp`)
Steps 5 and 6 need a mate and steps 7 and 8 a non-back opponent; with none the Stage guard logs each verb of the step as `step=S verb=V ok=0 ... skipped=no_mate|no_opp`, goes on, and never
reads `Sk[-1]`. `accept.mjs` shows such a row as SKIP (not a fail). Test: `node tools/qa/accept/nomate-test.mjs` runs the drill (steps 5 to 8, headless, `-nullrhi -Multiprocess`) on two staged
cuts, `nomate-cut.json` (2 a side with a back each) and `noopp-cut.json` (the other team two backs): both reach the summary with the skipped verbs and no crash line. Not done: the run on
a binary without the guard (`--expect-crash`), none was kept. H30's own Marathon is 3 v 3 and does not trigger it.

## Item 3: the pass-key probe (`accept.ps1`)
Each key gets 2 s and the Q/F pair is re-probed until a `btn=pass held=1` line arrives or 5 s have passed; `events.log` says the round and how late. The twelve modes on `Accept_H30`:
eleven 19 of 19 plus the pixel row (lit 0.878 to 0.974), pond 16 of 19 + 3 by design (check, crosscheck, checked), pass key Q in round 1 (0.06 s) every time, no assertion line in any `game.log`.

## Item 4: Shipping compile in the gate
`node tools/gate/shipping-gate.mjs --uproject Game/ThirdJam.uproject [--base main] [--force]` runs `tools/run/build.ps1 -Config Shipping` when the branch changed `Game/Source/**`, a
Target.cs / Build.cs, `.uproject` or plugin Source; otherwise SKIP. Cost: 124 s for a forced incremental Shipping compile; a planted `!UE_BUILD_SHIPPING`-only symbol called from unguarded code fails it in 7 s (C3861). A new .cpp file is not seen until UBT re-globs. Not wired into `premerge.ps1` (needs SOURCE OPEN).
