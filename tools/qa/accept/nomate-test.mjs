#!/usr/bin/env node
// Lane A179 (item 2): the acceptance drill on a staged cut without a mate / without a non-back opponent must neither crash nor read Sk[-1]; the steps that need one are
// recorded `skipped=no_mate` / `skipped=no_opp` and the drill goes on to its summary.
//   node tools/qa/accept/nomate-test.mjs [--uproject Game/ThirdJam.uproject] [--timeout-s 240] [--port 4779] [--out tools/qa/out/nomate]
// Two cuts (tools/qa/accept/nomate-cut.json: 2 a side, a back each, so no mate; noopp-cut.json: the other team is two backs). One headless UnrealEditor-Cmd per cut (-game -nullrhi
// -Multiprocess, the Rink map, -cut=), the drill started with the QA console command `jam.ctrl.Drill 20` over the remote console, steps 5 to 8 (`jam.accept.From/To`).
// PASS per cut: a `JAMACCEPT summary` line, a `skipped=<why>` line for each verb of the steps the cut cannot stage, no `Assertion failed` / `Fatal error` / `Array index out of bounds`.
// With the --expect-crash flag (run on a package built before the guard) the expectation is inverted: the test PASSES only when the crash is seen (the seeded-fault proof).
// Exit 0 pass, 1 fail, 2 could not run.
import { spawn, spawnSync } from 'node:child_process';
import { existsSync, mkdirSync, readFileSync, rmSync, statSync, writeFileSync } from 'node:fs';
import net from 'node:net';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const HERE = dirname(fileURLToPath(import.meta.url));
const REPO = resolve(HERE, '..', '..', '..');
const argv = process.argv.slice(2);
const arg = (k, d) => { const i = argv.indexOf(`--${k}`); return i >= 0 ? argv[i + 1] : d; };
const uproject = resolve(arg('uproject', join(REPO, 'Game', 'ThirdJam.uproject')));
const timeoutMs = Number(arg('timeout-s', '240')) * 1000;
const port = Number(arg('port', '4779'));
const out = resolve(arg('out', join(REPO, 'tools', 'qa', 'out', 'nomate')));
const ueRoot = process.env.UE_ROOT || 'C:/Program Files/Epic Games/UE_5.9';
const exe = join(ueRoot, 'Engine', 'Binaries', 'Win64', 'UnrealEditor-Cmd.exe');
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
export const CASES = [
  { name: 'nomate', cut: join(HERE, 'nomate-cut.json'), skipped: ['no_mate'], verbs: ['pass', 'saucer', 'onetimer'] },
  { name: 'noopp', cut: join(HERE, 'noopp-cut.json'), skipped: ['no_opp'], verbs: ['check', 'crosscheck', 'checked'] },
];

function rcExec(p, cmd) {
  return new Promise((res) => {
    const s = net.connect(p, '127.0.0.1');
    let buf = '', stage = 0;
    const done = (v) => { try { s.destroy(); } catch { /* closed */ } res(v); };
    s.setTimeout(8000, () => done(null)); s.on('error', () => done(null));
    s.on('connect', () => s.write('{"id":1,"op":"hello","version":1}\n'));
    s.on('data', (d) => { buf += d.toString('utf8'); let i; while ((i = buf.indexOf('\n')) >= 0) { const l = buf.slice(0, i); buf = buf.slice(i + 1); if (stage === 0) { stage = 1; s.write(`${JSON.stringify({ id: 2, op: 'exec', cmd })}\n`); } else if (l.startsWith('{"id":2,')) return done(l); } });
  });
}

async function runCase(c) {
  mkdirSync(out, { recursive: true });
  const log = join(out, `${c.name}.log`); rmSync(log, { force: true });
  const args = [uproject, '/Game/Maps/Rink', '-game', `-cut=${c.cut}`, '-nullrhi', '-Multiprocess', '-unattended', '-nopause', '-nosplash', '-log', `-ABSLOG=${log}`, '-ForceLogFlush',
    '-dpcvars=jam.accept.From=5,jam.accept.To=8,jam.ctrl.Log=0', '-RemoteConsoleIP=127.0.0.1', `-RemoteConsolePort=${port}`];
  const child = spawn(exe, args, { stdio: 'ignore', windowsHide: true });
  let exited = false; child.on('exit', () => { exited = true; });
  // -ABSLOG is not honoured by a -game start of the Rink map here: the project's own Saved/Logs/ThirdJam.log is the log (rotated at each start)
  const sv = join(dirname(uproject), 'Saved', 'Logs', 'ThirdJam.log'); const tStart = Date.now() - 2000;
  const read = () => { try { return statSync(sv).mtimeMs >= tStart ? readFileSync(sv, 'utf8') : ''; } catch { return ''; } };
  const t0 = Date.now(); let started = false, tries = 0;
  while (Date.now() - t0 < timeoutMs && !exited) {
    await sleep(1500);
    const t = read();
    if (/JAMACCEPT summary/.test(t) || /Assertion failed|Fatal error|Array index out of bounds/.test(t)) break;
    if (!started && /JAMMATCH level=/.test(t) && tries < 5) { await sleep(3000); const r = await rcExec(port, 'jam.ctrl.Drill 20'); tries++; started = !!r; }
  }
  const text = read(); const wasExited = exited;
  spawnSync('taskkill', ['/PID', String(child.pid), '/T', '/F'], { stdio: 'ignore' });   // our own child only
  writeFileSync(log, text);
  const crash = /Assertion failed[^\r\n]*|Fatal error[^\r\n]*|Array index out of bounds[^\r\n]*/.exec(text)?.[0] ?? '';
  const summary = /JAMACCEPT summary[^\r\n]*/.exec(text)?.[0] ?? '';
  const skipLines = [...text.matchAll(/JAMACCEPT step=(\d+) verb=(\w+) ok=0 [^\r\n]*skipped=(\w+)/g)].map((m) => `${m[2]}:${m[3]}`);
  const begin = /JAMACCEPT begin[^\r\n]*/.exec(text)?.[0] ?? '(no begin line)';
  return { name: c.name, begin: begin.replace(/^.*JAMACCEPT /, ''), summary: summary.replace(/^.*JAMACCEPT /, ''), skipLines, crash, exitedEarly: wasExited, log };
}

async function main() {
  if (!existsSync(exe)) { console.log(`FAIL nomate-test no ${exe}`); process.exit(2); }
  const expectCrash = argv.includes('--expect-crash');
  let bad = 0;
  for (const c of CASES) {
    const r = await runCase(c);
    const okSkips = c.verbs.every((v) => r.skipLines.some((s) => s === `${v}:${c.skipped[0]}`));
    const pass = expectCrash ? !!r.crash : (!r.crash && !!r.summary && okSkips);
    if (!pass) bad++;
    console.log(`${pass ? 'PASS' : 'FAIL'} ${c.name}: ${r.begin.slice(0, 80)} | summary=[${r.summary}] skipped=[${r.skipLines.join(' ')}] crash=[${r.crash.slice(0, 140)}] (log ${r.log})`);
  }
  console.log(`${bad ? 'FAIL' : 'PASS'} nomate-test ${CASES.length - bad} of ${CASES.length}${expectCrash ? ' (expecting the crash)' : ''}`);
  process.exit(bad ? 1 : 0);
}
if (process.argv[1] && resolve(process.argv[1]) === fileURLToPath(import.meta.url)) main();
