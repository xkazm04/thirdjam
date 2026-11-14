#!/usr/bin/env node
// Lane A184 (copy of A177's, records every editor pid in pids.txt). Lane A177 (track F1; lane A157's, with -aitune; lane A132's driver, by way of A151): the three target cells of R-0046 on hockey.regular for ONE arm: seeds 1..n, one headless JamTournament process per cell (three at once,
// never more: the common brief's rule), the raw match lines to raw/<arm>-<cell>.jsonl. Arms are compared afterwards by pair.mjs (the same seeds, so paired).
//   node cells.mjs <arm> <n> [set] [ENV=value ...]        set = "contact.check_reach_m=1;..." (a cartridge variant) or "-" for the cartridge as it is
// Never kills a process it did not start.
import { spawn } from 'node:child_process';
import { mkdirSync, writeFileSync, appendFileSync } from 'node:fs';
import { join, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { writeVariantShort } from './variant.mjs';   // lane A177: a short file name (the shared helper's passes 260 characters with six keys)

const here = dirname(fileURLToPath(import.meta.url));
const repo = resolve(here, '..', '..', '..', '..');
const uproject = join(repo, 'Game', 'ThirdJam.uproject');
const exe = 'C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe';
const [arm = 'default', nArg = '600', set = '-', ...envs] = process.argv.slice(2);
const n = Number(nArg);
const cells = [['rk', 'novice', 'rookie'], ['pro', 'regular', 'pro'], ['as', 'good', 'all_star']];
const cart = set && set !== '-' ? writeVariantShort('hockey.regular', set, arm) : 'hockey.regular';
const env = { ...process.env };
let tune = '';   // lane A177: TUNE=a=1;b=2 among the ENV arguments is the commandlet's -aitune
for (const e of envs) { const i = e.indexOf('='); if (i <= 0) continue; if (e.slice(0, i) === 'TUNE') tune = e.slice(i + 1); else env[e.slice(0, i)] = e.slice(i + 1); }
mkdirSync(join(here, 'raw'), { recursive: true });
const t0 = Date.now();
function run([key, cls, lvl]) {
  return new Promise((res) => {
    const args = [uproject, '-run=JamTournament', '-sim=jam', `-cartridge=${cart}`, `-difficulty=${lvl}`, `-player=${cls}`, `-seeds=1-${n}`, '-maxframes=36000',
      '-nullrhi', '-Multiprocess', '-unattended', '-nopause', '-nosplash', '-stdout', '-FullStdOutLogOutput'];
    if (tune) args.push(`-aitune=${tune}`);
    const p = spawn(exe, args, { windowsHide: true, env });
    appendFileSync(join(here, 'pids.txt'), `${new Date().toTimeString().slice(0, 8)} ${arm}-${key} pid ${p.pid} (node ${process.pid})
`);   // lane A184: the ids I may stop
    let buf = '';
    p.stdout.on('data', (d) => { buf += d; });
    p.stderr.on('data', () => {});
    p.on('close', () => {
      const lines = [];
      for (const l of buf.split(/\r?\n/)) { const i = l.indexOf('{"tool":"thirdjam.playtest/1"'); if (i >= 0) { try { lines.push(JSON.parse(l.slice(i))); } catch {} } }
      if (/FAIL tournament/.test(buf)) console.error(buf.split(/\r?\n/).filter((l) => /FAIL/.test(l)).join('\n'));
      writeFileSync(join(here, 'raw', `${arm}-${key}.jsonl`), lines.map((m) => JSON.stringify(m)).join('\n') + '\n');
      const W = lines.filter((m) => m.result === 'win').length;
      console.log(`${arm} ${cls} at ${lvl}: ${lines.length} matches, win ${(100 * W / Math.max(1, lines.length)).toFixed(1)} % (${Math.round((Date.now() - t0) / 1000)} s)`);
      res();
    });
  });
}
await Promise.all(cells.map(run));
