#!/usr/bin/env node
// Lane A177 (track F1; lane A157's, with -aitune): single-cell arms, three headless JamTournament processes at a time (never more), for sweeps where one cell answers the question
// (the masher against the timed press at pro). Raw lines to raw/<arm>-<cell>.jsonl, the same files cells.mjs writes, so pair.mjs and read.mjs read them.
//   node arms1.mjs <n> "<arm>|<rk|pro|as>|<set or ->|ENV=value,ENV=value" ...
// Waits while the machine has more than 50 GB in use. Never kills a process it did not start.
import { spawn, execSync } from 'node:child_process';
import { mkdirSync, writeFileSync } from 'node:fs';
import { join, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { writeVariantShort } from './variant.mjs';   // lane A177: a short file name (the shared helper's passes 260 characters with six keys)

const here = dirname(fileURLToPath(import.meta.url));
const repo = resolve(here, '..', '..', '..', '..');
const uproject = join(repo, 'Game', 'ThirdJam.uproject');
const exe = 'C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe';
const [nArg, ...specs] = process.argv.slice(2);
const n = Number(nArg);
const cells = { rk: ['novice', 'rookie'], pro: ['regular', 'pro'], as: ['good', 'all_star'] };
mkdirSync(join(here, 'raw'), { recursive: true });
const usedGb = () => Number(execSync('powershell -NoProfile -Command "$o=Get-CimInstance Win32_OperatingSystem; [math]::Round(($o.TotalVisibleMemorySize-$o.FreePhysicalMemory)/1MB,1)"').toString().trim());
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
function run(spec) {
  const [arm, key, set = '-', envs = ''] = spec.split('|');
  const [cls, lvl] = cells[key];
  const cart = set && set !== '-' ? writeVariantShort('hockey.regular', set, `${arm}-${key}`) : 'hockey.regular';
  const env = { ...process.env };
  let tune = '';   // lane A177: TUNE=a=1;b=2 among the ENV list is the commandlet's -aitune
  for (const e of envs.split(',').filter(Boolean)) { const i = e.indexOf('='); if (i <= 0) continue; if (e.slice(0, i) === 'TUNE') tune = e.slice(i + 1); else env[e.slice(0, i)] = e.slice(i + 1); }
  const t0 = Date.now();
  return new Promise((res) => {
    const args = [uproject, '-run=JamTournament', '-sim=jam', `-cartridge=${cart}`, `-difficulty=${lvl}`, `-player=${cls}`, `-seeds=1-${n}`, '-maxframes=36000',
      '-nullrhi', '-Multiprocess', '-unattended', '-nopause', '-nosplash', '-stdout', '-FullStdOutLogOutput'];
    if (tune) args.push(`-aitune=${tune}`);
    const p = spawn(exe, args, { windowsHide: true, env });
    let buf = '';
    p.stdout.on('data', (d) => { buf += d; });
    p.stderr.on('data', () => {});
    p.on('close', () => {
      const lines = [];
      for (const l of buf.split(/\r?\n/)) { const i = l.indexOf('{"tool":"thirdjam.playtest/1"'); if (i >= 0) { try { lines.push(JSON.parse(l.slice(i))); } catch {} } }
      if (/FAIL tournament/.test(buf)) console.error(buf.split(/\r?\n/).filter((l) => /FAIL/.test(l)).join('\n'));
      writeFileSync(join(here, 'raw', `${arm}-${key}.jsonl`), lines.map((m) => JSON.stringify(m)).join('\n') + '\n');
      const W = lines.filter((m) => m.result === 'win').length;
      const avg = (k) => (lines.reduce((s, m) => s + (Number(m[k]) || 0), 0) / Math.max(1, lines.length)).toFixed(2);
      console.log(`${arm} ${cls} at ${lvl}: ${lines.length} matches, win ${(100 * W / Math.max(1, lines.length)).toFixed(1)} % | presses ${avg('check_presses')} dropped ${avg('checks_cooling')} whiffs ${avg('check_whiffs')} landed ${avg('checks_landed')} lunges missed ${avg('lunges_missed')} | taken ${avg('checks_taken')} (${Math.round((Date.now() - t0) / 1000)} s)`);
      res();
    });
  });
}
for (let i = 0; i < specs.length; i += 3) {
  for (;;) { const u = usedGb(); if (u < 50) break; console.log(`memory ${u} GB in use: waiting`); await sleep(20000); }
  await Promise.all(specs.slice(i, i + 3).map(run));
}
console.log('arms1 done ' + new Date().toTimeString().slice(0, 8));
