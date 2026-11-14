#!/usr/bin/env node
// lane A203: two arms on the seeds they share (paired): the scripted human's win rate, the paired delta with its standard error, the flips, and per match the shots and
// goals of the human, his bot mates and the bot opponents (shots_mates, shots_opp; goals[0] - goals_by_human, goals[1]).   node pair.mjs <off> <on>
import { readFileSync } from 'node:fs';
const load = (l) => new Map(readFileSync(new URL(`raw/${l}.jsonl`, import.meta.url), 'utf8').trim().split('\n').map(JSON.parse).map((m) => [m.seed, m]));
const [la, lb] = process.argv.slice(2); const a = load(la), b = load(lb);
const seeds = [...a.keys()].filter((s) => b.has(s)); const n = seeds.length;
const w = (m) => (m.result === 'win' ? 1 : 0);
let wa = 0, wb = 0, up = 0, down = 0; const d = [];
for (const s of seeds) { const x = w(a.get(s)), y = w(b.get(s)); wa += x; wb += y; if (y > x) up++; if (y < x) down++; d.push(y - x); }
const mean = d.reduce((p, q) => p + q, 0) / n; const sd = Math.sqrt(d.reduce((p, q) => p + (q - mean) ** 2, 0) / Math.max(1, n - 1));
const per = (M, f) => +(seeds.reduce((p, s) => p + f(M.get(s)), 0) / n).toFixed(2);
const cols = { shots_human: (m) => +m.shots || 0, shots_mates: (m) => +m.shots_mates || 0, shots_opp: (m) => +m.shots_opp || 0,
  goals_human: (m) => +m.goals_by_human || 0, goals_mates: (m) => (m.goals?.[0] ?? 0) - (+m.goals_by_human || 0), goals_opp: (m) => m.goals?.[1] ?? 0 };
const row = { off: la, on: lb, n, win_off: +(100 * wa / n).toFixed(1), win_on: +(100 * wb / n).toFixed(1), delta: +(100 * mean).toFixed(1), se: +(100 * sd / Math.sqrt(n)).toFixed(1), flips_up: up, flips_down: down };
for (const [k, f] of Object.entries(cols)) { row[`${k}_off`] = per(a, f); row[`${k}_on`] = per(b, f); }
console.log(JSON.stringify(row));
