# Lane A159, track S3 (fuzz and hardening), evolution 4 (device A, Sonnet)

You are lane A159 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then R-0056 to R-0061 at the end of `contracts/RULINGS.md` (R-0057 point 3 is your first item), the track's `docs/experiments/FUZZ.md`, `status/A141.md`,
`tools/gate/premerge.ps1` and `tools/fuzz/`.

Your worktree and the commit it stands on are named in the Director's message that starts you: the track's worktree moved to the newest main. It was built
before: an incremental build is enough. Your status file: `status/A159.md`. Port 4759. Family: `tools/fuzz/**`, `tools/gate/**` (new steps only: never loosen
one), node tests under `tools/**/test`, `docs/experiments/FUZZ-2.md`. You change no game source: a fault you find is a finding with its seed and its file.

## Evolution 4: what broke three release attempts tonight cannot break the operator's package
1. **Test code outside its guard** failed only the Shipping compile, an hour after the landing (H27, first attempt). A gate step (a node script, no
   Unreal: seconds): every `IMPLEMENT_*_AUTOMATION_TEST`, `BEGIN_DEFINE_SPEC` and test-only helper in `Game/Source/**` sits inside
   `#if WITH_DEV_AUTOMATION_TESTS` (or `WITH_AUTOMATION_TESTS`) that closes after it; files under `Tests/` that carry no guard at all; non-test code that
   calls a symbol defined only under the guard. Seeded-fault proof: three planted faults caught, the tree as it is clean. Wire it into `premerge.ps1`.
2. **The menu's first press ignored the launch cut** (H27, second attempt): a node or automation test that every `-cut=` the acceptance run and the matrix
   use reaches a match with that cut's id in its `JAMMATCH` line (headless, `-Multiprocess`), for the three tiers and the twelve modes.
3. **Fuzz tonight's keys and switches**: every key added since the last fuzz (`stick.passive_take`, `stick.body_shield`, `stick.take_grace_frames`,
   `stick.poke_assist_deg`, `stick.miss_recovery_frames`, `skater.reverse_brake_decel`, the `burst.*` keys, `skater.spin.exposed_frames`,
   `contact.board_bonus`, `contact.release_grace_frames`, `contact.check_reach_m`, the brain's tune keys, the level switch, the modes at five a side): each
   at its limits and at random inside them, 200 matches a class: no crash, no NaN, no hang (a match ends inside its frame budget), the same seed twice
   gives the same hash.
4. **The twelve mode cuts and the eight takes, each 20 matches at each level** with the fuzz oracles on: the cuts the operator will click at 10:00.
5. A table for the Director: what is covered by a gate step now and what is still only caught by the release chain.

Write each row's verdict into the status file as it completes; `GATE STEP READY` with the file list after item 1. `-Multiprocess`; three processes at
most; check memory before each batch. Time box 100 minutes.
