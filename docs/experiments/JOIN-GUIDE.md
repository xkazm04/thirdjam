# Playing a match between two machines (lanes A120, A129, A140; track Q1)

One page for the operator, in the order you will do it. Protocol of this build: **8**. Both machines on the **same network** (same router, Wi-Fi or
cable) and running **the same package**: a different build is refused at once with a message (section 5).

## 1. Before you start (both machines)
- Copy the **same package folder** (`Windows\` with `ThirdJam.exe`) to both machines. Play from that folder; do not move it afterwards (see the firewall).
- The cut you will play (`hockey.regular`, `modes/hockey.mode.shotgun`, ...) is part of the package: nothing to copy if both run the same package.
- Note each machine's network address: Start > `cmd` > `ipconfig`, the **IPv4 Address** of the adapter that is connected (on the A device today:
  `10.0.0.141`, *Ethernet 2*). The two machines' addresses must start the same (for example both `10.0.0.` or both `192.168.1.`).

## 2. The firewall prompt (once per machine, per package folder)
The first time `ThirdJam.exe` **from a given folder** hosts or joins, Windows shows **"Windows Security Alert: Windows Defender Firewall has blocked some
features of this app"**, naming *ThirdJam.exe* (publisher unknown) and its path. Tick **Private networks** (the home network is *Private* on the A device:
*Ethernet 2* is Private) and press **Allow access**. Windows remembers the folder: a package in a **new folder asks again** (the A device already holds one
allow rule per package folder made so far, about 45). If you pressed Cancel: Windows Security > Firewall & network protection > *Allow an app through
firewall* > *Change settings* > find ThirdJam (the right path) > tick **Private** > OK. Nothing else is needed on one network. Across the internet the host's
router would have to forward UDP 7700 to the host (not tested, not needed this weekend).

## 3. Host (either machine)
1. Main menu > **BRAWL WITH A FRIEND**. The screen **PLAY WITH A FRIEND** opens; the status line reads *Host a match and read your friend the code, or type
   theirs.*; the warning under it: *LAN only: your friend must be on the same network, unless your router forwards UDP port 7700 to this computer.*
2. Pick the cut with `<` `>` and press **HOST**. The status line: *Waiting for a friend... cut: modes/hockey.mode.shotgun*. Below it the **invite codes**, one per
   network address of this machine, as `1800-138F   (10.0.0.141)`. On the A device there are three: `10.0.0.141` (the LAN: read the guest this one),
   `172.25.208.1` (WSL's virtual network: never reachable from another machine) and `127.0.0.1` (two copies on one machine only). **Read the guest the code whose
   address starts like the guest's own address.** The game listens on UDP **7700** (7701 to 7715 if 7700 is taken by another copy; the code carries the port).
3. When the guest arrives: *Bo is here. Press Start.* and the names *Ada vs Bo*. Press **START**: *Starting...*, then both machines load the rink (*In the match.*).

## 4. Guest
1. Main menu > **BRAWL WITH A FRIEND**. Type the host's code in the box (*friend's code: ABCD-EFGH*; the dash and capitals do not matter), press **JOIN**.
2. *Calling the host...*, then within a second *Joined Ada. Waiting for the host to start...*. The rink opens when the host presses START; the puck drops about
   3.4 s after JOIN (measured over the LAN address on one machine: 3.37 to 3.56 s, most of it the rink loading).
3. You play team 1's forward (orange), a full human seat: sprint, saucer, brake, spin, check and **F** (the mode's ability and pickups), your own stamina bar,
   ability slot, HUD, barks, booth, team signals (keys 1 to 4) and camera; the bots plan with you as a human.

## 5. What the screens say
| You see | Means | Do |
|---|---|---|
| *"XYZ" is not an invite code (8 letters and digits, shown as ABCD-EFGH)* | a typo | type it again |
| *Calling the host...* for long, then *no answer from the host in 60 s (same network? port forwarded?)* | the code's address is not reachable, or the host's firewall said no | the host's code for **your** network (section 3); the host's firewall (section 2) |
| *the host runs a different build (protocol 8, you have 7): update both* (orange, at once) | the two packages differ | copy the same package to both |
| *your cut "X" differs from the host's; both need the same file* | the cut file differs | copy the host's package (or its `Content/Cartridges` file) |
| **YOU WIN** (the winner) / **REMATCH?** (the loser) | the match ended; each machine shows its own team's card | **R** on both: *rematch asked: waiting for Ada*, then *REMATCH: the puck drops* (a fresh match, lane A140); Esc leaves |
| **THE HOST LEFT** / **YOUR FRIEND LEFT**: *Ada left the match* | the other side pressed Esc, quit or closed the game | back to the menu, host again |
| **CONNECTION LOST**: *no word from Ada for 3 seconds* | the other machine went silent (crash, Wi-Fi gone, sleep) | back to the menu, host again |
| **DESYNC**: *the two games disagreed at frame N; this cut is over* | the two simulations stopped being identical (a bug: never seen) | note the frame and the cut, save both `Saved/Logs`, tell Q1 |
| *a live cut is off in an online match: both games must stay the same (change it before you host)* (orange) | the Designer panel, a Director's Call or `jam.livecut` tried to change one machine's game | nothing: the match goes on; change the cut before hosting |

## 6. How it was tested (one machine, two headless processes, the real lobby flow)
`tools/net/twoproc.ps1` starts a host and a guest (`-nullrhi`), each seat played by the good playtester using every verb, to the end of the match, both
ask for the rematch on the card and play 10 s of it. `-Lan` binds the host on every address on UDP 7700 (not loopback) and has the guest **type the host's
invite code** for the LAN address read from the host's screen; `-Exe <ThirdJam.exe>` runs a package instead of the editor binary. Every run prints each
machine's status lines and its press accounting (*read / applied / in flight / folded*: lost = read - applied - in flight).
```
powershell -NoProfile -ExecutionPolicy Bypass -File tools\net\twoproc.ps1 -Lan -Cut modes/hockey.mode.shotgun -Name lan -GuestPort 4741 -Player good -Verbs -ToEnd -Rematch -Frames 36000 -TimeoutSec 900
powershell -NoProfile -ExecutionPolicy Bypass -File tools\net\twoproc.ps1 -Cut modes/hockey.mode.shotgun -Name lossy -HostPort 4720 -GuestPort 4721 -Player good -Verbs -ToEnd -Rematch -Frames 36000 -TimeoutSec 900 -Cvars "jam.net.SimDelayMs=40,jam.net.SimLoss=0.05"
powershell -NoProfile -ExecutionPolicy Bypass -File tools\net\twoproc.ps1 -Exe <package>\Windows\ThirdJam\Binaries\Win64\ThirdJam.exe -Cut hockey.regular -Name pkg -HostPort 4720 -GuestPort 4721 -Player good -Verbs -ToEnd -Rematch -Frames 36000 -TimeoutSec 900
```
Results by run: `status/A120.md`, `status/A129.md`, `status/A140.md`. **Not tested:** two physical machines, a real firewall prompt (the editor binary is
already allowed on the A device, and the package runs bind loopback), Wi-Fi loss. The first real two-machine match is the operator's.

## 7. Results (lane A140, Fri 13 Nov, protocol 8)
Every run: two processes, the real lobby, the good playtester on both seats, the match played to its end, each machine's card for its own seat, the rematch
asked on the card by both and started on the same lockstep frame, 600 frames of the second match. Since lane A140 every rematch starts at the hash of a fresh
match (shotgun `d4a5a086d4cc5b1e`, regular `2709733ac85dd9be` in every run below; lane A129's rematch hashes all differed: the restart kept history).

| Run | Ended (frame, score host:guest) | Rematch (lockstep frame / hash) | f600 of match 2 | Presses lost (host / guest) |
|---|---|---|---|---|
| **LAN** shotgun: host on UDP 7700, guest typed `1800-138F` (10.0.0.141) | f8610, 1:3 | 8734 / `d4a5a086d4cc5b1e` | `e9bb4b1d38505d7e` equal | (build before the queue) |
| **LAN** regular, final build | f5573, 3:0 | 5697 / `2709733ac85dd9be` | `52a85ef47d232c8b` equal | 0 / 0 |
| **package** shotgun 1, 2, 3 | f8549 3:2, f4936 3:2, f10354 3:2 | `d4a5a086d4cc5b1e` | `22387dafc893d8bc`, `e0f8bc8429f7d8fa`, `a41a4b24fd5f4fda` equal | 0 / 0 in all |
| **package** regular 1, 2, 3 | f6751 0:3, f11412 3:2, f12147 0:3 | `2709733ac85dd9be` | `52a85ef47d232c8b`, `1824c25bd5c306ff`, `2fabe3559bc7de45` equal | 0 / 0 in all |
| **package** refuse (guest on protocol 7) | refused in 0.012 s | | | *the host runs a different build (protocol 8, you have 7): update both* |
| **package** host leaves at f1200 | guest card **THE HOST LEFT** 0.02 s after the host went | | | |
| 40 ms + 5 % loss, shotgun 1, 2, 3 (final build) | f9484 2:3, f8661 3:2, f4251 3:1 | `d4a5a086d4cc5b1e` | `0abac334437d6d3f`, `c66c7e31785036e6`, `da5ce91f5c823016` equal | 0 / 0 in all (F 20/20 + 16/16, 23/23 + 20/20, 12/12 + 9/9) |
| soak (in process) | clean hockey.regular 600 matches; shotgun 10 % loss + jitter + duplicates x50, jitter x50 | | 0 desync | 0 of every edge (F 932 and 978 on the bad links) |

Status lines seen on the LAN run (as on the screen): host *Waiting for a friend... cut: hockey.regular* > *Bo is here. Press Start.* > *Starting...* >
*In the match.* > *Match over: 3:0, agreed on 92 checkpoints* > *In the match.* (the rematch); guest *Calling the host...* > *Joined Ada. Waiting for the
host to start...* > *In the match.* > the same. No firewall prompt appeared: the editor binary used for the LAN run is already allowed on the A device.

**The package against the editor binary:** the same lines, cards, hashes per checkpoint and status lines; two differences: the first frame comes
0.6 to 0.9 s after the join (editor binary 2.5 to 3.7 s: the rink loads from the pak) and the wait before it is 2 ticks (editor binary 92 to 110).

**Presses (lane A140).** Each machine counts its own edge presses (shoot, check, pass, poke, lift, deke, spin, F) as read from its controls, as applied to
its seat, still in flight at the end, and folded. Protocol 8 OR'd every press read during a stall into one frame: on 40 ms + 5 % loss the scripted seats lost
16 of 32, 13 of 29, 21 of 50 and 26 of 47 F presses that way, and the in-process soak could not see it (it expected the OR). Now a stall's presses queue one
frame each (`jam.net.PressQueue`, 16; 1 gives back the fold), presses before the first frame are not read, and a held key's repeats are absorbed. No wire
change: protocol 8.
