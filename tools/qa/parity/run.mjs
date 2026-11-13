#!/usr/bin/env node
// Lane A57: one parity run of one seed in one packaged build. Starts the exe through tools/qa/eyesgrid/shoot.ps1 (A54's driver: PostMessage keys when a foreign
// dialog holds the foreground, PrintWindow stills of the client area), presses ENTER at the title card, takes a still every --every seconds.
//   node tools/qa/parity/run.mjs --pkg <...\Windows> --kind dev|ship --seed 1 --out <dir> [--args "<extra game args>"] [--seconds 110] [--every 2]
// Dev: -log -abslog=<out>/game.log and -dpcvars=jam.ScriptHuman=1 (kept in --args by the caller). Ship: no log exists.
import { spawnSync, spawn } from 'node:child_process';
import { existsSync, mkdirSync, writeFileSync, copyFileSync } from 'node:fs';
import { join, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
const here = dirname(fileURLToPath(import.meta.url));
const argv = process.argv.slice(2);
const opt = (k, d) => { const i = argv.indexOf('--' + k); return i >= 0 ? argv[i + 1] : d; };
const pkg = resolve(opt('pkg')), kind = opt('kind', 'ship'), seed = opt('seed', '1'), out = resolve(opt('out'));
const seconds = Number(opt('seconds', 110)), every = Number(opt('every', 2)), extra = opt('args', '');
const enterAt = Number(opt('enter', kind === 'dev' ? 18 : 12));
const cutDir = join(here, 'cuts');
if (!existsSync(join(cutDir, `par.s${seed}.cartridge.json`))) spawnSync('python', [join(here, 'make-cuts.py'), cutDir], { stdio: 'inherit' });
const cut = join(cutDir, `par.s${seed}.cartridge.json`);
const sh = join(pkg, 'ThirdJam', 'Binaries', 'Win64', 'ThirdJam-Win64-Shipping.exe');
const exe = existsSync(sh) ? sh : join(pkg, 'ThirdJam', 'Binaries', 'Win64', 'ThirdJam.exe');
mkdirSync(out, { recursive: true });
const steps = [{ at: enterAt, keys: '{ENTER}' }];
for (let t = enterAt + 2, i = 0; t < seconds - 2; t += every, i++) steps.push({ at: t, shot: `t${String(Math.round((t - enterAt) * 10)).padStart(4, '0')}` });
const pf = join(out, 'plan.json');
writeFileSync(pf, JSON.stringify({ seconds, steps }));
const log = kind === 'dev' ? join(out, 'game.log') : '';
const args = ['-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', join(here, '..', 'eyesgrid', 'shoot.ps1'), '-Exe', exe, '-PlanFile', pf, '-OutDir', out,
  '-GameArgs', `-cut=${cut} ${extra}`.trim(), '-ResX', '1280', '-ResY', '720'];
if (log) args.push('-Log', log);
if (opt('park', '1') !== '0') spawn('powershell', ['-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', join(here, 'park.ps1'), '-Exe', exe, '-MaxSec', String(seconds + 10)], { stdio: 'ignore', detached: false });
const r = spawnSync('powershell', args, { stdio: 'inherit' });
// Development: -abslog is sometimes ignored (the log then lands in the package's own Saved\Logs\ThirdJam.log); keep the run's log as game.log either way
if (kind === 'dev' && !existsSync(join(out, 'game.log'))) {
  const fb = join(pkg, 'ThirdJam', 'Saved', 'Logs', 'ThirdJam.log');
  if (existsSync(fb)) copyFileSync(fb, join(out, 'game.log'));
}
console.log(JSON.stringify({ tool: 'a57.parity.run', kind, seed, out, exit: r.status }));
