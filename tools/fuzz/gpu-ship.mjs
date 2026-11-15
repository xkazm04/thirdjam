#!/usr/bin/env node
// GPU crash at a match start, Shipping package (lane A198; Director 16:07, point 3). A Shipping build has no remote console and no log, and the
// menu needs a key, so each start opens the rink directly: ThirdJam.exe /Game/Maps/Rink -cut=<cut> -RenderOffScreen (the rink's own load, the rig
// stream-in, the persistent sky light hidden, the first frames of play: the moment the Development deaths came). A process still alive after
// --alive-s seconds passed its start (stopped by its pid); one that exits earlier is a death: its exit code (3 = D3D12 TerminateOnGPUCrash) and any
// new GPU dump or crash folder under the package's and the user's Saved folders are recorded. GPU processes alive at each start are listed (read only).
//   node tools/fuzz/gpu-ship.mjs --exe <ThirdJam.exe of a Shipping package> [--starts 20] [--alive-s 40] [--out tools/fuzz/out/gpu-ship]
import { spawn, execFileSync } from 'node:child_process';
import { writeFileSync, appendFileSync, mkdirSync, existsSync, readdirSync, statSync } from 'node:fs';
import { join, resolve, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';
import os from 'node:os';

const HERE = dirname(fileURLToPath(import.meta.url));
const REPO = resolve(HERE, '..', '..');
const argv = process.argv.slice(2);
const arg = (k, d) => { const i = argv.indexOf(`--${k}`); return i >= 0 ? argv[i + 1] : d; };
const EXE = arg('exe'); const N = +arg('starts', '20'); const ALIVE = +arg('alive-s', '40');
const OUT = resolve(arg('out', join(HERE, 'out', 'gpu-ship')));
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
const cutDir = join(REPO, 'tools', 'perf', 'cuts');
const CUTS = ['perf.p3_clay', 'perf.l2_night', 'perf.l3_neon', 'perf.p3_clay_5v5'].map((c) => join(cutDir, `${c}.cartridge.json`).replace(/\\/g, '/'));
if (!EXE || !existsSync(EXE)) { console.log(`FAIL gpu-ship: no exe ${EXE}`); process.exit(2); }
mkdirSync(OUT, { recursive: true });
const ROWS = join(OUT, 'starts.jsonl'); writeFileSync(ROWS, '');
const gpuProcs = () => { try { return execFileSync('powershell', ['-NoProfile', '-Command', "Get-Process -ErrorAction SilentlyContinue | Where-Object { $_.ProcessName -match '^(UnrealEditor|ThirdJam.*|Hokej.*|blender)$' } | ForEach-Object { '{0}:{1}' -f $_.ProcessName, $_.Id }"], { encoding: 'utf8' }).split(/\r?\n/).filter(Boolean); } catch { return []; } };
const pkgRoot = resolve(dirname(EXE));
const savedDirs = [join(pkgRoot, 'ThirdJam', 'Saved'), join(os.homedir(), 'AppData', 'Local', 'ThirdJam', 'Saved')];
function dumps() { const out = []; for (const d of savedDirs) for (const sub of ['Logs', 'Crashes']) { const p = join(d, sub); if (!existsSync(p)) continue; for (const f of readdirSync(p)) out.push(`${p}/${f}@${statSync(join(p, f)).mtimeMs}`); } return new Set(out); }
let deaths = 0;
for (let s = 1; s <= N; s++) {
  const cut = CUTS[(s - 1) % CUTS.length];
  const before = dumps(); const alive = gpuProcs();
  const child = spawn(EXE, ['/Game/Maps/Rink', `-cut=${cut}`, '-RenderOffScreen', '-ResX=1920', '-ResY=1080', '-ForceRes', '-unattended', '-nosound'], { windowsHide: true, stdio: 'ignore' });
  let code = null; child.on('exit', (c) => { code = c; });
  for (let i = 0; i < ALIVE * 2 && code === null; i++) await sleep(500);
  const died = code !== null;
  if (!died) { try { process.kill(child.pid); } catch { /* gone */ } for (let i = 0; i < 20 && code === null; i++) await sleep(250); }
  const fresh = [...dumps()].filter((x) => !before.has(x));
  if (died) deaths++;
  const r = { s, pid: child.pid, cut: cut.replace(/.*perf\./, '').replace('.cartridge.json', ''), alive_gpu: alive.length, alive, died, exit: died ? code : null, new_files: fresh };
  appendFileSync(ROWS, JSON.stringify(r) + '\n');
  console.log(`start ${s}/${N} ${r.cut} pid ${child.pid}: ${died ? `DIED exit ${code}` : 'alive at ' + ALIVE + ' s'} (gpu procs alive ${alive.length})${fresh.length ? ' new ' + fresh.join(' ') : ''}`);
  await sleep(2000);
}
console.log(`${deaths ? 'FAIL' : 'PASS'} gpu-ship: ${N} starts, ${deaths} deaths (${ROWS})`);
console.log(JSON.stringify({ tool: 'thirdjam.gpuship/1', verdict: deaths ? 'fail' : 'pass', exit: deaths ? 1 : 0, starts: N, deaths, out: OUT }));
process.exit(deaths ? 1 : 0);
