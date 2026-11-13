# Lane A144, track U2: the menu and the first run (device A, Fable)

You are lane A144 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/VISION.md`, R-0041 (the brand: raw and brutal, teen rating) and R-0048 to R-0054 at the end of `contracts/RULINGS.md`, `docs/experiments/VOICE.md`, the
design eye's `docs/design/REVIEW-1.md` (its items on the menu and the first five minutes), `submission/TRUTH.md` (what the page promises), the modes' own table
`Game/Content/Cartridges/modes/MODES.md`, `docs/experiments/JOIN-GUIDE.md`, and the screen track's stills tool (`tools/qa/screen/u1-stills.ps1`: an off-screen game
process of your own, no cursor).

Your worktree and the commit it stands on are named in the Director's message that starts you: a **fresh worktree at the newest main**. It has no binaries: build
the editor first. Your status file: `status/A144.md`. Port 4744. Family: `MenuFlow.*`, `MenuWidget.*`, `MenuStyle.h`, `SettingsWidget.*`, the season and
end-of-match cards, the lobby's host and join screens' texts and layout (`Game/Source/JamNet/**` widgets only: not the protocol), `docs/experiments/MENU.md`.
Not yours: the HUD in a match (track U1), cartridges (the cards read their `game.meta`: a wrong text is a finding for the modes track), anything under
`Game/Plugins/JamAssets`.

## Evolution 1: a judge's first sixty seconds, before the puck drops
A judge opens the game, reads the first screen, picks something and plays. Nobody has looked at that path since Thursday, and the game under it changed:
twelve Director's modes, a season of eight takes, a joined match, new verbs.
1. **Take stills of every menu screen as it is** (the first screen, Play the season, Choose your cut with its twelve cards, Direct a cut, the settings, the
   host and join screens, the end-of-match card after a win and a loss), on 1920 x 1080 and 1280 x 720, and read them: write the list of what a first-time
   player cannot find, cannot read or is told wrongly (known: a "SCENE 3 v 3" line that may be stale; a hint bar without the brake; twelve cards on one
   screen; the join flow's wording against `JOIN-GUIDE.md`).
2. **Fix what is in your family**, by rank: (a) the first screen says what the game is in one line and makes "play now" the obvious first press (a first
   match on the regular cut, no choices); (b) the twelve mode cards read at a glance (name, one sentence, what is different) and fit both resolutions
   without clipping; (c) the controls are one screen away and complete (mouse aims; LMB shoot, a tap is a flat shot; RMB check; MMB cross-check or lift; Q
   pass, held is a saucer; F the ability; E spin; the wheel deke; Shift sprint; S against your motion brakes; Space poke; and the pad's equivalents);
   (d) every stale line gone; (e) the host and join screens say what `JOIN-GUIDE.md` says, in the same words.
3. **Stills after**, the same set, and a test for each rule you add (the twelve cards present, no clipped text at 1280 x 720 by the widget's own desired size,
   the controls list complete against the bindings).

Voice: short, rough, never cruel; no brands, no real people. Look at your stills before you report. The acceptance run walks the menu with the real cursor:
name every button you moved or renamed, so the cursor owner's driver can follow. The gate: the full `Jam` filter green (the menu tests count twelve cards);
the hash unchanged. `-Multiprocess` on every headless start. Time box 100 minutes; interim in the status file.
