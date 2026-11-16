---
status: DRAFT (lane R, 2026-11-16; nothing dispatched; for the owner's gate on night 2 of NIGHTLY-FORGE-PLAN.md)
source: thirdjam/docs/registry/LESSONS.md rows J31, J33, J34, J37, J40 (and the applications J35, J36, J38, J39); thirdjam briefs/EVOLUTION-COMMON.md, briefs/NIGHT-COMMON.md, handoff/README.md, contracts/RULINGS.md; the Director's skill improvement log
run_id: spark@jam-lessons-home
bundle: agent-operations
category: coordination
subject: directed-parallel-lanes
resolved_path: knowledge/agent-operations/coordination/directed-parallel-lanes/
category_count: coordination holds 1 subject (shared-research-record) and 2 after. The alternative home, game-production/craft-judgment (6), was rejected on the merits: nothing in this subject is specific to producing a game, and the craft-judgment neighbour unattended-build-loop owns an unattended loop, not a person directing several live sessions.
---

# Spec: directed-parallel-lanes

## Why a subject, and why in agent-operations

The jam ran for four days as one director (an agent session working for one owner) and, at its peak, eight concurrent lanes (agent
sessions each with one brief) on one machine, plus a second machine with its own lanes, joined by request files and one remote.
Five candidates from that run share one home-if-new: how a director runs such lanes so that the work survives the director's own
restarts, lands without sweeping a neighbour's edits, and is accepted on evidence. The neighbours, checked on this machine:

- **software-engineering** (a different bundle: no links, one prose discriminator each): `fleet-orchestration` (19 techniques)
  owns how a *dispatcher system* is built (registry, harvest, parallel dispatch, briefs, liveness); `concurrent-vcs` owns how
  sessions share one checkout (integration door, physical isolation, commit verification, arbitration). This subject owns the
  *operating discipline* of a director who runs lanes by hand with those tools, the way `agent-run-budgeting` owns a budget and not
  the meter. Four jam lessons (J34 to J36, J39) are applications of those techniques and are filed there, not here.
- **agent-operations**: `shared-research-record` (the only coordination subject) owns many sessions publishing to one record with
  no director; `unattended-run-isolation` owns where one unattended run executes; `deterministic-run-verification` owns grading a run
  against the tree. None owns the director-to-lane contract.
- **game-production** `unattended-build-loop`: owns an unattended loop's gates and pass rates; here a person's director is present
  and the lanes are live sessions.

## The subject's job, in one sentence

**One director runs several live agent lanes (and possibly a second machine) for one owner; this subject owns the contract between
them: what a brief carries, what a lane writes and when, when its work may be taken, how premises and causes travel, how the
director stays alive, and how the owner's words stay apart from the director's reading of them.**

## Golden path (draft, purity-clean, nine sentences)

A director's sessions restart, its turns end, and its context is lost more often than any lane's, so disk is the memory: every lane
writes a status file from its first minute, with each measurement as it completes and each decision with its number, so that a new
agent can resume the lane from that file and its tree alone. A lane's work is taken only at a declared ready line with no
"source open" line after it, because a tree taken mid-edit lands half-saved work. A brief states the goal, the file family the lane
may touch, the gate it must pass and the report form (each acceptance line met, not met or not done, with its figure), and it
states that the lane's measurement outranks the brief. A premise or a cause the director believes goes into the brief as a reading
for the lane to prove or refute, never as a fact, because the director's reading of a red result is a guess made without the lane's
instrument. Landings happen one at a time through one door under a lock, after each lane has run the full gate on its own tree;
changes that pass alone are landed together through an integration lane that owns their seam. Work that crosses an ownership line,
to another machine or another owner's files, is asked for in one request file per ask with an acceptance check and a deadline, a
decline is a counter-proposal, and the landing step proves the remote received what it printed. The director never ends a turn
without a live watcher or a queued next step, and where it may stand still for hours it hands the queue to a scheduled chain and
leaves the owner a one-line liveness check. The owner's words are recorded verbatim and apart from the director's reading of them,
in an append-only ledger in which a reading is marked as one the owner can overturn by a word. Lanes stop only what they started,
by the identifiers they recorded.

## Candidate techniques

| Slug | One-line rule | use_when |
|---|---|---|
| `status-file-from-the-first-minute` | Every lane writes its status file from its first minute and marks ready and source-open states explicitly; the director takes a lane's work only at a ready line with no source-open line after it. | dispatching a lane; the director's session restarted; deciding when a lane's tree may be exported |
| `brief-premises-are-readings` | A brief carries the director's premises and causes as readings for the lane to prove or refute, says the lane's measurement outranks the brief, and asks for each acceptance line as met, not met or not done with its figure. | writing a brief from a red result; a lane's finding contradicts its brief; a report says "done" without figures |
| `one-landing-door` | One landing or release at a time under a lock that queued work waits on; each lane runs the full gate on its tree first; changes that pass alone are landed together through an integration lane and rehearsed in a scratch tree. | two lanes finish at once; a landing swept another lane's patch; two changes pass alone and fail together |
| `request-files-across-an-ownership-line` | Cross an ownership line (another machine, another owner's files) only by one request file per ask with an id, a status line, an acceptance check and a deadline; a decline is a counter-proposal; the landing proves the remote received each push. | two machines share one remote; a lane needs a change in files it does not own; a second machine reports receiving nothing |
| `director-liveness` | The director ends no turn without a live watcher or a queued next step, restarts a watcher in the turn that reads its catch, and hands an overnight queue to a scheduled chain with a liveness line for the owner. | a lane finished and waited for hours; the director's session may be interrupted; a night is planned with lanes in a queue |
| `owner-words-and-director-readings` | Keep the owner's words verbatim, the director's reading beside them marked as a reading, in an append-only ledger the owner can overturn by a word. | relaying an owner's play notes into lane briefs; a reading turns out wrong; several rulings disagree |

Not proposed as techniques here, because they are applications of existing techniques in other subjects: stopping only recorded
process ids (`refuse-rather-than-destroy`, game-production L10; and `unattended-run-isolation`), unlinking junctions before removing
trees (`concurrent-vcs/physical-isolation`), reading the remote back after a push (`concurrent-vcs/commit-verification`). The golden
path mentions them in one sentence as walls and cites nothing across the bundle line.

## Laws cited (agent-operations `_laws.md`, present on this checkout)

- `the-repository-outranks-the-instruction`: the lane's measurement outranks the brief.
- `measure-the-tree-not-the-summary`: take a lane's work from its tree at a ready line, not from its report.
- `the-harness-is-a-suspect-in-every-red`: a red landing is first read for harness causes (a stopped run, a stale checkout).
- `a-refusal-is-not-a-result`: a lane that cannot run its gate says so; it does not report a pass.

## Evidence this jam gives (n = 1 run of four days, one owner)

- EC "Disk is the memory": "The Director's process has restarted three times in two days; a new agent must be able to resume your
  evolution from that file and your worktree alone"; MEM landing guards: two exports caught half-saved edits before the ready-line
  rule.
- R-0053 and R-0055: the director's stated cause of a red gate was wrong; the lane measured 6,000 carries with no shove from behind
  and found the real cause; the cure for it landed. LOG: five brief premises wrong in one night, each refuted with evidence.
- R-0050 (a): a detached checkout for six and a half hours; every push printed success; the second machine received nothing; the
  landing step now refuses off its branch and reads the remote back. handoff/README: one file per request, ids per direction.
- R-0058 (3): two overlapping landings; one commit swept another lane's patch; a lock now.
- R-0053 (2), R-0068 (5): an integration lane for two changes that passed alone; combined landings rehearsed in a scratch tree.
- MEM landing guards (14 Nov 18:55): a turn ended without a watcher, fifty minutes lost before a freeze; LOG (13 Nov): a director
  session stood still for seven hours and eight of eleven queued lanes never started.
- RULINGS from R-0064 on: the owner's words verbatim, the director's readings marked; R-0069 addition: a reading traced to the wrong
  build and corrected because the words were verbatim.

## What a second project must show

- A second directed run (another domain than a game) in which a director restart was survived from lane status files, and a
  premise written as a reading was refuted by a lane. The jam's operator ran an earlier non-game project the same way (the log's
  core-perfection run): that is a second run by the same operator and director, which is corroboration of the habit, not
  independence. A run by a different operator is what moves the subject from `draft`.
- For `request-files-across-an-ownership-line`: a second two-machine run; the jam had one.
- For `director-liveness`: a measured before and after (lanes idle per night) on a second run.

## Open questions the drafter must DECIDE

1. **Bundle.** agent-operations (recommended) or software-engineering's `llm-agent/orchestration` (where `fleet-orchestration`
   lives). Decide by the discriminator: does the subject describe a system someone builds (software-engineering) or a discipline a
   director practises with whatever system exists (agent-operations)?
2. **Whether `owner-words-and-director-readings` belongs with `hitl-approval` in software-engineering.** Decide; do not write it
   twice.
3. **Stage.** Several techniques start to pay only with three or more concurrent lanes; consider `stage: team` on
   `one-landing-door` and `request-files-across-an-ownership-line`.

## What the drafter must NOT do

- Name the game, the event, the models, the machines or the operator in the upper layers; "a second machine" and "the owner" are
  the words.
- Restate `fleet-orchestration` or `concurrent-vcs`; one prose discriminator each, no links across the bundle line.
- Run any git command.
