# Contact 8: a mashed button lands nothing, and the board slam is earned (lane A165, track F1, evolution 7; 2026-11-14)

On the track's tree as it stood after evolution 6 (main `1f5b6389`: `a7d89871` plus lane A157's four keys at 0). Three things: **a check press inside
the cooldown starts it again** (the fast masher is finished; what is left of mashing, and the value that ends it), **the board slam as a play** (earned by
place and by speed into the wall, paid in frames down), and **all_star at 2,000 paired seeds** for the two rules of evolution 6 that moved it. Every rule
is behind a key at its neutral value: the default match is hash-identical to the tree as it stood. Every cell of this document is measured with **the
ladder's instrument** (`JAM_PLAYTESTER_CHECK=closing`, R-0061). Status with timestamps: `status/A165.md`. Raw: `Game/logs/a165*`,
`tools/perf/results/a157/raw/` (the track's drivers stay in `tools/perf/results/a157/`; not landed).

Code: `Game/Source/JamCore/Private/JamSim.cpp`, `Public/JamSim.h` (one line in the dropped-press branch; the slam in `ResolveContact`; two read-outs, two
constants), `Public/JamCartridge.h`, `Private/JamCartridge.cpp` (two keys), `Public/JamBaseline.h` (their neutral line), `AI/JamPlaytester.cpp/.h` (three
read-outs on the playtest line; the masher's jitter, measurement only), tests in `JamCore/Private/Tests/JamContactTests.cpp`,
`JamNet/Private/Tests/JamNetGuestTests.cpp` (the read-out list), `schema/` (rows in the four files), the two generated copies of the schema.

## 0. What ships, in one table

Cells: novice at rookie / regular at pro / good at all_star on `hockey.regular`, seeds 1 to 600, paired on the seed, the ladder's instrument. The tree
with every key at its default (`offc` = `off7c`): **86.5 / 53.2 / 30.0**.

| Rule | Key (under `game.contact`) | Ships | Recommended value | What the value costs: rookie / pro / all_star |
|---|---|---|---|---|
| A press inside the cooldown starts it again | `check_cooldown_restart` | **false (neutral)** | **true, with `check_cooldown_frames` 60** | at 60: **+0.5 / 0.0 / -0.5** (87.0 / 53.2 / 29.5); at 36: **+0.3 / 0.0 / -0.2**; at 30: **0.0 / 0.0 / 0.0** |
| The board slam, earned and paid | `board_slam_frames` | **0 (neutral)** | **30** | **0.0 / +0.1 / +1.8** |
| Everything the track recommends, together: `downed_radius_m` 0.6, cooldown 60 with the restart, the lunge 12 / 1.5, the slam 30 | | | | **+0.8 / +0.3 / +4.3** (87.3 / 53.5 / 34.3 at 600 seeds; all_star at 2,000 seeds 29.7 against 33.8 = **+4.0** (standard error 1.5; by seed range +4.3 / +4.3 / +3.6; not one frame within 0.6 m in 2,000 matches); before the fix of section 3b: -0.2 / +0.3 / +5.0) |
| The push-out's own circle made hard (a fault of evolution 6, section 3b) | `downed_radius_m` | 0 (neutral) | 0.6 as before | **-2.2 / +0.3 / +5.2** with the ladder's instrument at 600 seeds; all_star **+2.7** at 2,000 seeds (standard error 1.5) |
| Evolution 6's two rules at all_star, 2,000 paired seeds (before that fix): `downed_radius_m` 0.6 | | | | all_star **+2.9** (standard error 1.5) |
| ... the lunge 12 / 1.5 | | | | all_star **+2.5** (standard error 1.5) |

Nothing goes on before the operator has played at 10:00. Each of these moves the default hash the moment it is on.

## 1. A mashed button lands nothing (item 1)

**The rule** (`game.contact.check_cooldown_restart`, boolean, default false = neutral; read only with `check_cooldown_frames` above 0): a check press that
arrives inside the cooldown is dropped as before, and starts the cooldown again from that press. One line in `UJamSim::Step`; no new state
(`CheckReadyFrame` is lane A157's, hashed behind 0). Staged (`Jam.Core.Contact.CooldownRestart`, cooldown 36, 160 frames): a button mashed every 12 frames
opens **one window (12 frames) and no other**, where the cooldown alone gave it five; a press every 37 frames opens every window; presses at 0, 20 and 60:
the third opens (the dropped one moved the cooldown to 56), at 0, 20 and 50 it does not; the mashed hit at frame 27 does not land, the press timed
three frames before it does; without a cooldown the key is not read.

**The masher at pro, 600 matches an arm** (the timed script lands 3.28 checks a match and wins 53.2 %; the masher with no cooldown lands **20.41** and
wins **60.3 %**; `node tools/perf/results/a157/read.mjs press mc0 mcr30 ...`):

| the hand (with the restart on) | cooldown 30 | cooldown 36 | cooldown 60 |
|---|---|---|---|
| RMB every 12 frames | **3.53 landed, 51.7 %** | **3.57, 51.0 %** | **2.93 landed, 49.0 %** |
| a perfect metronome one frame above the cooldown (every 31 / 37 / 61 frames) | 10.65, 58.3 % | 8.94, 57.5 % | 6.49, **51.5 %** |
| a rhythm at that period, give or take 5 / 6 / 10 frames | 8.10, 53.8 % | 6.74, 53.5 % | 4.69, 52.2 % |
| a rhythm safely above the cooldown (every 46 frames give or take 8, at 36) | | 8.31 landed, 53.0 % | |

**Verdict.**
- **The bar is met**: the button mashed every 12 frames lands 3.5 checks a pro match (20.4 without the rule; the bar was under 4.6) and wins 1.5 to
  2 points LESS than the timed script. It still gets about 24 windows a match: a press the sim never takes (the masher is down, stumbling, carrying the
  puck, or a faceoff is being set) is not a press, and a faceoff clears the cooldown. Those windows land what a timed player lands.
- **The three cells are inside one point at every value**: at 30 not one result flips and not one press is dropped for the scripted human or a bot
  (both space their own presses more than 30 frames apart); at 36: +0.3 / 0.0 / -0.2; at 60: +0.5 / 0.0 / -0.5.
- **What a restart cannot touch**: a hand that presses SLOWER than the cooldown. Every such press is a good press, and its window still catches what
  skates into it. Above 36 frames such a hand lands 6.7 to 8.9 checks a match (two to almost three times the timed script's 3.28); above 60 frames 4.7
  to 6.5. In wins a single arm carries about 2.7 points of noise: the one clear gain is the masher with no cooldown (60.3 % against 53.2 %); with the
  restart the slow hands read 53.0 to 58.3 % at 30 and 36, and **51.5 to 52.2 % at 60: nothing gained**. So the value is **60 with the restart** (one
  second between two checks that both count); 36 is enough with the lunge on, whose miss charges every empty window 1.5 m/s (lane A157).

**What a player sees when a press is dropped.** Today nothing: no window opens, the figure does not move (`PlayCheck` is not called: right, he did not
check), no sound, and the HUD's contact line stays empty. With the restart on that silence is a trap: the player mashes, nothing happens, and each
press pushes the next good one further away. **The line this lane asks the screen track for: `TOO SOON`**, on the HUD's contact line, in the same slot
and with the same hold as `TOO SOFT`, `MISSED` and `JUST GOT UP`, shown when `FJamSkaterState::ChecksCooling` of the human rises (the read-out is in
the sim since lane A157, counted exactly as `ChecksMissed` is for `MISSED`). One line a frame as now; `JUST GOT UP` and `TOO SOFT` win over it. No HUD
code was written here.

## 2. The board slam as a play (item 2)

**The rule** (`game.contact.board_slam_frames`, integer 0..120, default 0 = neutral; `UJamSim::ResolveContact`).
- **Earned**: the hit fells its victim (after the release grace has had its say; an armed hip check keeps its own rule), the victim's centre is within
  1 m of the boards (`BoardSlamRangeM`: 0.6 m of ice between his body and the wall), and the attacker closes on him **along the boards' normal** at 4 m/s
  or more (`BoardSlamMinMps`): driven into the wall, not along it. Read on the velocities before the hit moves anything.
- **Paid**: `board_slam_frames` down on top of what the hit earned (scaled with `skater.knockdown_frames` as the other frames are), the victim's board
  read-out and the `board` event of a board hit (also at `board_bonus` 1, where lane A101's bonus is silent), and two read-outs: `bContactSlam` (one frame)
  and `ContactSlams` (a count). No rule state.

Staged (`Jam.Core.Contact.BoardSlam`, a standing victim 0.8 m from the boards, 9 m/s): straight in: down 81 frames at 0, **111 at 30** (the same ratio,
1.458); along the boards, 1.3 m out, and 70 degrees off straight in (3.1 m/s along the normal): no slam, the frames of the neutral value; 30 degrees off
(7.8 m/s along the normal): a slam.

**Its read in a match, `board_slam_frames` 30, 600 matches a cell:**

| | rookie | pro | all_star |
|---|---|---|---|
| slams a match (all skaters) | 0.15 | **0.33** | 0.44 |
| the human's own slams a match | 0.03 | **0.17** | 0.24 |
| matches with a slam / with one by the human (of 600) | 83 / 15 | 157 / 92 | 202 / 131 |
| frames down, a slam | 145.7 | **144.7** | 160.6 |
| frames down, an open-ice knockdown | 115.2 | **120.2** | 123.1 |
| frames down, a board knockdown that is not a slam | 103.0 | 108.2 | 121.4 |
| the human's landed hits at the boards a match, key off and on | 0.42, 0.42 | 0.69, 0.71 | 0.65, 0.64 |
| the three cells against off | 86.5 (0.0) | 53.3 (**+0.1**) | 31.8 (**+1.8**) |

**Verdict against the bar.**
- *A slam's frames down at least 20 above an open-ice hit*: **met**: +24.5 at pro, +30.5 at rookie, +37.5 at all_star. (Before the rule a board knockdown
  was the SHORTER one: 110 against 120 frames at pro.)
- *The cells within 4*: **met**: 0.0 / +0.1 / +1.8.
- *The human's board knockdowns from 0.76 to 1.5 a pro match*: **not met, and not by any value of this key**: paying a hit does not make more of them.
  The human lands 0.7 hits a pro match at the boards and 0.17 of them are slams: one board knockdown in three is driven in at 4 m/s along the normal
  within a metre. The scripted human never aims at the wall; a person who is told the slam exists can. The number that would move it is the script's (or
  the brain's) choice of line, not a contact rule: see the next evolutions.

**The event (a proposal; `contracts/events.md` not edited, nothing new emitted).** The contract has the event already: `board`, with Kind `skater`
("a skater hits the boards at 4 m/s or more, or is checked into them", lane A101). A slam sends that event today, as any board hit does. Proposed text:
a second Kind on the same event, **`board` with Kind `slam`**: "a skater is slammed into the boards (`game.contact.board_slam_frames` above 0: felled within
1 m of them by a hit closing along their normal at 4 m/s or more)"; Who = the skater slammed, Where = the point of the boards nearest to it, Strength =
the check's (closing speed / `game.skater.speed`), Dir = the boards' normal into the rink; in the binder's table `board`: every other Kind 0, `slam` 1
(one `NS_board` / `MS_board` with the Variant input: the glass and the bigger sound). The HUD's line for the attacker: **`SLAM`** on the contact line when
the human's hit set `bContactSlam` on its victim (`ContactBy` names the attacker). Once the text is accepted the sim's change is one argument
(`KindSkater` to a new `KindSlam` in the `Emit` of the board block when `bSlam`).

## 3. All_star at 2,000 paired seeds (item 3)

Evolution 6 left the curve lane three numbers for one cell (+8.4 on the first 600 seeds, +4.0 at 1,200, -0.3 on the second 600, all with the shipped
press). The same two rules, the ladder's instrument, seeds 1 to 2,000, paired (`node tools/perf/results/a157/pair1.mjs offc2k <arm> as`):

| rule at all_star | off | on | delta | standard error | results that flipped (off only / on only) | by seed range: 1-600 / 601-1,200 / 1,201-2,000 |
|---|---|---|---|---|---|---|
| `downed_radius_m` 0.6 | 29.7 | 32.5 | **+2.9** | 1.5 | 399 / 456 | +3.7 / +2.8 / +2.3 |
| the lunge, 12 frames and 1.5 m/s | 29.7 | 32.1 | **+2.5** | 1.5 | 405 / 454 | +3.3 / +4.7 / +0.1 |

**The number to use: each rule lifts the human at all_star by about 2.5 to 3 points** (two standard errors: 0 to 6). The push-out's three ranges agree
(+3.7, +2.8, +2.3); the +8.4 of the first read was a lucky 600 with the other press. Each alone is inside the four-point rule.

**After the fix of section 3b, on the final build, the same 2,000 seeds** (`on06c2k8`, `all8c2k` against `offc2k`): the push-out alone 29.7 against 32.4 = **+2.7** (standard error 1.5; by seed range +5.2 / +3.8 / 0.0; not one frame within 0.6 m of a downed skater in 2,000 matches); everything the track recommends on together 29.7 against 33.8 = **+4.0** (standard error 1.5; by seed range +4.3 / +4.3 / +3.6; not one frame within 0.6 m in 2,000 matches).

## 3b. A fault of evolution 6 found and fixed: the downed body's own circle is hard

With everything on together (the first read, `all7c`) B's bar broke at all_star: a root within 0.6 m of a downed skater's for more than 6 frames, **2
spells in 200 matches, the longest 10 frames** (0 with the push-out alone). Cause: lane A157's push-out replaces the ordinary collision for a downed
pair and eased the skater out at 6 m/s from any depth; a skater squeezed onto a downed man (a pile-up at all_star, a lunge's extra speed) was no longer
moved out of the body's own circle at once, as the ordinary collision had always done. **Fixed** (`UJamSim::PushOutOfDowned`): inside the two body
circles (roots under 0.8 m) the skater is moved out to them at once; from 0.8 m to the keep-out it is eased as before. Staged in
`Jam.Core.Contact.DownedPushOut`: a skater put 0.3 m from a downed man's root is at 0.800 m one step later and at 1.019 m four steps later. At the
key's neutral value the function is never called (the default hash does not move). After the fix, 200 matches a cell:

| a root within 0.6 m of a downed skater's for more than 6 frames (B's bar) | rookie | pro | all_star |
|---|---|---|---|
| the push-out alone (0.6), before the fix (lane A157's `on06`, the shipped press) | 0 spells, longest 3 frames | 0, longest 1 | 0, longest 6 |
| the push-out alone, after | 0 spells, not one frame within 0.6 m | 0, not one frame | 0, not one frame |
| everything on, before the fix | 0, longest 3 | 0, longest 1 | **2, longest 10** |
| everything on, after | 0 spells, not one frame within 0.6 m | 0, not one frame | 0, not one frame |
| within 0.9 m for more than 6 frames (standing on the figure), everything on, after | 1 spell | 1 | 8 |

The cells of the push-out alone with the ladder's instrument after the fix: **-2.2 / +0.3 / +5.2** (84.3 / 53.5 / 35.2 at 600 seeds; all_star at 2,000 seeds 29.7 against 32.4 = **+2.7** (standard error 1.5; by seed range +5.2 / +3.8 / 0.0; not one frame within 0.6 m of a downed skater in 2,000 matches)); everything on, after the fix: **+0.8 / +0.3 / +4.3**.

## 4. The proof

- **The default match is hash-identical to the tree as it stood** (and so to `a7d89871`). `off7c` (the final build, every key at its default, the
  ladder's instrument) against `offc` (lane A157's final build) and `basec` (`a7d89871`): **600 of 600 matches hash-identical in each of the three cells**.
  `off7` (the final build, the shipped press, 100 seeds a cell) against `base`: **100 of 100 at pro and 100 of 100 at all_star (and 100 of 100 in all three cells on the build before the last fix)**.
- **The gate**: the full `Jam` filter on the final tree: **254 PASS, 0 FAIL (252 on the tree as it stood; the two new tests are the difference)**. `Jam.Net.HashCoverage` green: no rule state was added; `bContactSlam` and
  `ContactSlams` are listed read-outs. Of the 228 lines that carry a hash, 226 are identical to lane A157's final gate character for character and the other two (`Determinism`, a timing in their text) carry the same hashes; the 11 `LAW3` lines are identical.
- **Both validators**: `node --test schema/**/*.test.mjs` 99 pass, 0 fail; the cartridge tests and `Jam.Designer.PromptParity` green with both embedded
  copies regenerated (`gen-schema-data.mjs`, `gen-designer-data.mjs`).
- **Neutral values**: one line in `JamBaseline::MakeNeutral` (false and 0). No pinned hash of a shipped cut was added.
- The Shipping compile of the Game target on the final tree: PASS (125 s, 93 actions; `Game/logs/build-ThirdJam-Shipping-20261114-054327.log`). Not run: the acceptance run (nothing a player can do changes at the defaults).

## 5. Findings outside the family

1. `Game/Source/JamCore/AI/JamAICore.cpp:1124` (as in CONTACT-7): the brain spends its own 30-frame spacing on a press the sim drops. With the restart
   on and a cooldown above 30 that press also restarts the bot's cooldown: at 36 the bots lose 0.05 presses a match, at 60 0.17 of 14.3 at pro. The brain
   should read `CheckReadyFrame` before it presses.
2. The scripted human and the brain never choose the boards for a hit (0.17 slams a pro match by the human): the slam is a play only for a player who
   aims. The script's check press is this track's; the brain's line is the brain track's.
3. `tools/barks/barks.test.mjs:64` still fails in this worktree (as in CONTACT-7): not this lane's files.

## 6. The next three evolutions for the track

1. **The hand that aims for the wall.** The scripted human (and, proposed to the brain track, a pro defender) steers its check so that the man is
   between it and the boards when he is within 2 m of them. Proving numbers: the human's slams a pro match from 0.17 to 0.6 or more over 600 matches,
   board knockdowns from 0.7 to 1.5 (the bar this evolution could not meet with a rule), the three cells within 4.
2. **The dropped press and the slam on the screen, proven headless**: the sim-side half of `TOO SOON` and `SLAM` (the counters exist; a log line a
   press and a slam as `JAMHITSTOP` has, and the `board` event's Kind `slam` once the contract has it), and the hit-stop and camera kick scaled for a slam.
   Proving numbers: in one scripted pro match every dropped press and every slam has its log line (dropped presses logged = `checks_cooling`, slams
   logged = `slams`), and a slam's hold is at least 2 frames longer than the same ratio's open-ice hold.
3. **The whole contact set on, with the curve**: `downed_radius_m` 0.6, cooldown 60 with the restart, the lunge 12 / 1.5, the slam 30: measured together
   here at +0.8 / +0.3 / +4.3. With the curve lane: 2,000 paired seeds a cell, at most one profile number a level moved. Proving numbers: every cell within
   3 of its target with the set on, law 3's gate unchanged, and the acceptance run green on a package of that tree.
