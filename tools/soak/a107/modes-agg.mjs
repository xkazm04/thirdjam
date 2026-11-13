// node modes-agg.mjs <rawdir>: the character number of each mode against regular at the mode's level (A67's rule: differs by a third)
import { readFileSync, readdirSync, existsSync } from 'node:fs';
import { join } from 'node:path';
import { CHARACTER } from '../../playtest/modes.mjs';
const dir = process.argv[2];
const load = (l) => existsSync(join(dir, `${l}.done`)) ? readFileSync(join(dir, `${l}.jsonl`), 'utf8').split('\n').filter(Boolean).map((x) => JSON.parse(x)) : null;
const avg = (ms, k) => ms.reduce((a, m) => a + (Number(m[k]) || 0), 0) / Math.max(1, ms.length);
const rows = [];
for (const f of readdirSync(dir).filter((f) => /^m-.*\.done$/.test(f))) {
  const label = f.replace('.done', ''); if (label.startsWith('m-regular')) continue;
  let slug = label.slice(2); const sprint = slug.endsWith('-sprint'); const base = sprint ? slug.slice(0, -7) : slug;
  const ms = load(label); const lvl = ms[0].difficulty;
  const reg = load(sprint ? 'm-regular-sprint' : `m-regular-${lvl}`); if (!reg || !CHARACTER[base]) continue;
  const num = CHARACTER[base].of(ms), rnum = CHARACTER[base].of(reg);
  const W = (x) => +(100 * x.filter((m) => m.result === 'win').length / x.length).toFixed(1);
  rows.push({ mode: label.slice(2), level: lvl, n: ms.length, character: CHARACTER[base].label, mode_value: +num.toFixed(2), regular_value: +rnum.toFixed(2), ratio: +(num / (rnum || 1e-9)).toFixed(2),
    differs_by_third: rnum ? Math.abs(num / rnum - 1) >= 1 / 3 : num > 0, win_mode: W(ms), win_regular: W(reg), unfinished: ms.filter((m) => m.result !== 'win' && m.result !== 'loss' && m.result !== 'draw').length,
    draws: ms.filter((m) => m.result === 'draw').length, kd_human: +avg(ms, 'knockdowns').toFixed(2), kd_all: +avg(ms, 'knockdowns_all').toFixed(1), match_s: +(avg(ms, 'frames') / 60).toFixed(0), max_no_touch_s: Math.max(...ms.map((m) => m.max_no_touch || 0)) / 60 });
}
console.log(JSON.stringify(rows, null, 1));
