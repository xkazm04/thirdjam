# Submission form: answers to paste (lane A233, track D1, Sun 15 Nov 2026 17:54 machine clock, main 81dbe14e; the operator pastes at the event page about midnight)

One section per field of the jam's submission, as lane A232 read the event page at 17:41 (`tools/qa/a233-in/A232-status.md`). Each section: the text to paste (in a quote block,
pasted as it is), then the row that proves it. `[O<n>: ...]` = only the operator can type it (the question is in `submission/README.md` section 1); `[the final package: ...]` = the
Director fills it at 22:30 (R-0073). Nothing in a slot is a guess. Before pasting: `grep -n "[[]O[0-9:]\|[[]the final" submission/form.md` lists every blank.

**What this describes.** The game on main (`81dbe14e` when this was written) and the package the Director cuts at 22:30 from the last green landing (R-0073). **DLSS is back**: the
operator fetched the plugin again at 17:14 and every package from H44 on carries it (the Director's word), so each DLSS sentence below is the "with DLSS" version of `TRUTH.md` 5.6.
**Windows 10 or 11 (DirectX 12) only. No web build.** (A232: NO-GO; R-0074.) Rows named `P6-..` are in `TRUTH.md` pass 6 (the candidate H45 and lane A235's fixes, 21:00); rows named `P5-..` are in pass 5 (true of H43, the last package read sentence by
sentence); nothing below says more than those rows and the ledger.

The jam's deadline: Mon 16 Nov 02:59 CET (A232: "Nov 15 5:59 PM PST"). He submits about midnight (R-0073).

---

## 1. The game file (zip, named TeamName_GameName, under 1 GB)

> `Hokej_Hokej.zip` [the final package: about 283 MiB]

- Made by `tools/run/package.ps1 -Submission` from the final package: top folder `Windows/`, start `ThirdJam.exe`, `README.txt` beside it (`submission/package/README.txt`).
- Size with DLSS: about 283 MiB zipped, 432 MB unpacked (the candidate H45: 296,480,940 bytes; P6-FM01). Well under 1 GB. The Director writes the real byte count at 22:30.
- Windows 10 or 11 (DirectX 12) only. No web build.
- Not yet done on any package: the clean-machine test on the second machine (`[O7]`; the page says "not tested").

## 2. Team name

> Hokej

P5-FM01.

## 3. Members: each member's name and the e-mail linked to their Epic account

> [O1: each member's name, and the e-mail address linked to that member's Epic account: typed by the operator at the form, never written into a file]

No name, e-mail or account is written in this repository, on purpose. For the Solo Driver box see section 10.

## 4. Game name

> Hokej

P5-FM01. On screen the game prints HOKEJ; the executable is `ThirdJam.exe` (the project's internal name).

## 5. Gameplay video (30 to 60 s, a link)

> [O17: the link to the uploaded film, 30 to 60 s]

What exists: `submission/video/SHOTLIST.md` (every shot, and what is recorded). `submission/video/hokej-v1.mp4` (39.5 s, silent) is Thursday's three-a-side build: a record, not
the film to submit (P5-FM07). The film must show the package that is uploaded (DLSS in it), no vendor or engine name on screen, the theme in its first five seconds (`SUNDAY.md`, 14:00 row).

## 6. Dev log (Glass Box, a link)

> [O8: the public link to the dev log, once `submission/devlog/index.html` is hosted]

The file to host: `submission/devlog/index.html` (one self-contained HTML page, 8,154,024 bytes = 7.8 MiB, its 58 pictures inside it; rebuilt by lane A233 at 18:02 with `--project-only` from main
`81dbe14e`: 941 commits, 228 agent sessions of this jam only, 34,638 turns, Wed 11 Nov 20:40 to Sun 15 Nov 17:35). It needs no other file: upload it where a single HTML page is served
publicly and paste that link. What is in it, what is not (device B's transcripts are not), and the privacy check: `status/A233.md`.

## 7. Tools used

> - Unreal Engine 5.9: C++ gameplay, Blueprint visuals, Niagara, MetaSounds.
> - Claude Code with Claude models (Opus, Sonnet, Fable): planning, all C++ and tooling, the tests, the measured playtests, the Designer's prompts, the dev log builder.
> - Codex CLI image generation (OpenAI): menu and HUD art, the fourteen MODES illustrations, in-game textures, the concept sheets the skater figures were built from.
> - Hyper3D Rodin: 3D generation of the skater figures and their textures. Meshy: rigging of the figures (its text-to-3D and text-to-motion were tried, not shipped). Tripo3D through fal.ai: 3D generation tried, not shipped.
> - ElevenLabs: the commentator's and crowd's voices, sound effects and music.
> - NVIDIA DLSS 4.5 (Unreal plugin v8.9.0, Super Resolution and DLAA, ported to 5.9): on by default on RTX GPUs, set by the game itself (no switch in SETTINGS).
> - Google Gemini: image recognition of game stills during development, as a second pair of eyes (advice only; nothing of it ships).
> - Blender: the arena's pieces, the stands and the crowd, figure takes, checking generated meshes.
> - ffmpeg: capture and video encoding.
> - Anthropic API: the in-game Designer (MAKE YOUR OWN) turns a player's sentence into a game; the player types their own key, none is bundled.

Proof: `submission/credits.md` (16:21) and `ASSETS.md`: Codex rows 76 (64 shipped); Rodin rows 70 (44 shipped, among them the shipped figure built from a Codex concept and
rigged by Meshy); Meshy-only rows 81 (0 shipped); Tripo3D 10 (0 shipped); ElevenLabs 519 (477 shipped); P5-FM10; the DLSS line is P5-FM11 with DLSS; Gemini: R-0024 and
`tools/eyes/` (lane A27). The page itself names no vendor (`[O5]`); this field asks for tools, so it names them (R-0006: partner prizes depend on it).

## 8. NVIDIA questions

A232 did not copy the questions' wording from the page. The answers below follow the draft of `docs/experiments/DLSS.md` section 3, corrected where the game differs (there is no DLSS row in SETTINGS;
the CREDITS screen names NVIDIA DLSS as text since H45). Match each answer to the question the page asks.

> **Which NVIDIA technology does the game use?** NVIDIA DLSS 4.5 Super Resolution and DLAA, through NVIDIA's DLSS Unreal Engine plugin v8.9.0, rebuilt for Unreal Engine 5.9 (NVIDIA ships it for 5.8; the port needed ten one-line format-string fixes).
>
> **How is it used?** DLSS is on by default on any RTX GPU: the game picks the mode for the window size itself, with no switch in the menus. On other GPUs it runs Unreal's own TSR, with no error.
>
> **What did it bring?** The passes that scale with resolution shrink: at 1080p in DLSS Quality the Lumen reflections went from 0.79-1.02 ms to 0.45-0.53 ms of GPU time, and at 1440p the base pass from 0.42-0.49 ms to 0.13 ms. On our development card the whole game takes 4 to 7 ms a frame at 60 fps, so the saving matters most on mid-range RTX cards.
>
> **Not used:** Frame Generation, Ray Reconstruction, Reflex. Kept out for size and latency in a 60 Hz arcade game.

Proof: `docs/experiments/DLSS.md` sections 1.1, 2.3 to 2.5 (measured with other editors on the same GPU: "trends only"); `DlssJam.cpp` (`UJamDlssSubsystem`); P5-FM11.
Closed: the plugin's licence asks for a text credit (DLSS.md 1.4); the CREDITS screen reads "Uses NVIDIA DLSS technology, (c) NVIDIA Corporation." (`MenuWidget.cpp:1207`; lane A234's walk of H45: "the DLSS line"; P6-FM04).

## 9. OpenAI Awards (optional): which OpenAI models or products you used, and how

> We used [O: the model / product name as your account shows it] through the Codex CLI's image generation (the `codex exec` command with its built-in image tool, one picture a run, every prompt kept in a job list in the repository). It made the game's 2D art. 76 generated pictures went into the asset ledger, and 64 of them ship in the game:
> - **Menu and HUD art (32):** the title slate, the button plates (normal, hovered, pressed), the panel, the opening match card, the pause card's frame, the countdown ring, the film-strip row frame, six game-type card backgrounds and seventeen icons (game types, pickups, team signals, settings, voice, barks, upscaling).
> - **In-game textures (15):** the two team crests and the jersey number atlas (each with a copy for the scoreboard), five pickup glyphs, the fast-ice zone tile, a felt fibre mask, a clay thumb-dent normal map made from a generated height map, and the painted hills backdrop behind the stands.
> - **The skater figure (3):** the concept sheet the shipped skater was built from by a 3D-generation service (its mesh, skeleton and base colour are ledgered with that concept).
> - **The MODES screen (14):** one illustration for each mode, drawn in the look that mode plays (four on the neon arena), made the same way on the last evening, in place of screenshots.
>
> Nothing in the game calls an OpenAI model while you play; the pictures are files in the package.

Proof: `ASSETS.md` rows with tool "Codex image generation" or "Codex concept sheet + ..." (76 rows; `shipped` y on 64: device A's UI 32, the MODES illustrations 14 (`Game/Content/UI/Modes/T_UI_ModeArt_*`, lane A231, landed on main c7057ed0), device B's textures 15, the P2C figure 3);
`tools/images/gen.mjs` (header: "codex exec with the built-in image_gen tool"); `status/B-images-n1.md`, `status/B-skater-skc.md` (the concept sheets). The ledger counts imported
pictures, not every try: rejected tries are not in it, so 76 is not the number of images ever generated. The status files show no other OpenAI use in development.
The fourteen are landed (main c7057ed0) and seen in the candidate H45: lane A234's walk, MODES: "each illustration whole in 16:9 ... Arsenal, Cage, Marathon show the neon arena and play it" (P6-FM02, P6-FM03).

## 10. Modifier categories

> Solo Driver; Render Royalty: OpenAI Award; Glass Box.

The operator's choice (R-0074; slot O4 answered). Per category:

- **Solo Driver.** The page (A232): "Best game built by a solo developer"; teams are "Solo, or up to five people"; "Up to five humans. As many AI agents as you want." and "There's no
  limit on agents". The page says nothing about machines or devices. What is true here: **one person directed the AI agents, on two machines** (device A and device B). That fits
  the page's words for a solo entry; the page gives no more than that. **The official rules document could not be read by A232** (its viewer did not render): check its definition
  of "solo" before ticking the box. `docs/PLAN.md` section 8 reads the same way: "Solo Driver is available because there is one human".
- **Render Royalty: OpenAI Award.** Render Royalty is the "best visuals" modifier; the OpenAI Award is its sub-award for entries that "use OpenAI models or products, during
  development, gameplay, or both" (A232). The page never says "image generation": the answer of section 9 is what qualifies.
- **Glass Box.** Asks for the dev log's link (A232): section 6, slot O8.
- How many modifier prizes one entry may win: the page is silent (A232). `docs/PLAN.md` section 8 assumed one.

## 11. Pitch, blurb, track, theme (if the page asks for them)

> Hockey with the whistle taken out: clay skaters, and the speed you bring is the hit you get.

`page.md` section A (P5-PG01). The 60-word blurb: `page.md` section B (P5-FM03). Track: Unreal Engine. Theme: Director's Cut (P5-FM01). A232 did not list these among the
fields; they are here in case the page asks.

## 12. Sourced assets (if the page asks)

> Sourced, not ours: four font assets made from the engine's own Roboto (Apache 2.0) and the NVIDIA DLSS library (NVIDIA RTX SDK licence). Everything else is original or AI-generated and declared as such.

`submission/credits.md` (16:21): 66 sourced rows, 5 shipped: the four fonts and the DLSS library (P5-FM13, with DLSS). The other 61 are the engine's template clips and materials,
`shipped` n. The ledger gate was not rerun on the final tree (`[O6]`).
