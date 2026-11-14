#!/usr/bin/env node
// Lane A157 (track F1; lane A132's driver, by way of A151): two arms of cells.mjs compared on the seeds both played. Appends one JSON line a cell to cells.jsonl and prints a table row.
//   node pair.mjs <armA> <armB> [label]
import { appendFileSync, readFileSync } from 'node:fs';
import { join, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';
const here = dirname(fileURLToPath(import.meta.url));
const [armA, armB, label = `${armA}-vs-${armB}`] = process.argv.slice(2);
const cells = [['rk', 'novice at rookie', 85], ['pro', 'regular at pro', 49], ['as', 'good at all_star', 30]];
const read = (arm, key) => readFileSync(join(here, 'raw', `${arm}-${key}.jsonl`), 'utf8').split(/\r?\n/).filter(Boolean).map((l) => JSON.parse(l));
const wilson = (k, m, z = 1.96) => { if (!m) return [0, 0]; const p = k / m, d = 1 + z * z / m, c = (p + z * z / (2 * m)) / d, h = z * Math.sqrt(p * (1 - p) / m + z * z / (4 * m * m)) / d; return [100 * (c - h), 100 * (c + h)]; };
const keys = ['check_presses', 'check_whiffs', 'checks_landed', 'checks_taken', 'knockdowns', 'knockdowns_all', 'stumbles', 'lifts', 'pokes_clean', 'fouls', 'fouls_from_behind', 'box_frames', 'faceoffs', 'faceoffs_without_touch'];
const stat = (ms) => {
  const N = Math.max(1, ms.length), W = ms.filter((m) => m.result === 'win').length, [lo, hi] = wilson(W, ms.length);
  const o = { n: ms.length, win: +(100 * W / N).toFixed(1), lo: +lo.toFixed(1), hi: +hi.toFixed(1), gf: +(ms.reduce((s, m) => s + (m.goals?.[0] ?? 0), 0) / N).toFixed(2), ga: +(ms.reduce((s, m) => s + (m.goals?.[1] ?? 0), 0) / N).toFixed(2) };
  for (const k of keys) o[k] = +(ms.reduce((s, m) => s + (Number(m[k]) || 0), 0) / N).toFixed(2);
  return o;
};
for (const [key, cell, target] of cells) {
  const a = read(armA, key), b = read(armB, key);
  const bs = new Map(b.map((m) => [m.seed, m]));
  let both = 0, aOnly = 0, bOnly = 0, identical = 0;
  for (const m of a) { const o = bs.get(m.seed); if (!o) continue; ++both; const wa = m.result === 'win', wb = o.result === 'win'; if (wa && !wb) ++aOnly; if (!wa && wb) ++bOnly; if (m.hash && m.hash === o.hash) ++identical; }
  const A = stat(a), B = stat(b);
  const row = { label, cell, target, a: armA, b: armB, [armA]: A, [armB]: B, delta_points_b_minus_a: +(B.win - A.win).toFixed(1), paired: { seeds: both, a_wins_only: aOnly, b_wins_only: bOnly, identical_hash: identical }, at: new Date().toTimeString().slice(0, 5) };
  appendFileSync(join(here, 'cells.jsonl'), JSON.stringify(row) + '\n');
  console.log(`${cell}: ${armA} ${A.win} (${A.lo}-${A.hi}) | ${armB} ${B.win} (${B.lo}-${B.hi}) | delta ${row.delta_points_b_minus_a} | flips ${aOnly}/${bOnly} identical ${identical}/${both} | presses ${A.check_presses}/${B.check_presses} whiffs ${A.check_whiffs}/${B.check_whiffs} = ${(100*A.check_whiffs/Math.max(1e-9,A.check_presses)).toFixed(1)} %/${(100*B.check_whiffs/Math.max(1e-9,B.check_presses)).toFixed(1)} % | checks landed ${A.checks_landed}/${B.checks_landed} human down ${A.knockdowns}/${B.knockdowns} by check ${A.checks_taken}/${B.checks_taken} all down ${A.knockdowns_all}/${B.knockdowns_all} goals ${A.gf}:${A.ga} / ${B.gf}:${B.ga}`);
}
