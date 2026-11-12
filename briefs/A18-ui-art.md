# Lane A18: the UI art and typography pass (device A, from H7.3)

You are lane A18 of the Third Jam (team Hokej). The operator played the build and said: "HUD elements and typography are ugly to look and read, with image generation tool we have cheap way
how to create nice interface from the start" and "No need to mention Director's cut in the name of the game". Read first: `contracts/RULINGS.md` R-0013 points 4-5 and R-0008 (Codex images for
shipped 2D content: every shipped image ledgered, no brands, invented text only), `docs/THEME.md`, `GAME.md`, the 19 A8 screenshots and the A5/A12 ones in `devlog/shots/` (what exists),
`Game/Source/JamGame/Public/HUDWidget.h`, `HUDFeed.h`, `BarkWidget.h`, `DirectorsCallWidget.h`, `MenuWidget.h`, `MenuStyle.h` (A5/A8/A12: C++-drawn UMG, no widget blueprints; you take over
their **visual** layer: layout, type, brushes, colours, motion; the logic and the feed structs stay), `tools/images/gen.mjs` and `menu-jobs.json` (A8's generator; reuse), `ASSETS.md`,
`docs/experiments/CAMERA-120M.md` (readability on ice), `tools/skills/look-painted-cel.md` (the look the UI sits on: night ice, blue and orange).

You own: the visual layer of `Game/Source/JamGame/Private/HUDWidget.cpp`, `BarkWidget.cpp`, `DirectorsCallWidget.cpp`, `MenuWidget.cpp`, `SettingsWidget.cpp`, `MenuStyle.h`,
`Game/Content/UI/**`, `tools/images/ui-jobs.json`, `docs/experiments/UI.md`. Run no git command; editor on MCP port **8005** for proofs; link windows at :00 and :30; keep the tree compiling
(lanes A14-A17 are editing JamCore and the game mode in parallel; you touch only the files above). Never kill a process you did not start.

## Deliverables
1. **Type scale and kit** (first hour): one `FJamUiStyle` in `MenuStyle.h` with a readable scale from the engine's Roboto (sizes 14/18/24/36/64 at 1080p, weights, tracking, a 2 px dark outline
   or shadow for anything over ice), colours (ink, paper, blue `#1D4ED8`, orange `#E8651A`, take-yellow, muted), spacing, corner radius; a generated **slate kit** through Codex: 9-slice
   panel, score plate, TAKE clapperboard frame, bark bubble with tail (left/right), Director's Call card frame, cut card frame, button states, countdown ring, icons (cuts, pickups, signals 1-4,
   settings, voice, barks, DLSS), all in the painted-cel night palette, invented text only, each ledgered (`ai-generated`, Codex, prompt file, `size_mb`; textures under 2K, BC7 with mips off for UI).
2. **HUD**: a high-contrast scoreboard at the top (plates for both teams, the TAKE line, the timer), the far/own-net markers restyled, the radar framed, the controls hint as a bottom strip
   that fades, the faceoff countdown (A16 adds the feed field `FaceoffCountdownFrames`; show "DROP" at 0), announce/Director's Call lines as a single styled line, bark bubbles with tails anchored
   to figures, active-effect icons (A14 adds a feed struct; style a slot row), the win/lose cards.
3. **Menu**: the slate without the subtitle (title **HOKEJ** only; no "a director's cut" anywhere; the theme lives in the cuts, the takes and the Director's Call), cut cards with the frame kit,
   Settings laid out in two columns, Credits, the season screen (A17 adds it; style a placeholder), the lobby (A9's `LobbyWidget` stays A9's code; style through `FJamUiStyle` only if it reads it).
4. **Proof**: screenshots of every screen and the HUD during play at 1080p and at 720p (the menu must fit 720p), plus one shot with the UI over the darkest ice; a readability note (text
   contrast ratios, minimum sizes) in `docs/experiments/UI.md`.

Report: what changed, the kit list with sizes, screenshots, the ledger rows, every error. Time box 3 hours; report at 60 minutes with the kit and the HUD.
