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

## R-0015 (H7.7, 2026-11-12 13:45): bark register (operator: "you can be also more offensive with the mockery. The current scripts are too safe and childish")
1. The bank is regenerated at a **harsher register**: cutting sarcasm about the player's skating, aim, speed, courage and decisions; bravado; gloating; genuine annoyance from teammates;
   the voice of a bored veteran, not a kids' show. **Mild language is allowed sparingly** ("damn", "hell", "crap", "ass" as in "kick your ass"; at most one in ten lines), which the official
   rules' Teen (13+) bar tolerates. Still forbidden: slurs of any kind, sexual content, threats of real harm, references to real people, brands, teams, bodies or identities, anything targeting
   who the player is rather than how they play.
2. Tones become `mock`, `savage`, `complain`, `bravado`, `encourage`, `praise`; `game.barks.intensity`: `quiet` = encourage/praise/complain only, `normal` = mock + the rest, `loud` = savage
   allowed. Regular ships `normal`; Raw ships `loud`. The filter keeps its hard list and adds the mild-words quota; `REVIEW.md` is regenerated for the Director's skim.

## R-0016 (H8, 2026-11-12 14:05): answers to device B's handoffs B-to-A-0007..0011 (second merge window, dev-b at 5886a1b)
1. **0007 wiring accepted.** The binder (A5, R-0002) already sets `User.Strength`, `User.Colour` and the MetaSound `Strength` on exactly B's names; lane A16 (owner of the pawns) attaches the
   puck kit: `SM_PuckDisc` as a child of the puck with `bAffectDynamicIndirectLighting=false`, `CastShadow=false`, stencil 0, no collision, plus `NS_puck_trail`. A one-editor PIE check on A proves both.
2. **0008 accepted:** track T-B1 is **B's scripted original skater** (5.3k tris, 24 mannequin-named bones with a real root, ten exactly-looping clips, a real stick; `original` in the ledger);
   the Fab pack is out of the plan. `game.skater.visual` IS read since `432791b`: `AHokejGameMode` loads `/JamAssets/Skaters/BP_SkaterVisual_TB1.BP_SkaterVisual_TB1_C` for `humanoid_b1` and
   `..._TB2_C` for `generated_b2` (B's paths match). The promotion rule runs with `hockey.test.visual-b2` through the Jam.Bot suite: lane A16 adds `Jam.Bot.VisualSwap` (both tracks load, bots pass).
3. **0009 answered:** (1) toy `Emissive` reset bug: A16 fixes `SetBoost(NAME_None)` to restore 2.2; (2) the persistent sky light and the off directional light in `Rink.umap` are hidden by
   `ULookTierSwitcher::SetTier` when a rig is loaded (lane A15); (3) A's under-ice floor moves to z = -20 cm (A15, `RinkActor`); (4) the puck is **radius 0.12 m = 24 cm diameter** by design,
   nothing to do; `GAME.md` says "0.12 m" meaning radius; (5) blue lines: the rule is **one third of the half-length** on every rink, goal lines at `half-length - 4 m`; A15 drives `BlueLineFrac`
   (0.3333) and a new `GoalLineOffsetCm` (400) on the dynamic ice instance so B's paint and A's triggers agree at any size.
4. **0010 closed:** `MenuStyle.h` and `Menu.umap` have been in git since `ec82e81` (12:40); B's check ran two minutes earlier.
5. **0011 accepted:** `SetTier` streams the rig with uniform scale `s = rink.length_m / 90`, then scales every `ULocalLightComponent`'s intensity by `s*s` and attenuation by `s`; re-applied on
   cartridge load and on a Director's Call that changes the rink (lane A15). B authors rigs for 90 x 45 from now on.
6. **GPU hang verdict:** B could not reproduce it in 7 single-editor runs and hardened the rig anyway (dressing casters off, spots tightened, VSM load about 80 pages); A's dumps were page faults under
   4-5 editors. **Lane A10 runs the one-editor 60 s soak on A in the 14:30 window** with B's grep list and `stat gpu`; the shipped tier stays `l2_night` unless it hangs with one editor.
7. **Operator items from B:** Hyper3D's licence wording for shipping Rodin output from this account, and whether the prize needs Rodin content in the build or only in the pipeline (B shipped
   seven props and the T-B2 base mesh through Rodin, so both readings are covered); the voice test on B needs a human at the microphone.

## R-0017 (H8.1, 2026-11-12 14:15): lane A14's names and seams; lane A12's register
1. Accepted as implemented: `effects[].zone`, `kind` enum (five looks + `custom`), `icon` enum (12), `max_on_ice` default 1, custom-effect magnitude as a deviation scale, `director` as any live
   cut or call with the human as source, triggers read from the sim's frame events (the bus stays cosmetic). Built-ins live under `Game/Content/Designer/builtin-effects.json` (A's folder).
2. `reverse_other` gets a real stat: lane A16 adds `EJamStat::SkaterControlsSign` (the sim multiplies the human's and bots' move input by it); until then the half-speed fallback stands.
3. The editor crash on `jam.say` (LoadCut from the Designer callback inside the TEDS processing window): lane A7 defers `LoadCut` to the next tick.
4. Brain: `FJamAIWorld` gains `Pickups[] { Kind, Pos }` filled by the game mode; only `all_star` detours for a pickup within 8 m, never while carrying and never when the puck is closer
   (lane A3 resumes for this, for pass awareness and interception once A16's pass lands, for poke/lift use at pro and all_star, foul avoidance, and the last-man-as-goalie rule).
5. Barks: the regenerated register ships; Regular `normal`, Raw `loud`; the Director skims `REVIEW.md` for cuts before H24. A's TextToSpeech package delta is measured at the next green Shipping package.

## R-0018 (H8.2, 2026-11-12 14:20): lane A16's names (seams, stick, faceoff)
Accepted as implemented: `FJamRuleRequest { Cause, Action }` as the queued form of SpawnPickup / ApplyModifier; `FJamSimEvent::Text` as the announce side channel; `OnFrame` fires every
frame and the hook filters by `every_frames`; first-to mode's `Period` advances only on `EndPeriod` (the TAKE card's "1 + lead" stays presentation); `ShrinkRink` with Value > 1 reads as
1/Value and `GrowRink` the mirror; `UJamCartridge::ApplyValueKey` as the one values-to-config mapping; `game.stick` defaults (reach 1.6 m, poke window 8 frames, lift 1.0 m, stumble 20,
sensitivity 1); the faceoff as a sim phase with a 90-frame countdown; fouls park the fouler 300 frames at the side boards and give a faceoff at the fouler's end. The Determinism
baseline hash moves with R-0013 (the faceoff changes play from frame 1); the new baseline is recorded by A16's `Jam.Core.Determinism` once linked.

## R-0019 (H8.3, 2026-11-12 14:25): lane A15's names and the zone seam
Accepted: `rules.hooks[].after_frame`; `score: { at_least, at_most, lead_at_least }`; event `shot_blocked` (Who = shooter, Kind = zone id) joins C-0003; B assets `MI_Zone_<kind>` and
`MI_Toon_Bumper` (handoff to B). The zone seam `IJamZoneSource` (`JamZones.h`) is implemented by lane A16 in the sim: `SetZoneSource`, ice speed/friction multipliers per skater and puck,
`BumpPuck` per substep, `BlocksShot` in `TryShoot`, pickups through `AllowsPickupAt`; obstacles `bumper` as a circle with restitution 1.6, `goal_shield` as a 0.3 m box. Brain (A3):
`FJamAIWorld` gains `SlowIce[] { Centre, Radius }`; the carrier steers around a slow circle within 3 m of its line to the net; bots never shoot from inside `no_shoot`.

## R-0020 (H8.4, 2026-11-12 14:30): operator decisions on licence, voice and verdict lists
1. **Rodin licence agreed**: Rodin output ships; `docs/SUBMISSION.md` lists Hyper3D Rodin under tools used; the ledger rows stand.
2. **Voice parked behind keys**: team signals ship on keys 1-4 and the d-pad; the SAPI voice path stays off by default and is not tested until the keyed signals prove fun in play
   (handoff A-to-B-0005 moves to "later"); the Settings row stays with its status caption.
3. **Verdict lists go to the operator as review pages**: any list of items that needs a human verdict (bark lines, feel proposals, generated art, cut sentences, cut lines) is rendered by
   `tools/review/build.mjs` into one self-contained HTML page: grouped items, pass / reject / comment per item, a "listen" button for text lines (browser speech synthesis), filters by
   group, a running count, and an export line (JSON and a markdown summary) the operator pastes back; decisions are remembered in the browser. The Director publishes it as a private
   artifact and applies the exported decisions. First instance: the bark bank.

## R-0021 (H8.5, 2026-11-12 14:40): operator verdicts on feel, the bark bank and the arena
1. **Feel proposals: all pass** (docs/reviews/feel-2026-11-12.decisions.json). Standing rule from the operator: until the game is playable and watchable, calibrations (physics, AI
   curve, feel numbers) need no operator approval; lanes apply and record them in docs/experiments/*.md, and the operator reviews calibrations again near the finished phase. The one
   comment (feel.00) is work: the stick must read as a hockey stick (shaft and blade), the figures need hands on it, and a carried puck must stay on the blade (A16 for the carry and the
   code-built toy figure's stick; B for the TB1/TB2 figures, handoff A-to-B-0007).
2. **Barks are hockey first.** The operator: "We are pushing the theme too far, it should be hockey first theme, keeping director cut in the menu for its adjustments." Every bark line
   that leans on film vocabulary (cut, take, director, script, scene, reel, credits, reshoot, rehearsal, frame, blooper, editor, prop, camera, clip, sequel, draft, tape, rewind, movie,
   screen test, wrap, rewrite, plot, footnote, headline, makeup: 224 of 1,340 lines) is rewritten as rink talk at the same register, same id, same tone and the same mild budget (lane A20).
   directors_call reactions speak of "the rules changed", "new rule", "your tweak", never of directors or cuts. The register of R-0015 stands.
3. **Director's Cut lives in the menu and the Designer, not on the ice.** In-match copy is hockey: PERIOD, FACEOFF, GOAL, the score; the card between matches reads as a match card
   (MATCH 3 OF 8), not a TAKE card; the feature name Director's Call stays as the name of the one live change, with hockey wording on its card. Cuts, takes and the season's film
   framing stay in the menu, the Designer and the season screen. Lane A18 applies this at its next pass. (Director's reading of "keeping director cut in the menu"; the operator corrects it if the in-match cards were meant to keep the film words.)
4. **Props out of the arena for now.** The operator: focus on the night sky, the arena surface, the construction and the lighting; the film props (Rodin set: lights on stands, chair,
   slate, dolly, crane, cases, truss lamps) come back later, placed to the final art style, to carry the concept that the whole game is a filming session. The assets stay in the plugin
   and in the ledger, unplaced. The painted backdrop flats count as props and wait with them; haze and light shafts are lighting and may proceed (B's atmosphere pass, veto window kept).

## R-0022 (H9.1, 2026-11-12 15:12): clean builds before every push; C-0001 v3; stick reach; B's Variant table
1. **No push to main without a clean-worktree build.** B-to-A-0017 was right twice: per-lane commits cut from the shared tree left `main` compiling only with device A's working
   tree (103 errors at 5760168 in a fresh checkout). From now on the Director commits the whole working tree at a window after the gate's build step, builds that commit in
   `.claude/worktrees/clean-main` (detached at the commit), and pushes only on PASS; `status/A.md` carries the line "clean build PASS <sha>" B waits for. Lane commits of subsets stop.
2. **C-0001 v3 signed** as lane A16 wrote it in `contracts/skater-visual.md`: `SetStickAim`, `PlayPoke`, `PlayLift`, `PlayPass`, `SetShotCharge`, `SetFaceoff`, `PlayFaceoffDraw`,
   `SetGoalieStance`, `PlayFoulReaction`, all BlueprintNativeEvent with empty defaults; the carried puck draws at the visual's `puck_socket` when exposed. B mirrors them on TB1/TB2
   (answer to B-to-A-0014).
3. **Stick reach**: `game.stick.reach_m` default moves from 1.6 to **1.2** (rest 1.0 m, full aim 1.6 m from the skater centre), which both of B's figures can pose; `lift_range_m` stays
   1.0, `shot.reach_m` 1.5. Applied by A16 at its next slot together with a re-recorded `Jam.Core.Determinism` baseline (calibration, R-0021 point 1).
4. **B's `Variant` table (B-to-A-0012) accepted** as the C-0003 addition; lane A21 writes it into `contracts/events.md` and the binder; `bark`, `signal`, `announce` are 2D; `period`
   stays silent. Zone, pickup and obstacle looks (B-to-A-0016) are assigned by A21 as B specified; tier `p3_clay` (B-to-A-0018) likewise, with `look.trim_emissive` per tier.

## R-0023 (H10.1, 2026-11-12 16:40): the phase-2 curve, the draw, the net slide
1. **Curve accepted** (lane A3, 60 seeds, worktree and shared-tree gates 61/61): regular 85 / 55 / 28, large 87 / 48 / 25 (rookie / pro / all_star), faceoffs without a human touch
   0 / 0 / 2 on both rinks. Profiles: rookie speed 0.96, red skill 0.88, stall 60 (large rink: 1.0 / 0.93), pro shot range reach + 13 m, carriers yield under the human's forecheck
   (pro after 20 frames, all_star after 12: pass, else dump).
2. **The draw guide of 30 / 50 / 65 is withdrawn**: the opposing centre moves early (and loses the draw) in every faceoff at rookie and pro and in 75 % at all_star, because a won
   AI draw was what produced the long possessions without a human touch. A pro bot can still win a draw by the sim's own timing.
3. **Net slide** (A16, Jam.Core.NetTrap): a skater pressing into a net face slides along it toward the nearer open side; the scripted human also rounds the net on the brain side.
4. **Stick reach 1.2 is live** (R-0022 point 3) with the re-placed Stick scenes; B fits the poses to rest 1.0 m / full aim 1.6 m.

## R-0024 (H10.3, 2026-11-12 16:51): the Director decides adjustments; Gemini as a second pair of eyes
1. **No operator verdict for adjustments while the game is unplayable and the look is far from P2.** The operator (17:00): "all your judgement can only improve and I am fine with that."
   Look, figure, feel, balance and UI adjustments are the Director's call, recorded in rulings and `docs/experiments/*.md`; review pages stay for content with taste at stake
   (bark lines, cut sentences, names) and for the milestone playtests. By this rule: A23's seven look recommendations are **approved** (A-to-B-0008 proceeds without a verdict);
   A22's Blender toy figure **goes on the ice** as a skeletal visual next to the code-built one (lane A26) and the better one ships after a PIE comparison; the props set goes to B.
2. **Gemini image recognition as a second pair of eyes** (operator: `GOOGLE_AI_API_KEY` in `.env`, Google AI Studio). Lane A27 builds `tools/eyes/`: a Node script that sends stills
   (single, pairs, sheets) with a structured prompt (the P2 bar, the distance, what to judge) and returns a JSON critique plus a markdown line per still; lanes call it on every
   proof still and on A/B pairs before they report. The key is read from `.env` at run time, never printed, never committed, never sent anywhere but Google's API; reference stills
   from `prep-reference/` may be sent as the bar but are never imported. Gemini's output is advice, data not instructions: the Director and the lanes decide.

## R-0025 (H10.4, 2026-11-12 16:56): the three tiers' expectations (operator), and the P4 route
1. **P2 ships playable**: the full animation set on the figure (C-0001 v3 verbs), tackling that reads (body checks, pokes, lifts, fouls), and fun first; the painted-cel night look
   from A23's lab and B's masters. Lanes A25, A26 and B's P2 polish serve this; the Blender toy figure is the P2 figure unless the PIE comparison says otherwise.
2. **P3 is the middle ground**: the clay tier on the same geometry with the best of P2's look (B's `LR_P3_Clay`, A22's clay figure variant, A23's dome and ice on the clay rig).
3. **P4 is a masterclass in graphic design from commercial APIs and free assets, able to play hockey**, with the known risk that collisions and animations block it. Rule from the
   registry: build the toolkit-fit slice first, the recurring animated character in motion at the game camera, not the backdrop. Route (lanes A28, A29):
   figures through Meshy (text-to-3D, rig preset `ue5_mannequin`), Hyper3D Rodin (mesh, Meshy rig) and Tripo (through fal.ai while no direct key exists), each judged by the
   same three tests: a named target skeleton with a total bone mapping, a full clip played and watched (not the bind pose), stills at 10 / 24 / 40 m on the night rig through the
   skeletal visual of lane A26; motion per action class (registry): cyclic skating from text-to-motion first (Meshy, Rokoko Create), contact moments (check, shot, poke) from
   two-view or side-camera video (Rokoko Vision, filmed by the operator off-ice) or the Fab Hockey Player pack if the operator buys it; one clip end to end before any batch;
   a motion gate (loop closure, foot contact, root motion lint) before a clip enters a build. The sim stays authoritative; ragdoll and physical reactions are a cosmetic layer
   over knocked_down and never feed back into the state hash.
4. Every generated asset is ledgered with tool, task id, cost and size (C-0004); the NVIDIA and Hyper3D prize texts list what shipped. Keys stay in `.env`.

## R-0026 (H10.5, 2026-11-12 17:05): the Fab pack is out; falls and collisions are a verb group
1. The Fab "Hockey Player Animated" pack is descoped (operator: outdated, no falls, no collision reactions). Strides from text-to-motion; contact moves from Rokoko Vision
   (YouTube or the operator's own footage, 30 s on the contact moves); falls, hit reactions and get-ups from the Meshy animation library and the engine's Manny clips.
2. Falling and collision reactions are a first-class verb group in C-0001 v3's table: knocked down from the side, from behind, a trip; get up; the stumble after a mistimed poke;
   a board collision; each with a source, a gate result and a contact window; a cosmetic ragdoll over knocked_down on the skeletal figures (A26, A28), never feeding the sim.
3. Tripo runs through fal.ai; Hyper3D's rigging (2026-10-23) is a fourth figure route if it has an API.

## R-0027 (H11.3, 2026-11-12 18:35): device B's H11.9 read; duplication ends; the figure bake-off; B's asks become lanes
1. **B was right about the duplication.** While A built a Blender toy figure (A22/A26) and a generated-figure route (A28/A29), B had four figures with every C-0001 v3 call,
   numbers, crests and the carried puck on the blade (`BP_SkaterVisual_TB1` rebuilt, `_TB2`, `_TB2_P3` clay, `_TB2_P4` broadcast), a star-dome rig without props, a broadcast
   rig, and the Rodin + Meshy-rig + scripted-clips route proven since 15:56. The Director did not read `status/B.md` between the H8 and H10 merges; from now on the Director reads
   B's status head and the open `B-to-A` list before every brief.
2. **The P2 figure is decided by a bake-off, not by lane** (lane A30, after the dev-b merge): `toy_skeletal` (A26), `humanoid_b1` (B's TB1v2), `generated_b2`, each on the night rig,
   same frozen frames at 10 / 24 / 40 m in play and at a faceoff, the stick verbs forced on the frozen frame, eyes `ab-pair` and `figure` on every pair, GPU cost per figure, and
   a bot-match clip of each. The Director picks from the table (R-0024 point 1); B's proposal (`humanoid_b1` default, `generated_b2` alternative) is the favourite going in
   because it has the most original motion and hands on the stick in every take. The clay and broadcast figures ride their tiers (B-to-A-0021).
3. **P4 route merged into B's**: A28 keeps only the image-to-3D look experiment (concept sheet -> Rodin / Tripo), judged against `_TB2_P4`; A29 keeps only what B lacks (falls and
   collision reactions from the Meshy library, contact moves from Rokoko Vision when the operator films), delivered as Manny-skeleton clips plus a note for B's `retarget.py`;
   neither spends credits on a figure B already has.
4. **B's asks become device A lanes**: A31 (code: figure class per tier and `SetNumber` at spawn with fixed numbers 7/13/42 and 9/21/88; floor Movable; `MI_Glass`; stanchions spawned
   with the glass by `RinkActor` so B removes its 66; rim 1.3 on the toy; `PlayShoot` no longer fired with every pass; `SetGoalieStance` from the brain's owner; bots charge
   their shots; tier `p4_broadcast` with `MI_Bcast_Glass` and `MPC_Look.Clay` reset in `SetTier`; pickup props at `PickupTop` with spin and bob, B's props preferred over
   the token; `CornerRadiusCm` on the dynamic ice), A32 (audio: stage and cook the VO bank, a commentary player beside the barks with B's rules, music and skate-loop hooks,
   the seven missing events). A22's props go to B as FBX (B-to-A-0022 point 2). B-to-A-0020 is answered by R-0022 (clean builds, unity off).

## R-0028 (H11.5, 2026-11-12 18:45): four unattended hours; Fable 5.2 on the 3D lanes; more scope for fun
1. The operator is away from 18:45 to about 22:45 CET. The Director runs unattended: a machine check every 30 minutes (CPU, RAM, GPU, disk, editors, build-tool instances) written to
   `status/A.md`; the dev-b merge, clean build and push at every window where B has pushed; a Shipping package every hour or so when the gate is fully green; B's status head and
   open handoffs read before every brief (R-0027 point 1). No destructive step without the operator: no deletes outside lane outputs, no force pushes, no contract rewrite.
2. **Fable 5.2 runs the 3D lanes** (figures, rigs, animation, scene and look work through the editor and Blender): the bake-off (A30), tackling that reads (A33), and any lane whose
   deliverable is judged by a still or a clip. Code and tooling lanes stay on the default model.
3. **Scope added for "fun first" (R-0025 point 1)**, started as editor capacity allows (three editors): A33 tackling that reads (hit reactions by direction, board collisions as an
   event with sound and reaction, stumbles, puck loss and spray, camera punch, all cosmetic over the sim); A34 a human-like playtester through the real input path with fun
   metrics per difficulty (touches, shots, goals, lead changes, time to first goal) and calibration proposals; A37 every shipped cut plays (the 30 Designer cuts and the eight
   takes through the bot sweep and the eyes tool on each cut's look); A38 lockstep multiplayer re-proven on the new sim. Queued behind them: HUD pass 2 (EYES.md), goalie saves
   as events for the commentator.

## R-0029 (H11.7, 2026-11-12 19:38): the curve is measured with the playtester, not the scripted human; three payoffs become scope
1. Lane A34's human-like playtester (novice / regular / good through the real input path) is the measure of the curve from now on. Targets: the novice wins 85 % at rookie and
   scores inside two minutes in at least half of 20 seeds (the M1 bar), the regular player about 50 % at pro, the good player about 25 % at all_star and never idles 15 s at
   all_star. Today's numbers: novice 15 / 5 / 10, regular 60 / 40 / 30, good 70 / 50 / 40 on the regular rink: rookie is far too hard for a first-time player.
2. **Calibrations applied** (R-0021 point 1, lane A3 in its worktree, reported in BALANCING.md and PLAYTEST.md): P1 the back's guard post 5 / 45 -> 10 / 20 m at rookie (per side
   if the sim allows, else global at rookie); P2 rookie speed 0.85 / red skill 0.75; P3 rookie shot range +5 m. Measured together: novice 85 %, 10 of 20 inside two minutes.
   The large-rink offsets get their own numbers from the same grid. Pro and all_star untouched until the regular and good players are measured against them again.
3. **Sim rule** (lane A16): no `foul_no_puck` when the victim carried the puck within the last 12 frames (today two of three fouls on a novice are this one). Until then the rookie
   cut keeps `rules.penalties` true; the rule lands with A16's change. `puck.drag` stays 0.5.
4. **Three payoffs become scope** (lane A39): the one-timer (a shot within 10 frames of a received pass gets the full charge and a wider cone), the breakaway burst (+20 % speed for
   60 frames once past the last man, once per possession), the deke (a tap moves the puck 1 m to the blade side and commits the crease back for 15 frames). Each a calibration
   with a cartridge key, a test, and the playtester grid before and after.
5. The gate: `Jam.Playtest.NoviceScoresAtRookie` becomes the 20-seed form (`scored_2min` at least 10 of 20) and `HumanHasSomethingToDo` runs with `-player=novice` with a budget of
   at most 2 faceoffs without a touch per 20 matches; lane A3 wires both when the calibrations land.

## R-0030 (H11.8, 2026-11-12 19:50): the shipped figure is humanoid_b1; the toy's materials were lost in a commit; a camera lane
1. **Bake-off result** (lane A30, `docs/experiments/FIGURE-BAKEOFF.md`): B's rebuilt scripted figure `humanoid_b1` is the shipped default (absent key = `humanoid_b1`), `generated_b2`
   the alternative (it took four of six pairs on toy-ness, b1 both faceoff pairs and the hockey read with legible numbers), `toy_skeletal` and `toy` stay values. Cost decided
   nothing (all within noise). B's figures carry two faults for B: no far-reach side aim holds (in progress on B), the lift take goes vertical.
2. The toy's three materials (`M_ToonFigure`, `MI_SkaterFigure_Trim`, `MI_SkaterFigure_Stick`) were deleted by the pass-3 import commit (0882dab) and never restored, so the toy drew the
   default grid material in every build since H11; restored from a284b0a. The gate's new "missing usage flag" check would have caught it; it is on from 969fbc3.
3. **Camera**: every bake-off strip shows the game camera inside the boards along the glass (severity 4). Lane A40 implements the board camera clamp (FEEL proposal feel.07, approved
   R-0021) and the camera height at the end boards.

## R-0031 (H12.4, 2026-11-12 21:02): C-0001 v4 (B's tackling calls), bots check at the top levels, the figure recipe runs on B
1. **C-0001 v4** (B-to-A-0028): `IJamSkaterVisual` gains `PlayHitReaction(DirX, DirZ)`, `PlayBoardHit()`, `SetBoardPin(bOn)`, `PlayStickHit(bLifted)` as BlueprintNativeEvents with
   empty defaults, called from the pawn on A33's read-outs (a landed check that does not knock down or a collision over 3 m/s closing; the skater `board` event at 4 m/s; ten frames
   on the boards with an opponent within 1.2 m; a clean poke or a lift that takes the puck); `SetKnockedDown(true, DirX, DirZ, KnockdownFrames)` carries the down time so B's get-up
   ends when the sim releases the skater. Lane A45. `SetTripped` waits for a leg foul that does not exist.
2. **Bots check at pro and all_star** (ends R-0017 point 4's "no bot ever body-checks" for those levels): from the front or the side only, never from behind, only at closing speed
   above `check_speed_min` and only on the carrier; rookie bots never check. The curve is re-measured with the playtester after it lands (lane A3's next pass); the aim is that
   tackling reads in both directions, as the operator asked.
3. **The figure recipe** (A-to-B-0009) runs on B for a cel toy, the broadcast figure and a clay figure, each with an A/B sheet against the figure it would replace; the Director
   decides the swap per tier from the sheets with the eyes tool.
4. The glass hit (`HitWobble` on B's glass instances) is bound by A's effects binder for 12 frames on the nearest panel when B's lane lands it (A45 leaves the hook).

## R-0032 (H12.9, 2026-11-12 22:50): the operator's H18 playtest: unplayable; controls first
The operator: "Camera does not stay where player moves, mouse is not binded to work with the stick, the stick blinks back and forth. Player should have option with mouse to tackle or
cross-check instead of the shift. Using shift rather for sprint. No player reaction on tackle making impossible to take puck from opponent. Difficulty does not matter if the game is
trash." Rulings: (1) lane A49 owns input, the control path, the follow camera and the stick aim until it reports; no other lane edits them; (2) the bindings are the operator's:
mouse aims the stick, left button shoots (hold to charge), right button tackles, middle or mouse 4 cross-checks (lift), Shift sprints (new mechanic with a stamina bar, a cartridge
key), Space pokes, E passes, Q dekes; gamepad mirrors it; (3) every control proof comes from a packaged build through the real input path, not from PIE; (4) no calibration
verdicts are asked of the operator until a person can play; the playtester keeps measuring in the background.

## R-0033 (H13.2, 2026-11-13 00:10): the operator's decisions relayed by B (22:50, 23:55) and the controls fix
1. **Tiers**: `p3_clay` is the tier B pushes next, against the P3 concept; `p4_broadcast` stays a lab tier (never in a shipped cut) narrowed overnight to the skater and the ice;
   the operator decides in the morning between P4 and a full P3 focus. A28/A29 feed the lab only.
2. **Goalies**: no dedicated goalie; any skater near its own crease takes the stance (the human included); the goal opening is halved in height by a cartridge key
   (`game.rink.goal_height_m`, default 0.61, width unchanged) read by the net mesh, the frame, the sim's goal test and the bots' aim (lane A50).
3. **Commentary accepted as generated** (voice C1, all 340 lines); the player picks at random, never the last line of a situation, preferring lines unheard this match (A50).
4. **Crowd instead of horns**: B replaces the horns with crowd reactions; `MS_goal` Variant 0 when the human's team scores, 1 when it concedes; a crowd bed starts with the music (A50).
5. **Controls (lane A49, R-0032)**: five root causes found and fixed in a packaged build through the real input path: the menu's input mode left the viewport without mouse capture
   (the stick only moved with a button held), the camera's lead and lift turned away from the player, the raw blade angle flipped at the clamp behind a turning skater, Shift
   was the check, and the check needed a bot's speed and a 6-frame press. Bindings as the operator asked; a sprint on Shift with a stamina bar; the human's check lands from
   2 m/s with a 12-frame window. H19 is the first package with it. `FJamInput::bSprint` needs a lockstep flag bit (A50).

## R-0034 (H13.2, 2026-11-13 00:40): stick handling with the mouse, the spin on E, the pass on F
The operator asks for advanced stick and puck control with the mouse so a player beats a defender by movement and the blade, and a special move on a key such as E. Director's
decisions (lane A51): (1) the carried puck lives where the blade is: its offset follows the mouse aim every frame within `game.stick.reach_m`; a fast sweep shields the puck
from a poke on the side it left for `game.stick.sweep.shield_frames`, and a defending bot pokes at where the puck was `bot_react_frames` ago by difficulty, so the mechanic
is real in the sim and not an animation; (2) a spin move (`game.skater.spin`: 24 frames, cooldown 90, speed 0.7, a quarter of the stamina bar) on **E** and gamepad B; the puck
rides the far side of the body; a check landing mid-spin knocks the carrier down longer; (3) the **pass moves to F** (gamepad X unchanged); A39's tap deke stays on Q and the
wheel; (4) the spin takes lockstep flag bit 512 (protocol 5); (5) every step is proven in a packaged Development build through SendInput, as R-0032 requires for all controls work.

## R-0035 (H13.3, 2026-11-13 00:45): the puck gets a height
A50 showed the half-height goal (R-0033 point 2) bites only visually: the sim's puck is two-dimensional. Director's decision (lane A52): the sim's puck gains a height and a
vertical speed, hashed; a shot's release height follows the charge (`game.skater.shot.lift`: tap on the ice, half charge 0.45 m, full 0.95 m), so a full charge sails over the
0.61 bar from distance by design (amended 01:40: the default full charge arrives at 0.6, just under the bar, after the novice gate fell to 8 of 20 with 0.95; a cartridge may set full_m 0.95 for the sailing shot); the goal test uses the height; posts and the bar ring and reflect; a skater in the crease stance saves a puck under its block height within
its reach (`game.skater.goalie`), the human included; bots charge by the net's height. The carried puck stays on the ice. The determinism baseline is re-recorded by the lane
and once more by the Director after A51 and A52 merge.

## R-0036 (H14, 2026-11-13 09:56): P4 descoped; P3 is the tier to master
The operator: "P4 descoped as it seems the realistic output is not worth the chase. P3 remains target to master - physics and animations especially around collisions,
gameplay mechanics, director's choices -> ways/modes to change character of the game for creative variations of possibilities." Rulings: (1) `p4_broadcast` leaves the
plan: no lane on A or B spends time on it; the tier stays in the code as a lab row until the Director removes it, never in a cut; (2) `p3_clay` is the shipped look of the
jam entry: B perfects it against the concept, A's remaining ask for it is the ring meshes for zones (B-to-A-0035); (3) the work of the last two days, in this order of
value: collisions (the body check, the board hit, the knockdown and the get-up, the stick lift, the poke: sim rule and figure reaction together, C-0001 v4 landed from
`dev-b-a45`), gameplay mechanics (the sweep, the spin, the lifted shot, passes, one-timers, dekes: each proven in a package through the real input path), Director's
choices: modes that change the character of a match (cartridge-defined, chosen from the menu as a Director's cut: rules, rink, team size, physics, bots, look) so a
player makes creative variations without editing JSON; (4) P2 stays what it is: the fallback tier, no further look work.

## R-0037 (H15, 2026-11-13 10:48): every match ends; the lockstep soak's findings
1. A `first_to` match has a horn at 30,000 frames of live play (8:20): the leader wins, a tie goes to sudden death; a hard stop at 33,000 ends any match still running,
   the leader wins, a tie goes to the team that last touched the puck (the sim has no draw state; a draw would be new sim and UI, not worth it two days out). Amended 11:30: a timer match's hard stop is its clock plus 6,000 frames (a 10:00 Marathon clock is 36,000 frames; A72 caught that the 33,000 stop would have cut it). Lane A71
   measured 53 of 11,400 soak matches past the old 10-minute cap, none of them stuck: goals were simply rare on the big boost rink. The hash moves only for matches
   past 30,000 frames.
2. Lane A71's idle measurement: the longest no-touch stretch per match is p50 16 s, p90 32 s, 12 % of matches over 30 s, almost all a loose puck the novice does not
   reach, never teammates keeping it. The lever is a teammate deflection to the human after 10 s without a touch (A70 costs it with the curve; not before the curve lands).
3. Lockstep (lane A56): 7,260 joined matches with loss, delay, duplicates, reorder, a 30 Hz guest and real UDP: zero desyncs, zero lost inputs after one fix (the driver's
   pending-flags word was a byte and cut the spin's bit 512 off during a stall). Protocol 5 has no refusal message: a protocol-4 guest is ignored and told the wrong reason
   after 60 s; a REFUSE message (about 20 lines) is accepted for the next JamNet lane. The sprint and the spin were dead in a joined match (`ReadLocalInput` never read
   them): fixed by the Director with the landing.

## R-0038 (H15, 2026-11-13 11:19): the camera stays bound to the human; the HUD points at what is off screen
Lane A64's camera lead toward the puck fought the mouse aim (the cursor aims the stick in rink space; a camera turn under the cursor changes what the player is
aiming at), so it ships off (`jam.camera.KeepYaw` 0). Ruling: no camera lead, lift or turn is added that moves under the cursor while the player aims; what is off
screen (the puck, its carrier, the far net) is pointed at by the HUD (lane A74: an edge arrow with the distance, the carrier's team colour), and the camera's bounds
stay A49's. A camera that leaves the human (a replay, an end-zone shot) is a separate camera mode, never the play camera.

## R-0039 (H16, 2026-11-13 12:25): the curve re-tuned on the soak (R-0029 amended)
Lane A70, 2,000 matches per cell on A53's seeds (`docs/experiments/CURVE-2.md`): novice at rookie 85.8, regular at pro 51.0, good at all_star 26.8 on hockey.regular;
84.7 / 51.1 / 27.2 on the large rink; every cell inside its interval of the R-0029 targets (85 / 50 / 25). Levers: rookie red skill 0.75 -> 0.8 (large 0.97); pro red
skill 0.94 -> 0.95 (large 0.97); all_star speed 1.15 -> 1.3; bots check from 4 m/s at pro and 3 at all_star (A73's finding; +10-13 % knockdowns of the human, the win
share barely moves). `HumanHasSomethingToDo`'s budget is the soak's measured share plus 2 points (10.4 %), as R-0037 point 2 intended. Flat and not adopted:
`bot_react_frames`, rookie speed. Open: all_star is a steep step (a regular player wins 13 %, a novice 3 %); R-0029 names no target there; a skill lever that only the
good player beats is the next curve ask if the operator wants all_star reachable for a regular player. The gate's 20-seed tests keep their bars (novice 14 of 20).

## R-0040 (H18, 2026-11-13 12:57): the lift is shielded like the poke; the saucer pass; protocol 6
Lane A77 (`docs/experiments/MECHANICS.md`) proved the input path and the read-outs of the sweep, the spin and the saucer in a package, and found that the stick lift at contact
(no shield, 1.7 m body to body) took the puck inside every shield window the sweep and the spin give against the poke. Rulings: (1) the spin shields the lift fully (the body
is between), and the sweep shields the lift on the side the puck left for `game.stick.sweep.shield_frames`, the same rule as the poke (lane A82); (2) the saucer pass stays
(`game.pass.saucer`: a held F lifts the pass over a blade, a puck over 0.2 m passes every blade), with its lane margin at 0.9 m and the airborne window from 1.5 m along
(A77's measured calibration); (3) `PassHeld` is lockstep flag bit 1024 and the protocol is 6: device B merges main before any joined test; (4) the mechanics' bars (R-0034,
A51's brief) stand: ten of ten in the package, reported with the count and the cause when not met, never tuned past this ruling.

## R-0041 (H18, 2026-11-13 13:00): difficulty is aggression; 5 v 5; abilities on F; a raw and brutal brand
The operator: "I would reframe also the difficulty direction to focus more on aggressivity. I feel like the arena can be built for 5v5 game, tackles and cross-checks can
be vital for fun gameplay which will become on harder difficulty a beating massacre. Director's mode then can produce via boosts fun aspects like one-shot shotgun,
binding special abilities to 'F' key. This can make whole concept of the game more raw and brutal promoting into its brand (logo, banner, tone of the game)."
Rulings: (1) **the difficulty ladder is an aggression ladder**: rookie bots rarely check and never cross-check; pro bots check and lift as a matter of course; all_star bots
hunt the carrier: body checks from low closing speeds, cross-checks at contact, knockdowns every shift ("a beating massacre"); speed comes back toward 1.2 as the aggression
levers take the win rate (lanes A79, A83); the R-0029 targets stand as the measure. (2) **5 v 5 is the regular game**: `hockey.regular` and the ladder's takes go to five
skaters a side (the schema allows 2..5, the brain 10 skaters, A58 measured no hitch at 3 v 3: A83 measures 5 v 5 on the 4090 and at the minimum spec before the switch);
the rink stays 90 x 45 unless 5 v 5 crowds it (then 100 x 50 by measurement). (3) **Abilities on F** (lane A84): a Director's mode or a boost pickup grants the human one
special ability, used with F: the one-shot shotgun (a shot that knocks down every skater it passes within a metre and goes in through a crowd), the hip-check burst, the
shield, the slow puck; the **pass moves to Q** (the deke keeps the mouse wheel and gamepad Y; Q was the deke's second key). Each ability is a cartridge-defined key under
`game.abilities[]`, unknown keys errors, with a bark and a HUD slot. (4) **The brand goes raw and brutal within the jam's teen rating** (R-0021: no gore, clay figures, mild
words at most one in ten): the title, the logo, the banner, the menu copy, the commentator's tone and the crowd sell a rough game, not a cruel one; device B owns the logo and
the banner (A-to-B-0019), A owns the copy and the commentary tone. (5) P3 stays the shipped look; nothing here reopens P4. (6) **The carrier is slower and cannot sprint** (the operator, 13:15: "skater with puck will need slight speed reduction and inability to sprint, to incentivize passing with other players and ability to defend"): `game.skater.carry_speed_mult` (default 0.92, range 0.6..1) scales the carrier's top speed and acceleration; the sprint is refused while carrying (the bar does not drain); bots and the human alike; a calibration with the hash re-recorded (lane A82), the curve re-measured after (A79).

## R-0042 (H19, 2026-11-13 13:47): the carrier's reach; the tap stays on the ice
Lane A82's release read-out found why the human's shots, passes and saucers never fire with the blade extended: since A51 the carried puck lives at the blade (0.4 +
`stick.reach_m` 1.2 = 1.6 m) and every carrier test uses `shot.reach_m` 1.5, so nothing charges or fires while the player aims; the human also keeps losing a blade-length
puck (A66 capped the bots' own blade to work around the same thing). Rulings (lane A85, top priority): (1) one carrier reach for shooting, passing, charging and carrying:
`max(shot.reach_m, body radius + stick.reach_m) + 0.05`, used by every carrier test in the sim and by the brain; A66's cap goes if bots shoot without it; (2) a tap stays on the
ice: `shot.lift.tap_m` 0 and a charge-0 shot flies flat, so a rookie bot's tap can be deflected by a stick again (A82's SeasonTakes coin lands the same way every time);
`half_m` 0.45 and `full_m` 0.6 stand; (3) H24 ships with the fault; H25 packages the fix. The bars of R-0034 stand.

## R-0043 (H20, 2026-11-13 14:14): the plan reconciled
The operator after the plan and model switch: "please reconcile the plan as previous model may not lead into best possible directions we can achieve". The Director's read
and the order of work are in `docs/PLAN-RECONCILED.md` (the live plan from now). Rulings: (1) **no H package from H25 on without the acceptance run green**: a script that
plays the first five minutes in a package through the cursor and the buttons, pass or fail per verb (lane A87); H19 to H24 shipped unable to shoot or pass while aiming
because no gate drives the real input; (2) **one curve pass on the real player** (A86): the playtester classes hold the stick the way the mouse does first; no other lane
edits a difficulty number; (3) **no new mechanic after A84's abilities**; a mechanic whose bar still fails after A85 is cut to what passes; (4) **the submission has a lane
from now** (A88: video, page text, dev log, ledger, clean-profile test); (5) **freezes**: the sim Saturday 12:00, content Saturday 22:00, Sunday is submission only; submit
Sunday 22:00; (6) one lane per file family at a time (the sim, the profiles, the input mapping, the HUD).

## R-0044 (H21, 2026-11-13 15:09): the reach fix landed; the last sim calibrations; the mechanics' bars restated
Lane A85 landed R-0042 (one carrier reach, the flat tap; in the package 30 shots fired at three charges where A82 had none) and lane A87's acceptance run failed the old build
on shooting, passing and the one-timer exactly as it should, and turned 17 of 18 verbs green on a stand-in for the fix. Rulings: (1) `shot.lift.full_m` 0.5 (the full charge
at 0.6 rings the 0.61 bar from 10 m); (2) a hold of 4 frames or fewer is a tap, flat (`game.shot.tap_frames` 4): a human's click is never zero frames long; (3) the bars of
R-0034 restated to what each mechanic is for (R-0043 point 3): the sweep and the spin are proven by their shield (every poke and lift inside the window refused; A85: 9 lift
misses in the spin run), not by the puck kept against a body check, which is the risk the ruling gave them; the saucer by its flight over a blade when one is on the lane;
(4) A66's bot blade cap stays: without it about thirty brain decisions lose the carrier (A85's measurement); it is the curve lane's to revisit, not before the submission;
(5) the bots' round-the-net rule stays; (6) lane A91 makes these calibrations and takes the acceptance run to green: that run is H25's gate. Amended 16:35 on A91's measurement: `full_m` is 0.47 (at 0.5 the full charge still rang the bar from 15 to 24 m; 0.47 scores from every metre of 3..30 m, 728 shots asserted); the brain's long wind-up for a real net applies only to nets of 1 m or more; the sweep's shield covers both sides for its window (a cursor crossing read as two sweeps flipped a one-sided shield): lane A93 makes that last change.

## R-0045 (H22, 2026-11-13 15:46): the vision: inertia, hockey principles, speed change, the stick and the sprint
The operator gave four topics "so they become part of the vision of chasing"; they are written as laws with numbers in `docs/VISION.md`, which wins over older rulings where
they differ. (1) **Physics**: a contact's strength is the closing speed along the contact line times mass (same direction and speed: almost nothing; head-on at full speed:
the biggest hit), against the victim's stability (a standing braced skater resists a side hit; pace, the sprint, a spin lower it; a cross-check from the front beats the
standing brace); three outcomes: bump, stumble, down; the attacker pays; **the carrier cannot body-check or cross-check** (lane A92). (2) **AI**: the bots read open ice and
decide to carry or pass by it, with an attack shape, a defence shape and transitions; charging a set defence is the wrong decision (lane A94). (3) **Gameplay**: braking by
reversing the input (W to S), a first-step burst; the carrier wins by changing speed and direction (lane A93). (4) **The stick and the sprint**: the puck rides closer when not
sprinting; **the carrier may sprint** (replacing R-0041 point 6), with the puck farther out and easier to take, and a defender's sprint is always the faster; pace costs
stability (A93 with A92). Process: each rule has a neutral value that gives back today's game and its lane proves it; the acceptance run and the gate stay the bar; the levels
are tuned once more after the three lands (A86 stops after its job 0 and baseline); the sim freeze stays Saturday 12:00 and what is not green by Saturday 10:00 goes back to
its neutral value. R-0043 point 6 is narrowed: one lane per function family in the sim, named in each brief.

## R-0046 (H24, 2026-11-13 18:39): the vision is on main; the ladder set once on the real player
Lanes A92 (contact by inertia), A93 (braking by reversing, the first-step burst, the carrier's sprint, the puck's distance by pace, the two-sided sweep shield) and A94 (the
hockey brain) landed together through lane A95, which made the merged tree green (the full `Jam` filter 156 of 156; the acceptance run 19 of 19 through the cursor and the
buttons), built device B's lift cone and directed stick events (B-to-A-0037), taught the playtester and the bots the new verbs, and set the ladder on 2,000 matches per cell
(`docs/experiments/CURVE-5.md`): novice at rookie 85.3 / 86.0 on the large rink, regular at pro 48.3 / 48.4, good at all_star 26.3 / 24.9: all six inside their intervals
of R-0029's targets (85 / 50 / 25). Rulings: (1) the profiles stand as measured: rookie knows the principles late and loosely, rarely hits (checks only from 6 m/s), sprints
little; pro shoots from 30 m, hunts the carrier inside 8 m, checks from 2 m/s; all_star hunts inside 20 m, always sprints, checks from 2 m/s, carries through pressure, at
bot speed 1.4 (1.22 on the large rink); (2) pro's shot range (13 to 30 m) is accepted as a lever: the hockey brain made the bots pass instead of shoot and cost them about
40 points; shooting from range is hockey; (3) **all_star's speed of 1.4 is flagged to the operator**: aggression alone left the good player at about 37 %; the 25 % target
was reached with speed; the operator may prefer 1.2 with the good player near 37 %; (4) the budgets follow the soak: the human-touch share plus 2 points (9.3 %), the novice's
goal inside two minutes at 7 of 20 (the measured share is 44.8 %); (5) nothing went back to its neutral value; the neutral proofs hold on this tree; (6) the sim freeze of
Saturday 12:00 stands: from here only fixes that the acceptance run or the gate demand, and the operator's read of H26.

## R-0047 (H25, 2026-11-13 18:46): the difficulty accepted; evolution tracks until the freeze
The operator: "difficulty setup fine for me" (R-0046 stands, all_star's speed included) and "We need to iterate on top of existing concepts and make evolution after
evolution in tracks where possible. Next 24h will be key ... RAM 60-80% used, Fable agents thinking how to improve game mechanics and design, executing in blender. Opus
subagents to improve the codebase and quality of implementation, Sonnet subagents to run tests and harden the game." Rulings: (1) ten standing tracks (`docs/EVOLUTION.md`):
Fable F0 design eye, F1 contact feel, F2 carrier skill, F3 the arena in Blender; Opus Q1 determinism and lockstep, Q2 review and repair, Q3 performance at five a side;
Sonnet S1 soak, S2 the acceptance matrix (the cursor's owner), S3 fuzz and the loader; each lane does one evolution, lands through the gate (and the acceptance run when a
player's verb changes), and proposes the next three; the Director keeps every track busy; rules in `briefs/EVOLUTION-COMMON.md`; (2) the machine's band is 38 to 50 GB in
use; soaks scale their workers to fill it; (3) **the freezes move**: rule changes in the sim land until Saturday 19:00, each with a neutral value and a green gate; content
freezes Saturday 22:00; Sunday is the submission; submit Sunday 22:00; (4) the ladder of R-0046 is set: an evolution that moves a target cell by more than 4 points ships
with its neutral value until the curve is re-set; (5) the release chain compiles the Shipping target before the acceptance run (H26's first try played green and then failed
to compile for Shipping: a test outside its guard).

## R-0048 (H26, 2026-11-13 19:34): what the design eye measured: the carried puck, the whistle, the first ten minutes
Track F0's first review (`docs/design/REVIEW-1.md`, 924 headless matches on the vision's build) found the game's largest gap against `docs/VISION.md` law 3: **nobody keeps
the puck**: the human's carry lasts a median 0.87 / 0.75 / 0.67 s at rookie / pro / all_star, because a carried puck is taken by any aimed blade or body that touches it,
with no press, even by a teammate. Rulings: (1) **a carried puck leaves its carrier only by an action**: a shot, a pass, a poke, a lift, a check or stumble that loosens it,
or the carrier's own release; never by an unpressed blade or a body's touch (`game.stick.passive_take`, default false, neutral true), never to a teammate by touch under
either value (track F2, in its current evolution); the ladder is re-set right after on the track's report (a curve lane), since the human keeping the puck moves every cell;
(2) **the whistle is taken out of the regular game**: `hockey.regular` and the season's first takes play with `rules.penalties` false (the newcomer's first hit was a foul
in 37 of 60 first matches, under a banner that promises no whistle); modes that are about the whistle keep it (Glass House); (3) **the first ten minutes**: take-3 is no wall
(it is pro today: the novice wins 22 % and has no shot in a third of them): the season's first three takes are rookie, measured (a content lane with the curve lane);
(4) at all_star the speed scale is the opposing side's only: the human's own mates do not outskate him (15.5 against 11.3 m/s today); set with the curve; (5) the brain
brakes by reversing 670 to 2,330 times a match and never sprints with the puck: track Q2's next two evolutions; (6) the screen says what the sim decides (track F1, in
progress); (7) process: every editor or commandlet start takes a turn at the one build tool lock, so a measurement uses few processes with many seeds each.

## R-0049 (H26, 2026-11-13 19:38): device B's play-through (B-to-A-0038): a tackle is an event, not a rhythm
Device B played the build before the vision for 267 s and found the human knocked down every 7 seconds (7 of 15 within a second of his release) and the carrier losing the
puck 1.4 times a second. The vision's contact law and R-0048 answer part of it; the rest is ruled here: (1) **release grace**: a skater released from a knockdown cannot be
stumbled or knocked down for `game.contact.release_grace_frames` (default 90, neutral 0): a hit on it is a bump at most, and the brain does not start a check on it
(track F1's next evolution; proof: in 100 s at pro no figure is down again within 2 s of its release, the human down fewer than five times); (2) **take grace**: a skater
that has just won the puck cannot be lifted or poked for `game.stick.take_grace_frames` (default 30, neutral 0); a body check still loosens it (track F2's next, with
R-0048's rule: together they are "the puck is kept"); (3) the fade-in waits for the light rig, the human's marker at five a side, the hint bar, the commentary box, the
distance labels and the camera in the corners: a new track **U1 the screen** (Fable, lane A111); (4) the whistle's early release of a knocked-down figure stays where a
mode keeps the whistle; the regular game loses the whistle (R-0048), and B's quick get-up covers the rest; (5) `NS_deke` / `MS_deke`: B builds them small (a flick of
shavings at the blade, a short scrape): asked in the answer.

## R-0050 (H26, 2026-11-13 20:14): the puck is kept, the ladder is off until the curve is re-set; process faults of the day
Lane A102 built R-0048's rule (`game.stick.passive_take` false, plus `game.stick.body_shield` true: the carrier's body shields the puck from a stick behind it). On
600 matches a cell it moves the ladder to 99.8 / 72.0 / 39.8 (targets 85 / 48 / 26): the human finally keeps the puck, and the bots have not learned to take it.
Rulings: (1) the rule lands with its default as R-0048 set it (false), not at its neutral value: the game with it is the game the vision describes, and the curve
track (C0, lane A112) starts on its landing; **no package is cut from main until the curve lands** (H26 stays the newest package); (2) **the body shield refuses
the stick, never the shove**: a lift or a poke through the carrier's body is refused; a cross-check's shove from any side inside its cone is contact and obeys the
contact law (track F1's next evolution; today `TryLift` refuses both); (3) the brain reads the shield and closes under control instead of charging head-on at
11 m/s (track Q2, lane A115); (4) rookie cannot take a carried puck except by a hit (a straight charge at a rookie passes 53.5 %): the curve track decides what
rookie may do with its stick; (5) track F2's next: the take grace of R-0049, the stop's grip (`reverse_brake_decel` 26), the first step that belongs to the
carrier (lane A121); (6) lane A107's soak: three modes no longer differ from the regular game by a third (cage, outnumbered, shotgun), the marathon plays past its
own horn, take-3 and take-6 play identically: a modes lane (M1) starts with the curve lane, on the same build; (7) lane A109's fuzz: seven fault classes go to
lane A118 with the found cartridges as regression cases; a hook's `set_param` value lies inside the named parameter's range.
**Process faults found today, and their guards:** (a) the main checkout sat on a detached HEAD from 13:39 to 20:03, so every push sent a stale branch and device B
received nothing for six and a half hours: the landing script now refuses to run off the branch and reads the remote back after each push; (b) a running landing
script was edited in place and derailed after its commit: scripts a chain is executing are never edited; (c) every headless start queued on the build tool's mutex
for minutes: `-Multiprocess` on every headless start (lane A107: 5 m 17 s to 9.5 s; the gate's test step 398 s to 89 s); (d) landings go in batches through one
gate (six evolutions in four minutes at 20:12), each lane having run the full filter on its own tree.

## R-0051 (H26, 2026-11-13 20:39): the release grace is 120 frames; a line with no recording keeps its caption
(1) Lane A113 measured R-0049's release grace: at 90 frames one figure still went down again 111 frames after its release, at 120 none in 1,000 s, and the three
cells stay within 1 point of their targets either way: **`game.contact.release_grace_frames` defaults to 120** (neutral 0). A full-speed charge into a skater
inside its grace costs the attacker its own stumble, as any bump does, and the HUD says JUST GOT UP; the shotgun's knockdown respects the grace as it respects
the shield. A hit on a skater inside its grace emits no `check` event: it is a bump. (2) Lane A114 found 72 of 412 commentary lines without a recording, most of
them the contact lines, and made the game skip such a line: **the default stays "caption without sound"** (`jam.vo.SkipAbsent` 0): the caption at a hit is the
brand's voice even when silent, and the lookup that hitched is gone either way (an absent wave is asked for once per process). The 72 recordings go to device B.
(3) The match-start frame is under 100 ms in 3 of 6 runs with the map preloaded behind the menu (0 of 6 before): the track continues on the audio flush and the
figures' spawn.
