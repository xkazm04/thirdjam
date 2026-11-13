# HOKEJ

<!--
  Page text v2 (lane A96, 2026-11-13 17:20). Voice: docs/experiments/VOICE.md (rough, funny, never cruel, teen-rated, no names).
  No trademark words or logos: vendor names live in credits.md and the dev log (text credits), not here. Unreal Engine is named as the engine.
  Facts read from: Game/Source/JamGame/Private/InputMapping.cpp (bindings, main bb18439), Game/Content/Cartridges/modes/*.json (game.meta pitches),
  docs/VISION.md (R-0045). Lines marked VERIFY depend on a lane that had not landed at bb18439: re-read them on the final package.
  Three lengths below: A (one line), B (60 words), C (the full page).
-->

## A. The one-line pitch

Hockey with the whistle taken out: clay skaters, a mouse for a stick, and a Director who rewrites the rules mid-game.

## B. The 60-word blurb

HOKEJ is rough hockey for clay skaters. The mouse is your stick; the other team wants you flat. Hits obey momentum, so the carrier wins by stopping and cutting, not hiding. Five a side, and the bots pass. Nobody is polite. Once a period the Director rewrites one rule mid-match: a shotgun on F, or a pond with no hits.

## C. The full page

*Clay skaters. Real hits.*

You skate. You aim the stick with the mouse. The other team is trying to put you on the ice, and the ice is not soft.

### What kind of game this is (four sentences)

1. **Hits obey momentum.** Two skaters gliding the same way just bump; head-on at full speed is the biggest hit in the game, and the one who throws it pays for it too. (VERIFY: the contact law, lane A92, R-0045 law 1.)
2. **The carrier wins by stopping and cutting.** With the puck you cannot check, so you win with your feet: hit S against your own motion to brake hard, then burst away, because a defender at full sprint is always faster than you in a straight line. (VERIFY: brake and burst, lane A93, R-0045 laws 3 and 4.)
3. **Five a side, and they pass.** The bots read open ice: they carry when the lane is clear, pass when you close in, and have stopped charging a wall of defenders. Difficulty is aggression, not speed: rookies barely touch you, all-stars hunt whoever has the puck.
4. **Modes and abilities.** Once a period you can stop the game and let the Director change one rule: a mode can hand you a one-shot shotgun, a hip-check burst, a shield or a slow puck on **F**, or turn the rink into a pond where nobody hits.

### What you do

- Skate behind your skater, aim the blade with the mouse, hold the left button to charge a shot, let go to fire.
- Poke the puck off a carrier, sweep it, spin off a check, deke past a stick, pass to a team-mate (tap Q for a flat pass, hold it for a saucer that flies over a blade).
- Body-check a carrier into the boards. Cross-check a skater who is not looking. Watch the clay figure go down, slide, and get up angry.
- Take the season (a ladder of rivals), or pick a cut from the Director's choices.

### Controls

| Action | Keyboard and mouse | Pad |
|---|---|---|
| Skate | W A S D or arrows | left stick |
| Brake hard | S against your motion (or Ctrl) | left stick click |
| Aim the stick | mouse (the blade follows the cursor on the ice) | right stick |
| Shoot (hold to charge) | left mouse | RT |
| Body check | right mouse | RB |
| Cross-check | middle mouse or mouse 4 | LB |
| Pass (hold: saucer) | Q | X |
| Ability (when a mode or pickup gives you one) | F | R3 |
| Spin off a check | E | B |
| Deke | mouse wheel | Y |
| Poke the puck | Space | A |
| Sprint | Shift | LT |
| Pause, Time Out, Director's Call | Esc | Start |
| Restart | R | Back |

### The Director's twelve

Pick one from Choose Your Cut, or call one mid-match. Each is a rule set written as data, no code:

- **Pond**: pond hockey, pass, skate, nobody gets hit. **Cage**: a tight box, live boards, hit or get hit. **Sniper**: a big net and a low guard, lift it, bury it.
- **Marathon**: ten minutes of endless ice, pace it, then punish. **Power Play**: four on three, move it, fire it, repeat. **Shotgun**: F fires a shot that drops the crowd.
- **Sudden Death**: one goal ends it, the meanest bots alive. **Outnumbered**: three against five, get hit, hit back. **Glass House**: every hit is a gamble, fouls cost 10 s.
- **Arsenal**: grab a boost, get a trick on F. **Bruisers**: five a side, no whistle, hit everything. **Fog of War**: slow bots in the fog, slip by, then hit.

Direct a Cut also takes one sentence from you and hands it to the Designer, which turns it into a cut.

### Look and sound

Clay-and-ink toy figures under four spot cones on a night rink, a thick ink outline that thins with distance, blue against orange. A commentator who brags when you hit and winces when you get hit, and a crowd that never forgets.

### Tools, in plain words

Made with Unreal Engine 5.9 (C++ gameplay, Blueprint visuals, Niagara, MetaSounds). The code, the tests, the build gate and the dev log were written and run by AI coding agents directed by the team, and the dev log (Glass Box) shows every turn, hour by hour. AI-generated content is declared as such and is **not** claimed as sourced: concept and menu images from an image-generation model, skater meshes and some animation from 3D-generation services, the commentator, crowd voices and sound effects from a voice-and-audio service. Original: the rink, the materials, the effects, the figures' animation retargets, every MetaSound and everything procedural. Sourced (not ours, not scored): the engine's own content and one shipped graphics-upscaling library. Vendors, licences and every row: `credits.md` and the dev log.

### Team

TEAM: <names and roles, filled by the operator>

### How to run

Unzip `Hokej_Hokej.zip`, run `ThirdJam.exe` (the game's internal project name; the game is HOKEJ). Windows 10 or 11, a GPU that does DirectX 12. First launch compiles shaders for a few seconds. Windows may ask about the firewall the first time: solo play does not need network access, so Cancel is fine; Play With A Friend does. If it will not start, run `Engine\Extras\Redist\en-us\vc_redist.x64.exe` once. `README.txt` in the zip says the same, with the controls.
