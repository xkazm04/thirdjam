# Lane A103, track F3: F3-arena-blender (device A, evolution 1, Fable)

You are lane A103 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full**
(how a track works, the machine's band and the cursor, the walls, the status file, the report with the next three evolutions). Then `docs/EVOLUTION.md` and `docs/VISION.md`.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a103` (the Director created it at the commit that carries the whole vision: contact by inertia, the
movement laws, the hockey brain, the ladder of R-0046). Your status file: `status/A103.md`. Your remote-console port if you run a game: 4703.

## Track F3: the arena in Blender
Family: `tools/blender/arena/**` (the scripts are the source of truth), `Game/Content/Arena/**` (the imported meshes: A's own content; a ledger row in `ASSETS.md` for each,
origin `original`, tool "Blender Python script"), `RinkActor.*` for placement only, `docs/experiments/ARENA.md`. Read `docs/experiments/BLENDER.md`, `contracts/assets-naming.md`,
`contracts/look-tiers.md`, `docs/experiments/RINK.md`, `CLAY-RINGS.md`, and look at device B's banner (`devlog/shots/20261113-160913-B-br3-banner-1920.png`) for the clay voice.
First evolution: **the benches and the penalty boxes.** Two team benches and two penalty boxes outside the boards on the long sides, to the rink's dimensions (parametric by
the rink's length and width as script arguments), in the clay voice (chunky, slightly uneven, thumb-pressed edges), under 3,000 triangles each, with UVs; no hand-made
materials: find the material instances the rink's boards use in `RinkActor.cpp` / `LookTierSwitcher.cpp` and assign those by reference (on tiers without them the engine's
basic material). `RinkActor` places them from the cartridge's rink size and hides them on tiers the look contract says are bare. Find how a boxed skater (the sim's penalty
state) is placed today and say what the box needs to hold it.
Import headless (a Python import script run with `UnrealEditor-Cmd`; one interactive editor on MCP port 8103, closed after, only if the import cannot be done headless).
Proof: the Blender renders of each piece, the import log, the triangle counts, the ledger check green, the gate green; a still from the game only if you hold the cursor.
Tell device B in your report what you made and which of its materials you reference.

Time box 90 minutes for this evolution. Interim in the status file at 40 minutes.
