#!/usr/bin/env node
// Lane A101 (track F1): the three target cells of R-0046 on hockey.regular, game.contact.board_bonus at its default against a second value, PAIRED: the same
// seeds in both arms, ONE headless JamTournament process per cell arm (every UnrealEditor-Cmd start queues behind the UBT mutex, so few processes, long ranges).
//   node paired.mjs <label> <n> [set]          set default "contact.board_bonus=1" (the neutral value); seeds 1..n
// Appends one JSON line per cell to tools/perf/results/a101/cells.jsonl and the raw match lines to raw/<label>-<cell>-<arm>.jsonl. Never kills a process it did not start.
import { spawn } from 'node:child_process';
import { appendFileSync, mkdirSync, writeFileSync } from 'node:fs';
import { join, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { writeVariant } from '../../../playtest/run.mjs';

const here = dirname(fileURLToPath(import.meta.url));
const repo = resolve(here, '..', '..', '..', '..');
const uproject = join(repo, 'Game', 'ThirdJam.uproject');
const exe = 'C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe';
const [label = 'paired', nArg = '600', set = 'contact.board_bonus=1'] = process.argv.slice(2);
const n = Number(nArg);
const cells = [['rk', 'novice', 'rookie', 85], ['pro', 'regular', 'pro', 50], ['as', 'good', 'all_star', 25]];
const arms = [['default', 'hockey.regular'], ['set', writeVariant('hockey.regular', set)]];
mkdirSync(join(here, 'raw'), { recursive: true });

function run(cart, cls, lvl) {
  return new Promise((res) => {
    const args = [uproject, '-run=JamTournament', '-sim=jam', `-cartridge=${cart}`, `-difficulty=${lvl}`, `-player=${cls}`, `-seeds=1-${n}`, '-maxframes=36000',
      '-nullrhi', '-Multiprocess', '-unattended', '-nopause', '-nosplash', '-stdout', '-FullStdOutLogOutput'];
    const p = spawn(exe, args, { windowsHide: true });
    let buf = '';
    p.stdout.on('data', (d) => { buf += d; });
    p.stderr.on('data', () => {});
    p.on('close', () => {
      const lines = [];
      for (const l of buf.split(/\r?\n/)) { const i = l.indexOf('{"tool":"thirdjam.playtest/1"'); if (i >= 0) { try { lines.push(JSON.parse(l.slice(i))); } catch {} } }
      if (/FAIL tournament/.test(buf)) console.error(buf.split(/\r?\n/).filter((l) => /FAIL/.test(l)).join('\n'));
      res(lines);
    });
  });
}
const wilson = (k, m, z = 1.96) => { if (!m) return [0, 0]; const p = k / m, d = 1 + z * z / m, c = (p + z * z / (2 * m)) / d, h = z * Math.sqrt(p * (1 - p) / m + z * z / (4 * m * m)) / d; return [100 * (c - h), 100 * (c + h)]; };
const t0 = Date.now();
const jobs = [];
for (const [key, cls, lvl, target] of cells) for (const [arm, cart] of arms) jobs.push(run(cart, cls, lvl).then((ms) => ({ key, cls, lvl, target, arm, ms })));
const done = await Promise.all(jobs);
for (const [key, cls, lvl, target] of cells) {
  const a = done.find((d) => d.key === key && d.arm === 'default').ms, b = done.find((d) => d.key === key && d.arm === 'set').ms;
  writeFileSync(join(here, 'raw', `${label}-${key}-default.jsonl`), a.map((m) => JSON.stringify(m)).join('\n') + '\n');
  writeFileSync(join(here, 'raw', `${label}-${key}-set.jsonl`), b.map((m) => JSON.stringify(m)).join('\n') + '\n');
  const stat = (ms) => {
    const N = ms.length, W = ms.filter((m) => m.result === 'win').length, [lo, hi] = wilson(W, N);
    const sum = (k) => ms.reduce((s, m) => s + (Number(m[k]) || 0), 0);
    return { n: N, win: +(100 * W / Math.max(1, N)).toFixed(1), lo: +lo.toFixed(1), hi: +hi.toFixed(1), checks_taken: +(sum('checks_taken') / Math.max(1, N)).toFixed(2),
      knockdowns: +(sum('knockdowns') / Math.max(1, N)).toFixed(2), gf: +(ms.reduce((s, m) => s + (m.goals?.[0] ?? 0), 0) / Math.max(1, N)).toFixed(2),
      ga: +(ms.reduce((s, m) => s + (m.goals?.[1] ?? 0), 0) / Math.max(1, N)).toFixed(2), match_s: +(sum('frames') / Math.max(1, N) / 60).toFixed(0) };
  };
  // paired: the seeds both arms played; how many flipped each way
  const bs = new Map(b.map((m) => [m.seed, m]));
  let both = 0, same = 0, toWin = 0, toLoss = 0, identical = 0;
  for (const m of a) {
    const o = bs.get(m.seed);
    if (!o) continue;
    ++both;
    const wa = m.result === 'win', wb = o.result === 'win';
    if (wa === wb) ++same; else if (wa) ++toWin; else ++toLoss;
    if (m.hash && m.hash === o.hash) ++identical;
  }
  const A = stat(a), B = stat(b);
  const row = { label, cell: `${cls} at ${lvl}`, target, set, default: A, with_set: B, delta_points: +(A.win - B.win).toFixed(1), paired: { seeds: both, same_result: same, default_wins_only: toWin, set_wins_only: toLoss, identical_hash: identical },
    secs: Math.round((Date.now() - t0) / 1000), at: new Date().toTimeString().slice(0, 5) };
  appendFileSync(join(here, 'cells.jsonl'), JSON.stringify(row) + '\n');
  console.log(JSON.stringify(row));
}
