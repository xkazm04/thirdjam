| mode | level | W-L-D-unf | win % (regular) | GF / GA (regular) | touches/min (regular) | checks L/T (regular) | character | mode | regular | change | verdict |
|---|---|---|---|---|---|---|---|---|---|---|---|
| Blind Side | pro | 14-6-0-0 | 70 (35) | 2.5 / 1.7 (1.9 / 2.4) | 21.5 (23.9) | 3 / 3.5 (3.6 / 1.9) | dekes / match | **1.85** | 0.75 | +147 % | pass |
| Cage | pro | 3-17-0-0 | 15 (35) | 2.8 / 5 (1.9 / 2.4) | 23.7 (23.9) | 5.4 / 4.4 (3.6 / 1.9) | checks landed / match | **5.4** | 3.6 | +50 % | pass |
| Marathon | pro | 16-3-0-1 | 80 (35) | 6.8 / 4.1 (1.9 / 2.4) | 21.4 (23.9) | 8.7 / 5.3 (3.6 / 1.9) | minutes per goal | **1** | 0.66 | +53 % | fail |
| Pond | rookie | 13-7-0-0 | 65 (100) | 1.6 / 1.2 (3 / 0.3) | 24.6 (22) | 0 / 0 (1.2 / 0) | passes + received / play min | **2.22** | 1.44 | +55 % | pass |
| Power Play | pro | 11-9-0-0 | 55 (35) | 3.4 / 2.7 (1.9 / 2.4) | 22.7 (23.9) | 6.1 / 2.2 (3.6 / 1.9) | passes + received / match | **4.85** | 1.65 | +194 % | pass |
| Sniper | pro | 11-9-0-0 | 55 (35) | 2.2 / 2 (1.9 / 2.4) | 25.8 (23.9) | 2.7 / 1.5 (3.6 / 1.9) | human goals per human shot | **0.31** | 0.16 | +93 % | pass |

Lane A72: the table above is `node tools/playtest/modes.mjs --seeds 1-20 --max-frames 48000` with the new keys (Marathon without `--sprint` shows A67's pace number; seed 8
is a 7-7 sudden death that ends at frame 49,284, `modes-keys-marathon-seed8.jsonl`). Marathon's own number needs the sprinting playtester:

| mode | level | W-L-D-unf | win % (regular) | GF / GA (regular) | touches/min (regular) | checks L/T (regular) | character | mode | regular | change | verdict |
|---|---|---|---|---|---|---|---|---|---|---|---|
| Marathon (`--sprint`) | pro | 17-3-0-0 | 85 (40) | 6.9 / 4.6 (2.1 / 2.4) | 23.3 (26.3) | 11.2 / 6 (4.3 / 1.6) | human sprint frames / match | **2525.6** | 654.45 | +286 % | pass |

Before (A67's files on this binary, `a72-variants/before.*`, 20 seeds): Cage checks landed 4.9 (win 15 %, taken 6.9), Blind Side dekes 1.0 (win 25 %), Marathon 5:00
human sprint frames 1312 with `--sprint` (win 60 %), minutes per goal 0.92 without. Regular pro on this binary: 35 % wins, checks landed / taken 3.6 / 1.9.
