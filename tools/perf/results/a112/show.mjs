// lane A112: print cells.jsonl as a table (label n win (lo-hi) checks_taken gf:ga carry% opp shots noshot fouls tune)
import { readFileSync } from 'node:fs';
const want = process.argv[2] ? new RegExp(process.argv[2]) : null;
for (const l of readFileSync(new URL('./cells.jsonl', import.meta.url), 'utf8').trim().split('\n')) {
  const r = JSON.parse(l); if (want && !want.test(r.label)) continue;
  console.log([r.label, r.n, `${r.win} (${r.lo}-${r.hi})`, `ct ${r.checks_taken}`, `${r.gf}:${r.ga}`, `carry ${r.carry_pct}`, `opp ${r.shots_opp}`, `noshot ${r.noshot}`, `fouls ${r.fouls}`, r.tune, r.set].join(' | '));
}
