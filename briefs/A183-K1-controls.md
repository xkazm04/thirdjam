# Lane A183, track K1 (the controls), evolution 1 (device A, Opus)

You are lane A183 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then **R-0064** at the end of `contracts/RULINGS.md` (the operator's words after playing package H31; point 1 is your charter, word for word), `docs/PLAY-ME.md`
(the controls he was given), `Game/Source/JamGame/Private/InputMapping.cpp` (today's bindings and `BindingTableText`), the settings screen
(`SettingsWidget.cpp/.h`), the HUD's first hint strip and the controls card (`HUDWidget.cpp`, the menu's CONTROLS screen in `MenuWidget.cpp`), and the
acceptance driver that presses the real keys (`tools/qa/accept/accept.ps1`, `accept.mjs`, `docs/experiments/ACCEPT.md`).

Your worktree and the commit it stands on are named in the Director's message that starts you. Status file `status/A183.md`. Port 4783. Family:
`InputMapping.cpp/.h`, the player controller's input code (the functions that turn actions into `FJamInput`), `SettingsWidget.cpp/.h` for the bindings and
their screen, the controls card's and the hint strip's TEXT source (one function each: other lanes edit the HUD and the menu today), the acceptance
driver's key table, `docs/experiments/CONTROLS-2.md`.

## Evolution 1: the operator's hands
His words: "Key bindings: Possibility to rebind needs to be there. And rebinding: a) Cross check on Left mouse. available only if player not on puck, if
yes then shoot b) Right mouse if on puck: Pass. Hold for saucer pass c) Crosscheck should be holdable so player with speed momentum can run with held
crosscheck into another player."
1. **The new defaults**: **LMB**: the human carries the puck: shoot (tap flat, hold to lift: unchanged); he does not: the cross-check (today's `bLift`
   verb), and the button's HELD state goes to the sim as a level every frame (the contact lane A184 adds the field `FJamInput::bCrossHold` and what it
   does: you set it; until its landing reaches you, set a local flag behind one line). **RMB**: with the puck: pass (hold for the saucer: today's `Pass`
   rules); without it: body check (`bCheck`). "With the puck" is read from the sim on the frame of the press (the carrier index), on both machines of a
   joined match alike (the lockstep hash must not move for a recorded input stream: the mapping happens before the input is recorded). A press that
   began with the puck stays a shot or a pass until released even if the puck leaves. Q, MMB and mouse 4 stay as second bindings. The pad: propose and
   build the same rule (RT shoot / cross-check, RB pass / check) and say what moves.
2. **Every verb can be rebound**: a CONTROLS screen in the settings: each verb's row with its key and pad button, click or press to listen, Esc cancels,
   a conflict is shown and swapped, RESET TO DEFAULTS; saved with the profile (the settings file's own section, written in one save, read at start,
   survives a restart in a package); the two-role buttons are shown as two rows ("LMB: shoot (with the puck)", "LMB: cross-check (without)").
3. **The words follow the bindings**: the first hint strip, the controls card, the pause card and the brake prompt take their key names from the live
   bindings (one function, `BindingTableText` or its successor), never from fixed text; `Jam.Menu.FitsTheScreen` and the HUD's fit test hold with the
   longest key name.
4. **The acceptance run presses the new defaults** (its key table: shoot LMB with the puck, cross-check LMB without, pass RMB with, check RMB without;
   its drill steps 4 to 8 stage "with" and "without" already): headless proof by the driver's own test; the window proof is the release chain's.
5. Tests: the mapping by possession (a staged carrier and non-carrier, each button, each role), a press that outlives possession, the rebinding's save
   and load, a conflict swap, the lockstep's recorded stream unchanged by the mapping. `Jam.Net.*` and the full `Jam` filter green; Shipping compiles.

Write `CONTROLS READY` in the status file with the file list when items 1, 3 and 4 are closed, built and gated (the Director lands them for the 15:30
package; the rebinding screen may follow as `CONTROLS READY 2` by 17:30); `SOURCE OPEN` before any later source change. `-Multiprocess`; headless only
(no cursor); stop only processes whose ids you recorded. Time box 150 minutes.
