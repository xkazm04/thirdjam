#!/usr/bin/env node
// lane A202: rows of cells.jsonl by prefix against the baseline B-*, with the bots' braced hits a match from raw/<label>.jsonl. node rows.mjs X4 X8 ...
import { readFileSync, existsSync } from 'node:fs';
const rows = new Map(readFileSync(new URL('cells.jsonl', import.meta.url), 'utf8').trim().split('\n').map(JSON.parse).map((r) => [r.label, r]));
const brace = (l) => { const p = new URL(`raw/${l}.jsonl`, import.meta.url); if (!existsSync(p)) return NaN; const ms = readFileSync(p, 'utf8').trim().split('\n').map(JSON.parse); return ms.reduce((a, m) => a + (Number(m.bot_brace_hits) || 0), 0) / ms.length; };
for (const pre of process.argv.slice(2)) for (const c of ['rk-reg', 'pro-reg', 'as-reg', 'rk-large', 'pro-large', 'as-large']) {
  const r = rows.get(`${pre}-${c}`), b = rows.get(`B-${c}`); if (!r) continue;
  console.log(`${pre}-${c}\tn ${r.n}\twin ${r.win} (${(r.win - b.win >= 0 ? '+' : '') + (r.win - b.win).toFixed(1)})\thuman kd ${r.knockdowns} (B ${b.knockdowns})\tbot braced hits ${brace(r.label).toFixed(2)} (B ${brace(b.label).toFixed(2)})\tbot checks ${r.bot_checks_landed} (B ${b.bot_checks_landed})\tbot cross ${r.bot_cross_landed}\tgf/ga ${r.gf}/${r.ga} (B ${b.gf}/${b.ga})\tshots opp ${r.shots_opp} (B ${b.shots_opp})`);
}
