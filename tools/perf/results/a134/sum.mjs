#!/usr/bin/env node
// Lane A134: one line per label from tools/perf/results/a95/raw/<label>.jsonl (the playtest lines of one cell).
import { readFileSync } from 'node:fs';
import { join, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';
const here = dirname(fileURLToPath(import.meta.url));
const [sLo, sHi] = (process.env.SEEDS || "0-999999999").split("-").map(Number);   // SEEDS=a-b keeps only those seeds (a paired subset of a bigger cell)
for (const L of process.argv.slice(2)) {
  const ms = readFileSync(join(here, '..', 'a95', 'raw', `${L}.jsonl`), 'utf8').trim().split(/\n/).map((l) => JSON.parse(l)).filter((m) => +m.seed >= sLo && +m.seed <= sHi);
  const n = ms.length;
  const s = ms.map((m) => +m.max_no_touch / 60).sort((a, b) => a - b);
  const mean = (k, d = 1) => (ms.reduce((a, m) => a + (+m[k] || 0), 0) / n).toFixed(d);
  const pct = (f) => (100 * ms.filter(f).length / n).toFixed(1);
  const g = (i) => (ms.reduce((a, m) => a + (m.goals?.[i] ?? 0), 0) / n).toFixed(2);
  console.log(`${L} n=${n} idle30=${pct((m) => +m.max_no_touch >= 1800)}% p50=${s[n >> 1].toFixed(0)}s p90=${s[Math.floor(n * 0.9)].toFixed(0)}s win=${pct((m) => m.result === 'win')}% goals=${g(0)}:${g(1)} `
    + `touches/min=${mean('touches_per_min')} passes_received=${mean('passes_received')} passes=${mean('passes')} pokes_clean=${mean('pokes_clean')} pokes=${mean('pokes')} lifts=${mean('lifts')} checks_landed=${mean('checks_landed')} `
    + `knockdowns=${mean('knockdowns')} shots=${mean('shots')} shots_mates=${mean('shots_mates')} shots_opp=${mean('shots_opp')} bot_rev_brakes=${mean('bot_rev_brakes', 0)} frames=${mean('frames', 0)}`);
}
