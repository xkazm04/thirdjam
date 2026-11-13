// Fuzz driver (lane A109, track S3). Runs Bar.Fuzz.* in headless UnrealEditor-Cmd workers over a corpus made by gen.mjs, survives crashes and hangs.
//
//   node tools/fuzz/run.mjs --corpus tools/fuzz/out/run1 [--workers 4] [--frames 600] [--long-every 40] [--case-timeout 120]
//                           [--test Bar.Fuzz.Cartridges|Bar.Fuzz.ModesAndCuts] [--project Game/ThirdJam.uproject] [--found tools/fuzz/found]
//
// The corpus is cut into one contiguous shard per worker. A worker is one UnrealEditor-Cmd process; the C++ test writes progress.txt (the case about to
// run) before every case and faults.jsonl for every fault it detects. When a worker dies without "done" in progress.txt the case named there is the culprit
// (class crash, or hang when this driver killed it because progress.txt did not change for --case-timeout seconds); the cartridge is saved under found/,
// and the worker restarts at the next case. Only processes this script started are ever killed.
// Memory band (docs/EVOLUTION-COMMON): no new worker above --max-used-gb (default 50) in use; none above 56 ever.
// Output: <corpus>/report.json (also printed): faults by class with the case names; found/ gets one cartridge per fault (first --keep per class).
import { spawn, spawnSync } from 'node:child_process';
import { existsSync, mkdirSync, readFileSync, writeFileSync, statSync, copyFileSync, rmSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import os from 'node:os';

const HERE = dirname(fileURLToPath(import.meta.url));
const REPO = resolve(HERE, '..', '..');
const argv = process.argv.slice(2);
const arg = (k, d) => { const i = argv.indexOf(`--${k}`); return i >= 0 ? argv[i + 1] : d; };

const corpus = resolve(arg('corpus', join(HERE, 'out', 'run1')));
const test = arg('test', 'Bar.Fuzz.Cartridges');
const workers = Number(arg('workers', '4'));
const frames = arg('frames', '600');
const longEvery = arg('long-every', '0');
const longFrames = arg('long-frames', '36000');
const caseTimeout = Number(arg('case-timeout', '120')) * 1000;
const maxUsedGb = Number(arg('max-used-gb', '50'));
const keep = Number(arg('keep', '3'));
const found = resolve(arg('found', join(HERE, 'found')));
const project = resolve(arg('project', join(REPO, 'Game', 'ThirdJam.uproject')));
const ueRoot = process.env.UE_ROOT || 'C:\\Program Files\\Epic Games\\UE_5.9';
const exe = join(ueRoot, 'Engine', 'Binaries', 'Win64', 'UnrealEditor-Cmd.exe');
if (!existsSync(exe)) { console.error(`FAIL no ${exe} (set UE_ROOT)`); process.exit(2); }

const usedGb = () => (os.totalmem() - os.freemem()) / 2 ** 30;
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
const manifest = test === 'Bar.Fuzz.Cartridges' ? JSON.parse(readFileSync(join(corpus, 'manifest.json'), 'utf8')) : { cases: [] };
const total = manifest.cases.length;
const faults = [];
const crashes = [];

function readProgress(dir) {
  try { return readFileSync(join(dir, 'progress.txt'), 'utf8').split('\t'); } catch { return null; }
}
function mtime(dir) {
  try { return statSync(join(dir, 'progress.txt')).mtimeMs; } catch { return 0; }
}

async function runShard(k, first, last) {
  const dir = join(corpus, `w${k}`);
  mkdirSync(dir, { recursive: true });
  let next = first;
  let restarts = 0;
  while (next <= last && restarts < 200) {
    while (usedGb() > maxUsedGb) { console.log(`INFO worker ${k}: ${usedGb().toFixed(1)} GB in use, waiting`); await sleep(15000); }
    rmSync(join(dir, 'progress.txt'), { force: true });
    const env = { ...process.env, JAM_FUZZ_DIR: corpus, JAM_FUZZ_OUT: dir, JAM_FUZZ_FIRST: String(next), JAM_FUZZ_LAST: String(last), JAM_FUZZ_FRAMES: frames, JAM_FUZZ_LONG_EVERY: longEvery, JAM_FUZZ_LONG_FRAMES: longFrames };
    const abslog = join(dir, `ue-${next}.log`);
    const args = [project, `-ExecCmds=Automation RunTests ${test}; Quit`, '-nullrhi', '-unattended', '-nopause', '-nosplash', '-log', `-ABSLOG=${abslog}`];
    const child = spawn(exe, args, { env, stdio: 'ignore', windowsHide: true });
    let killedForHang = false;
    const startedAt = Date.now();
    const exitP = new Promise((res) => child.on('exit', (code) => res(code)));
    const watch = setInterval(() => {
      const last_ = mtime(dir) || startedAt;
      // the first case waits for engine start-up: allow three times the case timeout before progress.txt exists
      const limit = mtime(dir) ? caseTimeout : caseTimeout * 3;
      if (Date.now() - last_ > limit) {
        killedForHang = true;
        spawnSync('taskkill', ['/PID', String(child.pid), '/T', '/F'], { stdio: 'ignore' });   // our own child only
      }
    }, 5000);
    const code = await exitP;
    clearInterval(watch);
    const p = readProgress(dir);
    if (p && p[0] === 'done') { console.log(`INFO worker ${k}: shard ${first}..${last} done (exit ${code})`); break; }
    // crash or hang: p[0] is the index of the case that was running
    restarts++;
    const idx = p ? Number(p[0]) : next;
    const name = p ? p[1] : '(before the first case)';
    const file = p ? p[2] : '';
    const cls = killedForHang ? 'hang' : 'crash';
    console.log(`FAIL worker ${k}: ${cls} on case ${idx} ${name} (exit ${code}); restarting at ${idx + 1}`);
    crashes.push({ class: cls, case: name, index: idx, file, exit: code });
    faults.push({ class: cls, case: name, detail: `exit ${code} while running case ${idx}`, file });
    next = idx + 1;
  }
}

async function main() {
  mkdirSync(found, { recursive: true });
  let shards = [];
  if (test === 'Bar.Fuzz.Cartridges') {
    const per = Math.ceil(total / workers);
    for (let k = 0; k < workers; k++) { const a = k * per, b = Math.min(total - 1, a + per - 1); if (a <= b) shards.push([k, a, b]); }
  } else shards = [[0, 0, 999]];
  console.log(`INFO ${test}: ${total} cases, ${shards.length} worker(s), ${usedGb().toFixed(1)} GB in use`);
  const t0 = Date.now();
  // stagger starts so engine start-up does not spike the memory
  await Promise.all(shards.map(async ([k, a, b], i) => { await sleep(i * 8000); return runShard(k, a, b); }));
  for (const [k] of shards) {
    const f = join(corpus, `w${k}`, 'faults.jsonl');
    if (existsSync(f)) for (const line of readFileSync(f, 'utf8').split('\n').filter(Boolean)) { try { const o = JSON.parse(line); o.file = manifest.cases.find((c) => c.name === o.case)?.file ?? ''; faults.push(o); } catch { /* a torn last line after a crash */ } }
  }
  const byClass = {};
  for (const f of faults) (byClass[f.class] ??= []).push(f);
  const kept = {};
  for (const [cls, list] of Object.entries(byClass)) {
    for (const f of list.slice(0, keep)) {
      if (!f.file) continue;
      const dst = join(found, `${cls}-${f.case}.json`);
      try { copyFileSync(join(corpus, f.file), dst); (kept[cls] ??= []).push(dst); } catch { /* a modes case has no corpus file */ }
    }
  }
  const report = { test, corpus, total, seconds: Math.round((Date.now() - t0) / 1000), faults: faults.length, byClass: Object.fromEntries(Object.entries(byClass).map(([c, l]) => [c, { count: l.length, first: l.slice(0, 5).map((f) => ({ case: f.case, detail: f.detail })) }])), kept };
  writeFileSync(join(corpus, `report-${test}.json`), JSON.stringify(report, null, 1));
  console.log(JSON.stringify(report, null, 1));
  console.log(`${faults.length === 0 ? 'PASS' : 'FAIL'} fuzz ${test}: ${faults.length} fault(s) in ${Object.keys(byClass).length} class(es)`);
  process.exit(faults.length === 0 ? 0 : 1);
}
main();
