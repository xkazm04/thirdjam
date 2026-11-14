#!/usr/bin/env node
// Shipping compile gate (lane A179, evolution 5, item 4): the Development editor build and every automation test stay green when code compiles only outside the
// Shipping game target's rules (a symbol under `#if !UE_BUILD_SHIPPING` called from code that Shipping compiles; a test below its guard is the shape step 3b of
// premerge catches without Unreal). The Shipping Game target is the only thing that fails on those, an hour after the landing. This step runs that compile
// (tools/run/build.ps1 -Config Shipping, the game target) when the branch changed code the Shipping target compiles.
//
//   node tools/gate/shipping-gate.mjs --uproject Game/ThirdJam.uproject [--base main] [--paths a,b] [--force] [--timeout-s 3600]
//
// Scope: any changed path under Game/Source/ (cpp, h, inl, Build.cs, Target.cs), Game/*.Target.cs, Game/Plugins/**/Source/ or Game/*.uproject / *.uplugin. Paths come from
// changedPaths of check-ownership.mjs (the read-only git diff that module runs), or --paths for a test. No scope: SKIP, exit 0. --force always builds.
// Exit 0 pass or skipped, 1 the Shipping compile failed or timed out, 2 could not run. Last stdout line: JSON thirdjam.shippinggate/1 with the seconds the build took.
import { spawn, spawnSync } from 'node:child_process';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';
import { changedPaths } from './check-ownership.mjs';

const here = dirname(fileURLToPath(import.meta.url));

export function needsShipping(paths) {
  return paths.filter((p) => {
    const q = p.replace(/\\/g, '/');
    return /^Game\/Source\/.+\.(cpp|h|hpp|inl|cs)$/i.test(q) || /^Game\/[^/]+\.(Target|Build)\.cs$/i.test(q) || /^Game\/Plugins\/.+\/Source\/.+\.(cpp|h|hpp|inl|cs)$/i.test(q)
      || /^Game\/[^/]+\.uproject$/i.test(q) || /^Game\/Plugins\/.+\.uplugin$/i.test(q);
  });
}

export async function main(argv) {
  const opt = (n, d) => { const i = argv.indexOf(`--${n}`); return i >= 0 ? argv[i + 1] : d; };
  const done = (verdict, exit, extra = {}) => { console.log(JSON.stringify({ tool: 'thirdjam.shippinggate/1', verdict, exit, ...extra })); return exit; };
  const uproject = opt('uproject'); if (!uproject) { console.log('FAIL shipping-gate --uproject is required'); return done('could_not_run', 2); }
  let hit = [];
  if (!argv.includes('--force')) {
    let paths;
    try { paths = opt('paths') ? opt('paths').split(',') : changedPaths(resolve(opt('repo', join(here, '..', '..'))), opt('base', 'main')); }
    catch (e) { console.log(`FAIL shipping-gate could not read the changed paths: ${String(e.message).split('\n')[0]}`); return done('could_not_run', 2); }
    hit = needsShipping(paths);
    if (!hit.length) { console.log('SKIP shipping (no changed path under Game/Source, a Target.cs, a Build.cs or a plugin Source)'); return done('skipped', 0, { changed: paths.length }); }
  }
  const cap = Number(opt('timeout-s', '3600')) * 1000;
  console.log(`INFO shipping compile: ${argv.includes('--force') ? 'forced' : `${hit.length} changed path(s), first ${hit[0]}`} (cap ${cap / 1000} s)`);
  const t0 = Date.now();
  const build = join(here, '..', 'run', 'build.ps1');
  const lines = [];
  const code = await new Promise((res) => {
    const p = spawn('powershell', ['-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', build, '-Project', resolve(uproject), '-Config', 'Shipping', '-TimeoutSec', String(Math.floor(cap / 1000))], { windowsHide: true });
    const to = setTimeout(() => { console.log(`FAIL shipping-gate exceeded ${cap / 1000} s: stopping the build it started`); spawnSync('taskkill', ['/T', '/F', '/PID', String(p.pid)]); res(124); }, cap + 60000);
    const eat = (d) => { for (const l of d.toString().split(/\r?\n/)) if (l) lines.push(l); };
    p.stdout.on('data', eat); p.stderr.on('data', eat);
    p.on('close', (c) => { clearTimeout(to); res(c ?? 1); });
    p.on('error', () => { clearTimeout(to); res(2); });
  });
  const seconds = Math.round((Date.now() - t0) / 1000);
  const errs = lines.filter((l) => /error (C|LNK|MSB)\d+|: error:|Error: /.test(l)).slice(0, 12);
  for (const e of errs) console.log(`  | ${e.trim().slice(0, 240)}`);
  console.log(`${code === 0 ? 'PASS' : 'FAIL'} shipping-compile Game Shipping ${code === 0 ? 'built' : `failed (build.ps1 exit ${code})`} in ${seconds} s`);
  return done(code === 0 ? 'pass' : code === 2 ? 'could_not_run' : 'fail', code === 0 ? 0 : code === 2 ? 2 : 1, { seconds, build_exit: code, errors: errs.length });
}
if (process.argv[1] && import.meta.url === pathToFileURL(resolve(process.argv[1])).href) process.exit(await main(process.argv.slice(2)));
