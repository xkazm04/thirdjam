# Multiplayer by invite: deterministic lockstep on UJamSim (lane A9, 2026-11-12, from H5)

Status line at the top is kept current; the sections below are the design the prototype proves or breaks.

**Verdict (11:45): HOLDS.** In-process: `Jam.Net.LockstepTwoPeers` passes (3600 frames at 10% loss, 60 of 60 checkpoints, final
hashes equal; plus a half-rate peer and D = 1). Two processes: host and guest `-game` windows on this machine through the lobby
(`jam.host` / `jam.join 127.0.0.1:7700`), a full match on the 90 x 45 cut (`hockey.regular` since R-0010) in lockstep, **9000 frames, 149 of 149 checkpoints agreeing,
identical hash at every 600-frame log line on both sides, guest won 3:0 at frame 3565, D = 3, 4.74 KB/s each way**, screenshots in
`devlog/shots/20261112-11*-A-a9-*`. Section 9 has the numbers, section 8 the lessons. The `JamNet` module, lobby screens, lockstep game mode, desync and
connection-lost cards and the three tests are in `Game/Source/JamNet/`.

How to run it: console `jam.lobby` (or lane A8's "Play with a friend"), or from the command line
`-ExecCmds="jam.host hockey.regular Ada auto"` on one window and `-ExecCmds="jam.join <code> Bo"` on the other; `jam.net.AutoShot 1`
saves proof screenshots; `-cut=` on both command lines must match (the lobby refuses a differing cartridge file anyway).

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
| Input encoding | Move x, y as int16 (/32767), flags byte (shoot, check, brake) = **5 bytes** | both sims consume the *quantised* value (the sender quantises its own input before use), so the inputs are bit-identical on both sides |
| Input packet | 39 B header (magic, version, type, first frame, sender frame, checkpoint frame + hash, count; int64 everywhere, no bit-packing yet) + 8 x 5 B = **79 B payload, 107 B on the wire (IPv4 + UDP)**, measured by `Jam.Net.PacketRoundTrip` | **4.7 KB/s per direction (38 kbit/s)**, measured 4738 B/s out, 4286 B/s in with 10% loss; both directions under 10 KB/s. Packing the header to 12 B would give 52 B payload; not worth it at this rate |
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
| `Tests/JamNetTests.cpp` | `Jam.Net.LockstepTwoPeers`, `Jam.Net.InviteCodeRoundTrip`, `Jam.Net.PacketRoundTrip` | two drivers, two sims, a loopback link with 2-frame delay and 10% loss, 3600 frames, hashes equal at every checkpoint |

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
