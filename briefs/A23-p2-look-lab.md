# Lane A23: P2 look lab on device A (from H9.3)

You are lane A23 of the Third Jam (team Hokej). The operator at H9: "we are not close to P2"; the 4090 sits idle between windows. You run a look lab in your own worktree
with your own editor and measure what moves the picture toward the P2 reference (painted cel, night arena, toy figures), without touching device B's assets.
Read first: `docs/HOCKEY.md` (art ladder P2), `docs/experiments/LOOK-P2-NIGHT.md`, `prep-reference/` stills (reference only: never import them), `contracts/look-tiers.md` (C-0002),
`engine`-side look code in `Game/Source/JamGame/Private/{RinkActor,LookTierSwitcher,ToySkaterVisual}.cpp`, the post-process toon ramp and outline passes (grep `CustomDepth`,
`Toon`, `Outline` under `Game/Source/JamGame` and `Game/Content/`), B's reports `status/B*-report.md` on `origin/dev-b` (read-only `git show origin/dev-b:<path>`), R-0021 point 4
(props out; night sky, ice surface, construction, lighting), `devlog/shots/` for today's stills, `tools/mcp/mcp.mjs` and `tools/run/editor.ps1` (one editor per lane, your own port).

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a23-look` (detached at 6a2681e). Build it yourself (`tools/run/build.ps1 -Project <worktree>\Game`); unity is off,
so it compiles like a clean checkout. Your editor runs from that worktree on MCP port 8011 (`tools/run/editor.ps1 -Project <worktree>\Game -Port 8011`); stop it whenever you are
not capturing. At most three editors run on device A at once: before starting yours, check `tasklist` for `UnrealEditor.exe` and wait if there are three. Run no git command
in the worktree (the Director cherry-picks what you report); no Shipping package; never touch `Game/Plugins/JamAssets` (device B's).

You own in the worktree: `Game/Content/LookLab/**` (your materials, post materials, test maps), `Game/Source/JamGame/Private/LookLab*.cpp` (console commands for the lab only),
`docs/experiments/LOOK-LAB.md`, `status/A23.md` in the main tree (status only).

## Job: measured experiments, each a before/after still pair at 10, 24 and 40 m, same seed, same frame
1. **Toon ramp**: two, three and four bands; band softness 0.02 / 0.08; a cool shadow tint vs neutral. Which reads as paint rather than as flat shading at 24 m.
2. **Outline**: width by distance (constant 2 px vs 1.5 to 3 px scaled), colour (ink vs darkened base), inner lines from normals on the figures.
3. **Ice surface**: painted scuff and frost tiles (make two tiles procedurally in the material or with Codex per R-0008: one image per run, keep the prompt beside the output),
   spot reflections on the ice (planar vs screen-space vs Lumen), the puck's halo against each.
4. **Night volume**: the dark above the rink: black vs deep blue gradient vs a faint haze card; the spot pools' falloff; rim light on the figures.
5. **Construction**: boards with a kick plate and a glass line, nets with a visible frame; the far net's readability at 40 m with and without the outline.
6. **Cost**: `ProfileGPU` per variant on the 4090 at 1440p; anything over 1.5 ms per effect is marked.
Pick the best combination, render a 6-still proof (3 distances x play/faceoff) and write a one-page recommendation in `docs/experiments/LOOK-LAB.md` with the parameter values and
asset names B would need (B owns the shipped masters; your materials are a proposal).

Status in `status/A23.md` with real `date` timestamps. Report: the recommendation, the still paths, the cost table, what B needs to change. Time box 2 hours; interim at 45 minutes.
