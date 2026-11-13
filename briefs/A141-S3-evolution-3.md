# Lane A141, track S3 (fuzz and the loader), evolution 3 (device A, Sonnet)

You are lane A141 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/EVOLUTION.md`, and the track's doc and status: `docs/experiments/FUZZ.md`, `status/A118.md`, `tools/fuzz/**`, `schema/README.md`.

Your worktree and the commit it stands on are named in the Director's message that starts you: a **fresh worktree at the newest main** (the track's two
evolutions are on it: the generator, the runner, the regression corpus, the strict parser both validators share). It has no binaries: build the editor first.
Your status file: `status/A141.md`. Port 4741 is another lane's: use 4742. Family: `tools/fuzz/**`, `schema/validate.mjs` and `schema/strict-json.mjs`, the
Designer-reply parser on both sides (`parseDesignerReply` in `validate.mjs` and its C++ mirror), `JamFuzzTests.cpp`, `schema/game.schema.json` and
`schema/limits.json` for the two limit changes named below, `docs/experiments/FUZZ.md`.

## Evolution 3: every key added tonight is fuzzed, and the Designer's replies parse one way
The game gained about fifteen cartridge keys tonight (`contact.board_bonus`, `contact.release_grace_frames`, `contact.check_reach_m`, `stick.passive_take`,
`stick.body_shield`, `stick.take_grace_frames`, `skater.reverse_brake_decel`, `skater.burst.own_stop`, the modes' use of them). The Designer (the player's
"direct a cut" panel) writes cartridges from a model's reply: a player-made cut is the likeliest place for a bad one.
1. **The generator knows every key in the schema** (derive the list from `game.schema.json` and `limits.json`, so the next key is covered without an edit) and
   pushes each to both ends of its range, alone and in pairs with the keys it interacts with (the two graces with the hooks that end a period; a board bonus
   with a shrinking rink; a take grace with `set_param`). A fresh 6,500-case run: crashes, hangs, non-finite state, a puck or skater outside, a match that
   never ends, non-determinism: zero, or each fault minimised into `tools/fuzz/found/` with its class.
2. **Designer-reply parity** (the track's own next-1): `parseDesignerReply` and its C++ mirror go through the strict parser; 1,000 mutated Designer replies
   (duplicate keys, a byte order mark, truncated, a key outside its range, an unknown key, prose around the JSON): both sides agree on accept or refuse and
   on the pointer, 0 parity faults.
3. **Two limits the lanes asked for**: `rules.timer_frames` at most 28,800 (a clock above 29,400 can end past the 36,000-frame cap every tool assumes: ruling
   R-0052 point 6; check no shipped cartridge exceeds it); a hook action's `value` range against `set_param goal_pause_frames` (30 to 600 can never be valid
   under the -20..20 limit: drop that tunable from the list or widen the value for it: choose and say).
4. **The timer oracle in the smoke test**: "the match ends by its hard stop" checked on every smoke case, the smoke under 10 s.
5. If time remains: rule chains (hooks that trigger each other at depth 8): 10,000 cases, the hook depth cap never exceeded.

After a schema change regenerate the embedded data (`schema/README.md` says how). The gate: the full `Jam` filter green; the 35 shipped cartridges hash as
before (prove it as lane A118 did). `-Multiprocess` on every headless start. Time box 100 minutes; interim in the status file.
