// Lane A19: apply the operator's exported decisions on the bark bank.
//   node tools/review/apply-barks.mjs --decisions <decisions.json> [--dry-run] [--bank <barks.json>] [--review <REVIEW.md>] [--no-embed]
// Removes every rejected line from barks.json (counts follow), appends the comments to REVIEW.md, re-runs the content filter on what is left,
// checks every combination still has 12 lines, then rebuilds the embedded copy (tools/barks/embed.mjs). Exit 0 = applied and clean, 1 = applied
// or planned but the bank needs attention (a filter failure or a short combination), 2 = could not run.
import { readFileSync, writeFileSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { checkLine, mildCount, MILD_QUOTA } from '../barks/filter.mjs';
import { embed } from '../barks/embed.mjs';

const here = dirname(fileURLToPath(import.meta.url));
const repo = resolve(here, '..', '..');
export const MIN_PER_COMBO = 12;
const VERDICTS = new Set(['pass', 'reject']);

export function parseDecisions(text) {
  let j;
  try { j = JSON.parse(text); } catch (e) { throw new Error(`decisions file is not JSON: ${e.message}`); }
  const list = Array.isArray(j) ? j : j && Array.isArray(j.decisions) ? j.decisions : null;
  if (!list) throw new Error('decisions JSON needs a "decisions" array');
  const seen = new Set();
  for (const d of list) {
    if (!d || typeof d.id !== 'string') throw new Error(`decision without an id: ${JSON.stringify(d)}`);
    if (d.verdict !== undefined && !VERDICTS.has(d.verdict)) throw new Error(`decision ${d.id}: verdict must be pass or reject`);
    if (d.comment !== undefined && typeof d.comment !== 'string') throw new Error(`decision ${d.id}: comment must be a string`);
    if (seen.has(d.id)) throw new Error(`decision ${d.id} appears twice`);
    seen.add(d.id);
  }
  return { title: typeof j.title === 'string' ? j.title : 'Bark Bank Review', decided_at: typeof j.decided_at === 'string' ? j.decided_at : undefined, decisions: list };
}

// Pure core: returns the new bank, what changed and the problems found; touches no file.
export function planApply(bank, { decisions }) {
  const byId = new Map(bank.lines.map((l) => [l.id, l]));
  const unknown = decisions.filter((d) => !byId.has(d.id)).map((d) => d.id);
  const rejected = decisions.filter((d) => d.verdict === 'reject' && byId.has(d.id)).map((d) => byId.get(d.id));
  const rejectedIds = new Set(rejected.map((l) => l.id));
  const lines = bank.lines.filter((l) => !rejectedIds.has(l.id));
  const counts = {};
  for (const l of lines) { const k = `${l.situation}.${l.speaker}.${l.tone}`; counts[k] = (counts[k] ?? 0) + 1; }
  const next = { ...bank, counts, lines };
  const comments = decisions.filter((d) => d.comment && byId.has(d.id)).map((d) => ({ id: d.id, verdict: d.verdict, text: byId.get(d.id).text, comment: d.comment }));
  const problems = [];
  for (const l of lines) { const r = checkLine(l.text); if (r.length) problems.push(`filter ${l.id} ${JSON.stringify(l.text)}: ${r.join('; ')}`); }
  const mild = lines.filter((l) => mildCount(l.text) > 0).length;
  if (lines.length && mild / lines.length > MILD_QUOTA) problems.push(`mild quota exceeded: ${mild} of ${lines.length}`);
  const short = {};
  for (const [k, n] of Object.entries(bank.counts ?? {})) if (!(k in counts)) counts[k] = 0;
  for (const [k, n] of Object.entries(counts)) if (n < MIN_PER_COMBO) short[k] = n;
  for (const [k, n] of Object.entries(short)) problems.push(`short combination ${k}: ${n} lines (< ${MIN_PER_COMBO}); top up with: node tools/barks/generate.mjs --keep --only <situation>`);
  next.counts = Object.fromEntries(Object.entries(counts).filter(([, n]) => n > 0).sort(([a], [b]) => (a < b ? -1 : 1)));
  return { next, rejected, comments, unknown, problems, passed: decisions.filter((d) => d.verdict === 'pass' && byId.has(d.id)).length, mild };
}

export function reviewSection(parsed, plan, date) {
  const L = ['', `## Operator review "${parsed.title}" (${parsed.decided_at ?? date})`, '',
    `${plan.rejected.length} rejected and removed, ${plan.passed} passed, ${plan.comments.length} commented; ${plan.next.lines.length} lines remain.`, ''];
  if (plan.rejected.length) {
    L.push('Removed:');
    for (const l of plan.rejected) L.push(`- ${l.id}: ${l.text}`);
    L.push('');
  }
  if (plan.comments.length) {
    L.push('Comments:');
    for (const c of plan.comments) L.push(`- ${c.id} (${c.verdict ?? 'no verdict'}) ${c.text} -- ${c.comment}`);
    L.push('');
  }
  return L.join('\n');
}

export function applyDecisions(parsed, { bankPath, reviewPath, dryRun = false, doEmbed = true, now = new Date() }) {
  const bank = JSON.parse(readFileSync(bankPath, 'utf8'));
  const plan = planApply(bank, parsed);
  if (!dryRun) {
    writeFileSync(bankPath, JSON.stringify(plan.next, null, 2) + '\n');
    let review = '';
    try { review = readFileSync(reviewPath, 'utf8'); } catch { /* a missing review file is created */ }
    if (plan.rejected.length || plan.comments.length) writeFileSync(reviewPath, review.replace(/\n*$/, '\n') + reviewSection(parsed, plan, now.toISOString()));
    if (doEmbed) embed();
  }
  return plan;
}

function parseArgs(argv) {
  const a = { dryRun: false, doEmbed: true };
  for (let i = 0; i < argv.length; i += 1) {
    const k = argv[i];
    if (k === '--dry-run') a.dryRun = true;
    else if (k === '--no-embed') a.doEmbed = false;
    else if (['--decisions', '--bank', '--review'].includes(k)) a[k.slice(2)] = argv[++i];
    else throw new Error(`unknown argument ${k}`);
  }
  return a;
}

if (process.argv[1] && /apply-barks\.mjs$/.test(process.argv[1].replace(/\\/g, '/'))) {
  try {
    const a = parseArgs(process.argv.slice(2));
    if (!a.decisions) throw new Error('usage: apply-barks.mjs --decisions <decisions.json> [--dry-run] [--bank f] [--review f] [--no-embed]');
    const parsed = parseDecisions(readFileSync(a.decisions, 'utf8'));
    const bankPath = resolve(repo, a.bank ?? 'Game/Content/Barks/barks.json');
    const reviewPath = resolve(repo, a.review ?? 'Game/Content/Barks/REVIEW.md');
    const before = JSON.parse(readFileSync(bankPath, 'utf8')).lines.length;
    const plan = applyDecisions(parsed, { bankPath, reviewPath, dryRun: a.dryRun, doEmbed: a.doEmbed });
    console.log(`${a.dryRun ? 'DRY-RUN' : 'PASS'} ${plan.rejected.length} lines removed (${before} -> ${plan.next.lines.length}), ${plan.passed} passed, ${plan.comments.length} comments ${a.dryRun ? 'would be ' : ''}appended to REVIEW.md`);
    for (const l of plan.rejected.slice(0, 60)) console.log(`  removed ${l.id}: ${l.text}`);
    if (plan.rejected.length > 60) console.log(`  ... and ${plan.rejected.length - 60} more`);
    for (const c of plan.comments) console.log(`  comment ${c.id}: ${c.comment}`);
    for (const u of plan.unknown) console.log(`WARN unknown id ${u}: not in the bank (already removed?)`);
    for (const p of plan.problems) console.log(`FAIL ${p}`);
    console.log(`${plan.problems.length ? 'FAIL' : 'PASS'} bank check: ${plan.next.lines.length} lines, ${Object.keys(plan.next.counts).length} combinations, ${plan.mild} mild`);
    console.log(JSON.stringify({ tool: 'thirdjam.review.apply-barks/1', ok: !plan.problems.length, dry_run: a.dryRun, removed: plan.rejected.length, remaining: plan.next.lines.length, comments: plan.comments.length, problems: plan.problems.length }));
    process.exit(plan.problems.length ? 1 : 0);
  } catch (e) {
    console.error(`FAIL ${e.message}`);
    process.exit(2);
  }
}
