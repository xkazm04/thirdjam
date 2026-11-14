# Lane A203, track K1 (controls), evolution 4 (device A, Opus)

You are lane A203 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then **R-0064** at the end of `contracts/RULINGS.md` (the operator's words after playing; point 1 and today's clock: the freeze is 20:00), the track's
`status/A199.md` and `docs/experiments/CONTROLS-3.md` (the possession rule in the mapping layer, the thresholds, the pad, rebinding's edges, and the
one corner left open under `ASK`), `docs/experiments/ACCEPT.md` and `tools/qa/accept/` (the acceptance run: `accept.ps1`, `accept.mjs`, `keys.json`,
the drill `HokejAcceptDrill.cpp`: how a row is staged, pressed and judged).

Your worktree and the commit it stands on are named in the Director's message that starts you. Status file `status/A203.md`. Port 4803. Family:
`InputMapping.*`, the player controller's input code, `ControlsTests.cpp`, the rebinding rows' behaviour in `SettingsWidget.cpp` (lane A195 restyles
that file today: behaviour functions only, name each), the acceptance run's rows for the controls (`tools/qa/accept/keys.json`, the rows' judging in
`accept.mjs`, the drill's steps for them), and **for item 1 only** the shot's and pass's charge in `JamSim.cpp/.h` (the contact lane A200 works in
the same file on the wind-ups today: one small function and one call line, nothing else in the sim). You do not own the cursor.

## Evolution 4: the last corner of the possession rule, and the rows that prove the buttons in a package
1. **The charge is cleared when the carrier loses the puck** (the Director's ruling on the track's `ASK`): today the sim fires a held shot's or pass's
   charge on the release whenever the puck is in reach, so a puck stripped but still in reach is shot by the release of a button pressed before the
   strip. The rule: when a skater stops being the carrier, his shot and pass charges are cleared and a held button charges nothing until it is pressed
   again (the same words as the mapping layer's rule, now true inside the sim, for the human and the bots, host and guest alike).
   Behind a cartridge key (propose the name under `game.skater` or `game.puck`: say why; schema, limits, DEFAULTS, README, both generators), neutral =
   today. **Shipped ON only by the Director**: deliver it OFF in the shipped cartridges with: the sim test (release shots after a strip that stays in
   reach: 1 becomes 0), the hash proof at neutral (100 seeds a cell, six cells, against main), and with the key ON the six cells at 600 paired seeds
   (the ladder's instrument: `tools/perf/results/a189/` or `a193/` scripts are in your tree: copy), the bots' shots and goals a match before and
   after (a bot that loses its charge on every bump may stop scoring: that is the risk the row must show), `Jam.Net.HashCoverage` and the lockstep
   tests green (no new state field if the existing charge fields are simply zeroed: say).
2. **The acceptance rows**: `held_across_change` (a held brace takes the puck: no shot; let go, press: a shot), `tap_pass_flat` (a 12-frame press with
   the puck: a flat pass, no saucer), `saucer_held` at the new threshold, `rebind_refused` (a move that would leave a control with no key is refused
   and the line says so), and the pad's possession step if the drill can press a virtual pad. Staged in the drill, judged in `accept.mjs`, keyed in
   `keys.json`; proven headless as far as the drill runs headless (say which rows need a real window: the Director runs those in the release chain).
3. **Capture outside the panel**: while a rebinding cell listens, a press anywhere on the screen binds (today it reaches the menu behind): a test with
   a press 200 px outside the panel.
4. **The first five minutes, by script**: a scripted stranger's inputs through the real mapping layer for 3,000 frames at rookie (mouse buttons pressed
   as a newcomer does: late, long, both at once, held through faceoffs and goals): count every press that produced a verb he could not have meant
   (a shot with no puck in reach, a check while carrying, a brace that began on a tap, a saucer on a hurried tap): the table; fix what is in the
   mapping layer.

Write `CHARGE READY` (item 1: the file list, the rows, your recommendation in one sentence) **by 17:15**, `ROWS READY` (items 2 and 3) **by 17:45**;
built, the full `Jam` filter green, Shipping compiles, before each; `SOURCE OPEN` before any later source change. `-Multiprocess`; three measurement
processes at most; stop only processes whose ids you recorded. Time box 170 minutes.
