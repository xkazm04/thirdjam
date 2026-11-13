# B35 report: the streak fault, the cross-check one size up on clay, the hard board crash (device B)

Lane B35, device B, machine clock 16:52 to 17:17. Tree `C:\Users\kazda\jam\wt\b-vfx` (`lane/b-vfx` = `dev-b` 5a510ca), editor on port 8000 (pid 40928), left through `ExitEditor` at 17:12 with no dirty package; the process is gone (the one `UnrealEditor` left, pid 17308, is the look lane's on 8002; not touched). Saved: `VFX/NS_slide_trail`, `VFX/NS_board`, `VFX/NS_lift`, each with no play session open and each confirmed by the tool's flag and the file's new size and time. No Blueprint saved, no git writes, no build, no cook. `jam.ScriptHuman`, `jam.ctrl.Drill`, `jam.Cut` set back to 0 / 0 / empty; `jam.hitstop` never changed (3). Backups of the three assets as they were at lane start: scratch `bk/`. Scratch: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/b35` (`spec35.mjs` the three systems, `tk.py` with `watch2` the real-knockdown watcher and per-tick recorder, `real3.sh`, `posed35.sh`, `chain.sh`, `rec_*.txt` the recordings, all frames in `shots/`).

## Verdicts
| Item | Verdict | Still |
|---|---|---|
| 1. Streak fault | **PASS on your test, one blemish left.** Six real knockdowns with the hit-stop on (three on clay, three on night, each frozen at +4, +10, +20, +40 from the game camera): no line longer than the body's slide in any of the 24 frames. Left: in the hard knockdown on night, a short fan of stripes under 1 m at the start of the streak at +20 | `b35-streak-fault-before-after` |
| 2. `NS_lift` on clay | **PASS at game size in one real cross-check**: the mark can be made out at the call at full game size (drill 2); splinters gone by +6. Night is changed too and was not seen clearly | `b35-crosscheck-clay-options`, `b35-crosscheck-clay` |
| 3. `NS_board` hard against light | **PASS posed, not seen in a real match**: Strength 1.0 is 26 larger chips along the board foot and a snow cloud that lasts about 10 frames; Strength 0.5 unchanged. Shake line: **removed for good**, cause not found | `b35-board-light-hard` |
| 4. Night pale edge; detached patch | **Not done** (pale edge left as it was). The detached patch is the same fault as item 1 | |

## 1. The cause of the lines, plainly
The scrape quad took its **length (`30 + 0.11 x speed`) and its direction from `Engine.Owner.Velocity`, with no limit**. That value is the component's move since the last tick divided by that tick's time step. Two things feed it wrong numbers:

- **A frozen or slowed world clock (how stills are taken).** With the B33 asset and no freeze, the real knockdown shows **no lines** at +10 or at +20 (one knockdown per tier). After a single freeze at +10 or +20 (time dilation 0.0001, a time step of about 2 microseconds) the thin lines are there a few frames later (clear on night, faint on clay). Any move of the owner inside such a tick gives a speed thousands of times too high and a quad kilometres long, drawn as a thin line through the spot. The frame-crossing lines in `sk18-slide-streak` and in my B33 real frames are this. Which tick spawns them I did not isolate.
- **The catch-up after the hit-stop (in play).** Recorded per tick in six knockdowns: the hit-stop holds the figure for 3 ticks (dilation 0.0001 on the visual and the pawn, position constant), then the figure catches up. In the hard knockdown (the 7 m slide) it moves at up to **26 m/s for three frames and then slides at about 10 m/s**, not the 5 m/s B33 assumed. With the old formula that is quads 1.4 to 3.2 m long at the start of every hard slide, in normal play. Computed from the recording; I did not see it as a line in an un-frozen frame, because the un-frozen knockdown I captured was the slow one (peak 6 m/s).

Not the cause: a position jump at the hit (largest move in one tick: 19 cm) and the hit-stop's hold itself (the position does not change during it).

**Made impossible** (all in the spawn expressions of both emitters):
- nothing spawns when the time step is under 0.5 ms (a frozen world, or the hit-stop if the component's step follows the actor's dilation; not established whether it does);
- nothing spawns above 12 m/s, on a move of more than 30 cm in one step, or in the first 0.1 s after activation;
- the speed used is capped at 8 m/s and the quad length at 90 cm.

**The blemish left:** knockdown 2 on night at +20 shows four or five parallel stripes about 0.5 to 0.9 m long where the streak starts (third row of the still, last tile). They are the first quads after the 0.1 s gate, laid while the figure is still at about 10 m/s, at an angle to the path; why at an angle I did not find. On clay the same place is a soft smudge. By +40 the streak is the continuous band.

## 2. `NS_lift`
Two options, posed on clay (`b35-crosscheck-clay-options`): **option 2 chosen and saved**: three pale-hot stars with five thick dark brown-red spokes round the contact. Option 1, the stars alone in dark brown-red, draws thin outlines and is weaker.

| Emitter | B33 | B35 (clean) |
|---|---|---|
| Crack | 3 amber stars 18-26 cm, ring 26 cm at 0.9 m, 3 frames | 3 pale-hot stars (1.0, 0.90, 0.62) **26-34 cm**, ring 24 cm at **1.0 m**, 3 frames |
| Scar (new) | | 5 dark brown-red (0.38, 0.07, 0.03) spokes **5 x 24 cm** on a 20 cm ring at 1.0 m, 3 to 4 frames |
| Wood | 6 splinters 4 x 13 cm, 0.12-0.17 s | **8 splinters 5 x 16 cm**, darker (0.30, 0.15, 0.05), **0.07-0.1 s (4 to 6 frames)** |
| Tape | 3 chips 4.5 x 6.5 cm, 0.12-0.17 s | 3 chips 5 x 7 cm, 0.07-0.1 s |
| Kick, Puff | 6 chips, 2 puffs at the skates | unchanged |
Miss unchanged (two tiny chips). The ring is kept; `User.Dir` still moves it 30 cm to the attacker's side when A sends it.

In the real cross-check (drill 2, the human on a bot): at the call the pale stars read on the orange victim at full game size and the dark spokes read less on orange and black than on blue; splinters at +2 and +4, nothing at +6. A second lift 8 frames later puts the mark on the human's blue back, where the spokes read well.

## 3. `NS_board`, skater variant (`H = saturate((Strength - 0.5) x 2)`)
| Emitter | B33 | B35 |
|---|---|---|
| Body | `4 + 14 H` chips, 3.5-5 to 5-9 cm, radius 25 cm | `4 + 22 H` chips, 3.5-5 growing to **7-14 cm**, spread over `25 + 40 H` cm, faster and 0.08 s longer at H 1 |
| Puff | `1 + 3.2 H` | unchanged |
| Cloud (new) | | `9 H` blue-grey (0.60, 0.67, 0.83) soft puffs 44-60 cm at H 1, at 14 cm, pushed into the rink, 0.22-0.32 s |
| Shake | 2 lines above Strength 0.75 (never showed) | **removed** |
At Strength 0.5 nothing changed. On clay the cloud draws as a grey-blue smudge, darker than snow.

## Particle counts (from spawn counts and lifetimes; none measured live)
| System | B33 | B35 |
|---|---:|---:|
| `NS_lift` clean / miss | 20 / 2 | **27 / 2** |
| `NS_board` skater, Strength 0.5 / 0.75 / 1.0 | 5 / 13 / 24 | **5 / 21 / 39** |
| `NS_slide_trail` live at full slide speed | about 210 | about 210 (0 in the first 0.1 s) |
Worst case, two knockdowns and a cross-check overlapping, then both trails full with a hard board crash: about **486** (B33: 465).

## Stills (`C:/Users/kazda/jam/wt/b-vfx/devlog/shots/`, each with `.txt`)
`20261113-171511-B-b35-streak-fault-before-after.jpg`, `20261113-171511-B-b35-crosscheck-clay-options.jpg`, `20261113-171511-B-b35-crosscheck-clay.jpg`, `20261113-171511-B-b35-board-light-hard.jpg`. The streak still shows +10 and +20 (and two +40); the +4 frames and the rest are in scratch `shots/` (`aC-kd1..3-f4/10/20/40`, `aN-kd1..3-...`).

## Assets (bytes, `Game/Plugins/JamAssets/Content/VFX/`)
| File | Before -> after |
|---|---|
| `NS_lift` | 1,192,018 -> 1,414,652 |
| `NS_board` | 986,942 -> 993,939 |
| `NS_slide_trail` | 525,400 -> 528,873 |
`NS_poke` and `M_Fx_SlideSoft` unchanged. No asset created.

Ledger rows (replace the three existing rows):
```
| Game/Plugins/JamAssets/Content/VFX/NS_lift.uasset | vfx | 1.35 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-13 | B | y | event lift as a cross-check: Variant 0 three pale-hot stars 26-34 cm and five dark brown-red spokes at 1.0 m for 3 frames, eight splinters gone by +6 frames, a small snow kick at the skates, peak 27; Variant 1 two tiny chips; User.Dir read, default zero (lanes B3b, B17, B33, B35) (1414652 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_board.uasset | vfx | 0.95 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-13 | B | y | event board: Variant 0 low puck chips; Variant 1 a skater's crash scaled by (Strength - 0.5) x 2: four chips and one small puff at 0.5, 26 larger chips along the board foot and a snow cloud at 1.0, leaving the face along User.Normal; no shake line; peak 39 (lanes B3, B17, B30, B33, B35) (993939 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_slide_trail.uasset | vfx | 0.50 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-13 | B | y | slide trail while a knocked-down figure slides: small soft blue-grey puffs and one continuous soft streak of velocity-aligned quads fading over 1.5 s; speed capped at 8 m/s and quad length at 90 cm, nothing spawned on a near-zero time step, above 12 m/s, on a 30 cm jump or in the first 0.1 s; played by SlideFx on BP_SkaterVisual_TB1 and _TB1_P3 (lanes B30, B32, B33, B35) (528873 bytes) |
```

## What I could not verify
- Which tick of a freeze spawns the long quads, and whether the Niagara component's time step follows the actor's time dilation during the hit-stop.
- The over-long first quads in un-frozen play with the old asset (computed from the recorded speeds only).
- The cause of the short stripes at +20 on night; the streak in motion (stills only).
- The new cross-check on night with a clear view (the one real night lift was half hidden); the options still shows clay only; a real miss.
- The hard board crash in a real match and on night; the cloud's colour on night.
- The three knockdowns per tier are the same three on both tiers (the drill is deterministic), so the proof is three kinds of knockdown, not six.
- Cook: not run.

## Decisions for you
1. The stripes at the start of a hard slide on night: accept, or let B lengthen the start gate from 0.1 s to 0.25 s (the streak then starts about 15 frames after the knockdown instead of 6) or drop the pale edge, which is what makes them read as stripes. My recommendation: drop the pale edge; it adds little and item 4 asked for a decision on it anyway.
2. The board cloud is grey-blue and reads as a smudge on clay: keep (it is unmistakable), or lighten it towards snow at the cost of contrast on the cream boards.
3. The dark spokes read on blue and weakly on orange and black; the pale stars carry the mark there. Keep both, or make the spokes thicker (7 cm).
4. For A, unchanged from B33: `Dir` for `lift`. New for A: the knocked figure moves at up to 26 m/s for three frames after the hit-stop (the catch-up) and slides at about 10 m/s in a hard knockdown; if that is not intended, it is theirs to look at.
5. A house rule worth writing down: any effect that reads `Engine.Owner.Velocity` must clamp it, and stills taken by freezing the world clock can show faults that play does not.
