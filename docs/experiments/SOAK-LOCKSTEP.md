# The lockstep soak (lane A56, 2026-11-13, protocol 5)

**Verdict (11:20): the protocol holds.** 7 260 joined matches (3600 frames; 600 of them 9000 frames) on `hockey.regular` at pro (a regular playtester hosting, a good one as the
guest), each frame of both peers compared bit for bit: **0 desyncs on a clean link and 0 under every fault profile** (2 / 10 / 30 % loss, 0..6 ticks of
jitter with reordering, 20 % duplicates, the three combined, a 2 s blackout, a guest machine at 30 Hz, real UDP sockets on loopback). Faults only slow the match down; nothing is ever skipped or applied twice.
**One bug proven and fixed:** a spin pressed while the driver waited for the peer never reached either sim (`UJamLockstepDriver::PendingFlags` was a `uint8`,
the spin is flag 512): up to 1 spin in 10 lost on a jittery link. **One premise of the brief is wrong:** a protocol-4 peer is not refused with a reason;
it is ignored like noise (section 4). The lockstep step itself costs **1.9 us (Pump) + 0.13 us (TryStep) per tick**, p99 3.7 + 2.3 us, on the ticked
link; over real loopback sockets Pump is 19 us mean, p99 80 us (the two system calls). Either is under 0.5 % of a 16.7 ms frame; the sim frame beside it
(brain + `UJamSim::Step` + zones + effects) is 5.2 us mean.

Tables (regenerated every hour from the run files) are in section 6. Status with timestamps: `status/A56.md`.

## 1. How it runs

The brief allowed either an in-process harness or two packaged games. The packaged games have no seat for a playtester (`ALobbyGameMode` reads a pad or
`jam.ScriptHuman`), so that route needed new code in the lobby; the in-process route needed none in `JamNet`:

- `Game/Source/JamNet/Private/Tests/JamNetSoakTests.cpp`: `Bar.Net.SoakClean`, `Bar.Net.SoakLossy`, `Bar.Net.SoakMismatch` (the `Bar.` prefix keeps them out
  of the gate's `Jam` filter). Two peers, each with its own `UJamSim`, `UJamRuleSystem`, `UJamZoneSystem`, `UJamEffectSystem` (built in the game mode's
  order), a `UJamBrain` at `pro`, and a real `UJamLockstepDriver`; between them a fault-injecting `IJamLink` that carries the real packet bytes.
- Per tick, as `ALobbyGameMode::Tick` / `StepLockstep`: both drivers `Pump()`, then each peer reads its human's controls (the playtester, through
  `FJamAIInput` -> `FJamInput` as `FJamSimAdapter::Step` copies it), `TryStep`, and on success the brain plays the bots with `GatherBotInputs`' world and copy
  (pickups, slow ice, no-shoot zones, `bDeke`, `bShootHeld` false), the sim steps, zones and effects follow, the driver records the checkpoint hash.
- Host = the cartridge's `player` seat with `FJamPlaytester` regular; guest = team 1's first non-back skater (the lobby's rule) with `FJamPlaytester` good.
  Each playtester runs only on its own peer, as a person sits at one machine.
- Comparison: every frame both peers have stepped is compared field by field (every reflected `FJamSkaterState` and `FJamPuckState` property, phase, score,
  timer, RNG state, and the sim / effect / rule / zone state hashes). The first difference is the desync frame and field; both states are dumped to
  `Game/Saved/SoakLockstep/dumps/<profile>-seed<N>-f<frame>.txt`. The driver's own 60-frame checkpoint runs untouched beside it, so the table shows both
  the true first frame and the frame the game would have shown the DESYNC card.
- Input integrity: every edge press (shoot, check, pass, poke, lift, deke, spin) the local player made, merged over a stall as the driver promises, is
  checked against what the sim was handed for that frame. A press that never arrives is a lost input.
- Runner: `tools/soak/lockstep/run.ps1` (one headless `UnrealEditor-Cmd -nullrhi -unattended` process, seed range and profiles on the command line, one
  `thirdjam.soaklock/1` JSON line per match into a `.jsonl`), `tools/soak/lockstep/summarize.mjs` (the tables below; `--doc` rewrites section 6).
- Speed: 0.1-0.15 s per 3600-frame joined match (two sims and two brains); 1000 clean matches in about 2 minutes in one process.

Where the harness differs from a real game, honestly stated: one step attempt per tick (the game mode's accumulator allows up to 4 to catch up after a
stall; the protocol is the same, a catch-up only shortens the wait); the link is ticked, not wall-clocked (a delay of N ticks = N x 16.7 ms); the real UDP
socket is in the loop only in the `udp-loopback` profile (two sockets in one process; `tools/net/twoproc.ps1` covers two processes, MULTIPLAYER.md
section 10).

## 2. Clean link: no desync

`Bar.Net.SoakClean`, seeds 1..3000 (the brief asked for 200), 3600 frames each, 1 tick each way: **3000 of 3000 matches bit-identical at every frame on both
peers**, 60 of 60 checkpoints agreed in every match, both peers never more than 1 frame apart, one stalled attempt per peer per match (the very first tick,
before the first packet has crossed). Over the wire in those matches: 21 974 spins, 1659 dekes, 251 068 pokes, 280 698 lifts, 22 149 checks, 8670 shots,
3008 passes, every one of them applied on both sides. Full-length matches (9000 frames, 200 seeds) clean, at 30 % loss and at 10 % loss + jitter + dup:
0 desyncs, 150 of 150 checkpoints.

The same 200 seeds over the game's own `FJamUdpLink` (two real sockets on 127.0.0.1, `SendTo` / `RecvFrom`, the peer filter, the 512-byte scratch;
ticks paced at 250 us of wall clock): 200 of 200 bit-identical, the same edge counts as the ticked link, one stalled attempt per match.

## 3. Faults: what the protocol does on each

| fault | what the protocol does | recovers inside the window? |
|---|---|---|
| loss 2 / 10 / 30 % | waits: a lost datagram is covered by the next one (8 inputs per packet); the peer stalls only when the gap reaches the input delay. Match speed 0.999 / 0.982 / 0.877 frames per tick, worst stall 3 / 5 / 12 ticks | yes, always: the longest loss run seen (11 datagrams at 30 %) never outran the window, since a stalled peer keeps sending its window |
| jitter 0..6 ticks (packets reorder) | waits: inputs are keyed by frame, so order does not matter; a datagram that arrives after its frame was needed costs a stall. Speed 0.718: a one-way delay above D - 1 = 2 ticks stalls, as MULTIPLAYER.md section 1 predicts | yes |
| 20 % duplicates | nothing: the first copy of a frame's input wins, later copies are skipped (`!RemoteInputs.Contains(F)`), checkpoints compare once | yes |
| 30 % loss + jitter 0..6 + 10 % dup | waits, worst stall 14 ticks, speed 0.624 | yes |
| blackout 2 s (120 ticks) | both peers stall 119-120 ticks, then catch up in lockstep | yes (under the 180-tick stall limit) |
| blackout 4 s (240 ticks) | "connection lost" on both peers at 180 ticks, as designed | no, by design (the card) |
| guest at 30 Hz (a slow machine), clean or with 10 % loss + jitter 0..6 | the host waits for the guest: the match runs at the slow machine's rate (0.500 / 0.498 frames per tick), peers never more than 3 / 5 frames apart, worst stall 5 / 10 ticks | yes |

In no profile was an input dropped, applied on one side only, or applied twice, once the spin fix below was in.

The same profiles on `hockey.chaos` (rule hooks, zones, bumpers, a shrinking rink) and `hockey.boost` (pickups), 200 seeds each, clean, 30 % loss and
10 % loss + jitter + dup: 0 desyncs, 0 inputs lost. Their stall and datagram figures equal `hockey.regular`'s to the unit, as they must: each peer sends one
datagram per tick whatever happens on the ice, so the network side of lockstep does not depend on the game (the seeded link makes the same faults).

## 4. Mismatches: a protocol-5 host against a wrong peer

| case | what happens | the reason a player sees |
|---|---|---|
| protocol-4 guest joins (HELLO with version byte 4) | `FJamNetLobbyMessage::Read` refuses it, `UJamLobbySubsystem::TickLobby` `continue`s: nothing goes back | host: nothing, still "waiting for a friend". Guest: after 60 s, "no answer from the host in 60 s (same network? port forwarded?)": **the wrong reason** |
| protocol-4 inputs reach a protocol-5 match | every packet refused by `Read`; the peer is never "heard", so the not-yet-heard limit (10 x 180 ticks) applies | "CONNECTION LOST: no word from <name> for 3 seconds", after 30 s |
| unknown flag bit (1024, 0x8000) on a protocol-5 packet | accepted; `ToInput` drops the bit; no verb fires; no desync while the sender does not apply the bit itself | none |
| short packets (0, 5, 6, 26 bytes; count 8 with no inputs; one byte short; count 255 with 8 inputs) | refused by `Read`, skipped like noise | none |
| long packets (1 or 100 trailing bytes; count 255 with the full 2577 B payload, over `MaxDatagram` 512) | accepted; trailing bytes ignored | none |
| 20 000 fuzzed packets (random bytes changed, truncated, extended) | 12 724 read as input packets, 7 276 refused; a driver fed all of them did not crash | none |
| 1 % of the guest's datagrams with one flipped byte | applied as they are (no checksum in the packet): 18 of 20 matches diverged, 17 showed the DESYNC card at the next checkpoint, one healed (a 1-frame blade difference, unseen by the checkpoints) | DESYNC |

Nothing crashed in any case. The brief's premise "the host refuses with the right reason" does not hold: there is no refusal path in the protocol. The
version byte works as a filter, which keeps two builds from desyncing, but the person on the old build waits 60 s and is told to check the network.
**Proposal (not built: it is a lobby protocol change, the Director's call):** the host answers a HELLO whose magic matches and whose version does not with a
fixed-layout `REFUSE {my version}` datagram (magic, version byte = the host's, type 6), which every past and future build can read because it does not
depend on the version; the guest shows "the host runs a different build (protocol 5, you have 4): update both". About 20 lines in `JamNetPackets` and
`JamLobbySubsystem`.

## 5. Fixed, and found but not fixed

**Fixed (JamNet, proven by the soak):** `UJamLockstepDriver::PendingFlags` was `uint8`. `TryStep` keeps the edge presses read while it waits for the peer
(`PendingFlags |= Q.Flags & EdgeFlags`) and adds them to the next frame it records; since protocol 5 the spin is flag 512 and was cut off by the 8-bit
member. Measured before the fix (seeds 1..200 each): spins lost 1 of 1685 at 2 % loss, 10 / 1476 at 10 %, 49 / 1636 at 30 %, 137 / 1727 on a 0..6 tick
jitter, 176 / 1685 at 30 % + jitter + dup; every other verb 0 (they all fit in 8 bits). No desync: the press was lost on both sides alike. After: 0 lost in
every profile. The member is now `decltype(FJamNetQuantInput::Flags)`, so the next flag bit cannot be cut off again. `Jam.Net.*` after the fix: 4 of 4 pass.

**Found, not fixed (outside this lane or not proven by the soak):**
- `ALobbyGameMode::ReadLocalInput` does not read `bSprint` or `bSpin` (`AHokejGameMode::ReadHumanInput` does, since lanes A49 and A51). The wire carries both
  (protocol 4 and 5), but an online human's Shift / E never reach it: **in a joined match nobody can sprint or spin.** Two lines (`In.bSprint =
  Pi->IsSprintHeld(); In.bSpin = Pi->ConsumeSpinPressed();`). Not a desync (both sides see the same nothing), so not mine under the brief; for the JamNet owner.
- No refusal reason for a version mismatch (section 4).
- `UJamLockstepDriver::Receive`: `MaxFrameGap` takes `static_cast<int32>(abs(SenderFrame - Frame))`, so a garbage `SenderFrame` overflows the diagnostic
  (`gap_max=2147483647` in the corrupt profile). Diagnostics only.
- The lockstep hash (`Sim->StateHash() ^ Fx->StateHash() * 31`) does not include the rule and zone systems' own state hashes; the soak compares those
  separately and never saw them differ, so today a rule / zone divergence would show up only once it moves the sim.
- The packet has no checksum of its own. On a real wire the UDP checksum covers it; the corrupt-byte profile shows what an uncaught bad byte does (DESYNC card).
- The playtester has no sprint (`FJamAIInput` carries no `bSprint`), so the soak never pressed sprint; the flag is a held level and is never kept over a stall.

## 6. Tables

<!-- soak:tables -->

Generated 2026-11-13 09:44 UTC by tools/soak/lockstep/summarize.mjs from 14 files.

Matches: **7260**.

### Loss table (per fault profile)

| profile | matches | desync | lost | unfinished | speed (frames per tick) | stalled attempts | worst stall (ticks) | max peer gap | longest loss run | datagrams lost / dup / reordered | edge presses lost (lost/pressed) |
|---|---|---|---|---|---|---|---|---|---|---|---|
| clean | 3000 | 0 | 0 | 0 | 0.999 | 6000 | 1 | 1 | 0 | 0 / 0 / 0 | shoot 0/8670, check 0/22149, poke 0/251068, lift 0/280698, spin 0/21974, pass 0/3008, deke 0/1659 |
| loss2 | 200 | 0 | 0 | 0 | 0.999 | 1627 | 3 | 3 | 3 | 28809 / 0 / 0 | shoot 0/487, check 0/1475, pass 0/200, poke 0/17658, lift 0/19346, deke 0/100, spin 0/1685 |
| loss10 | 200 | 0 | 0 | 0 | 0.982 | 26061 | 5 | 3 | 5 | 145897 / 0 / 0 | shoot 0/582, check 0/1470, pass 0/189, poke 0/17193, lift 0/19523, deke 0/104, spin 0/1480 |
| loss30 | 200 | 0 | 0 | 0 | 0.877 | 201171 | 12 | 3 | 11 | 491477 / 0 / 0 | shoot 0/724, check 0/1543, pass 0/198, poke 0/17130, lift 0/19241, deke 0/128, spin 0/1610 |
| jitter0-6 | 200 | 0 | 0 | 0 | 0.718 | 564408 | 8 | 8 | 0 | 0 / 0 / 856537 | check 0/1517, poke 0/17044, lift 0/18315, deke 0/119, spin 0/1719, pass 0/191, shoot 0/600 |
| dup20 | 200 | 0 | 0 | 0 | 0.999 | 568 | 2 | 4 | 0 | 0 / 287943 / 217234 | shoot 0/474, check 0/1456, pass 0/186, poke 0/16528, lift 0/18520, deke 0/97, spin 0/1522 |
| loss10-jitter0-6-dup10 | 200 | 0 | 0 | 0 | 0.700 | 616959 | 10 | 7 | 6 | 205184 / 184807 / 849600 | check 0/1550, pass 0/227, poke 0/17536, lift 0/18746, deke 0/105, spin 0/1645, shoot 0/589 |
| loss30-jitter0-6-dup10 | 200 | 0 | 0 | 0 | 0.624 | 864577 | 14 | 7 | 13 | 691335 / 160814 / 637652 | shoot 0/586, check 0/1518, poke 0/17113, lift 0/18348, deke 0/117, spin 0/1732, pass 0/197 |
| blackout120 | 200 | 0 | 0 | 0 | 0.967 | 48000 | 119 | 1 | 120 | 48000 / 0 / 0 | check 0/1441, poke 0/16425, lift 0/18469, spin 0/1482, pass 0/200, shoot 0/519, deke 0/107 |
| blackout240 | 200 | 0 | 200 | 0 | 0.869 | 72800 | 180 | 1 | 183 | 73200 / 0 / 0 | check 0/589, poke 0/6318, lift 0/6829, spin 0/543, pass 0/47, deke 0/23, shoot 0/118 |
| unknown-bit-1024-from-guest | 20 | 0 | 0 | 0 | 0.999 | 40 | 1 | 1 | 0 | 0 / 0 / 0 | shoot 0/56, check 0/160, pass 0/13, poke 0/1924, lift 0/2234, deke 0/12, spin 0/200 |
| protocol-4-guest | 20 | 0 | 20 | 0 | 0.000 | 7340 | 184 | 3 | 0 | 0 / 0 / 0 |  |
| corrupt-1pct-from-guest | 20 | 18 | 0 | 0 | 0.999 | 72 | 1 | 2147483647 | 0 | 0 / 0 / 0 | shoot 0/20, check 0/94, pass 0/10, poke 0/1108, lift 0/1225, deke 0/3, spin 0/100 |
| clean@hockey.boost | 200 | 0 | 0 | 0 | 0.999 | 400 | 1 | 1 | 0 | 0 / 0 / 0 | check 0/1168, pass 0/146, poke 0/15047, lift 0/18546, spin 0/1276, deke 0/97, shoot 0/479 |
| loss10-jitter0-6-dup10@hockey.boost | 200 | 0 | 0 | 0 | 0.700 | 616959 | 10 | 7 | 6 | 205184 / 184807 / 849600 | check 0/1155, poke 0/14472, lift 0/16301, spin 0/981, pass 0/100, deke 0/113, shoot 0/535 |
| loss30@hockey.boost | 200 | 0 | 0 | 0 | 0.877 | 201171 | 12 | 3 | 11 | 491477 / 0 / 0 | check 0/1143, poke 0/15526, lift 0/18515, spin 0/1039, pass 0/112, shoot 0/436, deke 0/99 |
| clean@hockey.chaos | 200 | 0 | 0 | 0 | 0.999 | 400 | 1 | 1 | 0 | 0 / 0 / 0 | shoot 0/393, check 0/798, pass 0/142, poke 0/10503, lift 0/11692, deke 0/78, spin 0/1023 |
| loss10-jitter0-6-dup10@hockey.chaos | 200 | 0 | 0 | 0 | 0.700 | 616959 | 10 | 7 | 6 | 205184 / 184807 / 849600 | check 0/863, pass 0/170, poke 0/10532, lift 0/11750, spin 0/1180, shoot 0/368, deke 0/86 |
| loss30@hockey.chaos | 200 | 0 | 0 | 0 | 0.877 | 201171 | 12 | 3 | 11 | 491477 / 0 / 0 | shoot 0/358, check 0/836, poke 0/10843, lift 0/11811, deke 0/76, spin 0/946, pass 0/137 |
| slow30 | 200 | 0 | 0 | 0 | 0.500 | 719600 | 5 | 3 | 0 | 0 / 0 / 0 | check 0/1606, poke 0/15857, lift 0/16569, pass 0/192, spin 0/1407, shoot 0/651, deke 0/134 |
| slow30-loss10-jitter0-6 | 200 | 0 | 0 | 0 | 0.498 | 726452 | 10 | 5 | 6 | 288295 / 0 / 1048314 | shoot 0/739, check 0/1583, pass 0/192, poke 0/17212, lift 0/17715, deke 0/152, spin 0/1602 |
| udp-loopback | 200 | 0 | 0 | 0 | 1.000 | 200 | 1 | 1 | 0 | 0 / 0 / 0 | shoot 0/474, check 0/1456, pass 0/186, poke 0/16528, lift 0/18520, deke 0/97, spin 0/1522 |
| clean/9000f | 200 | 0 | 0 | 0 | 1.000 | 400 | 1 | 1 | 0 | 0 / 0 / 0 | shoot 0/889, check 0/2476, pass 0/425, poke 0/29562, lift 0/33711, deke 0/188, spin 0/2837 |
| loss10-jitter0-6-dup10/9000f | 200 | 0 | 0 | 0 | 0.700 | 1542627 | 10 | 8 | 6 | 514413 / 462257 / 2122562 | check 0/2608, pass 0/394, poke 0/30785, lift 0/33331, deke 0/209, spin 0/3121, shoot 0/1135 |
| loss30/9000f | 200 | 0 | 0 | 0 | 0.877 | 503799 | 12 | 3 | 12 | 1230628 / 0 / 0 | shoot 0/1204, check 0/2501, pass 0/396, poke 0/29901, lift 0/32923, deke 0/225, spin 0/2680 |

### Desync table

No desync in any honest profile: every compared frame of every match was bit-identical on both peers. The 18 rows below are the corrupt-byte profile, which flips bytes on purpose.

| profile | seed | first differing frame (bit-exact) | first differing field (host / guest) | lockstep hash differs from | driver checkpoint | dump |
|---|---|---|---|---|---|---|
| corrupt-1pct-from-guest | 1 | 774 | skater[3].BladePos (X=10.816996,Y=-10.374465) / (X=10.892685,Y=-10.370309) | 774 | -1 | corrupt-1pct-from-guest-seed1-f774.txt |
| corrupt-1pct-from-guest | 2 | 1403 | skater[3].BladePos (X=-16.761180,Y=2.917123) / (X=-16.304231,Y=3.106606) | 1403 | 2640 | corrupt-1pct-from-guest-seed2-f1403.txt |
| corrupt-1pct-from-guest | 3 | 910 | skater[3].Pos (X=-37.378317,Y=15.678870) / (X=-37.376548,Y=15.677917) | 910 | 960 | corrupt-1pct-from-guest-seed3-f910.txt |
| corrupt-1pct-from-guest | 4 | 839 | skater[3].Pos (X=42.454587,Y=2.400000) / (X=42.454446,Y=2.400000) | 839 | 840 | corrupt-1pct-from-guest-seed4-f839.txt |
| corrupt-1pct-from-guest | 5 | 3053 | skater[3].Pos (X=7.358454,Y=-20.692789) / (X=7.358495,Y=-20.692710) | 3053 | 3060 | corrupt-1pct-from-guest-seed5-f3053.txt |
| corrupt-1pct-from-guest | 6 | 1249 | skater[3].Pos (X=41.157338,Y=7.516005) / (X=41.157338,Y=7.516004) | 1268 | 1500 | corrupt-1pct-from-guest-seed6-f1249.txt |
| corrupt-1pct-from-guest | 7 | 838 | skater[3].Pos (X=-20.113934,Y=-18.057444) / (X=-20.115558,Y=-18.057085) | 838 | 840 | corrupt-1pct-from-guest-seed7-f838.txt |
| corrupt-1pct-from-guest | 8 | 1570 | skater[3].Pos (X=-3.238057,Y=11.878287) / (X=-3.238050,Y=11.878290) | 1570 | 3180 | corrupt-1pct-from-guest-seed8-f1570.txt |
| corrupt-1pct-from-guest | 9 | 1490 | skater[3].Pos (X=-43.094582,Y=1.339340) / (X=-43.092587,Y=1.339653) | 1490 | 1500 | corrupt-1pct-from-guest-seed9-f1490.txt |
| corrupt-1pct-from-guest | 10 | 1545 | skater[3].PokeWindow 7 / 0 | 1545 | 1560 | corrupt-1pct-from-guest-seed10-f1545.txt |
| corrupt-1pct-from-guest | 12 | 755 | skater[3].Pos (X=11.519951,Y=4.349823) / (X=11.519950,Y=4.349823) | 767 | 900 | corrupt-1pct-from-guest-seed12-f755.txt |
| corrupt-1pct-from-guest | 13 | 319 | skater[3].Pos (X=14.952948,Y=2.697527) / (X=14.952959,Y=2.697526) | 319 | 660 | corrupt-1pct-from-guest-seed13-f319.txt |
| corrupt-1pct-from-guest | 14 | 518 | skater[3].LiftReadyFrame 548 / 206 | 518 | 540 | corrupt-1pct-from-guest-seed14-f518.txt |
| corrupt-1pct-from-guest | 16 | 1355 | skater[3].Pos (X=-43.648973,Y=14.257452) / (X=-43.648997,Y=14.257418) | 1355 | 1380 | corrupt-1pct-from-guest-seed16-f1355.txt |
| corrupt-1pct-from-guest | 17 | 1730 | skater[3].Pos (X=-17.632925,Y=-1.532559) / (X=-17.632952,Y=-1.532540) | 1730 | 1740 | corrupt-1pct-from-guest-seed17-f1730.txt |
| corrupt-1pct-from-guest | 18 | 3570 | skater[3].Pos (X=3.983520,Y=-7.474511) / (X=3.983562,Y=-7.474503) | 3570 | 3600 | corrupt-1pct-from-guest-seed18-f3570.txt |
| corrupt-1pct-from-guest | 19 | 417 | skater[3].BladePos (X=-9.448626,Y=-2.745940) / (X=-10.984734,Y=-2.197361) | 417 | 420 | corrupt-1pct-from-guest-seed19-f417.txt |
| corrupt-1pct-from-guest | 20 | 621 | skater[3].Pos (X=-19.999492,Y=-11.429748) / (X=-19.999492,Y=-11.429748) | 638 | 720 | corrupt-1pct-from-guest-seed20-f621.txt |

### Mismatch table

| case | what the protocol-5 host does | reason the player sees | crash |
|---|---|---|---|
| protocol 4 HELLO to a protocol 5 host (lobby) | Read=0 PeekType=None: the datagram is skipped like noise; no WELCOME, no refusal goes back | host: nothing (stays 'waiting for a friend'); guest after 60 s: 'no answer from the host in 60 s (same network? port forwarded?)' | no |
| protocol 4 input packets to a protocol 5 driver | all 2000 refused by Read; peer never heard; frame stays 0; lost at tick 1800 (10 x 180, the not-yet-heard limit) | 'CONNECTION LOST: no word from <name> for 3 seconds' (after 30 s, since an unheard peer gets 10 x the limit) | no |
| unknown flag bit 1024 on protocol 5 | Read=1, accepted; ToInput drops the bit (any verb set: 0); a driver fed it steps to frame 600, desync 0 | none: silently ignored (a sender that applies its own bit would desync at the next checkpoint) | no |
| unknown flag bit 32768 on protocol 5 | Read=1, accepted; ToInput drops the bit (any verb set: 0); a driver fed it steps to frame 600, desync 0 | none: silently ignored (a sender that applies its own bit would desync at the next checkpoint) | no |
| 0 bytes | Read=0 (inputs -1) lobby Read=0; host fed it 300 ticks: heard 0 frame 0 | none: skipped like noise (the match side shows 'connection lost' if nothing else comes) | no |
| 5 bytes (magic + version) | Read=0 (inputs -1) lobby Read=0; host fed it 300 ticks: heard 0 frame 0 | none: skipped like noise (the match side shows 'connection lost' if nothing else comes) | no |
| 6 bytes (header type only) | Read=0 (inputs -1) lobby Read=0; host fed it 300 ticks: heard 0 frame 0 | none: skipped like noise (the match side shows 'connection lost' if nothing else comes) | no |
| 26 bytes (header minus the count) | Read=0 (inputs -1) lobby Read=0; host fed it 300 ticks: heard 0 frame 0 | none: skipped like noise (the match side shows 'connection lost' if nothing else comes) | no |
| 27 bytes, count 8, no inputs | Read=0 (inputs -1) lobby Read=0; host fed it 300 ticks: heard 0 frame 0 | none: skipped like noise (the match side shows 'connection lost' if nothing else comes) | no |
| one input byte short | Read=0 (inputs -1) lobby Read=0; host fed it 300 ticks: heard 0 frame 0 | none: skipped like noise (the match side shows 'connection lost' if nothing else comes) | no |
| one trailing byte | Read=1 (inputs 8) lobby Read=0; host fed it 300 ticks: heard 1 frame 0 | none: accepted | no |
| 100 trailing bytes | Read=1 (inputs 8) lobby Read=0; host fed it 300 ticks: heard 1 frame 0 | none: accepted | no |
| count 255, full payload (2577 B, over MaxDatagram 512) | Read=1 (inputs 255) lobby Read=0; host fed it 300 ticks: heard 1 frame 0 | none: accepted | no |
| count 255, 8 inputs of payload | Read=0 (inputs -1) lobby Read=0; host fed it 300 ticks: heard 0 frame 0 | none: skipped like noise (the match side shows 'connection lost' if nothing else comes) | no |
| count 0 | Read=1 (inputs 0) lobby Read=0; host fed it 300 ticks: heard 1 frame 0 | none: accepted | no |
| first frame 0xFFFFFFF0 | Read=1 (inputs 8) lobby Read=0; host fed it 300 ticks: heard 1 frame 0 | none: accepted | no |
| fuzz: 20000 mutated packets | 12724 read as input packets, 7276 refused; a driver fed all of them ends at frame 0, lost 1, desync 0 | none (no crash; mutated packets that keep magic+version+type and a fitting count are applied as inputs) | no |
| unknown-bit-1024-from-guest | 20 matches: 0 diverged, 0 desync cards, 0 lost;  | none | no |
| protocol-4-guest | 20 matches: 0 diverged, 0 desync cards, 20 lost;  | connection lost (no protocol reason) | no |
| corrupt-1pct-from-guest | 20 matches: 18 diverged, 17 desync cards, 0 lost;  seed 1 f774 skater[3].BladePos (X=10.816996,Y=-10.374465) / (X=10.892685 (driver -1);  seed 2 f1403 skater[3].BladePos (X=-16.761180,Y=2.917123) / (X=-16.304231 (driver 2640);  seed 3 f910 skater[3].Pos (X=-37.378317,Y=15.678870) / (X=-37.376548,Y=1 (driver 960);  seed 4 f839 skater[3].Pos (X=42.454587,Y=2.400000) / (X=42.454446,Y=2.40 (driver 840);  seed 5 f3053 skater[3].Pos (X=7.358454,Y=-20.692789) / (X=7.358495,Y=-20. (driver 3060);  | DESYNC card at the next 60-frame checkpoint | no |

### Lockstep step timing

- clean: Pump (receive + deserialize + hash compare + serialize + send): n=5764800 mean 1.88 us, p50 1.70 us, p99 3.70 us, max 30842.5 us
- clean: TryStep (quantise + gate + apply): n=5761600 mean 0.13 us, p50 0.10 us, p99 2.30 us, max 5219.4 us
- clean: sim frame (brain + UJamSim::Step + zones + effects), for scale: n=5760000 mean 5.15 us, p50 4.40 us, p99 9.80 us, max 20949.1 us
- clean: Pump (receive + deserialize + hash compare + serialize + send): n=1441200 mean 1.98 us, p50 1.70 us, p99 3.80 us, max 24008.1 us
- clean: TryStep (quantise + gate + apply): n=1440400 mean 0.13 us, p50 0.10 us, p99 2.30 us, max 715.0 us
- clean: sim frame (brain + UJamSim::Step + zones + effects), for scale: n=1440000 mean 5.36 us, p50 4.40 us, p99 10.00 us, max 18131.8 us
- lossy: Pump (receive + deserialize + hash compare + serialize + send): n=14406204 mean 1.79 us, p50 1.70 us, p99 3.60 us, max 37568.8 us
- lossy: TryStep (quantise + gate + apply): n=14396171 mean 0.11 us, p50 0.10 us, p99 2.20 us, max 2609.5 us
- lossy: sim frame (brain + UJamSim::Step + zones + effects), for scale: n=12000000 mean 5.12 us, p50 4.40 us, p99 9.70 us, max 24319.4 us
- clean: Pump (receive + deserialize + hash compare + serialize + send): n=1441200 mean 2.00 us, p50 1.70 us, p99 3.80 us, max 18286.9 us
- clean: TryStep (quantise + gate + apply): n=1440400 mean 0.13 us, p50 0.10 us, p99 2.30 us, max 624.7 us
- clean: sim frame (brain + UJamSim::Step + zones + effects), for scale: n=1440000 mean 5.24 us, p50 4.40 us, p99 10.40 us, max 5522.7 us
- lossy: Pump (receive + deserialize + hash compare + serialize + send): n=3701002 mean 1.85 us, p50 1.70 us, p99 3.60 us, max 36596.3 us
- lossy: TryStep (quantise + gate + apply): n=3698130 mean 0.11 us, p50 0.10 us, p99 2.20 us, max 1595.4 us
- lossy: sim frame (brain + UJamSim::Step + zones + effects), for scale: n=2880000 mean 5.18 us, p50 4.40 us, p99 9.90 us, max 22384.0 us
- clean: Pump (receive + deserialize + hash compare + serialize + send): n=1441200 mean 2.00 us, p50 1.70 us, p99 3.70 us, max 17781.4 us
- clean: TryStep (quantise + gate + apply): n=1440400 mean 0.14 us, p50 0.10 us, p99 2.30 us, max 7094.6 us
- clean: sim frame (brain + UJamSim::Step + zones + effects), for scale: n=1440000 mean 7.16 us, p50 6.70 us, p99 13.70 us, max 16142.6 us
- lossy: Pump (receive + deserialize + hash compare + serialize + send): n=3701002 mean 1.76 us, p50 1.60 us, p99 3.50 us, max 15553.1 us
- lossy: TryStep (quantise + gate + apply): n=3698130 mean 0.11 us, p50 0.10 us, p99 2.10 us, max 807.4 us
- lossy: sim frame (brain + UJamSim::Step + zones + effects), for scale: n=2880000 mean 6.82 us, p50 6.40 us, p99 11.80 us, max 21272.4 us
- lossy: Pump (receive + deserialize + hash compare + serialize + send): n=5771796 mean 1.82 us, p50 1.60 us, p99 3.30 us, max 17698.5 us
- lossy: TryStep (quantise + gate + apply): n=4326052 mean 0.11 us, p50 0.10 us, p99 1.90 us, max 1362.6 us
- lossy: sim frame (brain + UJamSim::Step + zones + effects), for scale: n=2880000 mean 5.38 us, p50 4.50 us, p99 9.90 us, max 20005.2 us
- clean: Pump (receive + deserialize + hash compare + serialize + send): n=7206000 mean 1.90 us, p50 1.80 us, p99 3.80 us, max 31222.4 us
- clean: TryStep (quantise + gate + apply): n=7202000 mean 0.13 us, p50 0.10 us, p99 2.20 us, max 1324.7 us
- clean: sim frame (brain + UJamSim::Step + zones + effects), for scale: n=7200000 mean 4.98 us, p50 4.40 us, p99 9.80 us, max 26266.2 us
- clean: Pump (receive + deserialize + hash compare + serialize + send): n=7206000 mean 1.84 us, p50 1.70 us, p99 3.80 us, max 23100.2 us
- clean: TryStep (quantise + gate + apply): n=7202000 mean 0.13 us, p50 0.10 us, p99 2.30 us, max 691.8 us
- clean: sim frame (brain + UJamSim::Step + zones + effects), for scale: n=7200000 mean 5.02 us, p50 4.40 us, p99 9.90 us, max 24911.7 us
- lossy: Pump (receive + deserialize + hash compare + serialize + send): n=1440800 mean 18.97 us, p50 12.50 us, p99 80.40 us, max 43024.0 us
- lossy: TryStep (quantise + gate + apply): n=1440200 mean 0.19 us, p50 0.10 us, p99 2.50 us, max 1110.0 us
- lossy: sim frame (brain + UJamSim::Step + zones + effects), for scale: n=1440000 mean 6.96 us, p50 5.50 us, p99 22.90 us, max 15609.3 us
- clean: Pump (receive + deserialize + hash compare + serialize + send): n=3601200 mean 2.01 us, p50 1.80 us, p99 3.80 us, max 15853.4 us
- clean: TryStep (quantise + gate + apply): n=3600400 mean 0.14 us, p50 0.10 us, p99 2.30 us, max 13544.3 us
- clean: sim frame (brain + UJamSim::Step + zones + effects), for scale: n=3600000 mean 5.77 us, p50 4.60 us, p99 10.30 us, max 30087.0 us
- lossy: Pump (receive + deserialize + hash compare + serialize + send): n=9249304 mean 1.85 us, p50 1.60 us, p99 3.60 us, max 33520.7 us
- lossy: TryStep (quantise + gate + apply): n=9246426 mean 0.11 us, p50 0.10 us, p99 2.20 us, max 9731.4 us
- lossy: sim frame (brain + UJamSim::Step + zones + effects), for scale: n=7200000 mean 5.75 us, p50 4.60 us, p99 10.40 us, max 28946.6 us

<!-- /soak:tables -->
