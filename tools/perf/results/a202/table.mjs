#!/usr/bin/env node
// lane A202: a markdown table of the six cells for each prefix against B-* (win and difference; goals against a match)
import { readFileSync } from 'node:fs';
const rows = new Map(readFileSync(new URL('cells.jsonl', import.meta.url), 'utf8').trim().split('\n').map(JSON.parse).map((r) => [r.label, r]));
const cells = ['rk-reg', 'pro-reg', 'as-reg', 'rk-large', 'pro-large', 'as-large'];
console.log('| arm | ' + cells.join(' | ') + ' |'); console.log('|---|' + cells.map(() => '---').join('|') + '|');
for (const pre of ['B', ...process.argv.slice(2)]) console.log(`| ${pre} | ` + cells.map((c) => { const r = rows.get(`${pre}-${c}`), b = rows.get(`B-${c}`); if (!r) return 'n/a'; if (pre === 'B') return `${r.win} (ga ${r.ga})`; const d = r.win - b.win; return `${r.win} (${d >= 0 ? '+' : ''}${d.toFixed(1)}; ga ${r.ga})`; }).join(' | ') + ' |');
