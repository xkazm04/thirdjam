# Audio picks by the operator (2026-11-12 14:30 CET)

Source: the listening page of lab lane AU1 (`C:\Users\kazda\jam\audio-lab\listen.html`; takes and measurements in `status/B-audio-au1.md`). One pick per group.

| Group | Pick | Goes to |
|---|---|---|
| Commentator voice | **C1, classic play-by-play** (a designed ElevenLabs voice saved on the operator's account) | the commentary line bank of lane N2 (`briefs/commentary/`), generated in this voice |
| Body check | `SFX_BodyCheck_02` | `check` (layered with or replacing `MS_check`) |
| Clapperboard | `SFX_Clapperboard_05` | `faceoff` drop |
| Pickup chime | `SFX_PickupChime_01` | `pickup_take` |
| Projector loop | `SFX_ProjectorLoop_01` | menu and pause ambience |
| Puck on boards | `SFX_PuckBoards_01` | `board` |
| Puck on post | `SFX_PuckPost_01` | post hits (needs a way to tell a post from a board: ask device A) |
| Rule-change sting | `SFX_RuleChangeSting_02` | `directors_call` |
| Skate carve | `SFX_SkateCarve_02` | sharp turns of the player's skater (the `Turn` trigger of the skate loop) |
| Skate stride loop | `SFX_SkateStrideLoop_01` | the player's skate loop (needs the cross-faded version from lane N3) |
| Slap shot | `SFX_SlapShot_01` | `shot` |
| UI click | `SFX_UiClick_01` | menus and the Director's Call card (device A's UI) |
| Match music | `MUS_MatchLoop_01` | in-match music (needs a loop edit from lane N3 and the music licence check) |

No pick in these groups: the **director's voice** (neither stock voice), the **goal horn** (both takes), the **sound-stage room tone** (both takes), the **menu
theme**, and the per-line commentator samples other than one line of C1. Until the operator says otherwise these stay on the synthesised MetaSounds
(`MS_goal`, `MS_arena_room`) or silent (director, menu music).

Next: lane N3 makes the picked takes import-ready (levels, edges, loops); they are imported as `SFX_*` / `MUS_*` into `/JamAssets/Audio/` when an editor slot is
free; the first commentary batch is generated in voice C1 when lane N2's script is in.
