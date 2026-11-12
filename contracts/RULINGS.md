# Director rulings on open contract questions (append-only, newest last)

## R-0001 (H5, 2026-11-12 10:50): answers to lane A0's open questions
1. **Skater visual swap key:** `game.skater.visual` enum `toy | humanoid_b1 | generated_b2`, default `toy` (absent = toy). Lane A4 adds it to the schema; the game mode
   picks the visual component class by it; device B's test cartridges set `generated_b2` / `humanoid_b1`.
2. **`SetTeam` takes `FLinearColor`.** The cartridge carries `colour_hex`; the loader converts once. Nobody else parses hex.
3. **Event payload gains `Kind` (FName, `NAME_None` when not applicable)** for `pickup_spawn` / `pickup_take` (the pickup kind) and `knockdown` (the cause: `check` | `boost`).
   Payload is now `{ Who, Where, Strength, Kind }`; lane A1 extends the struct, the bus and the log line (`kind=` appended).
4. **Effects binder lives in `JamGame` (lane A5 owns `UJamEffectsBinder`)**: a component on the game mode that listens to the bus and spawns, by soft object path, the assets
   named by convention in `contracts/events.md` (`/JamAssets/FX/NS_<event>`, `/JamAssets/Audio/MS_<event>`); a missing asset logs once and is skipped. A content-only plugin cannot hold code.
5. **`SetTier` is called by the game mode** after the cartridge loads (`game.look.tier`), and again on a Director's Call that changes the tier.
6. **Extra prefixes accepted:** `SK_ SKEL_ AS_ ABP_ PM_ DA_` join `contracts/assets-naming.md`.
7. **`tools/gate/check-assets-ledger.mjs` exists** (lane B2 built it, 45 tests); A0's note was wrong. It runs at every merge window.
8. **Ownership reconciliation:** the seam stubs under `Game/Plugins/JamAssets/Content/Rigs/` and the four contracts were created by A0 on the Director's behalf; from now on
   `ownership.json` stands: B owns the rig maps, the Director owns `contracts/**`.
9. **Packaging:** Development packages are measured but never shipped; the size gate applies to Shipping only. `package.ps1` keeps failing Development on size; that is noise, not a gate.

## R-0002 (H5.5, 2026-11-12 11:20): answers to device B's handoffs B-to-A-0001..0005
1. **B-to-A-0001 accepted.** `tools/run/common.ps1` gains `Set-EngineDotnet` (forces the engine's bundled x64 .NET on an ARM64 host; no-op on AMD64), called by `build.ps1` and `package.ps1`.
2. **B-to-A-0002 accepted, lands later.** `register-h0.ps1` will register through `Register-ScheduledTask` with a `[datetime]`, `StartWhenAvailable`, and battery settings off; the date bug is real
   (A's task was registered for 11 December as well; it would not have fired even without the reboot). Not urgent while the jam runs.
3. **B-to-A-0003 accepted.** `ownership.json`: `briefs/**` moves to `shared.any_device`; `GAME.md` is `director_only`.
4. **B-to-A-0004: the folder is `VFX/`** (the contracts win; R-0001 point 4 is corrected to `/JamAssets/VFX/NS_<event>` and `/JamAssets/Audio/MS_<event>`). `BP_EventFx` is struck from C-0003.
   The binder (lane A5) sets Niagara user parameters **`User.Strength`** (float) and **`User.Colour`** (linear colour: the team colour of `Who`, white when `Who` is -1), spawns at `Where` in
   world space, auto-destroy; MetaSounds get a float input **`Strength`** and play as a one-shot at `Where`. B delivers only `NS_<event>` and `MS_<event>`.
5. **B-to-A-0005 accepted in full.** Lanes A1 (rink, nets, puck, ice) and A5 (toy skater) set `render_custom_depth` and the stencil values from B's table (ice 2, boards/nets/figures 1, puck 3,
   trim strips and beacon 0 with no shadow) and assign B's material instances by the object paths `/JamAssets/Look/<name>.<name>`, falling back to the engine default when absent, never failing.
   A1 drives `RinkHalfLength` and `RinkHalfWidth` (cm) on the dynamic `MI_Ice_Night` instance from `game.rink`, and spawns the 10 cm emissive trim strips (`MI_Trim_Blue` x < 0, `MI_Trim_Orange`
   x > 0) and the two far-net beacon posts (`MI_Trim_Beacon`, d 12 cm, 8 m, behind each net) from the rink size. The table is an addition to C-0002.
6. **Device B's machine is not the plan's machine**: Windows 11 ARM64, 64 GB unified memory shared with the GPU, the editor running x64 under emulation. TRACKS.md section 5 caps for B are
   therefore **4 builders and at most 2 editors**, and B's lead measures before each new editor as it already does. B's H6 row is expected at about H9; accepted.
7. **New event name** `directors_call` (lane A7, payload `Kind` = the tunable key, `Strength` = the new value as a float where meaningful) joins C-0003.

## R-0003 (H4.9, 2026-11-12 10:55): seams from lanes A2 and A7, and the bot-match gate
1. **`AiBotPlays` on `hockey.regular` (120 x 60):** the "match ends within 36,000 frames" clause is relaxed to "at least one goal within 18,000 frames" for pure bot-vs-bot; the
   finish clause stays on `hockey.arena.small`. Evidence (A2, 20 seeds): regular finishes 2 of 20 bot-vs-bot matches in 10 minutes, small 16 of 20; with the scripted human both finish.
   Lane A3's `all_star` profile is expected to finish regular-rink matches; that is the real fix.
2. **Rink size evidence for the operator** (A2): seconds per goal with the scripted human 130 (120 x 60) vs 70 (90 x 45); bot-vs-bot 363 vs 127. Default stays 120 x 60 until the operator rules.
3. **Game mode seams (lane A1 resumes):** add `directors_call` to `JamEvents` (`Who` -1, `Where` zero, `Strength` = position of the new value in the tunable's range 0..1, `Kind` = key);
   `ApplyCartridgeLive(UJamCartridge*)` on the game mode changes the running config without resetting the match (sim setters for the live-changeable tunables); call
   `UJamDesignerSubsystem::NotifyPeriodStarted` at each TAKE and `SetCurrentCartridge` with the booted cut; `PublicIncludePaths` for `JamCore/AI`; stage `Content/Designer/` as UFS next to `Cartridges/`.
4. **Console names ignore case:** `jam.Cut` (cvar) and `jam.cut` (command) collided and crashed the editor; the command is `jam.playcut`. Rule: every new console name is grepped across `Game/Source` first.
5. **Shared Source tree:** two lanes broke the build for all others this hour (a missing module dependency, a missing include). Rule: a lane runs `tools/run/build.ps1` before every pause and
   every report, and never leaves a half-written file in `Game/Source`. The Director fixes a cross-lane break on sight and names it here.
6. **`A2`'s private build copy** `C:/Users/kazda/kiro/a2-build` is deleted by the Director. Lanes that need a private copy use a git worktree under `.worktrees/` (TRACKS.md 5), not an ad-hoc folder.

## R-0004 (H5, 2026-11-12 11:00): operator decisions on rink size and the Designer's transport
1. **Rink:** `hockey.regular` stays **120 x 60** as the default cut. `hockey.arena.small` (90 x 45, speed 11) stays in every test and sweep and in development as the comparison; lanes A3 and A10
   keep reporting both. No lane changes the default without a new ruling.
2. **Designer transport without a charged API key:** the operator has no billed key; the Claude Code CLI subscription is available for development. The Designer gains a second transport,
   **`FJamCliTransport`**, which spawns `claude -p --model claude-haiku-5-5 --output-format json --max-turns 1` (fallbacks `claude-sonnet-5-5`) with the same assembled prompt on stdin, parses the
   `result` field as the reply, and applies the same validation and repair chain as the HTTP transport. Selection order at runtime: API key present -> HTTP; else `claude` executable found on PATH
   and the build is not Shipping (or `bAllowCliTransport=True` in `DefaultGame.ini` for the operator's demo build) -> CLI; else offline cuts. The **submitted build ships with CLI off**
   (rule 7f: no additional software; subscription terms for a shipped service are not ours to assume) and relies on offline cuts plus the key field. The CLI transport is also used **now** to
   generate the shipped offline cut library (24-40 sentence -> cut pairs, validated, committed under `Game/Content/Designer/`), and to run the Designer eval for real first-try numbers.
   The dev log says plainly how the live feature was developed and what the judge's build does without a key.

## R-0005 (H5, 2026-11-12 11:05): device B's caps (handoff B-to-A-0006) and the first look merge
1. **B-to-A-0006 accepted:** device B runs **up to 5 builders and 3 editors**, each new editor only with 24 GB free; builders without an editor are not counted against the editor cap.
   Added rule from B's own numbers: with more than one editor rendering, GPU utilisation sits above 90%, so **no frame-time or GPU measurement is believed unless taken with one editor up**.
2. **First look merge (dev-b -> main, 11:03):** `/JamAssets/Look/` carries the painted-cel masters `M_Toon`, `M_Ice`, `M_Trim`, the post passes `M_PP_ToonRamp` / `M_PP_Outline` and seventeen
   instances with the names from C-0002 and R-0002 (ice, board, net frame/mesh, helmet, stick, puck, blue/orange/dark toon, trim blue/orange/beacon, puck rim and halo); `LR_L2_Night` is filled;
   `Arena/` holds the sound-stage dressing (289 basic-shape actors). Lane A1's rink and A5's toy figure already load these by path, so the next editor start on A shows the look. Lane A10 judges
   readability on it; B2's dressing pass 2 and B3's puck kit and effects follow. B's device-B briefs are tracked again now that `briefs/**` is any_device.

## R-0006 (H5.2, 2026-11-12 11:15): NVIDIA track and partner-prize requirements (operator's relay of the organisers' text)
> "Competing for the NVIDIA RTX Award? Install NvRTX, NVIDIA's branch of Unreal Engine. Your Unreal Engine game needs to use at least one feature from NvRTX, NVIDIA DLSS, or NVIDIA ACE.
> Answer the NVIDIA questions when you submit. Keep track of the tools you use. When you submit, list them in the 'Tools you used' field. Some partner prizes depend on it, including
> Hyper3D's $10,000 cash pool for teams that used Rodin and the NVIDIA RTX Award."
1. **Eligibility path: NVIDIA DLSS in the shipped build** (Super Resolution at least; Frame Generation if the plugin's 5.9 build offers it), through NVIDIA's official DLSS plugin for Unreal
   on the stock 5.9 engine. Lane A11 researches the exact plugin build for 5.9 (source, size, licence, EULA terms for redistribution in a jam build) and **reports before any download**;
   the operator approves the download. Integration: plugin in `Game/Plugins/` (ledgered as sourced, with the licence), a quality switch in Settings (DLSS auto / quality / balanced / performance / off),
   default on when an RTX GPU is detected, off otherwise; the packaged build must run on non-NVIDIA hardware with DLSS off. The `tools used` field and the NVIDIA questions name it.
2. **NvRTX** stays the optional trailer experiment on device A (5.8 branch, cannot open the 5.9 project); it is not the eligibility path.
3. **NVIDIA ACE** is out of scope (no NPC conversation in this game).
4. **Hyper3D Rodin:** handoff A-to-B-0001: if the operator has a Rodin account, device B's skater lab produces at least one shipped asset through Rodin (a prop or a figure base mesh,
   rigged in Meshy if needed), ledgered as `ai-generated` with the tool named, so the entry qualifies for the pool. No account, no Rodin: say so in the ledger.
5. **Tools used** (`docs/SUBMISSION.md`): Unreal Engine 5.9, NVIDIA DLSS (if integrated), Claude Code (Opus, Sonnet, Fable, Haiku), Meshy (figures, motion), Hyper3D Rodin (if used),
   Codex image generation (reference studies only), Blender (evaluation renders), ffmpeg. The NVIDIA questions are answered from `docs/experiments/DLSS.md` at submission.

## R-0007 (H5.4, 2026-11-12 11:25): AI voice side track (operator): barks, shouted signals, ACE
Operator's words: "experiment with ACE too as a side track - think about AI extensions, first phase purely for trash talking PG15+, opponents would mock or complain based on the
situation in game. Ideal state would be to shout at AI teammates for short strategy signals and get understanding for defending, going to free space, or just free complimenting."
1. **Content law:** the entry must be teen-appropriate (official rules, 13+). "PG15" is read as sharp but clean: mockery, complaints, bravado and praise with no profanity, slurs, sexual
   content, threats, or references to real people, brands or teams. Lines come from a **reviewed bank** generated offline (through the Claude Code CLI transport, R-0004) and filtered by a
   word list and a reviewer pass; **no raw model text reaches the player at runtime**. The bank is English, tagged by situation and tone, and committed as data.
2. **Phase 1, barks (ships at M2):** `UJamBarkSystem` (JamGame) listens to the event bus and the sim (goal for/against, check landed/received, puck stolen, stall, near miss, win/lose, a
   Director's Call) and picks a line for the speaking skater (opponents mock and complain, teammates encourage and complain at the human, the human's own figure stays silent), with a
   cooldown and no repeats; shown as a speech bubble over the figure and in a HUD strip; optional voice through the engine's own text-to-speech if any is free of downloads, else text only.
   Cartridge key `game.barks: { enabled, intensity: "quiet" | "normal" | "loud" }`; the Designer may set it ("make them talk trash").
3. **Phase 2, shouted signals (M3 stretch):** intents `defend | space | pass | praise` as `FJamTeamIntent` events the brain honours for N frames (teammates fall back, spread to free ice,
   the puck carrier looks for the human, a praised teammate gets a brief skill boost); inputs in this order: keyboard/gamepad (1-4 or d-pad, always shipped, judges have no mic), then
   **voice through Windows' built-in speech recognition** (no download, offline-capable), then NVIDIA ACE's ASR if phase 3 exists. Teammates answer with a bark ("got it", "on him").
4. **Phase 3, ACE (NVIDIA eligibility, experiment):** lane A12 researches whether an ACE for Games plugin exists for UE 5.9 (local NIM/LLM, Riva ASR, TTS, Audio2Face), its downloads, sizes,
   licence and GPU requirement, and whether a jam build can ship it; **reports before any download** (operator approves). If it ships, ACE voices the barks and hears the signals on NVIDIA
   hardware with the text path as the fallback everywhere else.
5. Ownership: lane A12 owns `Game/Source/JamGame/Bark*`, `Game/Source/JamGame/Signal*`, `Game/Content/Barks/**`, `docs/experiments/ACE.md`, `tools/barks/**`; the brain hook
   (`FJamTeamIntent` consumption) is a one-struct seam lane A3 implements in `JamBrain` from A12's header.

## R-0008 (H5.4, 2026-11-12 11:20): Codex image generation and editing for in-jam 2D content (operator: "allowed to use codex and its image generation or edit at any capacity")
1. From now on Codex image generation and editing may produce **shipped 2D content**: title and menu art (film slate, clapperboard, "Choose your cut" cards), UI icons (cuts, pickups,
   signals, settings), jersey crests and number decals (generic, invented), ice markings and board decals, scoreboard faces, painted texture tiles for the cel look (2K cap, tileable),
   pickup glyphs, the itch page cover and screenshots' frames, trailer title cards. The pre-jam reference stills stay reference only (nothing from `prep-reference/` is used).
2. Every generated image that ships is `ai-generated` in `ASSETS.md` with tool `Codex image generation`, the prompt file path in `url`, and `size_mb`; the tool is listed in "Tools you used".
3. Content law applies: no text that reads as a real brand, team, league or person; no logos but ours (invented); English; teen-appropriate. Prompts are kept with the generator.
4. The generator is `tools/images/gen.mjs` (lane A8 lifts it from `prep-reference/tools/gen.mjs`, one image per run, a job list file per batch, outputs to a scratch folder, only the picked
   files are imported). Lane A8 owns menu/UI art on A; device B owns textures, decals, crests and scoreboard faces (handoff A-to-B-0002); A6 owns the itch page and trailer cards.

## R-0009 (H5.7, 2026-11-12 11:40): link windows on device A, and lane A3's gate changes
1. **Link windows.** Five editors held the project DLLs at 11:36 and no lane could link. From now on, on device A: **every lane closes its editor at :00 and :30 for five minutes**
   (`tools/run/editor.ps1 -Stop`, or `ExitEditor` through MCP), and keeps the editor closed whenever it is not actively in PIE or capturing. A lane that needs to link outside a window
   builds its own git worktree copy (TRACKS.md 5) instead of the shared tree. Partial builds with `-ModuleWithSuffix` are forbidden (they removed `UnrealEditor.modules` once).
2. **Lane A3's deviations accepted:** `PlayerBotWins` (WIN at seeds 1, 2, 7 with identical hashes) is gated at `rookie`; at `pro` the gate is "the human scores at least once and the match
   has a result or reaches 36,000 frames with the human ahead or level in at least 2 of 3 seeds". `HumanHasSomethingToDo` treats the 10-minute window as a full match.
3. **Difficulty resolution** (A3): profile `rookie | pro | all_star` -> explicit `game.ai` numbers -> Designer values; presets carry `ai: { difficulty: "pro" }`; the game mode calls
   `Brain->ApplyCartridge(*Cartridge); Brain->ResetBrain();` (patch applied by the Director), or the game plays `pro` with the old hold radius whatever the cut says.
4. **Curve target restated** (from A3's first measurement, 20 seeds, scripted human): regular rookie 85 % / pro 50 % / all_star 25 % holds; small rink is too easy at every level (95 / 65 / 55)
   and gets its own profile offsets. `FJamAIIntent` (the R-0007 hook) already consumes defend / space / pass / praise.

## R-0010 (H5.8, 2026-11-12 11:45): the default rink is 90 x 45 (operator: "lets then keep 90x45 as default")
1. `hockey.regular` becomes **90 x 45 m, skater speed 11** (the measured small-rink cell: a goal every 70 s with the scripted human, 16 of 20 bot matches finish, the two-minute first
   play exists). Lane A3 makes the change in the presets (it is editing them) and re-targets the difficulty curve on it: rookie 85 % / pro 50 % / all_star 25 % on the new regular.
2. **120 x 60 stays a shipped cut**: `hockey.arena.large` (120 x 60, speed 8) replaces `hockey.arena.small`; `hockey.arena.wide` (140 x 70) stays. Tests and sweeps run on regular and large.
3. `GAME.md`, `HOCKEY.md` section 2 and the menu descriptions follow (Director edits the docs; lane A8 reads descriptions from the cartridges).
4. The camera defaults are unchanged (measured for 120 m; on 90 m the far net is nearer, which only helps); the far-net marker stays.

## R-0011 (H6.5, 2026-11-12 12:35): puck carrying, the instant goal, and the gates (lane A3's final report)
1. **Adopt `puck.drag` 0.5** in every preset: carrying is the fun (A10) and it gives +45 % touches and 40 % shorter matches (A3). It hands the human every match at the current
   profiles, so lane A3 **re-tunes the three profiles on drag 0.5** to the same targets (rookie 85 / pro 50 / all_star 25 on regular). The other four feel changes (glide drag 0.3, grip 4,
   accel 16, reach 2.0) are **not** adopted together; A3 may take at most one more if the curve needs it and says which.
2. **Net guarding:** rookie's back blocks long shots only (as shipped); pro and all_star block all. The curve wins over the clause.
3. **Gates:** `PlayerBotWins` runs on **regular only** at rookie (seeds 1, 2, 7: WIN, identical hashes), with **no** first-goal-frame clause at rookie; the clause "first goal after frame 600
   in at least 2 of 3 seeds" applies at **pro**, where the backs block. `AiBotPlays` on `hockey.arena.large` requires movement, puck path and shots only (no goal clause): guarding backs make
   bot-only big-rink matches goalless, which is fine for a cut nobody plays bot-only.
4. **Large Rink cut ships in timer mode** (`rules.mode timer`, 3 minutes = 10,800 frames) so it always ends; regular stays first-to-3.
5. Lane A3's `LiveConfig` fix (loads `hockey.arena.large` for the held-back-geometry check) and the intent plumbing (`FJamAIWorld` intent fields filled by `GatherInputs`) are accepted.

## R-0012 (H7, 2026-11-12 12:55): phase 2, depth on P2 (operator's direction)
1. Operator: "each dimension we touched is in very basic game design and gameplay state ... master game on P2 ... let players design custom boosts, rules and aspects ... in parallel upgrade into
   P3 and P4, where mainly in P4 we will be glad to make even regular hockey game work." Chosen depth, all four at once: composable effects (A14), rule hooks / variants / zones / obstacles (A15),
   puck control / passing / charged shots / deflections (A16), cuts as progression (A17). Contract C-0005 (`contracts/depth.md`) fixes the wire names and two seams; A16 lands the seams first.
2. Device B: P2 to full polish and P3 clay in parallel from now; P4 only after P3 ships, bar = "regular hockey works and looks broadcast-like"; the L2 GPU hang first (handoff 0006).
3. "Ahead of schedule" is retired as a status word: the milestones are met, the dimensions are first drafts. Gate 2 (H24) now reads: M1 plus effects, rules, feel and the season playable
   on P2, the operator's playtest done, P3 rig in progress.

## R-0013 (H7.3, 2026-11-12 13:15): the operator's first playtest of Hokej_Hokej_H7 (247.5 MB)
Operator's notes, verbatim: "Control: Ability to control stick with mouse and without puck with the mouse. Without to tackle with stick to make player fall, missing also clean tackles to take
over the puck with stick. It should also help last player in field to goalie. With stick to clean tackles. The tackles and hard fouls will be key for dynamic gameplay as now it is hard to take
puck from another player. / HUD elements and typography are ugly to look and read, with image generation tool we have cheap way how to create nice interface from the start. / Face-off needs
to be implemented to start the game. / No need to mention Director's cut in the name of the game."
1. **The stick is the core verb (lane A16, ahead of everything else in its brief):** the stick is aimed by the **mouse** (right stick on gamepad) relative to the skater: angle and reach;
   with the puck it steers the carry (dribble, drag, protect); without the puck it does **clean stick tackles**: a **poke check** that takes the puck when the blade reaches it inside a timing
   window (mistimed = a stumble, 20 frames of lost control), a **stick lift** at contact range that strips the carrier; **body check** (existing) stays the hard tackle that drops the carrier and
   becomes available in every cut (`check_enabled` default true; `hockey.raw` removes the stumble penalty and lengthens knockdowns); **hard fouls**: a check from behind or on a player without the
   puck is a foul: the fouled team gets a free faceoff at the fouler's end and the fouler sits 300 frames (`rules.penalties` becomes a real switch, default true in regular, false in raw).
   Wire names: `game.stick: { reach_m, poke_window_frames, lift_range_m, stumble_frames, mouse_sensitivity }`, input `IA_StickAim` (mouse delta / right stick), `IA_Poke` (left mouse / RB),
   `IA_Lift` (right mouse / LB), `IA_Check` stays (shift / B). Events `poke`, `lift`, `foul` join C-0003 (Who = actor, Kind = `clean | miss | foul_from_behind | foul_no_puck`).
2. **Faceoff (lane A16):** a real drop: both centres at the dot, a 90-frame countdown on the HUD, the puck drops, the first clean stick input after the drop wins it (early input = that skater
   loses the draw); bots draw with difficulty-based timing. `faceoff` event gains `Kind = drop | won | lost`.
3. **Last man as goalie (lane A3 patch, A16 proposes):** the `back` role, when the puck carrier is inside its blue line and no teammate is between, holds the crease, squares to the
   shooter and uses the stick (poke range) to block; the human can do the same with the mouse stick. No goalie figure, no new role.
4. **UI pass (new lane A18, Fable):** typography and HUD/menu visuals redone with generated art (R-0008): a consistent slate/film-counter kit (panels, 9-slices, score plates, TAKE card,
   bubbles, Director's Call card, cut cards), icons, a readable type scale from the engine's Roboto (sizes, weights, tracking, outlines over ice), high-contrast scoreboard at the top, bark
   bubbles with tails, faceoff countdown. A18 takes over the visual layer of `HUDWidget`, `BarkWidget`, `DirectorsCallWidget`, `MenuWidget`, `MenuStyle.h` and `Game/Content/UI/**`; logic stays.
5. **Name:** the game is **HOKEJ**. The subtitle "a director's cut" is removed from the slate and every screen; the Director's Call feature keeps its name; the theme is shown through the cuts,
   the takes and the Director's Call, not in the title.

## R-0014 (H7.6, 2026-11-12 13:35): season ladder details (lane A17)
1. `game.ladder.takes[].fallback` (a preset id) is added to C-0005: the take plays its fallback when its designed cartridge fails validation or references a primitive that has not landed.
2. The ladder as measured ships: levels rookie 1-2, pro 3-7, all_star 8; take 3's checks become a two-sided feature when lane A16's brain patch (bots check at pro and all_star) lands.
3. "Take" names both the in-match period and a season level; the menu and the end card use the take's title to keep them apart. Accepted.
