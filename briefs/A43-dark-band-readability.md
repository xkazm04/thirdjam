# Lane A43: figures readable in the dark bands (device A, from H12.1, Fable 5.2)

You are lane A43 of the Third Jam (team Hokej). Three lanes measured the same fault tonight: figures in the dark side bands of the night rink are unlit black silhouettes; team colours
read 1 of 5 at 24 m (A33's hit frames, A37's eight cut stills, A30's strips). Read first: `docs/experiments/TACKLING.md` (eyes lines), `docs/experiments/CUTS.md` (bug 2),
`docs/experiments/FIGURE-BAKEOFF.md` (readability), `docs/experiments/LOOK-LAB.md` (A23's measured changes: the pools, the fill, the dome; what was rejected and why: light rims lit the
ice), `handoff/B-to-A-0023.md` point 4 (B: `RimStrength` 1.2 to 1.5 on the skater material instances worked, light rims did not), `handoff/A-to-B-0008.md` (B is applying A23's
changes in lane B15), `status/B13-report.md` and `status/B15-report.md` (B's rig values), `Game/Source/JamGame/Private/{RinkActor,LookTierSwitcher,JamSkaterVisual,SkeletalSkaterVisual}.cpp`,
`tools/eyes/README.md`, R-0021 point 4, R-0027, R-0030.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a43` (the Director creates it at the current main; build it yourself, unity off; one editor on MCP port 8024 only while
measuring; at most three editors on the machine). Run no git command; never save B's masters or `Game/Plugins/JamAssets`; A proposes with dynamic instances and measurements, B
owns the masters.

You own: `docs/experiments/DARK-BANDS.md`, `docs/experiments/img/dark-bands/**`, `docs/experiments/eyes/a43-*.json`, `Game/Content/LookLab/**` (new instances), the skater visuals'
material-parameter calls (`RimStrength`, `FillStrength` if the master exposes it) and the rig's local-light scaling in `LookTierSwitcher` (a per-tier fill light attached to the
human's skater is allowed if it costs under 0.2 ms), `status/A43.md`.

## Job
1. Measure first: the human's figure and an opponent in the darkest band (the side boards between pools) at 10 / 24 / 40 m on `humanoid_b1` and `generated_b2`: luma of the jersey,
   the band, the ice; eyes `readability` on each still (team colours, carrier, puck).
2. Try, each as a dynamic instance or a runtime parameter, with the same stills and the eyes verdict: `RimStrength` 1.2 / 1.5 on B's figure instances; a figure fill parameter if
   present; a soft shadowless fill light attached to the human's skater (radius 6 m, warm, no shadow, cost measured); the band's own luma raised by the ice `Ambient` floor; an
   outline colour lift (the ink lightens in the bands). Reject anything that lights the ice or costs over 0.3 ms, as A23 did.
3. Pick the combination that brings team colours to at least 3 of 5 at 24 m in the band without touching the pools, and write the exact parameter values for B's masters
   (`MI_Toon_Blue/Orange`, `MI_Ice_Night`, `M_PP_Outline`) plus what A applies at runtime.
4. `docs/experiments/DARK-BANDS.md`: the luma table, the stills, the eyes verdicts, the cost, the recommendation and the handoff draft text for B.

Report: the recommendation with numbers, the still paths, the cost, the file list. Time box 75 minutes.
