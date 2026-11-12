#!/usr/bin/env node
// Lane A40: one camera proof run through the editor's MCP server: set jam.camera.Legacy, jam.ScriptHuman 1, jam.Cut <id>, StartPIE, inject
// tools/qa/cam40.py before PIE (HighResShot at exact sim frames, one row per sim frame), StartPIE, poll, cam40stop.py, StopPIE.
//   node tools/qa/cam40run.mjs --legacy 0|1 --frames 60,222,223,500,... --name <run> [--cut hockey.regular] [--res 2560x1440] [--slow 0.5] [--url http://127.0.0.1:8022/mcp]
// Output: tools/qa/out/<run>/ (rows.jsonl, status.json); the stills land in Game/Saved/Screenshots/WindowsEditor/<name>-fNNNNN.png. Exit 0 pass, 1 harness error, 2 could not run.
import { spawnSync } from 'node:child_process';
import { mkdirSync, writeFileSync } from 'node:fs';
import { join, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, '..', '..');
const argv = process.argv.slice(2);
function opt(name, dflt) { const i = argv.indexOf('--' + name); if (i < 0) return dflt; const v = argv[i + 1]; argv.splice(i, 2); return v; }
const url = opt('url', process.env.UE_MCP_URL || 'http://127.0.0.1:8022/mcp');
const legacy = opt('legacy', '0');
const frames = opt('frames', '60,222,223,500,650,800,950,1100,1250,1400,1550');
const cut = opt('cut', 'hockey.regular');
const res = opt('res', '2560x1440');
const slow = opt('slow', '0.5');
const name = opt('name', `cam40-${legacy === '1' ? 'legacy' : 'clamp'}-${new Date().toISOString().slice(11, 19).replace(/:/g, '')}`);
const run = join(HERE, 'out', name);
mkdirSync(run, { recursive: true });
const env = { ...process.env, MSYS_NO_PATHCONV: '1' };

function call(tool, args, extra = []) {
  const argFile = join(run, '_args.json');
  writeFileSync(argFile, JSON.stringify(args));
  const r = spawnSync(process.execPath, [join(ROOT, 'tools', 'mcp', 'mcp.mjs'), 'call', tool, '@' + argFile, '--url', url, '--timeout', '180', ...extra], { encoding: 'utf8', env });
  const lines = r.stdout.trim().split('\n');
  const verdict = (() => { try { return JSON.parse(lines.at(-1)); } catch { return { verdict: 'unparsed' }; } })();
  return { ok: r.status === 0 && verdict.verdict === 'pass', verdict, raw: r.stdout + r.stderr };
}
function py(file, vars = {}) {
  const a = [join(HERE, 'py.mjs'), file, '--url', url, '--timeout', '180'];
  for (const [k, v] of Object.entries(vars)) a.push('--var', `${k}=${v}`);
  const r = spawnSync(process.execPath, a, { encoding: 'utf8', env });
  const m = r.stdout.match(/"text":\s*"((?:[^"\\]|\\.)*)"/);
  let text = m ? JSON.parse('"' + m[1] + '"') : r.stdout;
  try { const j = JSON.parse(text); if (j.returnValue !== undefined) text = j.returnValue; } catch {}
  return { ok: r.status === 0, text, raw: r.stdout };
}
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
function step(ok, label, detail) { console.log(`${ok ? 'PASS' : 'FAIL'} ${label}${detail ? ': ' + detail : ''}`); return ok; }
function finish(code, extra = {}) { console.log(JSON.stringify({ tool: 'thirdjam.cam40/1', run, legacy, cut, frames, verdict: code === 0 ? 'pass' : 'fail', ...extra })); process.exit(code); }

(async () => {
  const t0 = Date.now();
  for (const cmd of [`jam.camera.Legacy ${legacy}`, 'jam.ScriptHuman 1', `jam.Cut ${cut}`]) {
    const w = call('ConsoleToolset.ConsoleToolset.RunConsoleCommand', { command: cmd });
    if (!step(w.ok, cmd)) finish(2, { error: w.verdict.error });
  }
  // the harness first (its tick waits for the PIE world), then PIE: that is how sim frame 60 gets its shot
  const d = py(join(HERE, 'cam40.py'), { OUT: run, FRAMES: frames, TAG: name, RES: res, SLOW: slow });
  if (!step(d.ok && /registered/.test(d.text), 'inject cam40.py', d.text.slice(0, 200))) { py(join(HERE, 'cam40stop.py')); finish(2, { error: d.text.slice(0, 300) }); }
  const s = call('EditorToolset.EditorAppToolset.StartPIE', {});
  if (!step(s.ok, 'StartPIE', `${s.verdict.ms} ms`)) { py(join(HERE, 'cam40stop.py')); finish(2, { error: s.verdict.error }); }
  await sleep(1000);
  const last = Math.max(...frames.split(',').map(Number));
  const deadline = Date.now() + (last / 60) * 1000 * (slow < 1 ? 2 / Number(slow) : 2) + 90000;
  let st = null;
  while (Date.now() < deadline) {
    await sleep(3000);
    const r = py(join(HERE, 'cam40status.py'));
    try { st = JSON.parse(r.text); } catch { console.log('WARN status unparsed: ' + r.text.slice(0, 160)); if (/no harness/.test(r.text)) break; continue; }
    console.log(`INFO frame=${st.last_frame} shots=${st.shots.length} pending=${st.pending.length} outside=${st.outside} min_margin=${st.min_margin} err=${st.err}`);
    if (st.done) break;
  }
  const stop = py(join(HERE, 'cam40stop.py'));
  step(stop.ok, 'cam40stop.py', stop.text.slice(0, 300));
  const sp = call('EditorToolset.EditorAppToolset.StopPIE', {});
  step(sp.ok, 'StopPIE', `${sp.verdict.ms} ms`);
  const hash = call('ConsoleToolset.LogsToolset.GetLogEntries', { pattern: 'JAMSIM|state_hash|StateHash|JAMCART loaded', maxEntries: 0, maxCharsPerEntry: 300 });
  writeFileSync(join(run, 'log-lines.json'), hash.raw);
  finish(st && !st.err && st.pending.length === 0 ? 0 : 1, { error: st?.err, frames_seen: st?.last_frame, shots: st?.shots, outside: st?.outside, min_margin: st?.min_margin, wall_s: Math.round((Date.now() - t0) / 1000) });
})();
