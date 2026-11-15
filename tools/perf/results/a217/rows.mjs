#!/usr/bin/env node
// lane A217: the six cells by both instruments from cells.jsonl (latest row per label): node rows.mjs [buttonsPrefix=B] [verbsPrefix=V]
import { readFileSync } from 'node:fs';
const [bp = 'B', vp = 'V'] = process.argv.slice(2);
const rows = new Map(); for (const l of readFileSync(new URL('cells.jsonl', import.meta.url), 'utf8').trim().split('\n')) { const r = JSON.parse(l); rows.set(r.label, r); }
const tgt = { rk: 85, pro: 48, as: 26 };
console.log('| cell (target) | verbs (n) | buttons (n) | gap | buttons: GA a match, GA by shot distance <10/10-20/20-28/>=28/no shot % | first GA median s | matches with a GA in f600 | on his back % | his passes to a mate % (to an opponent %) | open |');
console.log('|---|---|---|---|---|---|---|---|---|---|');
for (const lv of ['rk', 'pro', 'as']) for (const rk of ['reg', 'large']) {
  const b = rows.get(`${bp}-${lv}-${rk}`), v = rows.get(`${vp}-${lv}-${rk}`);
  const f = (r) => r ? `${r.win} (${r.n})` : '-';
  const d = b?.ga_dist_pct;
  console.log(`| ${lv} ${rk} (${tgt[lv]}) | ${f(v)} | ${f(b)} | ${b && v ? (b.win - v.win).toFixed(1) : '-'} | ${b ? `${b.ga}: ${d.lt10}/${d['10_20']}/${d['20_28']}/${d.ge28}/${d.none}` : '-'} | ${b?.first_ga_median_s ?? '-'} | ${b ? `${b.ga600_matches} of ${b.n}` : '-'} | ${b?.down_pct ?? '-'} | ${b ? `${b.pass_mate_pct} (${b.pass_opp_pct})` : '-'} | ${b?.open ?? '-'} |`);
}
