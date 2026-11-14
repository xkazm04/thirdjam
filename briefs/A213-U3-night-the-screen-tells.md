# Lane A213, track U3 (what the screen tells), the night: the first minute teaches the game it now is (device A, Opus)

You are lane A213 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md`, then `briefs/NIGHT-COMMON.md`: their rules are yours in full.**
Then **R-0067 and R-0068** at the end of `contracts/RULINGS.md`, `docs/PLAY-ME.md`'s Controls section (already out of date: the pass is by power
now), the HUD (`HUDWidget.cpp/.h`, `HUDBrace.cpp`, `HUDGoalBanner.cpp`, `HUDPassMeter.cpp` of lane A207, `HUDMarks.cpp`, the buttons' badge of the
controls track, the hint strip, the scoreboard line), the front end's style table (`MenuWidget.cpp`: `FJamMenuLook`), the CONTROLS screen
(`SettingsWidget.cpp`), the first-match hints and drills (`jam.ctrl.Drill`, the carrier's marks), and tonight's READY sections of lanes A206 to A210.

Your worktree and the commit it stands on are named in the Director's message that starts you. Status file `status/A213.md`. Port 4813... use 4815
(4812 to 4814 are the hardening lane's). Family: the HUD's in-play elements and their text (not the hits' marks: `HitRead.*` is lane A210's; not
the pass meter's sim side: `HUDPassMeter.cpp`'s LOOK is yours, its numbers are A207's), the hint strip, the badge, the CONTROLS screen's text and
the pause card's stick lines, the first-match teaching, `docs/experiments/SCREEN-TELLS.md`. Presentation only: the sim's hash untouched. Off screen
with `-dpcvars=r.RDG.AsyncCompute=0`; you do not own the cursor.

## The game changed under its own instructions tonight
A hold is now the pass's power; a pass goes to the man the stick points at; the shot is twice as fast; a loose puck is his who touches it; a check
finds its man; the cross-check shoves any man in front; the brace needs 13 frames of hold. The screen still explains yesterday's game, and it
carries a day's worth of marks from lanes that each added one. He said "the game itself does not look bad": keep that, and make the screen say
only what helps.
1. **Inventory** (first 30 minutes): every element drawn over the play in a match at rookie (stills at 1280 x 720 and 1920 x 1080 from the game
   camera): what it says, when, whether it is true under tonight's rules, whether a newcomer needs it. One table: keep / fix the words / move /
   remove. The rule: nothing covers the puck, the carrier or the net; nothing lies; at most one line of words on screen at a time outside the
   scoreboard and the buttons' badge.
2. **Possession is seen at a glance** (his second point, the part the HUD carries): whoever has the puck is marked the instant he has it (the ring
   or chevron in his team's colour: use what exists; the pickup's moment gets a short pulse: device B's `MS_possess` sound comes with the event
   `possess`: lane A208 emits it): you, a mate, an opponent: three states he can tell apart without reading; when nobody has it the puck's own
   ring says so. The carried puck must be visible from his camera (device B found it hidden behind his hips when carried straight: lane A210
   takes the drawing; you take the cue that says "you have it").
3. **The pass's power and aim are shown** (lane A207's `HUDPassMeter.cpp` draws a line on the ice: judge it by stills: does a newcomer understand
   "hold = further" in one look? the meter by the stick, the target mate marked while he holds: device B's `SetPassCharge` pose is wired by A210);
   the shot's charge the same way. One visual language for both.
4. **The words**: the buttons' badge, the hint strip, the CONTROLS screen, the pause card and `docs/experiments/SCREEN-TELLS.md`'s text for
   PLAY-ME: LMB: shoot (hold to lift) / cross-check (hold to brace); RMB: pass (hold for power) / body check: from the live bindings, true to the
   rules as landed (read the lanes' READY sections; ask in your status file under `ASK` where two disagree).
5. **The first minute teaches three things in order** without a tutorial screen: you have the puck (the cue), pass it (the badge pulses RMB when a
   mate is open ahead: once or twice a match, never nagging), hit a man (the badge pulses when an opponent carrier is in reach and in front). Each
   prompt shows at most three times a session and never again after he has done the thing. A cvar turns the teaching off.
6. Tests in `Jam.Hud.*` for each rule (a prompt's count, nothing over the puck, the three possession states); `Jam.Menu.FitsTheScreen` stays green.

`SCREEN READY 1` (items 1, 2 and 4) by 02:30 by the machine clock; `SCREEN READY 2` (3 and 5) by 05:30; built, the full `Jam` filter green in a
cleared environment, Shipping compiles, ten stills each. Two heavy processes at most. Time box: the night.
