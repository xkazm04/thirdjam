# Lane A129, track Q1 (determinism and lockstep), evolution 4 (device A, Opus)

You are lane A129 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/EVOLUTION.md`, and the track's three evolutions in your worktree: `status/A104.md`, `status/A110.md`, `status/A120.md`,
`docs/experiments/SOAK-LOCKSTEP-2.md`, `docs/experiments/JOIN-GUIDE.md`, `tools/net/twoproc.ps1`.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a104` (the track's: evolutions 1 to 3 are committed there as the baseline; 1 and 2 are on main, 3 is
landing; its editor is built). Your status file: `status/A129.md`. Ports 4720 and 4721 (the track's). Family as the track's: `Game/Source/JamNet/**`, the lockstep
hash and serialised state, the `Jam.Net.*` tests, `tools/soak/lockstep/**`, `tools/net/**`, plus the presentation sites named in item 2.

## Evolution 4: a joined match played to its end, and everything on the guest's screen is the guest's
The operator plays a joined match between two machines this weekend. Lane A120 got two real processes to agree on every hash, but no match reached its end (one
goal in 48 minutes: the scripted seats never shoot), and found sites that still speak to the host's seat on the guest's machine.
1. **A match played to its end in two processes**: the scripted human on each seat shoots on goal (reuse the playtester's "good" class as the scripted seat if
   the lobby can take it; otherwise teach `jam.net.ScriptVerbs` to shoot from the slot). Proof: 3 Shotgun and 3 regular matches reach the match-over line with
   equal final hashes, both processes show the win or lose card for their own seat, and a rematch from the card starts on both.
2. **The remaining presentation sites key on the local seat** (`GetLocalSeat()`): the HUD feed (`HUDFeed.cpp:33`), the signals (`SignalSystem.cpp:280`), the
   skate loop (`CommentarySystem.cpp:851-853`), `GetHumanSkater` (`HokejGameMode.cpp:371`), and the brain's `PlayerIndex` in the lobby (`LobbyGameMode.cpp:241`:
   the AI plans with the guest as a bot teammate: both seats are humans to the brain; this one is sim-side, so it goes through the hash and both peers).
   Proof: a guest-machine test per site; 600 soaked matches, 0 desyncs.
3. **A live cut is refused during an online match** (`AHokejGameMode::ApplyCartridgeLive`, the Director's trigger at `JamEffectSystem.cpp:68`, the Designer's
   live changes): one machine's sim must never change alone. Either the lobby refuses them with a line on screen, or they travel as lockstep input: choose the
   one you can prove tonight (the refusal) and name the other as a "next".
4. **An impaired link in two processes** (`jam.net.SimDelayMs=40`, `SimLoss=0.05`) on Shotgun with every verb: 3 runs with equal final hashes, the longest
   stall in seconds, no F press lost.
5. Update `JOIN-GUIDE.md` with what the cards say now.

Tell the Director in findings (not yours to fix): the bots' shot selection in your logs (273 shots, many from the corners, one goal).
The gate: the full `Jam` filter green; the protocol number bumps only if the wire changes (say so). `-Multiprocess` on every headless start. Builds share one lock
and can wait ten minutes or more: batch your edits into few builds. Time box 100 minutes; interim in the status file.
