# HOKEJ

*Raw ice hockey made of clay. You skate. You aim the stick with the mouse. You hit people.*

<!-- Status 2026-11-13 (H24 build). Text only: no logos, no trademark art. Voice raw, teen-rated (R-0021: no gore, mild words at most one in ten). -->

## The pitch (two lines)

Hokej is third-person arena hockey where the stick is your mouse and the other team is trying to put you on the ice. Every match is a *cut*: pick one, or let the Director change a rule mid-game and watch the rink turn on you.

## What you do

- Skate behind your skater, aim the stick blade with the mouse, charge a shot by holding the left button, let go to fire.
- Poke the puck off a carrier, sweep it, spin off a check, deke past a stick, pass to a team-mate.
- Body-check a carrier into the boards. Cross-check a skater who is not looking. Watch the clay figure go down, slide, and get up angry.
- Take the season (eight takes, a ladder of rivals), or pick a cut from the Director's choices.
- Difficulty is aggression: rookie bots barely touch you; on the hard rungs they hunt the puck carrier. Play it and find out.

## Controls (as shipped in H24: read from `Game/Source/JamGame/Private/InputMapping.cpp`)

| Action | Keyboard and mouse | Pad | Status |
|---|---|---|---|
| Skate | W A S D or arrows | left stick | live |
| Aim the stick | mouse (the blade follows the cursor on the ice) | right stick | live |
| Shoot (hold to charge) | left mouse | RT | live |
| Body check | right mouse | RB | live |
| Cross-check (lift the stick) | middle mouse or mouse 4 | LB | live |
| Sprint | Shift | LT | live |
| Brake | Ctrl | left stick click | live |
| Poke check | Space | A | live |
| Spin | E | B | live |
| Deke | Q, mouse wheel | Y | live in H24; Q moves to Pass when lane A84 lands (wheel and Y stay) |
| Pass | F | X | live in H24; **moves to Q** (R-0041) |
| Ability (one-shot shotgun, hip-check burst, shield, slow puck) | F | (to be decided by A84) | **planned, not live**: granted by a Director's mode or a pickup |
| Pause, Time Out, Director's Call | Esc | Start | live |
| Restart | R | Back | live |

Update this table from `InputMapping.cpp` on the final package: it is the one place that is allowed to disagree with the page.

## The Director

Once per period you can stop the game and change one thing about it. Pick a mode and the cut rewrites itself. Twelve ship as data (cartridges), no code, each with a one-line pitch:

- **Pond**: skate and pass on a pond, nobody hits. **Cage**: a tight box, live boards, hit, do not poke. **Sniper**: a full-height net, a low guard, lift it.
- **Marathon**: long ice, a long clock, pace it then go. **Power Play**: four against three. **Blind Side**: dekes that fool the back, checks that stick.
- **Sudden Death**: one goal ends it, the best bots. **Outnumbered**: three against five, hold on then strike. **Glass House**: every hit is a risk, fouls cost 10 s.
- **Pinball**: bumpers, live boards, a puck that never slows. **Bruisers**: five a side, heavy bodies, no whistle, hit (the mode the video shows). **Fog of War**: slow bots in the fog, slip past, hit soft.

Direct a Cut also takes one sentence from you and hands it to the Designer.

## Look and sound

Clay-and-ink toy figures under four spot cones on a night rink, a thick ink outline that thins with distance, blue against orange. The commentator and the bench talk back.

## Tools, in plain words

- Engine: Unreal Engine 5.9 (C++ gameplay, Blueprint visuals, Niagara, MetaSounds).
- Code, tests, the build gate and this dev log were written and run by AI coding agents driven by two people on two machines; the dev log (Glass Box) shows every turn, hour by hour.
- AI-generated content, declared as such and **not** claimed as sourced (see `ASSETS.md` / credits): concept and UI images (an image-generation model), skater meshes (Meshy, Hyper3D Rodin, Tripo3D), commentator and crowd voice and sound effects (ElevenLabs).
- Original: figures' animation retargets, rink, materials, VFX, MetaSounds, everything procedural.
- Sourced (not ours, not scored): Unreal Engine template mannequin animation (unshipped), the DLSS plugin library, the engine's own content. Full list: `credits.md`.

## Team

Team name: Hokej. Members: (the operator fills the names and roles on Sunday).

## How to run

Unzip `Hokej_Hokej.zip`, run `ThirdJam.exe` (the game's internal project name), expect a Windows firewall prompt on first launch (the game opens a network socket at start; Play With A Friend needs it, solo play does not: A88 finding, not yet confirmed). Windows 11, a GPU that does DirectX 12. First launch compiles shaders for a few seconds.
