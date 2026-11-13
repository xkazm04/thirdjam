#!/usr/bin/env node
// Lane A126 (track S1 soak, evolution 3): the drift gate as a premerge step. premerge.ps1 -Drift calls this; it is off unless that switch is given.
//   node tools/soak/drift-gate.mjs --uproject <abs .uproject> [--base main] [--repo <dir>] [--timeout-s 480] [--force modes|takes|all] [--paths a,b] [--max 16]
// It reads the branch's changed paths (changedPaths of tools/gate/check-ownership.mjs: read-only git diff, run by that module, not here), decides the scope, and runs
// tools/soak/drift.mjs on it under a wall-clock cap (default 8 minutes; the whole 36-cell matrix is about 5.5 minutes on a quiet machine). No relevant path: SKIP, exit 0.
//   cartridges under Game/Content/Cartridges/modes only -> --only modes; .../ladder only -> --only takes; any other cartridge, or the sim (JamCore Private, Public, Rules)
//   or the brain (JamCore/AI) -> both. Tests and Tools under JamCore, docs, tools: no run.
// Exit: 0 pass or skipped, 1 a number moved / fault / timeout, 2 could not run. Last line: {"tool":"thirdjam.driftgate/1",...}.
import { spawn, spawnSync } from 'node:child_process';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';
import { changedPaths } from '../gate/check-ownership.mjs';

const here = dirname(fileURLToPath(import.meta.url));
const SIM = /^Game\/Source\/JamCore\/(Private|Public|Rules|AI)\//;
const CART = /^Game\/Content\/Cartridges\/(.+)$/;

// 'none' | 'modes' | 'takes' | 'all'
export function scopeOf(paths) {
  let modes = false, takes = false, all = false;
  for (const raw of paths) {
    const p = raw.split(String.fromCharCode(92)).join('/');
    if (SIM.test(p)) { all = true; continue; }
    const m = CART.exec(p); if (!m) continue;
    if (m[1].startsWith('modes/')) modes = true; else if (m[1].startsWith('ladder/')) takes = true; else all = true;
  }
  if (all || (modes && takes)) return 'all';
  return modes ? 'modes' : takes ? 'takes' : 'none';
}

export async function main(argv) {
  const opt = (n, d) => { const i = argv.indexOf(`--${n}`); return i >= 0 ? argv[i + 1] : d; };
  const done = (verdict, exit, extra = {}) => { console.log(JSON.stringify({ tool: 'thirdjam.driftgate/1', verdict, exit, ...extra })); return exit; };
  const uproject = opt('uproject'); if (!uproject) { console.log('FAIL drift-gate --uproject is required'); return done('could_not_run', 2); }
  let scope = opt('force'), paths = [];
  if (!scope) {
    try { paths = opt('paths') ? opt('paths').split(',') : changedPaths(resolve(opt('repo', join(here, '..', '..'))), opt('base', 'main')); }
    catch (e) { console.log(`FAIL drift-gate could not read the changed paths: ${String(e.message).split('\n')[0]}`); return done('could_not_run', 2); }
    scope = scopeOf(paths);
  }
  if (scope === 'none') { console.log('SKIP drift (no changed path touches cartridges, the sim or the brain)'); return done('skipped', 0, { changed: paths.length }); }
  const args = [join(here, 'drift.mjs'), '--uproject', resolve(uproject), '--max', opt('max', '16'), ...(scope === 'all' ? [] : ['--only', scope])];
  const cap = +opt('timeout-s', 480) * 1000;
  console.log(`INFO drift scope ${scope} (cap ${cap / 1000} s)`);
  const t0 = Date.now();
  const code = await new Promise((res) => {
    const p = spawn(process.execPath, args, { stdio: 'inherit', windowsHide: true });
    const to = setTimeout(() => {   // only the process tree this gate started
      console.log(`FAIL drift-gate exceeded ${cap / 1000} s: stopping the run it started`);
      if (process.platform === 'win32') spawnSync('taskkill', ['/T', '/F', '/PID', String(p.pid)]); else p.kill();
      res(124);
    }, cap);
    p.on('close', (c) => { clearTimeout(to); res(c ?? 1); });
    p.on('error', () => { clearTimeout(to); res(2); });
  });
  const seconds = Math.round((Date.now() - t0) / 1000);
  return done(code === 0 ? 'pass' : code === 2 ? 'could_not_run' : 'fail', code === 0 ? 0 : code === 2 ? 2 : 1, { scope, seconds, drift_exit: code });
}

if (process.argv[1] && import.meta.url === pathToFileURL(resolve(process.argv[1])).href) process.exit(await main(process.argv.slice(2)));
