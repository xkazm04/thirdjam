# Hokej: the game on one page (H4, 2026-11-12 10:05)

**Pitch.** HOKEJ: ice hockey 3 vs 3 on a wide-open rink, played from behind your skater with a mouse-aimed stick, where every match is a *cut* you can direct: pick a cut, or tell the Designer
in one sentence how you want it, and play your Director's Cut. Theme: Director's Cut (`docs/THEME.md`).

**Core loop (the 2-minute first play).** Faceoff at centre ice; you skate (WASD or left stick), shoot (space / A), check (shift / B, in Raw Cut only).
Two bot teammates play roles (chaser, back); three bot opponents do the same with the yield rules so you always get the puck within seconds. First to 3
goals, or a 3-minute timer. Goal: 2-second pause, scoreboard counter clicks, faceoff. Win: "THAT'S A WRAP"; lose: "RETAKE?" and restart on R.

**Cuts shipped (cartridges, `Game/Content/Cartridges/`).** Regular (M1, L2 night painted-cel arena, **90 x 45 m**, skater speed 11; decided 2026-11-12 11:45 on measured pacing), Raw (M2: checks, no rules), Boost (M3: pickups),
Large Rink (120 x 60, speed 8), Wide (140 x 70), and `My Director's Cut` (whatever the Designer last produced).

**The twist: Director's Call.** Once per period, pause and change one tunable live; the scoreboard announces the cut. Data over existing systems.

**Five things the Designer may change from a sentence** (`prompts/designer.system.md`): skater speed and grip; shot power and aim cone; rink size, board bounce, ice
friction; pickups (kinds, rate, strength); look tier, trim colour, palette and the cut's name. Never: team roles, camera mode, the number of teams.

**Look.** P2 painted cel at night: toon ramp + ink outlines as post passes, four spot cones over scuffed ice, blue and orange emissive trim, no crowd (`tools/skills/look-painted-cel.md`).
Toy-figure skaters by code; a Meshy-generated figure as the first humanoid track on device B.

**Camera.** Third person: 6 m back, 3.5 m up, 5 m look-ahead, FOV 70, 4-frame lag, skater 0.7 m off-axis; far-net screen marker; radar bottom-right (`tools/skills/camera-third-person.md`).

**Readability aids.** Puck 0.12 m near-black with a trail and a glowing contact disc; jersey colours `#1D4ED8` and `#E8651A`.

**Tests that gate every milestone** (`docs/HOCKEY.md` 7): PlayerBotWins (seeds 1, 2, 7, same hash), AiBotPlays, HumanHasSomethingToDo, FarNetReadable, FrameBudget 8 ms, Package, CartridgeRoundTrip.

**Not in this game.** Goalies, online play, accounts, real brands or logos, a bundled API key, photoreal anything.
