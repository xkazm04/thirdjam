# Nightly (by day) package of main, smoke launch, size (lane A57, device A)

One Shipping package of main per hour when main moved (`tools/qa/nightly/loop.ps1`, 10:00 to 16:00 CET on 2026-11-13; the brief said until 06:30, the loop runs by day now).
Each row: the commit packaged, the time the hour's check started, the size (zip as the contest measures it, staged folder), the package folder under `Game/Packaged/`,
the cook warnings counted in the UAT log (distinct `Warning: ` lines, because UAT echoes every line twice and a raw count is double; `missing usage flag`; `LoadErrors`), the 30 s smoke launch of the Shipping exe
(`tools/qa/nightly/smoke.ps1`: window up, process alive, peak working set, one still of the menu that is not black), and how long package and zip took.
The Shipping exe writes no log, so `LoadErrors` is read from the cook log only. A row says `held` when free memory was under 10 GB at the check.
Size history of each package: `tools/size/history/<stamp>-<name>.json` (written by `tools/run/package.ps1`).

| main | check | size | package | smoke | time |
|---|---|---|---|---|---|
| 59675fe | 10:13 | staged 363 MB (no zip: the parity package) | Hokej_Hokej_N1015 (distinct warnings 1, usage flag 0, LoadErrors 0) | PASS alive 30 s, peak 1165 MB, menu still mean 26.8 | 4 min cook+package (by hand, `parity-packs.ps1`) |
| dfb808f | 10:45 | zip 242.8 MB / staged 364.1 MB | Hokej_Hokej_N1045 (distinct warnings 1, usage flag 0, LoadErrors 0) | PASS alive 30 s, peak 1279 MB, menu still mean 26.8 | 238 s cook+package+zip |
| d630740 | 11:45 | zip 242.6 MB / staged 364 MB | Hokej_Hokej_N1145 (distinct warnings 3, usage flag 0, LoadErrors 0) | PASS alive 30 s, peak 1103 MB, menu still mean 26.8 | 201 s cook+package+zip |

## Notes

- **N1145 (d630740) has two cook warnings that N1045 (dfb808f) did not**: `r.TSR.History.ScreenPercentage` and `r.Shadow.Virtual.MaxPhysicalPages` set `SetByScalability` "was ignored as it is lower priority than the previous SetByProjectSetting" (values stay 100 and 1024). A scalability group now sets two cvars that `DefaultEngine.ini` also sets; the project value wins, so nothing changes in play, but the scalability setting of those two is dead. The other warning, `LogOnline: OSS: TryLoadSubsystemAndSetDefault: LoadSubsystemModule([NULL]) failed`, is in every row (no online subsystem is configured).
| 1a33523 | 12:39 | zip 242.6 MB / staged 364.1 MB | Hokej_Hokej_N1239 (distinct warnings 3, usage flag 0, LoadErrors 0) | PASS alive 30 s, peak 999 MB, menu still mean 26.8 | 322 s cook+package+zip |
