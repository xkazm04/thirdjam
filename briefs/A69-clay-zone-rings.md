# Lane A69: zones as opaque emissive ring meshes on the clay tier (device A, from H14, B-to-A-0034 point 3 / 0035 point 4a, Sonnet)

You are lane A69 of the Third Jam (team Hokej). Device B rebuilt the clay tier (`p3_clay`, now the shipped look, R-0036) against the operator's concept: glossy scratched
ice with computed reflections. The concept's zones and pickups are glowing clay rings that reflect in the ice; A's zones are translucent decals, which never appear in
screen-space reflections. B's ask (handoffs `B-to-A-0034` point 3, `0035` point 4a, accepted by the Director): on `p3_clay`, spawn a thin ring mesh per zone on the zone's
outline with B's zone material.

Read first: `handoff/B-to-A-0034.md`, `handoff/B-to-A-0035.md`, `Game/Source/JamGame/Private/RinkActor.cpp` and `EffectsBinder.cpp` (how zones are drawn today: the decal
per zone, its material instance `MI_Zone_<kind>`, the zone's shape from the cartridge: `game.zones[]` circle or rect), `LookTierSwitcher.cpp` (how a tier is applied and
`SetGlassForTier`), `Game/Content/Cartridges/hockey.boost.cartridge.json` (zones and pickups), `docs/experiments/CUTS.md`, `engine/kits/KIT.md`-equivalent here: there is
none; meshes come from `/Engine/BasicShapes` or a `UProceduralMeshComponent`; B offers `SM_ZoneRing` if asked (ask for it in the report; build the ring procedurally
meanwhile), `tools/eyes/README.md`.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a69` (created at the current main; build it yourself, unity off; one editor on MCP port 8034 only while
proving; at most three editors on the machine). Run no git command; never touch `Game/Plugins/JamAssets`; never print `.env`; never kill a process you did not start.
Presentation work never changes the sim hash; no hand-made materials (B's `MI_Zone_<kind>` on the mesh; a missing instance falls back to the decal).

You own: `RinkActor.*` (the ring meshes: a flat ring 6 cm high on the zone's outline, width 12 cm, a circle as a torus-like procedural ring, a rect as four boxes; hidden on
every tier but `p3_clay`; the decal stays on the other tiers), `LookTierSwitcher.cpp` (the tier switch shows or hides them), `docs/experiments/CLAY-RINGS.md`,
`docs/experiments/img/a69/**`, `status/A69.md`.

## Job
1. The ring per zone on `p3_clay`, B's material on it, the decal hidden there; the live tier switch (`jam.livecut` to a clay cut and back) shows and hides them with no
   actor left behind (count the ring components in the log after three switches).
2. PIE stills on `hockey.boost` forced to `p3_clay` (a cartridge copy with `game.look.tier`): the rings reflecting in the ice from the game camera, one from 10 m; eyes
   `bug-hunt` and `readability` on both.
3. Frame cost: `stat unit` before and after on the clay tier with six zones (A58's `tools/perf/match.ps1` can run it headless if PIE numbers are noisy).
4. `CLAY-RINGS.md`: the geometry, the material binding, the stills, the cost, and the ask to B for `SM_ZoneRing` if the procedural ring shows seams.

Report: the stills, the cost, the file list. Time box 60 minutes.
