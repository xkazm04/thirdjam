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
