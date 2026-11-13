# Lane A80: collisions pass 2 with B's new takes: the strips re-taken, the figure measured against the ice, the hit-stop against the get-up (device A, from H17, Sonnet)

You are lane A80 of the Third Jam (team Hokej). Tackling is the operator's key feature (R-0036, B-to-A-0036). Lane A73 proved the chain in a package with the takes that were on
main at 7dfbeb5; device B's SK12 takes (new lying poses, side falls `Knockdown_Left/_Right` with spin variants, side get-ups) landed with the dev-b merge 9f6dfe6 and B asked:
"please take the next strips after merging dev-b" (`handoff/A-to-B-0017.md`, B's answer). Your job: the same strips on the current main, the figure measured against the
ice in centimetres, the hit-stop timing against the get-up, bot-vs-bot collisions counted, and B's remaining asks answered with numbers.

Read first: `docs/experiments/TACKLING-2.md` (A73: the drills `jam.ctrl.Drill 5/6`, `jam.ctrl.Stills 1`, `a73-prove.ps1`, `a73-analyze.mjs`, `a73-strip.py`, the counts),
`handoff/A-to-B-0017.md` (both sides), `handoff/B-to-A-0036.md`, `contracts/skater-visual.md` (v4), `Game/Source/JamCore/Private/JamPawns.cpp` (the hit-stop: `jam.hitstop`,
`CustomTimeDilation`, `SetKnockedDown(KnockFrames - N)`), `JamSim.cpp` (`BumpMinClosing` 3 m/s, the bump read-out, knockdown frames), `docs/experiments/PROFILING.md` (the remote
console), `tools/eyes/README.md`.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a80` (created at the current main; build the Development game package yourself with `-WaitMutex`). Run no git
command; never touch `Game/Plugins/JamAssets` (what the figure lacks goes to B with centimetres and frames); never print `.env` (copy the main checkout's `.env` for the
eyes tool without printing it; say so); never kill a process you did not start; no interactive editor unless a bone's world position must be read (then one on MCP port
8039, closed after). No sim change; the hit-stop's default stays 3 unless the measurement says otherwise (then say the number, do not change it).

You own: `tools/qa/ctrl/a80-*`, `docs/experiments/TACKLING-3.md`, `docs/experiments/img/a80/**`, `docs/experiments/eyes/a80-*.json`, `status/A80.md`, and the dated reply under
`## Answer` of `handoff/A-to-B-0017.md`.

## Job
1. A73's four runs again on the current main (`a73-prove.ps1`): the same table (hits, knockdowns of the human, strips, get-ups, hit-stops, kicks, camera on the lying human);
   strips of a human knocked down from the side, from behind, a bot knocked down by the human, a board hit, a stick hit; eyes `bug-hunt` on four.
2. The figure against the ice: for every knockdown in one run, the lowest bone (pelvis, hands, head) against the ice plane in centimetres per frame of the lying pose (a
   `JAMVIS` read-out of the visual's lowest bone Z, or the editor's bone query on a paused PIE at the lying frame); the table per take (back, face down, side left, side
   right, spin). Under the ice by more than 2 cm is a finding for B with the take's name.
3. The hit-stop against the get-up: with `jam.hitstop` 0, 3 and 6, the frame the figure stands versus the sim's release frame (`SetKnockedDown(false)`), ten knockdowns each;
   the offset per setting. If 3 drifts, write the number for B (SK13 asked for it).
4. Bot-vs-bot: in a 3-minute pro bot match, bumps, checks, knockdowns, board hits, pins between bots; whether any figure stays down after the release or pops.
5. `TACKLING-3.md` and the reply to B.

Report: the counts against A73's, the centimetre table, the hit-stop offsets, what B still lacks, the file list. Time box 75 minutes.
