# Lane A12: AI voice side track: situational barks, shouted signals, NVIDIA ACE (device A, from H5.4)

You are lane A12 of the Third Jam (team Hokej). Repo `C:/Users/kazda/kiro/thirdjam`. Read first: `contracts/RULINGS.md` R-0007 (your ruling: content law, three phases, ownership),
R-0004 (the CLI transport), R-0006 (NVIDIA track), `GAME.md`, `docs/THEME.md`, `docs/HOCKEY.md` (2 AI row, 4, 7), `contracts/events.md`, `Game/Source/JamCore/Public/JamEventBus.h`,
`Game/Source/JamCore/Public/JamSim.h`, `Game/Source/JamCore/AI/JamBrain.h` (lane A3 is editing the brain for difficulty right now: do not edit it; you give A3 one header to consume),
`Game/Source/JamGame/Public/HUDWidget.h`, `HUDFeed.h`, `InputMapping.h` (A5, finished: C++-drawn UMG, Enhanced Input), `Game/Source/JamDesigner/Public/*` (A7: how it spawns `claude -p`),
`tools/mcp/README.md`, `tools/run/*.ps1`.

You own: `Game/Source/JamGame/Bark*`, `Game/Source/JamGame/Signal*`, `Game/Content/Barks/**`, `docs/experiments/ACE.md`, `tools/barks/**`. Run no git command; editor on MCP port **8012**;
never kill a process you did not start; one heavy job at a time; keep the shared Source tree compiling (R-0003 point 5: build before every pause and report). Download nothing in this run.

## Phase 0: ACE research (30 minutes, then continue with phase 1 while the Director asks the operator)
Find NVIDIA ACE for Games / ACE Unreal plugin status for UE 5.9 on 2026-11-12: what the plugin bundles (on-device LLM NIM such as Nemotron-Mini, Riva ASR, TTS, Audio2Face-3D), what it
downloads at install and at runtime (model sizes), licence terms for shipping in a jam build, GPU and driver requirements, whether it degrades gracefully on non-NVIDIA hardware, and the
real integration cost for (a) TTS of a text bark and (b) ASR of a two-second shout. Write `docs/experiments/ACE.md` section 1 with sources and dates; state the download it would need
(URL, size) and stop that thread; the Director brings it to the operator. Also note Windows' built-in speech recognition as the no-download ASR path (which API a UE 5.9 game can reach on
Windows 11; cite).

## Phase 1: barks (build it)
1. **Line bank** `Game/Content/Barks/barks.json`: generated with `tools/barks/generate.mjs` through the CLI transport pattern (`claude -p`, haiku first; sonnet for quality if needed), 12-20
   lines per situation x speaker x tone, situations: `goal_for`, `goal_against`, `check_landed`, `check_received`, `steal`, `stall`, `near_miss`, `faceoff`, `win`, `lose`, `directors_call`,
   `signal_ack_defend`, `signal_ack_space`, `signal_ack_pass`, `praise_reply`; speakers: `opponent`, `teammate`; tones: `mock`, `complain`, `bravado`, `encourage`, `praise`. Every line passes
   `tools/barks/filter.mjs` (a word list for profanity, slurs, sexual terms, threats; a regex ban on proper nouns that look like names, brands or teams; max 60 characters), and the generator
   prints a review file the Director skims. English only. Hockey flavour welcome ("that's a wrap, rookie"; "nice stick, shame about the aim").
2. **`UJamBarkSystem`** (GameInstance or game-mode subsystem in JamGame): subscribes to the event bus and reads sim state for the situation; picks the speaker (nearest opponent to the
   action for mocks; a teammate for complaints at the human and for encouragement), the tone by `game.barks.intensity`, a line without repeats within 10 picks; global cooldown 4 s, per-speaker
   8 s; broadcasts `bark` on the bus (`Who` = speaker index, `Kind` = situation, text carried in a side channel) and shows it: a speech bubble widget above the figure (world-to-screen, fades in
   1.5 s) and a one-line HUD strip. Cartridge: add `game.barks { enabled, intensity }` to `schema/game.schema.json` (lane A4 is finished; you may edit schema and presets, keep `node --test` green,
   re-run `Game/Source/JamCore/Tools/gen-schema-data.mjs` and A7's `gen-designer-data.mjs` if they exist, and add the Designer rule: it may set barks).
3. **Voice (optional in phase 1):** if UE 5.9 or Windows offers text-to-speech without a download (check the engine's `TextToSpeech` plugin), voice the bark through it with a flag; else text only.
4. **Tests:** `BarkBankClean` (every line passes the filter; no duplicates), `BarkPicksBySituation` (events produce the expected situation and speaker), `BarkCooldown`.

## Phase 2: signals (if time remains in the box)
`FJamTeamIntent { Intent: defend | space | pass | praise; Frames; IssuedFrame }` in `Game/Source/JamGame/Public/SignalIntent.h` (A3 consumes it in the brain: teammates fall back / spread /
carrier seeks the human / praised teammate +10% speed for 300 frames; write the exact consumption rules in `docs/experiments/ACE.md` section 2 for A3), input actions 1-4 and d-pad,
a `signal` event on the bus, the teammates' acknowledgement bark, and `SignalsRouteToIntent` test. Windows speech recognition is phase 2b, only after the keyboard path works.

Report: Phase 0 findings first (and the download to approve), then what shipped in phase 1 (screenshots of bubbles in PIE), the filter's statistics (generated / rejected lines), test results,
every error, the A3 seam. Time box 3 hours; report at 60 minutes with the state either way.
