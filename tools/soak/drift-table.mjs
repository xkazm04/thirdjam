#!/usr/bin/env node
// Lane A174: the drift table by family. node tools/soak/drift-table.mjs <baseline.json> <drift out dir>
// Prints a markdown table per family (modes, then each take) of baseline vs this run: bit-identical or DRIFT, win %, mean frames, knockdowns of the human,
// goals for-against (when the baseline has them), unfinished. The out dir is a drift.mjs run folder (it holds plan.json).
import { readFileSync } from 'node:fs';
import { join } from 'node:path';
import { measureDir } from './drift.mjs';

const [bf, dir] = process.argv.slice(2);
const base = JSON.parse(readFileSync(bf, 'utf8')).cells;
const plan = JSON.parse(readFileSync(join(dir, 'plan.json'), 'utf8'));
const now = measureDir(dir, plan);
const f = (a, b, d = 0) => (a === undefined ? `- > ${+(+b).toFixed(d)}` : `${+(+a).toFixed(d)} > ${+(+b).toFixed(d)}`);
const row = (l) => {
  const b = base[l], n = now[l];
  if (!b) return `| ${l} | NEW | | | | | |`;
  const goals = b.gf === undefined ? `${n.gf}-${n.ga} now` : `${b.gf}-${b.ga} > ${n.gf}-${n.ga}`;
  return `| ${l} | ${b.digest === n.digest ? 'identical' : 'DRIFT'} | ${f(b.win, n.win)} | ${f(b.frames, n.frames)} | ${f(b.kd, n.kd, 1)} | ${goals} | ${f(b.unfinished, n.unfinished)} |`;
};
const head = '| cell | digest | win % | frames | kd of human | goals for-against | unfinished |\n|---|---|---|---|---|---|---|';
const fam = (t, p) => { console.log(`\n### ${t}\n${head}`); for (const c of plan.filter(p)) console.log(row(c.label)); };
fam('regular cuts (r-)', (c) => c.label.startsWith('r-'));
fam('arenas (a-, pro)', (c) => c.label.startsWith('a-'));
fam('modes (regular class at the mode level)', (c) => c.label.startsWith('m-'));
for (let k = 1; k <= 8; k++) fam(`take ${k}`, (c) => c.label.startsWith(`t-take-${k}-`));
const ids = plan.filter((c) => base[c.label]);
const same = ids.filter((c) => base[c.label].digest === now[c.label].digest).length;
console.log(`\nbit-identical ${same} of ${ids.length}`);
