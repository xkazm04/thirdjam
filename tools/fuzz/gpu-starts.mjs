#!/usr/bin/env node
// GPU crash at a match start (lane A198, track S3, evolution 5; Director 16:07). One packaged Development game at a time, off screen
// (-RenderOffScreen), driven by its remote console: PLAY NOW (jam.menuscreen play) starts the launch cut; 20 s of play; then a second start in the
// same process (jam.ctrl.EndMatch 1, jam.pausemenu quit, PLAY NOW again: a level change back to the rink); 20 s; quit. Each start records the GPU
// processes alive on the machine at that moment (read only: UnrealEditor.exe and game executables; nothing is stopped but my own child), and
// whether the process died with a GPU crash (log "GPU crash detected", D3D12 device removed / page fault), with the breadcrumb lines and the last
// 30 log lines of each death.
//   node tools/fuzz/gpu-starts.mjs --exe <ThirdJam.exe> [--processes 20] [--port 4797] [--dp "r.SkyLight.RealTimeReflectionCapture=0"] [--out tools/fuzz/out/gpu-<tag>]
//   --uproject <ThirdJam.uproject> with --exe UnrealEditor.exe: the editor binary as an off-screen game (where every death of 14 Nov was seen); --res 1920x1080
//   --cuts a,b,c   launch cuts rotated over the processes (default: none = the operator's PLAY NOW, then the perf cuts of clay, night and neon)
// Exit 0 no GPU crash, 1 a crash, 2 could not run.
import { spawn, execFileSync } from 'node:child_process';
import { readFileSync, writeFileSync, appendFileSync, mkdirSync, existsSync } from 'node:fs';
import { join, resolve, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';
import net from 'node:net';

const HERE = dirname(fileURLToPath(import.meta.url));
const REPO = resolve(HERE, '..', '..');
const argv = process.argv.slice(2);
const arg = (k, d) => { const i = argv.indexOf(`--${k}`); return i >= 0 ? argv[i + 1] : d; };
const EXE = arg('exe');
const N = +arg('processes', '20');
const PORT = +arg('port', '4797');
const DP = arg('dp', '');
const OUT = resolve(arg('out', join(HERE, 'out', 'gpu-starts')));
const PLAY_S = +arg('play-s', '20');
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
const cutDir = join(REPO, 'tools', 'perf', 'cuts');
const CUTS = (arg('cuts', '') || ['', 'perf.p3_clay', 'perf.l2_night', 'perf.l3_neon'].join(',')).split(',').map((c) => (c ? join(cutDir, `${c}.cartridge.json`).replace(/\\/g, '/') : ''));
if (!EXE || !existsSync(EXE)) { console.log(`FAIL gpu-starts: no exe ${EXE}`); console.log(JSON.stringify({ tool: 'thirdjam.gpustarts/1', verdict: 'could_not_run', exit: 2 })); process.exit(2); }
mkdirSync(OUT, { recursive: true });
const ROWS = join(OUT, 'starts.jsonl');
writeFileSync(ROWS, '');
const t0 = Date.now();
const note = (s) => console.log(`${((Date.now() - t0) / 1000).toFixed(1).padStart(7)}s  ${s}`);

function rc(cmd, timeoutMs = 5000) {
  return new Promise((res) => {
    const s = net.connect(PORT, '127.0.0.1'); let buf = ''; let sent = false; let done = false;
    const fin = (v) => { if (done) return; done = true; try { s.destroy(); } catch { /* closed */ } res(v); };
    s.setTimeout(timeoutMs, () => fin(null)); s.on('error', () => fin(null));
    s.on('connect', () => s.write('{"id":1,"op":"hello","version":1}\n'));
    s.on('data', (d) => { buf += d.toString(); let i; while ((i = buf.indexOf('\n')) >= 0) { const l = buf.slice(0, i); buf = buf.slice(i + 1);
      if (!sent && l.startsWith('{"id":1')) { sent = true; s.write(JSON.stringify({ id: 2, op: 'exec', cmd }) + '\n'); } else if (l.startsWith('{"id":2,')) fin(l.includes('"ok":true')); } });
  });
}
// GPU processes alive now (read only): the editor with a window or off screen, and game executables
function gpuProcs() {
  try {
    const out = execFileSync('powershell', ['-NoProfile', '-Command', "Get-Process -ErrorAction SilentlyContinue | Where-Object { $_.ProcessName -match '^(UnrealEditor|ThirdJam|Hokej.*|blender)$' } | ForEach-Object { '{0}:{1}' -f $_.ProcessName, $_.Id }"], { encoding: 'utf8' });
    return out.split(/\r?\n/).filter(Boolean);
  } catch { return []; }
}
const readLog = (f) => { try { return readFileSync(f, 'utf8'); } catch { return ''; } };
const count = (t, re) => (t.match(re) || []).length;

let crashes = 0, starts = 0;
for (let p = 1; p <= N; p++) {
  const cut = CUTS[(p - 1) % CUTS.length];
  const tier = cut ? cut.replace(/.*perf\.([a-z0-9_]+)\.cartridge\.json$/, '$1') : 'play-now (regular, clay)';
  const log = join(OUT, `p${p}.log`);
  const dp = ['jam.ScriptHuman=1', 'jam.screen.FadeSeconds=0', ...(DP ? DP.split(',') : [])].join(',');
  const [rx, ry] = arg('res', '1280x720').split('x');
  const pre = arg('uproject') ? [resolve(arg('uproject')), '/Game/Maps/Menu', '-game', '-Multiprocess', '-nosplash'] : [];   // --uproject: the editor binary as a game (--exe UnrealEditor.exe)
  const args = [...pre, ...(cut ? [`-cut=${cut}`] : []), `-dpcvars=${dp}`, '-RenderOffScreen', `-ResX=${rx}`, `-ResY=${ry}`, '-ForceRes', '-unattended', '-nosound', '-log', `-abslog=${log}`, '-ForceLogFlush',
    '-RemoteConsoleIP=127.0.0.1', `-RemoteConsolePort=${PORT}`];
  const child = spawn(EXE, args, { windowsHide: true, stdio: 'ignore' });
  let exitCode = null; child.on('exit', (c) => { exitCode = c; });
  note(`process ${p}/${N} pid ${child.pid} cut ${tier}`);
  let up = false;
  for (let i = 0; i < 90 && exitCode === null && !up; i++) { up = await rc('jam.menuscreen where', 3000); if (!up) await sleep(2000); }
  const row = (kind, alive, extra) => { const r = { p, pid: child.pid, kind, tier, alive_gpu: alive.length, alive, ...extra }; appendFileSync(ROWS, JSON.stringify(r) + '\n'); return r; };
  if (!up) { row('no-menu', gpuProcs(), { exit: exitCode }); note(`  the menu never answered (exit ${exitCode})`); if (exitCode === null) { try { process.kill(child.pid); } catch { /* gone */ } } await sleep(3000); continue; }
  await sleep(4000);
  for (const kind of (argv.includes('--first-only') ? ['first'] : ['first', 'second'])) {
    if (exitCode !== null) break;
    if (kind === 'second') { await rc('jam.ctrl.EndMatch 1'); await sleep(3000); await rc('jam.pausemenu quit'); for (let i = 0; i < 30 && exitCode === null; i++) { if (await rc('jam.menuscreen where', 3000)) break; await sleep(1000); } await sleep(3000); }
    const before = count(readLog(log), /JAMMATCH /g);
    const alive = gpuProcs();
    await rc('jam.menuscreen play');
    starts++;
    let matched = false;
    for (let i = 0; i < 60 && exitCode === null; i++) { if (count(readLog(log), /JAMMATCH /g) > before) { matched = true; break; } await sleep(500); }
    for (let i = 0; i < PLAY_S * 2 && exitCode === null; i++) await sleep(500);
    const t = readLog(log);
    const crash = /GPU crash detected|DXGI_ERROR_DEVICE|Device Removed|TerminateOnGPUCrash|page fault/i.test(t);
    let detail = {};
    if (crash) {
      crashes++;
      const lines = t.split(/\r?\n/);
      const at = lines.findIndex((l) => /GPU crash detected/.test(l));
      detail = { crash_line: lines[at] || '', reason: (lines.slice(at, at + 3).join(' ').match(/DXGI_ERROR_\w+|Page fault[^\n]*|DEVICE_\w+/i) || [''])[0],
        breadcrumbs: lines.filter((l) => /\[\s+Active\]/.test(l)).slice(-6).map((l) => l.trim()), sky: /ComputeSingleAverageBrightnessFromCubemap/.test(t),
        last30: lines.slice(Math.max(0, at - 30), at).map((l) => l.slice(0, 220)) };
    }
    const r = row(kind, alive, { matched, crash, exit: exitCode, ...detail });
    note(`  ${kind} start: matched=${matched} crash=${crash}${crash ? ' ' + r.reason + (r.sky ? ' (sky capture)' : '') : ''} gpu procs alive=${alive.length}`);
    if (crash) break;
  }
  if (exitCode === null) { await rc('quit', 3000); for (let i = 0; i < 20 && exitCode === null; i++) await sleep(500); }
  if (exitCode === null) { try { process.kill(child.pid); note(`  stopped my pid ${child.pid}`); } catch { /* gone */ } }
  await sleep(2000);
}
const code = crashes ? 1 : 0;
console.log(`${code ? 'FAIL' : 'PASS'} gpu-starts: ${starts} match starts in ${N} processes, ${crashes} GPU crashes (${ROWS})`);
console.log(JSON.stringify({ tool: 'thirdjam.gpustarts/1', verdict: code ? 'fail' : 'pass', exit: code, processes: N, starts, crashes, dp: DP, out: OUT }));
process.exit(code);
