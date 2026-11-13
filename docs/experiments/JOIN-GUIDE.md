# Playing a match between two machines (lane A120, track Q1)

One page for the operator. Both machines on the **same network** (same router, Wi-Fi or cable). Both run **the same build**: a different build is refused
at once with a message (below). Protocol of this build: **8**.

## 1. Before you start
- Both machines have the same package (the same `ThirdJam.exe` folder). The cut you pick (for example `hockey.regular`, `modes/hockey.mode.shotgun`) must be the
  same file on both: the guest's game checks it.
- The game talks over **UDP port 7700** on the host (7701 to 7715 if 7700 is taken by another copy). The guest uses any free port.
- **The firewall prompt.** The first time a machine hosts or joins, Windows may ask "Allow ThirdJam to communicate on these networks?". Tick **Private networks**
  and press **Allow**. If you pressed Cancel: Windows Security > Firewall > Allow an app > find ThirdJam (or `UnrealGame`) > tick Private. Nothing else is needed on
  one network; across the internet the host's router must forward UDP 7700 to the host (not tested, not needed this weekend).

## 2. Host (machine A or B, either)
1. Main menu > **BRAWL WITH A FRIEND**. The screen **PLAY WITH A FRIEND** opens.
2. Pick the cut with `<` `>` and press **HOST**. The status line reads *Waiting for a friend... cut: hockey.regular*; below it the **invite codes**, one per
   network address of this machine, as `ABCD-EFGH   (192.168.1.23)`. Read the guest the code whose address starts like the guest's own (usually `192.168.`).
   The last code (127.0.0.1) is for two copies on one machine only.
3. When the guest arrives: *Bo is here. Press Start.* and the names *Ada vs Bo*. Press **START**. Both machines load the rink.

## 3. Guest
1. Main menu > **BRAWL WITH A FRIEND**. Type the host's code in the box (*friend's code: ABCD-EFGH*), press **JOIN**.
2. *Calling the host...*, then *Joined Ada. Waiting for the host to start...*. The rink opens when the host presses START.
3. You play team 1's forward (orange), a full human seat: your own sprint, saucer, brake, spin, check and **F** (the mode's ability and pickups), your own
   stamina bar and ability slot. Barks and the booth speak to **you** on your machine (lane A120).

## 4. What the cards and messages mean
| You see | Means | Do |
|---|---|---|
| *the host runs a different build (protocol 8, you have 7): update both* (orange, at once) | the two packages differ | copy the same package to both |
| *your cut "X" differs from the host's; both need the same file* | the cut file differs | copy the host's `Content/Cartridges` file |
| *no answer from the host in 60 s (same network? port forwarded?)* | the code's address is not reachable, or the firewall said no | another code from the host's list; the firewall (section 1) |
| **THE HOST LEFT** / **YOUR FRIEND LEFT**: *Ada left the match* | the other side pressed Esc, quit or closed the game | back to the menu, host again |
| **CONNECTION LOST**: *no word from Ada for 3 seconds* | the other machine went silent (crash, Wi-Fi gone, sleep) | back to the menu, host again |
| **DESYNC**: *the two games disagreed at frame N; this cut is over* | the two simulations stopped being identical (a bug: never seen in 1,000 soak matches) | note the frame and the cut, save both logs (`Saved/Logs`), tell Q1 |
| the normal win / lose card | the match ended | the lobby keeps the link; leave with Esc |

## 5. Tested how (two processes on one machine, headless, the real lobby flow)
`tools/net/twoproc.ps1` starts a host and a guest (`UnrealEditor-Cmd -game -nullrhi -Multiprocess`, `jam.host` / `jam.join`, sockets on 127.0.0.1 ports
4720 / 4721), a scripted human on each seat using every verb, to the end of the match:
```
powershell -NoProfile -ExecutionPolicy Bypass -File tools\net\twoproc.ps1 -Cut modes/hockey.mode.shotgun -Name shotgun -HostPort 4720 -GuestPort 4721 -Verbs -ToEnd -Frames 21600
powershell -NoProfile -ExecutionPolicy Bypass -File tools\net\twoproc.ps1 -Mode refuse     -Cut hockey.regular -Name refuse -HostPort 4720 -GuestPort 4721
powershell -NoProfile -ExecutionPolicy Bypass -File tools\net\twoproc.ps1 -Mode hostleaves -Cut hockey.regular -Name leaves -HostPort 4720 -GuestPort 4721
powershell -NoProfile -ExecutionPolicy Bypass -File tools\net\twoproc.ps1 -Mode hostkilled -Cut hockey.regular -Name killed -HostPort 4720 -GuestPort 4721
```
Results (lane A120, Fri 13 Nov): see the table in `status/A120.md` and section 6. Not tested: two physical machines, a real firewall prompt (the proof binds
loopback only), Wi-Fi loss. The first real two-machine match is the operator's.

## 6. Results (lane A120, Fri 13 Nov, build of the a104 worktree, protocol 8)
| Run | What happened | Number |
|---|---|---|
| refuse | guest on protocol 7 (patched HELLO byte, `jam.net.TestVersion 7`) joins a protocol-8 host | refused in **0.023 s**; guest screen: *the host runs a different build (protocol 8, you have 7): update both* |
| hostleaves | host leaves at frame 1200 (BYE) | guest card **THE HOST LEFT** **0.02 s** after the host went (was: 3 s stall, then CONNECTION LOST) |
| hostkilled | host process stopped (no BYE) | guest card **CONNECTION LOST** **3.3 s** after the kill; the guest never freezes |
| shotgun 1 | scripted host and guest, every verb, 6 min cap | final hash `2333d1afa163f689` at frame 21600 equal on both, 360 checkpoints agreed, 0 desync; guest F used 1, saucer 1, sprint 10,710 frames; 0:0 (no goal in 6 min) |
| barks | regular run 1 | the faceoff taunt comes from the host's opponent on the host (skater 5) and from the guest's opponent on the guest (skater 0) |
| shotgun 2, 3 | same | final hash `430ebdf9152e1f68` equal on both in both runs, 0 desync, guest F used 1 each, 0:0 |
| regular 1, 2, 3 | scripted host and guest, every verb, 6 min cap, 5 v 5 | final hashes `a9d206bc8b2f2d9e`, `4ef01740458b4020`, `7dbcf6d4d1deb77f` equal on both, 0 desync; scores 0:0, 0:1, 0:0 |
| sudden death 1 | first to 1, 6 min cap | final hash `3172e998b6dfb046` equal on both, 0 desync, 0:0 |
| soak | in-process, 1,000 clean regular matches, whole-array field compare | 0 desync, 0 silent drift, 0 lost |

**Not proven:** a match played *to its end* in two processes. In 48 minutes of scripted two-process play (eight 6-minute runs) the only goal was one in regular 2:
the bots shoot from the corners and the scripted human never shoots. The lockstep held for all of it (equal hashes at every 600-frame line and at the end); the
end-of-match path (win card, both quit) is the operator's real match, or the next Q1 lane with a scripted human that scores.
