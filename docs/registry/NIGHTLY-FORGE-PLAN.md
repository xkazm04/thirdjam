# Nightly forge plan: the jam's lessons into the registry's game-production domain

Lane R, package R1, 16 Nov 2026. The owner's words: "/spark to prepare nightly plan how lessons learned from the project can be forged
into AI-registry's game development domain". Tonight produced the inputs only; nothing in `ai-registry` was edited. This plan is five
nights, each run in the registry's own checkout by a director session with the registry's own skills (`/intake`, `/forge`, `/deepen`,
`/conform`) and gate. The inputs, all in `thirdjam/docs/registry/` and `thirdjam/.ai/`:

| Input | What it is |
|---|---|
| `LESSONS.md` | 40 lessons, each with its evidence, nearest registry subject, kind and confidence |
| `.ai/registry-leads.jsonl` | 33 earned leads in the collector's schema (26 game-production, 5 software-engineering, 2 agent-operations); 0 withheld by the collector's own leak check (`lib/leaks.mjs`, FREE_TEXT, run read-only tonight) |
| `spec-person-hand-reproduction.md` | a DEEPEN spec: two new techniques and two amendments in existing subjects (decided: not a subject) |
| `spec-contact-readability.md` | a new game-production subject (asset-production/motion-and-audio, 5 -> 6) |
| `spec-ball-possession-and-pickup.md` | a new game-production subject (systems-canon, 7 -> 8) |
| `spec-directed-parallel-lanes.md` | a new agent-operations subject (coordination, 1 -> 2) |

Standing rules for every night: the registry's `AGENTS.md` (preserve other sessions' edits, stage per path, regenerate generated files
with their generators, never hand-edit `index.json`); query with `node scripts/registry-query.mjs "<decision>"` or the
`registry_query` tool, never read an `index.json`; the knowledge lane's gate (`node scripts/gate.mjs --lane knowledge --write`, then
`node scripts/gate.mjs --lane knowledge`) before any commit that touches `knowledge/`; never read anything under `.ai/cloud-dispatch/`;
upper layers name no repo, product, engine or tool.

---

## Night 1: sync the registry, settle the other session, connect the jam, collect the leads

**Input.** The registry checkout as it is: 169 commits behind its remote, with another session's uncommitted edits (the brief's
reading; this lane ran no git to confirm the count). The jam's ledger of 33 leads.

**Commands, in order.**
1. `git -C <registry> status --short --branch` (read only): list the other session's uncommitted paths and the behind count.
2. **Settle the other session's edits first, by its owner's decision, never by this run:** that session commits them on its own
   branch, or the owner names them abandoned. Never stash, never `git add -A`, never reset. Nothing below runs while they are there,
   because `register-project.mjs` refuses on a dirty checkout and `projects.json` is shared committed state.
3. `git -C <registry> pull --ff-only` on `main`; if it cannot fast-forward, stop: that is a merge decision for the owner.
4. `node scripts/gate.mjs --all` on the synced tree: the baseline. A red here is the registry's, not tonight's; record it and go on
   only with the knowledge lane green.
5. **Re-check every "nearest" in LESSONS.md against the synced index** (169 commits may have added a subject that owns possession,
   contact feedback or directed lanes): `node scripts/registry-query.mjs "<the lesson's claim>"` for J01, J11, J18, J23, J29, J31. A
   hit that owns the ground turns that spec into a deepen of the hit.
6. **Connect the jam** (what must be true for it to be in the collector's fleet): `projects.json` must declare a `thirdjam` project
   with a checkout for this machine, relative to the machine root (`kiro/thirdjam`), resolved through `.machine.local.json`; the
   collector reads `<checkout>/.ai/registry-leads.jsonl` and nothing else. The supported way is
   `node scripts/register-project.mjs --slug thirdjam --path <machine root>/kiro/thirdjam --domains game-production,agent-operations,software-engineering --dry-run`,
   then without `--dry-run`. **It will refuse at its second step today:** the jam has no `context-map.json`, so its registry map does
   not build and the script undoes its writes. Two ways through, the owner's choice:
   - (a) give the jam a context map first (the `adopt-bp` skill writes one), then register: three writes, the jam fully connected
     and `/conform`-ready on night 5;
   - (b) a one-line `projects.json` entry only (`"thirdjam": {"checkouts": {"<this machine>": "kiro/thirdjam"}}`), committed alone,
     enough for leads; manifest and map later.
7. `node scripts/leads-collect.mjs --dry-run`: expect 33 rows from `thirdjam`, `0 withheld`, `0 malformed`. Any withheld row is fixed
   in the jam's ledger, never in the inbox.
8. `node scripts/leads-collect.mjs`: appends to `librarian/inbox.md`. Commit `projects.json` and `librarian/inbox.md` by path.

**Done when.** The registry is on its remote's `main`, the other session's edits are settled, the gate baseline is recorded, the inbox
carries 33 rows `from thirdjam / spark@jam-lessons-home`, and LESSONS.md's nearest column is confirmed or corrected on the synced index.

**The owner decides.** What happens to the other session's edits; (a) or (b) for connecting the jam; whether `third-life` is
registered too (it has a context map and no manifest).

**Risk.** Pulling 169 commits moves the taxonomy under the specs (a category may have filled: craft-judgment, systems-canon,
motion-and-audio are 6, 7 and 5 here, and engine-integration, content-pipeline and the bundle's top level are already at 10). The
collector's dedup is by claim text: re-running is safe, re-wording a claim in the jam's ledger files it again.

---

## Night 2: intake the leads in four batches, through the admission gate

**Input.** The 33 inbox rows; LESSONS.md as the source note; the jam's repo as the tree the claims were read in.

**Commands.** `/intake` on the jam's lessons as pasted notes, one batch per run, with the admission gate as the skill states it:
vetoes first (V1 placement at a full category, V2 corroboration, V3 law, V4 strip, V5 contention), then the escalations, then the
score. Corroboration for V2 is "code read in a tree": each row's evidence names a ruling, a status file and a figure; the intake
director opens the cited file before scoring. Batches, smallest risk first:

| Batch | Rows | Expected shape |
|---|---|---|
| A. Applications and amendments to existing techniques | J02, J03, J04, J25, J26, J28, J33, J34, J35, J36, J38, J39 | amendments and applications; no new subject; the software-engineering and agent-operations rows land in their own bundles |
| B. New techniques under existing subjects | J01, J05, J06, J08, J09, J23, J27, J29, J30 | techniques; `spec-person-hand-reproduction.md` covers J01, J02, J04, J05 |
| C. The two game-production subjects | J11 to J16 (possession), J18 to J22 (contact) | "forge it now" or "bank as a lead" at the escalation gate; the specs are the forge briefs |
| D. The agent-operations subject | J31 (and J37, J40 from LESSONS, unfiled) | subject decision at the gate |

Strike each ruled row through in `librarian/inbox.md` and record the ruling in `librarian/subjects/<bundle>/<subject>.md` (validated by
`node scripts/review-coverage.mjs`).

**Done when.** Every one of the 33 rows is ruled: landed, declined with a reason, or banked as a lead with a return condition; the
batch A amendments are written and the knowledge gate is green (`--write`, then without).

**The owner decides.** At the E-gates: whether the three new subjects are forged now or banked; whether the person-hand material stays
techniques (this lane's recommendation) or becomes a subject (`stand-in-player-evidence`); the L15 question in the contact spec.

**Risk.** One project's evidence for every row (n = 1): the gate's corroboration rule may route the subject rows to "lead" rather than
"forge". That is a correct outcome, not a failure; the specs say what a second project must show.

---

## Night 3: forge the two game-production subjects; deepen the person-hand techniques

**Input.** `spec-contact-readability.md`, `spec-ball-possession-and-pickup.md` (if night 2 said forge), `spec-person-hand-reproduction.md`.

**Commands.**
1. The director writes the two taxonomy entries first (`knowledge/game-production/taxonomy.json`: `contact-readability` under
   asset-production / motion-and-audio, `ball-possession-and-pickup` under systems-canon) and checks the counts against the cap of 10.
2. `/forge` per subject with the spec as the dispatch (`docs/forge-brief.md`: expert draft first, then reconcile against the jam's
   tree; applications by the intake director, citing the jam's source with `path:line "quote"` and checked with
   `node scripts/check-anchors.mjs <doc> --root <thirdjam>`).
3. `/deepen` on `playtest-signal-to-defect` and `difficulty-design-and-adaptation` for the person-hand techniques (T1, T3), with the
   counter-evidence lanes the skill runs; the two amendments (T2, T4) were landed on night 2.
4. `node scripts/gate.mjs --lane knowledge --write`, then `node scripts/gate.mjs --lane knowledge`; the purity grep on every new upper
   layer (`grep -n -i` for the game's, engine's and tools' names) before reporting clean.

**Done when.** Two subjects at `status: draft` or `forged`, each technique with `use_when` and laws that exist; the deepen techniques
landed; the knowledge gate green; the forge reports say which claims were upward lessons from the jam.

**The owner decides.** The open questions each spec lists (slug, L15 reading, the controlled figure's asymmetric help).

**Risk.** Purity: the jam's vocabulary is vivid and the likeliest leak. A full category after the sync would veto a placement (V1):
then the spec's alternative placement is argued again, never a subcategory made as a side effect.

---

## Night 4: the agent-operations subject and the other domains' applications

**Input.** `spec-directed-parallel-lanes.md`; the five software-engineering rows (J28, J34, J35, J36, J39) if night 2 left their
applications unwritten.

**Commands.** The taxonomy entry in `knowledge/agent-operations/taxonomy.json` (coordination); `/forge` for `directed-parallel-lanes`;
the applications under `concurrent-vcs` and `quality-gates` (stack `process`, the jam's briefs and scripts cited by anchor); the
knowledge gate with `--write`, then without; then `node scripts/gate.mjs --all`, because the night crosses bundles.

**Done when.** The agent-operations subject exists at `draft` at least, with its boundary sentences against fleet-orchestration and
concurrent-vcs written in prose; the software-engineering applications landed; the full gate green.

**The owner decides.** The bundle question (agent-operations against software-engineering's orchestration category) before the forge.

**Risk.** Duplicating `fleet-orchestration` (19 techniques). The spec's discriminator (a discipline practised, not a system built) is
the test the reviewer applies to every technique.

---

## Night 5: subscribe the consumers and conform

**Input.** The landed subjects; the jam and `third-life` as consumers.

**What subscribing a repo takes** (the three writes of `register-project.mjs`): a `projects.json` entry for this machine; the repo's
`.ai/manifest.yaml` with `knowledge.domains` (the only authority on which domains govern it) and `registry.local` (default
`../ai-registry`); and `.ai/registry-map.json`, built by `node scripts/build-registry-map.mjs --project <slug>` from the repo's
`context-map.json`. **`third-life` has a context map and an `.ai/` folder but no `.ai/manifest.yaml` today**: it needs the manifest
(domains `game-production`, `software-engineering`, `agent-operations`) and the map; registering it with `register-project.mjs`
writes both. The jam needs its context map first (night 1, option a).

**Commands.** `node scripts/register-project.mjs --slug third-life --path <machine root>/kiro/third-life --domains game-production,software-engineering,agent-operations --dry-run`,
then for real (registry clean, by path); in each consumer, `/conform` with a small budget on the contexts the new subjects govern
(`/conform --subject ball-possession-and-pickup`, `--subject contact-readability`, `--subject directed-parallel-lanes`); the verdicts are
committed in the consumer by path, as the skill says.

**Done when.** Both consumers carry a manifest and a map; each new subject has at least one verdict (conformant, deviation with its
reason, or not-applicable with its anchor) in a consumer; the inbox rows that came back as applications are struck through.

**The owner decides.** Which domains govern `third-life` (the engine, kit and Foreman are not a ball game: possession and contact may
be `not-applicable` there, which is a correct verdict); whether the jam, now finished, stays registered.

**Risk.** Conform run on a frozen jam repo can only record; nothing will be fixed there. `third-life` is shared with other sessions:
claim a scope in its `.claude/sessions.json` before writing its `.ai/` files.

---

## The first command of night 1

```
git -C C:/Users/kazda/kiro/ai-registry status --short --branch
```

Read only. It shows the other session's uncommitted paths and how far the checkout is behind its remote; nothing else on night 1 runs
until those paths are settled by the session that owns them.
