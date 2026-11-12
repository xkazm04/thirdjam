# Multiplayer by invite: deterministic lockstep on UJamSim (lane A9, 2026-11-12, from H5; re-proven by lane A38 at H11.5)

Status line at the top is kept current; the sections below are the design the prototype proves or breaks. Section 10 is the H11.5 re-proof on the sim
that gained the faceoff phase, the stick verbs, fouls, effects, rule hooks, zones and difficulty profiles since H6.

**Verdict (H11.5, 19:30): STILL HOLDS, on protocol version 2.** In-process: `Jam.Net.LockstepTwoPeers` passes on `hockey.boost` (9000 frames on a 33 ms / 10 % link: 150 of 150
checkpoints, 3 faceoffs, 1 foul, 1 pickup taken, 12 pokes and 7 passes over the wire) and on `hockey.chaos` (9000 frames on an 83 ms one-way / 2 % link: 150 of 150, 2 rule-hook fires,
worst stall 8 ticks). Two headless processes on this machine (`tools/net/twoproc.ps1`, `-game -nullrhi`, real UDP on 127.0.0.1): `hockey.boost` **9000 frames, identical hash at
all 15 logged lines (f9000 `40fe9137abb5121a`), 149 of 149 checkpoints, worst stall 1 tick, invite code to first frame 1.4 s**; `hockey.chaos` on a **simulated 80 ms round trip
with 2 % loss: 9000 frames, 15 of 15 identical (f9000 `3a3370873a912534`), worst stall 7 ticks (117 ms), the match at two thirds speed** (D = 3 covers 33 ms one-way). The H6 wire
format carried only Move, shoot, check and brake: the stick, poke, lift, pass and the held shot never travelled (section 10.1). **99 B payload per packet, 5.9 KB/s each way.**

**Verdict (H6, 11:45): HOLDS.** In-process: `Jam.Net.LockstepTwoPeers` passes (3600 frames at 10% loss, 60 of 60 checkpoints, final
hashes equal; plus a half-rate peer and D = 1). Two processes: host and guest `-game` windows on this machine through the lobby
(`jam.host` / `jam.join 127.0.0.1:7700`), a full match on the 90 x 45 cut (`hockey.regular` since R-0010) in lockstep, **9000 frames, 149 of 149 checkpoints agreeing,
identical hash at every 600-frame log line on both sides, guest won 3:0 at frame 3565, D = 3, 4.74 KB/s each way**, screenshots in
`devlog/shots/20261112-11*-A-a9-*`. Section 9 has the numbers, section 8 the lessons. The `JamNet` module, lobby screens, lockstep game mode, desync and
connection-lost cards and the three tests are in `Game/Source/JamNet/`.

How to run it: console `jam.lobby` (or lane A8's "Play with a friend"), or from the command line
`-ExecCmds="jam.host hockey.regular Ada auto"` on one window and `-ExecCmds="jam.join <code> Bo"` on the other; `jam.net.AutoShot 1`
saves proof screenshots; `-cut=` on both command lines must match (the lobby refuses a differing cartridge file anyway). Unattended and headless since H11.5:
`powershell -File tools/net/twoproc.ps1 -Cut hockey.boost -Name boost` (two `UnrealEditor-Cmd -game -nullrhi` processes, `jam.net.QuitAtFrame=9000`, hash lines compared).
## 0. The one idea

Both machines run the same `UJamSim` (same cartridge, same seed, same frame count = same `state_hash`, the Determinism test
already guarantees it in one process). Only inputs travel: each peer sends its `FJamInput` for frame N, the sim advances
frame N on both machines only when both inputs for N are present. Bots are deterministic functions of sim state
(`UJamBrain::Think` reads the sim, never the clock, never `FMath::Rand`), so they run identically on both sides without
sending anything. No positions are replicated; the match is the single-player game with one more human in a bot's slot.

## 1. Numbers

| Quantity | Value | Why |
|---|---|---|
| Sim rate | 60 Hz fixed (`UJamSim::DT`) | the game already counts frames, never wall clock |
| Input delay D | **3 frames (50 ms)** | the local input entered at frame N is applied at N+3 on both sides; hides up to 50 ms one-way latency with no stall. LAN RTT is < 5 ms, so 2 would do; 3 leaves room for a Wi-Fi hiccup. A cartridge-independent constant in `JamLockstep.h`; the prototype logs stalls so the number can be lowered |
| Redundancy window K | **8 inputs per packet** | every packet carries the last 8 frames of local input. Peers can never be more than D frames apart (each waits for the other), so any single lost packet is covered by the next one with no retransmit protocol. K >= D + 5 |
| Send rate | one packet per engine tick (60/s), also while stalled | the stalled peer keeps re-sending the window the other side is missing |
| Input encoding | **Version 2 (H11.5):** Move x, y and Stick x, y as int16 (/32767), one flags byte (shoot, check, brake, shoot_held, pass, poke, lift) = **9 bytes**; version 1 was Move + 3 flags = 5 bytes | both sims consume the *quantised* value (the sender quantises its own input before use), so the inputs are bit-identical on both sides. `Jam.Net.InputCoverage` reflects over `FJamInput` and fails when a field is not on the wire |
| Input packet | **Version 2:** 27 B header (magic, version, type, first frame u32, sender frame u32, checkpoint frame i32 + hash u64, count) + 8 x 9 B = **99 B payload, 127 B on the wire (IPv4 + UDP)**, measured by `Jam.Net.PacketRoundTrip`. Version 1 was 39 B + 8 x 5 B = 79 B | **5.9 KB/s per direction (47 kbit/s)** at 60 packets/s, measured 5938 B/s out in both in-process runs and on the two-process loopback; both directions under 10 KB/s |
| Hash interval | **every 60 frames** | each packet also carries (frame, 64-bit `StateHash`) of the latest checkpoint; +12 B per packet, so no extra traffic. The receiver keeps its own last 4 checkpoints and compares when the remote one lands. Mismatch = desync card at once, match over; it is logged `JAMNET desync frame=... local=... remote=...` and never played on |
| Stall limit | 180 engine ticks (3 s) without a frame advancing | "connection lost" card. Ticks, not wall clock, since the sim is not advancing anyway |
| Lobby traffic | HELLO / WELCOME / START, < 200 B each, resent every 30 ticks until acknowledged | the lobby is 3 messages |
| Max latency before stalls | (D - 1) x 16.7 ms = 33 ms one-way | internet play beyond ~35 ms one-way starts stalling; D would become a lobby setting (4-6 for WAN) |

## 2. Transport decision

**Plain UDP socket pair through `ISocketSubsystem` (`FSocket`, modules `Sockets` + `Networking`).** Reasons:

- Lockstep wants "a datagram per tick, newest wins, loss covered by redundancy"; Unreal's `UNetDriver` gives reliable ordered
  RPCs, actor channels, a listen server with a `PlayerController` per client, and the whole join/travel machinery. All of it
  is there to replicate state, which this design does not do; using it for a 52-byte datagram means fighting its connection
  handshake, its map travel and its `NetMode` assumptions in a game mode that has no replicated actors.
- The socket is owned by a `UGameInstanceSubsystem` (`UJamLobbySubsystem`), so it survives the lobby -> `Rink.umap` transition
  without a seamless-travel story.
- Two `-game` processes on one machine (ports 7700/7701) are a full test of the protocol; `UNetDriver` would need a listen
  server and `open 127.0.0.1` plus a replicated input struct and the same lockstep logic on top anyway.
- Downside, honestly stated: no NAT traversal. Unreal's net driver has none either without EOS/Steam; both need either a
  forwarded port or a relay (section 6). So the downside is shared and the UDP version is the smaller one to prove in two hours.
- Not measured in the prototype: the `UNetDriver` variant. If the operator wants internet play through EOS later, the lockstep
  driver is transport-agnostic (`IJamLink`: `Send(bytes)`, `Poll(bytes)`), and an EOS P2P link slots in behind the same interface.

## 3. Invite code

**8 characters, Crockford base32** (digits + letters without I, L, O, U; typed case-insensitively), shown as `ABCD-EFGH`.
40 bits: 32-bit IPv4 of the host, 4-bit port index (port = 7700 + index, so a second host on the same machine gets 7701),
4-bit checksum (XOR of the other nibbles, so a typo is caught before a connect attempt).

The lobby screen says in plain words: **"LAN only: your friend must be on the same network, unless your router forwards UDP
port 77xx to this computer."** The host screen lists every non-loopback IPv4 the machine has (Ethernet, Wi-Fi, Tailscale,
WSL) as separate codes and marks the one on the default route; the guest types whichever reaches them. A machine on a
tailnet (device A and device B both run Tailscale) can therefore play over the internet with no router change: the code
simply encodes the 100.x address.

No account service, no EOS, no Steam (none is set up; the rules forbid extra software for judges).

## 4. Lobby flow

1. Menu (lane A8) -> **Play with a friend** -> `jam.lobby` (console command registered by `JamNet`; opens the lobby screen over the current map).
2. **Host**: pick a cut (from the shipped cartridges), press Host: the socket binds 7700 (+1 if taken), the code is shown, "waiting for a friend...".
3. **Guest**: type the code, press Join: HELLO {protocol version, name, cartridge hash of nothing yet}. Host answers WELCOME
   {host name, cut name, cartridge text FNV-64, seed, guest's skater index}. The guest loads the same cut by name and compares the
   FNV of its own cartridge file: a mismatch shows "your cuts differ (Regular on both sides?)" and refuses, since a different
   tunable would desync at frame 1. Both screens show both names and team colours (host = team 0 blue, guest = team 1 orange).
4. Host presses **Start**: START {seed, frame 0}. Both `OpenLevel(Rink, "game=/Script/JamNet.LobbyGameMode?cut=<name>")`.
5. `ALobbyGameMode` (subclass of `AHokejGameMode`, see section 5) builds the match as single player does, then hands the stepping to
   `UJamLockstepDriver`. The guest is possessed into team 1's first non-back skater; the host keeps the cartridge's `player`.
6. End: the normal win/lose cards (A5) with the desync and connection-lost cards added by `JamNet`.

Seam names for lane A8 (menu): one button "Play with a friend" that runs the console command **`jam.lobby`**; nothing else.
(`JamGame` must not depend on `JamNet`; the console command is the whole seam. Grepped `Game/Source` for `jam.lobby`, `jam.host`,
`jam.join`: no collision; R-0003 point 4.)

## 5. Where the code goes and what it touches

`Game/Source/JamNet/` (Runtime module, depends on `JamCore`, `JamGame`, `Sockets`, `Networking`, `UMG`; nothing depends on it):

| File | Class | Role |
|---|---|---|
| `JamNetPackets.h/.cpp` | `FJamNetInputPacket`, `FJamNetLobbyMessage`, `FJamInviteCode` | byte layout, quantisation, code encode/decode |
| `JamLink.h/.cpp` | `IJamLink`, `FJamUdpLink`, `FJamLoopbackLink` | the socket, and an in-process link with simulated delay and loss for the test |
| `JamLockstepDriver.h/.cpp` | `UJamLockstepDriver` | the lockstep state machine: local input ring, remote input ring, `TryStep`, hash checkpoints, stall count |
| `JamLobbySubsystem.h/.cpp` | `UJamLobbySubsystem` (GameInstance) | host/join/start, owns the link and the driver across the level change |
| `LobbyGameMode.h/.cpp` | `ALobbyGameMode : AHokejGameMode` | the match side: steps through the driver, writes the two humans' inputs, shows the desync card |
| `LobbyWidget.h/.cpp` | `ULobbyWidget` | host / join / waiting / ready screens (C++-drawn UMG, A5's style) |
| `Tests/JamNetTests.cpp` | `Jam.Net.LockstepTwoPeers`, `Jam.Net.InviteCodeRoundTrip`, `Jam.Net.PacketRoundTrip`, `Jam.Net.InputCoverage` (H11.5) | two drivers, two sims with rule / zone / effect systems and brains, a loopback link with delay and loss, 9000 frames on boost and chaos, hashes equal at every checkpoint; the coverage test reflects over `FJamInput` |
| `JamNetPackets.h/.cpp` (H11.5) | `FJamNetScriptedHuman` | the scripted human of the two-process proof and the test: poke at the drop, pass, charged shot, a body check into an opponent without the puck every 10 s |
| `tools/net/twoproc.ps1` (H11.5) | | two headless `-game -nullrhi` processes on 127.0.0.1, `jam.net.QuitAtFrame`, compares the JAMNET hash lines, PASS/FAIL + JSON line; `-Cvars 'jam.net.SimDelayMs=40,jam.net.SimLoss=0.02'` for a bad link |

What it needs from the game mode and does not have (seam request for lane A1, named in the report): `AHokejGameMode::StepOnce`,
`GatherInputs`, `DeliverEvents`, `PushPresentation`, `ReadHumanInput` are private and non-virtual, and `Skaters`, `Puck`, `Brain`
are private. The prototype therefore overrides `Tick`, calls `AGameModeBase::Tick` directly and re-implements the step loop in
`ALobbyGameMode` with the public API (`GetSim()`, `TActorIterator<AJamSkater>`, `UJamPresentation`, `UJamEventBus`, `UJamPlayerInput`).
About 80 lines are duplicated; the clean version is `protected virtual` on those five methods plus a `protected` accessor for
the brain and skaters, which lets `ALobbyGameMode` shrink to the lockstep gate and the second human.

Second seam, for the sim (lane A1): `UJamSim` has one `HumanIndex` and clamps `|Move|` to 1 for the human and 1.5 for bots; the
guest's skater is a bot slot and gets the bot clamp and the bot yield rules. A `SetHuman(int32 Index, bool bHuman)` on the sim (or a
`humans` list in the cartridge) makes the guest a real human; until then the guest is slightly faster than the host (fine for a proof,
not for the final game).

## 6. Risks

| Risk | Where it bites | Mitigation / what it would take |
|---|---|---|
| **NAT**: UDP to a home router drops | any two homes | v1 says "LAN or forwarded port" in words. A relay would be: one tiny UDP forwarder (about 150 lines of Node, or a Cloudflare/Fly VM) that pairs two peers by the invite code and forwards datagrams both ways; the code then encodes a relay-room id instead of an address, +1 RTT to the relay. The lockstep driver does not change. Judges would still need no extra software. STUN/hole-punching is the next step after a relay and is a day of work and testing |
| **Desync from float differences between machines** | the two devices | both A and the packaged game are x64 built by the same compiler with the same flags: bit-identical. Device B runs the same x64 binary under ARM64 emulation: IEEE arithmetic is exact under emulation, and the CRT's `sin`/`cos` are the same x64 code, so the result should match; the emulator's handling of FMA contraction and x87-free SSE paths is the unknown. **A cross-device match with the hash check on is the real test**; the hash check turns a silent divergence into a card within one second, so the failure is loud, never a ghost match |
| Non-deterministic input to the sim | game mode | the only inputs are the two quantised `FJamInput` streams and the cartridge; the brain reads only the sim. The in-process `LockstepTwoPeers` test runs the brain on both sides |
| Different cartridge on the two sides | "My Director's Cut" | WELCOME carries the cartridge text FNV-64; mismatch refuses to start. Sending the cartridge text itself (a few KB, chunked) is the v2 fix |
| Frame-rate difference (one machine at 30 fps) | the slow peer | the fast peer stalls at most D frames ahead; the match runs at the slower machine's rate. The accumulator caps at 4 steps per tick as today |
| A peer quits / crashes | the other | 180 ticks without progress = "connection lost" card; back to the menu |
| Two processes on one machine | prototype only | ports 7700 and 7701, loopback 127.0.0.1; the invite code encodes 127.0.0.1 when the host picks it |
| Time | the jam | see the cut line |

## 7. Cut line

This is cut before anything else (`docs/PLAN.md` 9 lists it after the numbered cuts, since it was never in the plan). Order inside it:

1. If `LockstepTwoPeers` (in-process) does not hold at the two-hour mark: stop, no module ships, this file is the record.
2. If in-process holds but two real processes do not: ship the test and the driver (useful for replays later), no menu entry.
3. If both hold: ship LAN-only with the invite code and the plain-words warning; the relay is a note here, not code.
4. Never: an account service, a dependency judges must install, state replication of positions.

## 8. Working notes (what the two hours taught, for whoever picks this up)

- **The shared `Binaries/` cannot be relinked while any editor on this checkout runs** (five editors held the DLLs at 11:25; R-0009 link
  windows at :00 and :30). The proof above was built and run from a throwaway copy of `Source/Config/Content/Plugins/.uproject` at
  `%LOCALAPPDATA%\Temp\a9\Game` (short path so the DDC path stays under 119 characters; full editor build 120 s, incremental 6-12 s),
  deleted at the end of the lane. The shared tree got the same sources and its own build at a link window.
- **A new module must land with its binaries**: the `.uproject` entry added at 11:08 before a successful link made an editor started
  at 11:01-11:22 by another lane fail on the missing `UnrealEditor-JamNet.dll` (R-0009). Add the entry and build in one go, at a link window.
- `-game` windows (not editors) are enough for the two-client proof: 1.5 GB each, no MCP, `-ExecCmds` drives the lobby, `jam.net.AutoShot`
  takes the screenshots; a match runs fine with two foreign editors and a commandlet on the same CPU.
- The first in-process run counted each checkpoint once per packet (3615 "agreements" for 60 checkpoints) because every packet repeats
  the latest checkpoint; `LastCheckedHashFrame` fixed it. Desync detection was never affected (the first comparison is the one that matters).
- The guest's skater is a bot slot in `UJamSim` (bot speed clamp 1.5, bot yield rules); it works, it is not fair. The sim needs `SetHuman`
  (section 5, second seam) before this ships as a feature.
- `R` on the host would restart only the host's sim: `ALobbyGameMode` unbinds the base game mode's restart. A coordinated restart
  (both sides `Restart()` at an agreed frame) is a 20-line addition to the lobby protocol, not done.
- Director's Call in a lockstep match changes tunables on one side only (`ApplyCartridgeLive`); it must travel as a lobby message with the frame it
  applies at. Not done; the pause key is A8's and the call UI does not know about the lobby yet.

## 9. Measurements (filled by the prototype)

| Test | Frames | Hash checkpoints agreeing | D | Bandwidth | Notes |
|---|---|---|---|---|---|
| `Jam.Net.LockstepTwoPeers` "lossy" (in-process, loopback link, 2-tick delay, 10% loss = 753 of 8128 packets dropped, both at 60 Hz) | 3600 both | 60 of 60, final `state_hash` equal, no desync | 3 | 4738 B/s out, 4286 B/s in (79 B payload) | 4064 ticks for 3600 frames: 459 stall ticks, longest stall 4 ticks; peers never more than 3 frames apart; first run 11:30 counted each checkpoint once per packet (3615), the counter was fixed to compare each checkpoint once |
| `Jam.Net.LockstepTwoPeers` "slow-peer" (B ticks at 30 Hz, 1-tick delay, no loss) | 1800 both | 30 of 30, final hash equal | 3 | 4738 B/s each way | A stalled 1797 of 3604 ticks (it waits for B, as designed), longest 6 ticks; gap never above 3 |
| `Jam.Net.LockstepTwoPeers` "delay-1" (perfect link, D = 1) | 1200 both | 20 of 20, final hash equal | 1 | 4733 B/s each way | with D = 1 and a one-tick send cadence each frame needs 1.5 ticks: 1802 ticks for 1200 frames. D = 3 is the right default |
| Two `-game` windows on this machine (host 7700, guest an ephemeral port, loopback), lobby by `jam.host` / `jam.join`, scripted humans, the 90 x 45 cut (then named `hockey.arena.small`; `hockey.regular` since R-0010, and the tests now load `hockey.regular`), 11:36-11:39 | 9000 (host and guest both logged frames 600..9000) | 149 of 149; the hash at every 600-frame line identical on both sides (f600 `951c001f006de408` ... f9000 `80c7344b15cda0ba`); match over at frame 3565, team 1 (the guest) 3:0 | 3 | 4745 B/s out, 4754 B/s in (host); 4739 / 4735 (guest); 79 B packets at 60/s | longest stall host 10 ticks, guest 19 ticks (the guest waited for the host's level load at the start); gap never above 3 frames; the two processes shared one CPU with two foreign editors and a tournament commandlet |
| Cross-device (device A x64 vs device B ARM64 emulation) | not run | | | | the real desync test; needs both devices on the tailnet or one LAN and the same cartridge files; the hash card makes any divergence loud |

## 10. H11.5 re-proof on the new sim (lane A38, R-0028 point 3)

The sim gained the faceoff phase, the stick (aim, poke, lift), fouls with the penalty box, the charged shot and the pass, the effect system (modifiers, pickups),
rule hooks (variants), zones and obstacles, and difficulty profiles since the H6 proof. Re-read before anything else: `FJamInput` has nine fields now; the H6 wire format
carried four.

### 10.1 What was wrong and what changed

| Found at 19:00 | Fixed |
|---|---|
| `FJamNetQuantInput` carried Move + shoot/check/brake. `bShootHeld`, `bPass`, `Stick`, `bPoke`, `bLift` never travelled. Because both sides consume the quantised value, the verbs were **silently dead online on both sides** rather than a desync; a lane that bypassed the driver's returned input would have desynced at the first poke | **Protocol version 2**: every field on the wire (9 B per input), header on 32-bit frames, 99 B payload. Version-1 datagrams are refused by the version byte, so an old build and a new one never parse each other's layout |
| `ALobbyGameMode::ReadLocalInput` read Move, shoot edge and check only | Mirrors `AHokejGameMode::ReadHumanInput`: held shot, pass, brake, stick (right stick or mouse pixels at `stick.mouse_sensitivity`), poke, lift |
| `ALobbyGameMode::StepLockstep` copied the brain's Move/shoot/check/brake, built the brain by hand (no difficulty profile), never stepped `UJamEffectSystem` (no pickups), `UJamZoneSystem` or mirrored the score into `UJamRuleSystem` | Mirrors the H11 `StepOnce` / `GatherInputs` / `DeliverEvents`: `UJamBrain::ApplyCartridge`, pickups and slow-ice / no-shoot into the brain's world, rule-score mirror before the step, zone update and effect step after it, effect / zone / announce events to the bus, pickup actors synced, rink rebuilt on ShrinkRink, faceoff countdown on the HUD |
| The checkpoint hashed `UJamSim::StateHash` only; pickups on the ice and modifier timers live in `UJamEffectSystem` | Checkpoint = `sim_hash ^ effects_hash * 31` (the effects test's formula) on both sides |
| No way to prove a faceoff, a foul, a pickup or a rule hook happened in a lockstep match | `FJamNetScriptedHuman` (shared by `Jam.Net.LockstepTwoPeers` and `jam.ScriptHuman 1` in a lockstep match): pokes at the drop, passes, charges shots, and every 10 s runs a body check into an opponent without the puck (a foul under R-0013). The test asserts the counters per cartridge and that both peers counted the same |
| No gate for the next field a lane adds to `FJamInput` | `Jam.Net.InputCoverage`: iterates `FJamInput::StaticStruct()`'s properties and fails on any name `FJamNetQuantInput::CoveredFields()` does not list (and on a stale name); every flag round-trips alone |
| No unattended two-process run, no bad-link simulation on the socket | `jam.net.QuitAtFrame` (both processes quit on their own), `jam.net.SimDelayMs` / `jam.net.SimLoss` (held and dropped datagrams on the real UDP link, seeded), the "invite code to first frame" clock (`JAMNET first frame ... s after ...`), `tools/net/twoproc.ps1` |

Not changed, stated: the guest's skater is still a bot slot in the sim (bot speed clamp 1.5; A9 section 5's `SetHuman` seam is still open); the team signals (keys 1-4) are
not read in a lockstep match on either side (so they cannot desync; they also do nothing); a Director's Call still applies to one side only; `AHokejGameMode`'s step methods are
still private, so `ALobbyGameMode` is a second copy of the step loop (A9's seam request to lane A1 stands: `protected virtual` on `StepOnce`, `GatherInputs`, `DeliverEvents`,
`PushPresentation`, `ReadHumanInput` would delete about 150 lines here and the "copy it twice" failure mode of 10.3).

### 10.2 Measurements (H11.5)

| Test | Frames | Hash checkpoints agreeing | Link | Bandwidth | Worst stall | Inside the proof | Notes |
|---|---|---|---|---|---|---|---|
| `Jam.Net.LockstepTwoPeers` "boost-lossy-lan" (in-process, `hockey.boost`, 2-tick = 33 ms one-way, 10 % loss = 1963 of 20420 packets dropped, both at 60 Hz) | 9000 both | **150 of 150**, final hashes equal (sim and sim ^ effects) | D = 3 | 5938 B/s out, 5366 B/s in (99 B x 60/s) | 4 ticks (67 ms); 1206 stall ticks in 10210; gap never above 3 | 3 faceoff drops, 1 foul, 1 pickup taken, 12 pokes, 7 passes over the wire; both peers counted the same | 0.1 s wall for 9000 frames x 2 sims |
| `Jam.Net.LockstepTwoPeers` "chaos-wan-83ms-2pct" (in-process, `hockey.chaos`: 2 hooks from `variants` (sudden death, shrink per goal), a slow-ice circle, two bumpers; 5-tick = **83 ms one-way, 2 % loss** = 714 of 36224 dropped) | 9000 both | **150 of 150**, final hashes equal | D = 3 | 5939 B/s out, 5820 B/s in | **8 ticks (133 ms)** on B, 7 on A; 9105 stall ticks in 18112 (every frame waits about one tick at this latency: D = 3 covers 33 ms, not 83), gap never above 3 | 2 faceoff drops, 1 foul, **2 rule-hook fires**, 8 pokes, 6 passes | the match runs at half speed on such a link; D would be 6 for it (section 1) |
| `Jam.Net.LockstepTwoPeers` "regular-slow-peer" (B at 30 Hz) | 1800 both | 30 of 30 | D = 3 | 5937 B/s | 6 ticks | 1 faceoff | as at H6 |
| `Jam.Net.LockstepTwoPeers` "regular-delay-1" (perfect link, D = 1) | 1200 both | 20 of 20 | D = 1 | 5927 B/s | 1 tick | 1 faceoff | 1802 ticks for 1200 frames, as at H6 |
| **Two headless processes** on this machine (`tools/net/twoproc.ps1 -Cut hockey.boost`: `UnrealEditor-Cmd -game -nullrhi`, host on 7700 via `jam.host hockey.boost Ada auto`, guest via `jam.join 127.0.0.1:7700 Bo`, `jam.ScriptHuman 1`, `t.MaxFPS 60`, real loopback UDP), 19:23:04-19:25:56 | 9000 both (both quit on `jam.net.QuitAtFrame`) | **identical hash at all 15 logged 600-frame lines** (f600 `1a778638070ef32a` ... f9000 `40fe9137abb5121a`, sim hash f9000 `5783804d3bbc2c0f`); 149 of 149 exchanged checkpoints agreed on both sides, no desync | D = 3 | 5941 B/s out, 5951 B/s in (host); 5929 / 5919 (guest); 99 B x 60/s | host 1 tick; guest 35 ticks (583 ms, once, at the start: it waited for the host's `LoadMap`, 0.87 s); gap never above 3 | 2 faceoff drops, 1 foul, 1 pickup taken, identical on both sides | **invite code to first frame: 1.42 s** on the host (from the guest's HELLO), **1.41 s** on the guest (from `jam.join`): lobby round trip, START for 0.5 s, `LoadMap(Rink)` 0.87 s, the first exchanged input. 172 s wall for the whole run including two engine boots (about 10 s each) and the guest's 8 s head start. Score 0:0: the scripted humans and pro bots did not score in 150 s on the 120 x 60 boost rink |
| **Two headless processes, simulated bad link** (`tools/net/twoproc.ps1 -Cut hockey.chaos -Cvars 'jam.net.SimDelayMs=40,jam.net.SimLoss=0.02'`: every datagram held **40 ms each way = an 80 ms round trip**, **2 % dropped**, on the real UDP socket), 19:38:44-19:43:43 | 9000 both | **identical hash at all 15 logged 600-frame lines** (f600 `473b65a8efb4ea6d` ... f9000 `3a3370873a912534`, sim hash f9000 `8b7c24862de9df17`); 149 of 149 agreed on both sides, no desync | D = 3 | 5940 B/s out, 5811 B/s in (host); 5936 / 5813 (guest) | **host 7 ticks (117 ms)**, guest 19 ticks (once, at the start); 13523 ticks for 9000 frames = 1.5 ticks per frame: at 40 ms one-way (D = 3 covers 33 ms) every other frame waits a tick and the match runs at two thirds speed, smoothly, never stalling longer than 117 ms | 1 faceoff drop, 2 rule-hook fires (sudden death armed at frame 600, then the 0:1 goal at frame 1195 ended the match: the remaining 7800 frames are the frozen Over phase, hashed and agreed), no foul in this seed's 20 s of play | invite code to first frame 1.68 s (host) / 1.78 s (guest) over the 80 ms link. A first attempt at 19:26 found two exit bugs in the unattended proof, not in lockstep (10.1 last row): the host quit the moment it reached frame 9000 while the guest still needed its inputs for 8998-8999 (now 120 grace ticks of pumping), and a peer lost after the match was already over never exited (now it does) |

The 80 ms result in one line: **lockstep holds, play slows.** D = 3 (50 ms of cover) is right for LAN and tailnet (RTT under 30 ms); for an 80 ms round trip the lobby should pick D = 5
or 6 (section 1 said so at H6: "D would become a lobby setting (4-6 for WAN)"), and the WELCOME message has room for it. Not done in this lane.

### 10.3 The menu path: host, join by code, play 60 s, leave

The real menu is `UJamMenuWidget` ("Play with a friend" -> `OnFriendClicked` -> console `jam.lobby`) and `ULobbyWidget` (Host / Join / Start / Back buttons, each a one-line
call into `UJamLobbySubsystem::Host / Join / StartMatch / Leave`). There is no UMG click harness in the project and none was built in this lane's hour; the bot-driven run above
exercises exactly what the buttons call (`jam.host` = the Host button with the cut picker on `hockey.boost`; `jam.join <code>` = the code box + Join; `auto` = the Start button the moment
the guest is ready; `jam.net.QuitAtFrame` = closing the game). What it does not exercise: the widget layer itself (text box, cut picker, the status and warning lines) and `Leave`
mid-match. Manual steps for the operator, two game windows on this machine (or two machines on one LAN / tailnet), console with `~`:

| Step | Window A (host) | Window B (guest) | Expect |
|---|---|---|---|
| 1 | menu -> **Play with a friend** (or `jam.lobby`) | the same | the PLAY WITH A FRIEND screen over the menu; "LAN only..." warning |
| 2 | pick the cut with < > (Regular / Boost / Chaos ...), press **Host** (or `jam.host hockey.boost`) | | A shows one code per address, e.g. `FW00-0089 (127.0.0.1)` for this machine, `1800-138F (10.0.0.141)` for the LAN; "Waiting for a friend... cut: hockey.boost" |
| 3 | | type A's code (or `127.0.0.1:7700`) in the code box, press **Join** (or `jam.join FW00-0089`) | both screens show both names and team colours; A: "Bo is here. Press Start."; B: "Joined Ada. Waiting for the host to start..." |
| 4 | press **Start** (or `jam.start`) | | both load the rink within a second (START goes out for 0.5 s); the log line `JAMNET first frame N s after ...` on both; A drives blue's player, B drives orange's first skater |
| 5 | play 60 s: skate, stick (mouse), poke, lift, pass, charged shot, check | the same | no DESYNC card; `JAMNET frame frame=600 hash=...` at 10 s and `frame=3600` at 60 s identical in both logs (`Saved/Logs/ThirdJam.log`); the HUD shows the faceoff countdown after a goal or a foul |
| 6 | **Esc** (the pause card) -> back to the menu (`UJamMenuFlow::ReturnToMenu`), or `jam.leave` | | the other side shows the CONNECTION LOST card after 3 s (180 ticks without a frame), its match is over, back to the menu with Esc. Found and fixed in this lane: the Esc route never called `Leave()`, so the leaving side's UDP socket stayed bound (no BYE) until its next Host / Join; `ALobbyGameMode::EndPlay` now leaves the lobby whatever the way out (the lobby's Back button always did) |

A second host on the same machine binds 7701 (code with port index 1). A code typed with one wrong character is refused before any connect attempt (checksum). A guest whose
cartridge file differs is refused with "your cut differs from the host's" (FNV-64 of the file bytes in WELCOME).

### 10.4 What breaks lockstep when a future lane adds an input field (the checklist, proposed to the Director)

The one rule: **everything that drives `UJamSim::Step` on one machine must either travel in the input packet or be derived from state both machines already share.** The
specific traps, in the order a lane meets them:

1. **A new `FJamInput` field that is not in `FJamNetQuantInput`.** It never leaves the machine. Because the driver hands back the *quantised* input to both sides, the field is dead
   online (both peers see zero), not a desync; it becomes a desync the moment any code applies the raw local input instead of `TryStep`'s `OutLocal`. `Jam.Net.InputCoverage`
   fails on the unlisted field: add it to `FJamNetQuantInput` (`From`, `ToInput`, `CoveredFields`, `WireBytes`) and **bump `JamNet::Version`** (an old build and a new one then refuse
   each other instead of parsing the other's layout as garbage).
2. **A quantisation the sim does not share.** The sender must consume what it sent (`TryStep` already does); a new field with a lossy encoding must round-trip exactly
   (`From(ToInput(q)) == q`, tested) and the sim must clamp the same range (|Move|, |Stick| <= 1 today).
3. **A new `FJamAIInput` field for the bots.** It must be copied into `FJamInput` in three places until lane A1's seam lands: `AHokejGameMode::GatherInputs` (JamGame),
   `ALobbyGameMode::GatherBotInputs` (JamNet) and the test's `FPeer::Step`. Missing one is not a desync (both peers run `ALobbyGameMode`) but a different game online.
4. **A new system stepped by the game mode outside `UJamSim::Step`** (the effect system and the zone system are the two today): `ALobbyGameMode::StepLockstep` must step it at the
   same point in the frame, and if its state can change play it belongs in `LockstepHash` (`sim ^ effects * 31` today) so a divergence shows as a desync card, not a ghost match.
5. **Anything read from local-only state**: the team signals (`UJamSignalSystem`), a Director's Call (`ApplyCartridgeLive`), a camera yaw used for anything but the already
   quantised Move, wall-clock time, `FMath::Rand`. Either it travels as a lobby message with the frame it applies at, or it stays out of the lockstep path (the signals are out today).

Proposed comment line for `FJamInput` in `Game/Source/JamCore/Public/JamSim.h` (JamCore is not this lane's; for the Director):

```
// LOCKSTEP (JamNet): every field here travels in FJamNetQuantInput (Game/Source/JamNet/Public/JamNetPackets.h). Adding a field = add it there
// (From, ToInput, CoveredFields, WireBytes) and bump JamNet::Version; Jam.Net.InputCoverage fails until you do. docs/experiments/MULTIPLAYER.md 10.4.
```

Proposed line for `contracts/events.md` (or wherever the Director keeps the "adding a verb" checklist): "A new verb that reaches `FJamInput` also reaches `FJamNetQuantInput`
(JamNet, protocol version bump) and `ALobbyGameMode::GatherBotInputs` when the brain produces it; `Jam.Net.InputCoverage` is the gate."
