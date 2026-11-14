# Lane A204, track N1 (the joined match), fault fixes (device A, Opus)

You are lane A204 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/experiments/JOIN-GUIDE.md`, the hardening lane's two faults below (found today by lane A198 with two processes on loopback), the lockstep
driver (`Game/Source/JamNet/Private/JamLockstepDriver.cpp/.h`), `LobbyGameMode.cpp`, `DirectorsCallController.cpp` (the pause card), and the net
tests (`Game/Source/JamNet/Private/Tests/*`).

Your worktree and the commit it stands on are named in the Director's message that starts you. Status file `status/A204.md`. Port 4804 (and 4805 /
4806 for two-process runs). Family: `Game/Source/JamNet/**`, the pause card's behaviour in a joined match (`DirectorsCallController.cpp`: the pause
functions only), the net tests, `docs/experiments/NET-FAULTS.md`. The lockstep **protocol stays 9 and the wire format is not touched**.
**Never change a firewall, network or system setting; never answer a security or network prompt (leave it, report it); loopback (127.0.0.1) only.
A still of a hosting lobby shows the OS user name and LAN addresses: it never leaves your worktree and never goes into a doc or your report.**

## Two faults, each fixed with a test that fails before and passes after
**FAULT 3: a joined match dies at frame 0 when the guest's rink loads more than 3 s after the host's.** The host's lockstep counts stall ticks while
it waits unheard (limit 1,800); the moment the guest's first input packet arrives `bHeard` turns true, the limit drops to 180, and the ticks already
counted (276 to 312 seen) exceed it: the host calls "connection lost" exactly as the guest arrives. Seen 2 of 2 on a busy machine (the guest's load
took 6 s). Where: `JamLockstepDriver.cpp:46` (the limit switches on `bHeard`) with `:76` (`bHeard = true` does not reset `StallTicks`). The lane's
remedy: at `:76`, `if (!bHeard) StallTicks = 0;` before `bHeard = true;`. Check the guest's side for the mirror case (the guest hearing the host
late), and the rematch path (is `bHeard` reset for the second match?).
**FAULT 4: the pause card ends a joined match.** Esc (or `jam.pausemenu open`) on one machine pauses that machine's world
(`UGameplayStatics::SetGamePaused`), its lockstep stops, the other machine calls "connection lost" 3.1 s later and both matches end. Where:
`DirectorsCallController.cpp:75-77` (the controller arms for `ALobbyGameMode` too) and `:214-219` (Pause -> SetGamePaused); `ALobbyGameMode`
(`LobbyGameMode.cpp:76-78`) does not tick while paused. **The Director's ruling: in a joined match the pause card opens WITHOUT pausing the world**
(the match goes on under the card, as in every online game; the card says so in one line: "The match goes on"; the paused player's skater gets
neutral input while the card is open: check what the lockstep sends for him: it must keep sending), and everything on the card still works
(CONTROLS, SETTINGS, QUIT: QUIT leaves the match with words on the other side: check). Solo play pauses as today.
Then, with what time is left before 17:30, in this order:
1. **Both repeats green**: the hardening lane's two-process driver (`tools/soak/lockstep/twoproc5.ps1` is in the hardening lane's tree
   `.claude/worktrees/s3b`: read it there and copy it into your tree; do not edit theirs): a slow guest (delay the guest's start by 8 s) plays to the
   final card; a pause of 10 s on the guest and then on the host: no "connection lost", hashes equal at the end.
2. **The same class of fault elsewhere** (read, do not guess): every place a counter or a timer is compared against a limit that changes with a flag
   (`bHeard`, `bStarted`, the rematch); every place the world can be paused or time dilated while a lockstep runs (the goal's slow motion? the
   settings screen? the window losing focus: does the game pause when unfocused in a packaged build?: that one would end every joined match the
   moment a player alt-tabs: test it headless as far as it can be, and say what needs a real window).
3. What the other side sees when a player quits, closes the window, or loses the network for 2 s and returns: words, not a frozen picture.

Write `NET READY` with the file list when both faults are closed, built, `Jam.Net.*` and the full `Jam` filter green, Shipping compiles: **wanted by
17:00**; `SOURCE OPEN` before any later source change. `-Multiprocess`; four processes at most; stop only processes whose ids you recorded. Time box
150 minutes.
