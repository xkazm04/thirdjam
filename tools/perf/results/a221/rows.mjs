#!/usr/bin/env node
// lane A221: rows of cells.jsonl as a table (latest row per label; label filter by substring): node rows.mjs [filter] [--gains]
import { readFileSync } from 'node:fs';
const f = process.argv[2] && !process.argv[2].startsWith('--') ? process.argv[2] : '';
const rows = new Map(); for (const l of readFileSync(new URL('cells.jsonl', import.meta.url), 'utf8').trim().split('\n')) { const r = JSON.parse(l); if (r.label.includes(f)) rows.set(r.label, r); }
if (process.argv.includes('--gains')) {
  console.log('| cell | n | opp shots a match | his gains a match: opp shot <6 / 6-18 / >=18 m/s (inside 1.2 m) | off an opp carrier | loose after an opp touch (inside 1.2 m) | after a mate | 6-18 blocks per goal against |');
  console.log('|---|---|---|---|---|---|---|---|');
  for (const r of rows.values()) console.log(`| ${r.label} | ${r.n} | ${r.opp_shots} | ${r.gain_shot.join(' / ')} (${r.gain_shot_ring.join(' / ')}) | ${r.gain_opp_carry} | ${r.gain_opp_loose} (${r.gain_opp_loose_ring}) | ${r.gain_mate} | ${r.ga ? (r.gain_shot[1] / r.ga).toFixed(2) : '-'} |`);
} else {
  console.log('| cell | tune | n | win % (95 %) | goals for : against | his shots | length median / p80 s | his passes first touched by a mate % | knocked down a minute | on his back % | GA inside f600 (matches) | GA from >=28 m % | open |');
  console.log('|---|---|---|---|---|---|---|---|---|---|---|---|---|');
  for (const r of rows.values()) console.log(`| ${r.label} | ${r.tune || '-'}${r.set ? ' ' + r.set : ''} | ${r.n} | **${r.win}** (${r.lo}-${r.hi}) | ${r.gf} : ${r.ga} | ${r.shots} | ${r.len_med_s} / ${r.len_p80_s} | ${r.pass_mate_pct} | ${r.kd_min} | ${r.down_pct} | ${r.ga600_matches} | ${r.ga_ge28_pct} | ${r.open} |`);
}
