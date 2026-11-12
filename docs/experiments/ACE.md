# AI voice side track: NVIDIA ACE, barks and shouted signals (lane A12, ruling R-0007)

Owner: lane A12 (device A). Section 1 is Phase 0: research only, **nothing downloaded** (sizes below come from HTTP `HEAD` requests, headers only).
Section 2 is the brain seam for lane A3 (`FJamTeamIntent`). Section 3 is what shipped in Phase 1 (barks) and how to run it.

## 1. NVIDIA ACE for Unreal on 2026-11-12: what exists, what it costs, the download to approve

### 1.1 Headline

There is **no ACE plugin for UE 5.9**. On 2026-11-12 NVIDIA's ACE for Games page lists the plugins per engine version and the newest build of every
ACE plugin is for **UE 5.7** (speech, LLM, TTS) or **UE 5.6** (Audio2Face-3D reference plugin). Our engine is UE 5.9.0. The same situation as DLSS
(`docs/experiments/DLSS.md`): a port of the 5.7 source against 5.9 is the only path, with unknown compile risk, and the models are multi-gigabyte downloads.

ACE is no longer one plugin. Since Unreal Fest (2026-06-16, NVIDIA blog "Build on-device AI companions with the NVIDIA ACE Game Agent SDK and Unreal Engine 5
plugins") it is a **suite of separate UE plugins on top of the NVIDIA In-Game Inferencing SDK (NVIGI)**, each bundled with one ready-to-use model:

| Plugin (file on developer.nvidia.com/ace-for-games) | UE versions | Bundled model | Zip size (HEAD, 2026-11-12) | Last-Modified |
|---|---|---|---|---|
| Speech recognition `ace_asr_v5_7_1.0.4.zip` (also 5.6, 5.5) | 5.5, 5.6, 5.7 | `nemo-conformer-ctc-120m` English; 7 more languages as separate downloads | **731,867,583 B (698 MiB)** | 2026-06-12 |
| Text-to-speech `ace_tts_v5_7_1.0.4.zip` (also 5.6, 5.5) | 5.5, 5.6, 5.7 | Chatterbox Turbo 350M, example voices | **1,963,104,106 B (1.83 GiB)** | 2026-06-12 |
| Character intelligence (LLM) `ACE_LLM_5.7_v0.5.zip` (also 5.6, 5.5) | 5.5, 5.6, 5.7 | Qwen 3.5 4B (GGUF, local text generation + function calling) | **4,049,945,595 B (3.77 GiB)** | 2026-07-18 |
| Facial animation (Audio2Face-3D) `nv_ace_reference-ue5.6-v2.5.0rc3.zip` | 5.4, 5.5, 5.6 | none in the zip; A2F-3D regression 2.3 / diffusion 3.0 models are separate zips (`ace_2.5_v2.3_a2f_models.zip`, `ace_3.0_a2f_models.zip`) | not measured (no use for us: no faces) | |
| Optional model packs | | Nemotron ASR (`nemotron-asr-igi-v1.zip`) 2,386,006,732 B; Qwen3-4B q4_k_m GGUF 2,429,369,116 B; Chatterbox multilingual 1,197,580,620 B; Nemotron 3.5 Bolt E4B "4gb" | | 2026-06 to 2026-08 |
| ACE Game Agent SDK `game-agent-sdk-v0.5.0-source.zip` | custom engines, C/C++ (beta) | none | not needed | |

Sources: NVIDIA ACE for Games page (`https://developer.nvidia.com/ace-for-games`, read 2026-11-12; its `.md` mirror lists the zips above);
NVIDIA Developer Blog 2026-06-16 (Phillip Singh), "Build On-Device AI Companions with the NVIDIA ACE Game Agent SDK and Unreal Engine 5 Plugins";
TweakTown and meatechwatch coverage of Unreal Fest 2026-06-16..18 (ASR + SLM + TTS plugins, DLSS 4.5 plugin, "runs on RTX GPUs");
ACE Unreal plugin 2.5 docs (`archive.docs.nvidia.com/ace/ace-unreal-plugin/2.5`, UE 5.5 + 5.6, Audio2Face only, RemoteA2F provider; dated April 2025).

Nemotron-Mini: the 2024-era "Nemotron-Mini 4B Instruct" on-device model is gone from the current page; the LLM plugin's bundled model is Qwen 3.5 4B, with
Nemotron 3 Nano (9B, 4B) and Nemotron 3.5 Bolt E4B as optional downloads. Riva ASR/TTS as a cloud microservice is still documented separately for the
"Kairos" digital-human workflow; the game plugins use NVIGI local inference (Nemo conformer CTC, Chatterbox), not Riva.

### 1.2 What it downloads, at install and at runtime

- **Install:** the plugin zips themselves (0.7 to 4 GiB each; ASR + TTS alone is 2.5 GiB) contain the model, the NVIGI runtime and the TensorRT/CUDA
  inference backends. The page says "In order to leverage the AI models and plugins below, please download NVIDIA In Game Inference SDK"
  (`developer.nvidia.com/rtx/in-game-inferencing`), another download whose size was not measured; whether the plugin zips already embed what they need
  for the bundled model is unverified until one is unpacked.
- **Runtime:** nothing is fetched at runtime; the models are local files staged with the game. That is also the problem: the **shipped build would
  carry the models** (hundreds of MB for ASR alone, the 350M TTS voice plus the inference DLLs), against our 1 GB zip budget (322 MB used at H6).
  A size breakdown inside the zips cannot be known without the download.

### 1.3 GPU, driver and graceful degradation

- NVIDIA states the plugins are "local, RTX-optimized workflows bundled with ready-to-use models". The blog and the page name GeForce RTX hardware
  and give **no minimum RTX generation, no VRAM figure and no driver version** on the public page (these sit in the SDK README after download).
- The NVIGI LLM path has a CPU backend ("run on NVIDIA GPUs and any CPU", ACE page); the ASR/TTS plugins' CPU fallback is **not documented** on
  the page. Assume: works on RTX, unknown elsewhere. Our R-0006 rule stands: the packaged build must run on non-NVIDIA hardware, so ACE could only
  ever be the voice on RTX machines, with this lane's text barks and keyboard signals as the fallback everywhere (already the design).
- A judge's machine without an NVIDIA GPU would load several hundred MB of inert model files.

### 1.4 Licence for a jam build

- The ACE page: the UE plugin suite is "**governed by the MIT license**"; the Audio2Face-3D SDK is MIT; A2F-3D model weights are under the **NVIDIA Open
  Model License**; Audio2Emotion models under a custom licence. The bundled speech models' own licences (Nemo conformer: NVIDIA; Chatterbox: Resemble AI,
  MIT upstream; Qwen 3.5: Apache-2.0 upstream) were not read: they are inside the zips. The ACE overview docs point at the **NVIDIA AI Enterprise EULA** (page last
  updated 2025-06-25) for the enterprise microservices; that is the cloud/Riva path, not the game plugins.
- Reading: redistribution of the plugin code in a free jam build is allowed (MIT). Redistribution of the model weights depends on each model's file-level
  licence; the Open Model License allows redistribution with attribution. Nothing on the public page forbids shipping; nothing on it confirms the exact terms
  for the speech models either. **Not ours to assume** (same stance as R-0004 on subscriptions): read the LICENSE files after a download before anything ships.

### 1.5 Real integration cost

(a) **TTS of a text bark** (the barks are 60-character lines from a reviewed bank):
- ACE TTS plugin: download 1.83 GiB, port 5.7 -> 5.9 (the UE wrapper module needs the recompile; the NVIGI binaries are engine-agnostic C ABIs),
  a `Synthesize(text) -> USoundWave` call through the sample's API, first-call model load latency unknown (hundreds of ms to seconds on a 4090 is the usual
  NVIGI figure), VRAM for a 350M model plus vocoder (1-2 GB). Estimate: **half a day** including the port and the staging of model files, and 300+ MB on the zip.
- **No-download alternative, available today:** UE 5.9 ships `Engine/Plugins/Experimental/TextToSpeech` (Epic, experimental, `EnabledByDefault: false`,
  Win64/Mac/Linux/Android/iOS). On Windows its factory creates `FFliteTextToSpeech`: the **Flite** synthesizer compiled into the plugin
  (`Source/ThirdParty/Flite/Flite-e0a3d25`), so the voice is offline, needs no OS voice and no download, at the cost of a robotic voice. API:
  `GEngine->GetEngineSubsystem<UTextToSpeechEngineSubsystem>()`, `AddDefaultChannel(Id)`, `ActivateChannel(Id)`, `SpeakOnChannel(Id, Text)`,
  `SetRateOnChannel`, `SetVolumeOnChannel`, `StopSpeakingOnChannel` (`TextToSpeechEngineSubsystem.h`, read 2026-11-12). Cost: one plugin line in the
  `.uproject` (`{ "Name": "TextToSpeech", "Enabled": true }`); this lane's bark system reaches the subsystem by reflection (section 3), so no Build.cs
  dependency and no compile break when the plugin is off. Package delta: Flite's English voice data, a few MB (to measure with the next package).

(b) **ASR of a two-second shout** ("defend", "space", "pass", "nice one"):
- ACE ASR plugin: download 698 MiB, same 5.7 -> 5.9 port, microphone capture through the plugin's own path, free-text output mapped to the four intents by
  keyword. Latency of a streaming conformer on RTX: well under a second. Estimate **half a day** plus 300-700 MB on the shipped zip, RTX machines only.
- **No-download alternative on Windows 11:** two built-in APIs.
  1. **SAPI 5.4** (`sapi.h`, COM: `ISpRecognizer` in-process `CLSID_SpInprocRecoInstance` or shared `CLSID_SpSharedRecoContext`, `ISpRecoGrammar` with a
     command-and-control grammar of our four phrases, `SPEI_RECOGNITION` events): part of every desktop Windows since Vista, works in an unpackaged Win32
     exe (a UE game is one), **offline**, English recognizer installed with the OS language pack. Source: Microsoft Learn "Speech API Overview (SAPI 5.4)"
     (`learn.microsoft.com/en-us/previous-versions/windows/desktop/ee125077(v=vs.85)`, archived 2012, still the shipping desktop API). This is the Phase 2b path:
     a Windows-only `SignalVoice` that loads SAPI through COM at runtime with a four-phrase grammar (list grammars are recognised on-device).
  2. **`Windows.Media.SpeechRecognition`** (WinRT): on-device list constraints run locally; dictation/web-search topics need the network. **Blocker:**
     "Speech recognition requires MSIX package identity. The `Windows.Media.SpeechRecognition` APIs are available only when your app runs with package
     identity ... Unpackaged apps cannot use these APIs." (Microsoft Learn, "Speech recognition in Windows apps", ms.date 2026-07-08). A UE jam zip is
     unpackaged, so this API is out; SAPI it is.
- Both need microphone permission from the player (Settings > Privacy > Microphone) and judges have no mic (R-0007 point 3): keyboard/gamepad first.

### 1.6 The download to approve, and this lane's recommendation

| Need | URL | Size |
|---|---|---|
| ACE TTS plugin for UE 5.7 (to port) | `https://developer.nvidia.com/downloads/ace/ue-plugins/ace_tts_v5_7_1.0.4.zip` | 1,963,104,106 B (1.83 GiB) |
| ACE ASR plugin for UE 5.7 (to port) | `https://developer.nvidia.com/downloads/ace/ue-plugins/ace_asr_v5_7_1.0.4.zip` | 731,867,583 B (698 MiB) |
| (not needed) ACE LLM plugin | `https://developer.nvidia.com/downloads/ace/ue-plugins/ACE_LLM_5.7_v0.5.zip` | 4,049,945,595 B (3.77 GiB) |

No login was needed for the `HEAD` (302 to a tokenised `developer.download.nvidia.com` URL, then `200 application/zip`); nothing was downloaded.

**Recommendation to the Director and operator:** do not download for M2/M3. The eligibility path is DLSS (R-0006); ACE would add a 5.7 -> 5.9 port of two
plugins, 2.5 GiB of downloads, several hundred MB on a zip that must stay under 1 GB, and a licence read before shipping, for a voice judges hear only
on RTX machines. Phase 1 ships text barks with the engine's own Flite TTS behind a flag (no download); Phase 2 ships keyboard/gamepad signals with
SAPI voice as 2b. If the operator still wants ACE as an experiment for the NVIDIA questions, approve the **TTS zip only** (1.83 GiB), time box the port
at 45 minutes of compile fixes like DLSS, and let ACE voice the barks on RTX with Flite as the fallback. This thread stops here until a ruling.

## 2. The brain seam for lane A3: `FJamTeamIntent` (Phase 2)

Header: `Game/Source/JamGame/Public/SignalIntent.h` (lane A12 owns it; A3 consumes it in `JamBrain`). The struct is in the file; the consumption rules:

| Intent | Who honours it | Rule for `Frames` frames from `IssuedFrame` (sim frames, 60 Hz) | Default `Frames` |
|---|---|---|---|
| `defend` | both bot teammates of the human | the chaser abandons the puck chase when the puck is in the human's attacking half and skates to a point 8 m in front of the own net on the puck's side; the back stays as it is (already defensive); yield rules unchanged | 300 |
| `space` | both bot teammates | each teammate skates to the free-ice target: the point among 6 candidates (own-half and attacking-half thirds, left/centre/right lanes) farthest from every opponent, at least 10 m from the human and from each other; a teammate carrying the puck keeps carrying | 300 |
| `pass` | the teammate that has the puck (if none: the one nearest the puck) | shoots towards the human's position + 1.5 x the human's velocity (a lead pass) on the first frame `TryShoot` would allow it, provided the human is within 25 m and no opponent is within 1 m of the pass line; after passing, falls back to its role | 180 |
| `praise` | the teammate nearest the human (chosen when the signal is issued; `TargetIndex` carries it) | `speed_scale` +10% for that skater (clamped to the sim's 1.5 input length), its reaction delay set to 0 | 300 |

- Intents arrive through `UJamSignalSystem::OnIntent` (native multicast, `const FJamTeamIntent&`), or by polling `UJamSignalSystem::GetActiveIntent(OutIntent)`,
  which returns false when none is active or `IssuedFrame + Frames <= current frame`.
- Because the brain lives in `JamCore` and the header in `JamGame`, the game mode (lane A1) copies the active intent into the `JamAI::FJamAIWorld` it builds
  each step (proposed fields `IntentKind` (0 none, 1 defend, 2 space, 3 pass, 4 praise), `IntentTarget`, `IntentFramesLeft` in `JamAICore.h`); the brain reads only the
  world, never a UObject, so the headless tournament stays deterministic (no intent = today's behaviour; a scripted intent sequence per seed = testable).
- A new intent replaces the old one (no stacking). The human's own figure never receives an intent.
- The bark system answers every accepted intent with a `signal_ack_<intent>` bark from the teammate that honours it ("got it", "on him"), and `praise` with `praise_reply`.

## 3. What shipped in Phase 1 (barks)

### 3.1 The bank (content law, R-0007 point 1)

- `Game/Content/Barks/barks.json` (`hokej.barks/1`): **1,225 lines**, 15 situations x speaker x tone, 12 to 20 lines per combination
  (opponent: mock, complain, bravado on 11 situations; teammate: encourage, praise, complain on all 15, the four `signal_ack_*` / `praise_reply`
  situations are teammate-only). English, ASCII, 6..60 characters, no capitalised word but sentence starts and "I".
- Made by `tools/barks/generate.mjs` through the R-0004 CLI transport (`claude -p --model claude-haiku-5-5`, one call per situation x speaker, the
  system prompt in a file, no tools, no settings, no MCP). Three runs: **1,518 raw lines, 48 rejected by `tools/barks/filter.mjs`** (22 word-list hits: cheat,
  dead/died, drunk, useless, money, punch, blood; 26 duplicates), **63 more removed by a reviewer regex** for things the game does not have
  (goalies, crowd, referees, offside, penalty box, coach), the surplus over 20 per combination trimmed. Cost: 36 CLI calls, about 5 minutes, $0.03.
- `Game/Content/Barks/REVIEW.md` lists every shipped line by combination for the Director's skim; delete a line in `barks.json`, run
  `node tools/barks/embed.mjs`, and the bank is updated. `tools/barks/barks.test.mjs` (premerge gate) proves the committed bank passes the filter,
  has no duplicates, has 12..20 lines per combination and equals the embedded C++ copy.

### 3.2 Runtime (`UJamBarkSystem`, `Game/Source/JamGame/Bark*`)

- `UJamBarkSystem` is a world subsystem (Game and PIE worlds only; no game-mode edit needed). It binds `UJamEventBus::OnJamEvent` and
  `UJamPresentation::OnSimFrame`, reads the sim through `AHokejGameMode::GetSim()` and the cartridge's raw JSON for `game.barks`.
- Situations: `goal_for` (human scored: a teammate praises; a bot teammate scored: the opponents complain), `goal_against` (the scorer mocks),
  `check_landed` (`check` with Who = human: the nearest opponent), `check_received` (`knockdown` of the human), `faceoff`, `win` (teammates), `lose`
  (opponents), `directors_call` (nearest opponent to the human). Derived in the frame handler: `steal` (puck `LastTouch` changes team: the thief mocks,
  or an opponent complains when the human took it), `stall` (300 frames without a touch and a puck under 2 m/s, at most every 600 frames), `near_miss`
  (a `board` hit within 45 frames of a `shot`, inside 1.5 m of the goal line and 3 m of the goal mouth). Shots, boards, pickups and periods say nothing.
- Picker (`FJamBarkPicker`, engine-free): speaker = nearest skater of the side to the event; tone by intensity (quiet: opponents only complain, teammates
  encourage/praise; normal: mock 50 / complain 30 / bravado 20; loud: mock 65 / bravado 25 / complain 10); no line repeats within 10 picks; global cooldown
  240 frames and per-speaker 480 at normal, doubled at quiet, halved at loud. Its PCG32 stream is seeded from the cartridge seed and never touches the sim.
- Shown by `UJamBarkWidget`: a rounded speech bubble above the figure's head (world-to-screen each render frame, 2.35 m above the feet, team-coloured
  outline, hidden when the speaker is off screen), full for 150 frames then a 90-frame fade, plus a one-line HUD strip above the bottom edge.
- Logged as `JAMBARK who=<idx> situation=<s> tone=<t> id=<id> frame=<f> text="..."`. `OnBark` native delegate for the Designer panel and Phase 2.
  Bus: broadcasts `bark` (Who = speaker, Kind = situation) **only once the bus knows the name**; until contracts/events.md gains `bark` and `signal`
  nothing is sent (an unknown name would be an error line). Request to the Director in the report.
- Cartridge `game.barks { enabled, intensity }` (schema, limits mirror, DEFAULTS.md, `DESIGNER_GAME_EDITABLE`, the Designer system prompt,
  `schema/hockey.test.mjs`). Console: `jam.Barks -1|0|1` (override), `jam.BarkVoice 0|1`, `jam.bark <situation> [teammate]` (force one, PIE proof).
- Voice: `UJamBarkSystem::SpeakWithEngineTts` reaches `UTextToSpeechEngineSubsystem` by reflection (`AddDefaultChannel`, `ActivateChannel`,
  `SetRateOnChannel 0.55`, `SetVolumeOnChannel 0.8`, `SpeakOnChannel`). It works the moment the `.uproject` enables the engine's `TextToSpeech` plugin
  (Flite on Windows, offline, no download); with the plugin off it logs once and the barks stay text. JamGame takes no module dependency on the plugin.

### 3.3 Tests

`Jam.Barks.BankClean` (the embedded bank parses, equals `barks.json` on disk, no duplicates, 6..60 ASCII characters, >= 12 lines per combination),
`Jam.Barks.PicksBySituation` (synthetic events and a synthetic six-skater world resolve to the expected situation and speaker side, the human never
speaks, quiet opponents never mock, disabled picks nothing, settings parse from a cartridge), `Jam.Barks.Cooldown` (240 / 480 frames, scaled by
intensity, no repeats within 10 picks, bubble opacity by frames). Node: `tools/barks/barks.test.mjs` (4 tests) and the new `barks` test in
`schema/hockey.test.mjs`.

### 3.4 Phase 2 state (keyboard and gamepad signals, 12:00)

- `UJamSignalSystem` (world subsystem, `Game/Source/JamGame/Signal*`): its own Enhanced Input context `IMC_HokejSignals` (keys 1-4; d-pad down = defend,
  up = space, left = pass, right = praise), bound on the player controller's input component (priority 1, above lane A5's context; A5's files untouched).
  `Issue(intent)` records `FJamTeamIntent` (frames: defend 300, space 300, pass 180, praise 300; target = the teammate nearest the human for praise and pass),
  fires `OnIntent`, broadcasts `signal` once the bus knows the name, and the nearest teammate answers with a forced `signal_ack_<intent>` / `praise_reply` bark
  (cooldown skipped so an answer always comes). Repeat signals inside 60 frames are ignored; nothing is accepted outside the Playing phase.
- The brain reads `GetActiveIntent()` through the game mode (section 2); until lane A3 lands the consumption, a signal only produces the acknowledgement bark.
- Console `jam.signal <defend|space|pass|praise>`; log `JAMSIGNAL intent=<name> frame=<f> frames=<n> issuer=<i> target=<t>`.
- Test `Jam.Signals.SignalsRouteToIntent`: slots and keys to intents, acknowledgement situations, the frame window, the key map. Passing.
- Phase 2b (Windows speech recognition through SAPI 5.4, section 1.5 b) is not started.
