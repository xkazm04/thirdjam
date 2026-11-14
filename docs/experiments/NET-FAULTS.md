# Net faults 3 and 4 (lane A204, track N1, Sat 14 Nov)

Found by lane A198 (two processes on loopback, `status/A198.md`). Protocol stays 9; the wire format is untouched.

## FAULT 3: a joined match died at frame 0 when one machine's rink loaded more than 3 s after the other's
The lockstep driver counts stall ticks while it waits. Before the peer is heard the limit is ten times the stall limit (1,800 ticks, 30 s: the other
side may still be loading); once heard it is 180 (3 s). The ticks counted while unheard were kept when the first input packet arrived, so a wait of
more than 180 ticks met the heard limit at once and the waiting side called the connection lost exactly as the late side arrived.

Fix: `UJamLockstepDriver::Receive` (`Game/Source/JamNet/Private/JamLockstepDriver.cpp`): `if (!bHeard) StallTicks = 0;` before `bHeard = true;`.
Both machines run the same driver, so the mirror case (the guest waiting for a late host) is the same line. The lockstep rematch does not restart the
driver (frames go on, `bHeard` stays true, the stall count restarts every stepped frame); a new lobby match calls `Start()`, which resets everything.

Proof:
- `Jam.Net.LateFirstPacket` (Tests/JamNetTests.cpp, from A198's `Bar.Net.LateFirstPacket`): the waiting side waits 100, 170, 200, 300, 600 and 1,700
  ticks, host waiting and guest waiting; neither side is lost and both reach frame 599. Before: 200, 300 and 600 lost at frame 0 (A198's run of the
  same driver code). The limits still hold: a heard peer that goes silent is lost after 180 ticks (+ the 3 frames of input already in hand); a peer
  never heard is lost at 1,800.
- Two processes, the guest opening its rink 8 s after START (`jam.net.TestRinkDelayMs=8000`, a new test cvar, guest only): the host waited 459 stall
  ticks unheard and played on to the end, hashes equal (`tools/soak/lockstep/twoproc5.ps1 -Cvars jam.net.TestRinkDelayMs=8000 -ToEnd`).
- Words: a peer never heard is lost at 30 s; its card now says "<name> never reached the rink (30 seconds)", not "no word for 3 seconds".

## FAULT 4: the pause card ended a joined match
Esc (or `jam.pausemenu open`) paused this machine's world; `ALobbyGameMode` does not tick while paused, so its lockstep stopped and the other machine
called the connection lost 3.1 s later. The Director's ruling: in a joined match the card opens without pausing the world, says "The match goes on",
the paused seat sends neutral input, and everything on the card still works. Single player pauses as before.

Fix:
- `ALobbyGameMode::AllowPausing` returns false while the lockstep runs (every pause goes through the game mode: the card's `SetGamePaused`, the
  engine's focus-loss pause). The constructor sets `PrimaryActorTick.bTickEvenWhenPaused` so a pause by a path the game mode cannot refuse still pumps
  the lockstep; `Tick` returns early when paused outside a lockstep match (single player unchanged).
- `ALobbyGameMode::ReadLocalInput`: while this machine's pause card is open (and the match is not on its result card, where the rematch ask must still
  go out) the seat sends an empty `FJamInput` every frame; presses latched under the card are dropped.
- `UJamDirectorsCallController::Pause` (the only function touched in that file): when the game mode refused the pause, the HUD overlay's announcement
  line says "The match goes on" (no layout); the JAMCALL line logs `world_paused=0`.

Proof:
- `Jam.Net.PauseCardOnline` (Tests/JamNetGuestTests.cpp): the policies, the tick flag, and a spawned `ALobbyGameMode` that allows the pause single
  player, refuses it in lockstep and allows it again after.
- Two processes, the guest's card open 10 s, then the host's 10 s: no connection lost, hashes equal at f5400, neutral input on each side for the
  card's frames, 0 presses lost.
- QUIT on the card: the guest leaves the rink, says BYE, the host's card says the guest left (see status/A204.md).

## The same class elsewhere (read)
- `JamLobbySubsystem` timers count `LobbyTicks` per state and reset in `ResetSession`; the 60 s join limit is not switched by a flag. No other limit in
  `Game/Source/JamNet` changes with a flag.
- The goal's slow motion is `AActor::CustomTimeDilation` on the skater pawns only (`JamPawns.cpp:199-203, 419, 805`): the game mode's tick is not
  dilated; there is no global time dilation in the source.
- The settings screen applies volume and bark variables only (`FJamSettings::Apply`): nothing pauses or dilates.
- Window focus: `bPauseOnLossOfFocus=false` (engine `BaseEngine.ini:334`, not overridden in `Game/Config`) and `t.IdleWhenNotForeground` defaults to 0,
  so an unfocused packaged game keeps ticking; if anything did pause it, `AllowPausing` refuses it in a joined match. Needs a real window to confirm.
- `ALobbyGameMode::ShowLine` draws with `GEngine->AddOnScreenDebugMessage`, which is compiled out of Shipping and Test builds
  (`UnrealEngine.cpp:11842`): in a Shipping package the lobby's lines ("rematch asked: waiting for ...", "REMATCH: the puck drops", the refused live
  changes) are never shown. Reported, not fixed.
- A BYE that arrives before the peer was heard is ignored (`JamLockstepDriver.cpp` Receive: `bHeard && !bLost`): a guest that quits while its rink
  loads leaves the host waiting the full 30 s. Reported, not fixed.

## Evolution 2 (lane A204, 16:24-17:45)
- **The lobby's lines reach a real HUD text.** `GEngine->AddOnScreenDebugMessage` had one caller in `Game/Source`: `ALobbyGameMode::ShowLine`
  (`LobbyGameMode.cpp`), compiled out of Shipping and Test, so "rematch asked: waiting for <name>", "REMATCH: the puck drops" and every refused live
  change ("a live cut / a cut load / a restart / a level is off in an online match ...") were never shown in a Shipping package. `ShowLine` now goes
  through `ALobbyGameMode::RouteLineToHud` to the Director's Call overlay's announcement line (`UJamDirectorsCallHudWidget::ShowAnnouncement`, 360
  frames; the sim stands still on the result card, so a line said there stays). "The match goes on" already took that road. Test:
  `Jam.Net.LobbyLineOnHud` (the debug overlay switched off, the line is the HUD's text and visible). The acceptance still must show, in a joined match
  on the result card after R: the announcement pill reading "rematch asked: waiting for <name>" above the WIN / LOSE card.
- **End cards say the way back**: "<reason>   -   ESC, then QUIT: back to the menu"; the end card steps aside while the pause card is open.
- **A BYE before the peer is heard is honoured** (`UJamLockstepDriver::Receive`), and the game says BYE when it shuts down in any lobby state with a
  peer (`UJamLobbySubsystem::Deinitialize`) and the moment the window's close is requested (`SayByeNow`, `WatchWindowClose`). Test:
  `Jam.Net.ByeBeforeHeard` (lost one tick after the BYE, as "peer left", frame 0). Two processes, the guest quitting while its rink waits to load: the
  host's card "Bo left" 0.02 s after the guest's BYE (before: 30 s, "no word for 3 seconds").
- **The network cut and back**: `jam.net.testcut <ms>` (`FJamUdpLink::CutFor`). Test `Jam.Net.CutAndBack` (cuts of 1, 2, 2.5 s: nobody lost, equal
  hashes at f1200; 4 s: both lost 181 / 182 ticks after the cut). Two processes: 2 s, no connection lost, hashes equal; 4 s, both cards
  "connection lost" 3.03 / 3.07 s after the cut, then QUIT on each pause card loaded the menu on both.
