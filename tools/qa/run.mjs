#!/usr/bin/env node
// Lane A10: one feel-QA run end to end, through the editor's MCP server (default port 8010).
//   node tools/qa/run.mjs --plan skate|play|raw|boost --cut <cartridge json path> [--frames 3600] [--shots 10] [--name <run>] [--url ...]
// Steps: jam.Cut <path> (console) -> StartPIE -> inject tools/qa/drive.py -> poll tools/qa/status.py every 4 s (a viewport capture with
// the HUD at every phase change, into <run>/shots/) -> tools/qa/stop.py -> StopPIE -> JAMEVENT and JAMSIM log lines into <run>/events.txt
// -> tools/qa/analyze.mjs. Output: tools/qa/out/<run>/. Exit 0 pass, 1 the harness reported an error, 2 could not run.
import { spawnSync } from 'node:child_process';
import { mkdirSync, writeFileSync, readFileSync, existsSync, readdirSync, renameSync } from 'node:fs';
import { join, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, '..', '..');
const argv = process.argv.slice(2);
function opt(name, dflt) { const i = argv.indexOf('--' + name); if (i < 0) return dflt; const v = argv[i + 1]; argv.splice(i, 2); return v; }
const url = opt('url', process.env.UE_MCP_URL || 'http://127.0.0.1:8010/mcp');
const plan = opt('plan', 'play');
const cut = resolve(opt('cut', join(HERE, 'cartridges', 'hockey.regular.qa.json')));
const frames = opt('frames', '3600');
const shots = opt('shots', '10');
const name = opt('name', `${plan}-${new Date().toISOString().slice(11, 19).replace(/:/g, '')}`);
const run = join(HERE, 'out', name);
mkdirSync(join(run, 'shots'), { recursive: true });
const env = { ...process.env, MSYS_NO_PATHCONV: '1' };

function call(tool, args, extra = []) {
  const argFile = join(run, '_args.json');
  writeFileSync(argFile, JSON.stringify(args));
  const r = spawnSync(process.execPath, [join(ROOT, 'tools', 'mcp', 'mcp.mjs'), 'call', tool, '@' + argFile, '--url', url, '--timeout', '120', ...extra], { encoding: 'utf8', env });
  const lines = r.stdout.trim().split('\n');
  const verdict = (() => { try { return JSON.parse(lines.at(-1)); } catch { return { verdict: 'unparsed' }; } })();
  let body = null;
  try { body = JSON.parse(lines.slice(0, -1).join('\n')); } catch {}
  const text = body?.content?.find((c) => c.type === 'text')?.text ?? '';
  return { ok: r.status === 0 && verdict.verdict === 'pass', verdict, body, text, raw: r.stdout + r.stderr };
}
function py(file, vars = {}) {
  const a = [join(HERE, 'py.mjs'), file, '--url', url];
  for (const [k, v] of Object.entries(vars)) a.push('--var', `${k}=${v}`);
  const r = spawnSync(process.execPath, a, { encoding: 'utf8', env });
  const m = r.stdout.match(/"text":\s*"((?:[^"\\]|\\.)*)"/);
  let text = m ? JSON.parse('"' + m[1] + '"') : r.stdout;
  try { const j = JSON.parse(text); if (j.returnValue !== undefined) text = j.returnValue; } catch {}
  return { ok: r.status === 0, text, raw: r.stdout };
}
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
function step(ok, label, detail) { console.log(`${ok ? 'PASS' : 'FAIL'} ${label}${detail ? ': ' + detail : ''}`); return ok; }
function finish(code, extra = {}) { console.log(JSON.stringify({ tool: 'thirdjam.qa/1', run, plan, cut, verdict: code === 0 ? 'pass' : 'fail', ...extra })); process.exit(code); }

(async () => {
  const t0 = Date.now();
  const w = call('ConsoleToolset.ConsoleToolset.RunConsoleCommand', { command: `jam.Cut ${cut.replace(/\\/g, '/')}` });
  if (!step(w.ok, 'jam.Cut', cut)) finish(2, { error: w.verdict.error });
  const s = call('EditorToolset.EditorAppToolset.StartPIE', {});
  if (!step(s.ok, 'StartPIE', `${s.verdict.ms} ms`)) finish(2, { error: s.verdict.error });
  await sleep(2500);
  const d = py(join(HERE, 'drive.py'), { OUT: run, PLAN: plan, SHOT_EVERY: shots, MAX_FRAMES: frames });
  if (!step(d.ok && /registered/.test(d.text), 'inject drive.py', d.text.slice(0, 200))) { py(join(HERE, 'stop.py')); call('EditorToolset.EditorAppToolset.StopPIE', {}); finish(2, { error: d.text.slice(0, 300) }); }
  let lastPhase = null, lastStatus = null, captures = 0;
  // the Python tick runs at about 6-20 sim frames per wall second (reflection reads, captures): budget 12x real time
  const deadline = Date.now() + (Number(frames) / 60) * 1000 * 12 + 60000;
  while (Date.now() < deadline) {
    await sleep(4000);
    const st = py(join(HERE, 'status.py'));
    let j = null;
    try { j = JSON.parse(st.text); } catch { console.log('WARN status unparsed: ' + st.text.slice(0, 160)); if (/no harness/.test(st.text)) break; continue; }
    lastStatus = j;
    const key = `${j.phase}:${j.phase_name}`;
    if (key !== lastPhase) {
      lastPhase = key;
      const c = call('SceneCaptureToolset.SceneCaptureToolset.CaptureGameViewport', { world: { refPath: '/Game/Maps/UEDPIE_0_Rink.Rink' }, maxResolution: 1280 }, ['--save-images', join(run, 'shots')]);
      if (c.ok) {
        const newest = readdirSync(join(run, 'shots')).filter((f) => f.startsWith('mcp-image')).sort().at(-1);
        if (newest) renameSync(join(run, 'shots', newest), join(run, 'shots', `hud-${String(j.phase).padStart(2, '0')}-${j.phase_name}-f${j.sim_frame_last}.png`));
        captures++;
      }
    }
    console.log(`INFO frame=${j.sim_frame_last} n=${j.n} phase=${j.phase}/${j.phase_name} first_touch=${j.first_touch} goals=${JSON.stringify(j.goal_frames)} shots=${j.shots} err=${j.err}`);
    if (j.done) break;
  }
  const stop = py(join(HERE, 'stop.py'));
  step(stop.ok, 'stop.py', stop.text.slice(0, 120));
  const sp = call('EditorToolset.EditorAppToolset.StopPIE', {});
  step(sp.ok, 'StopPIE', `${sp.verdict.ms} ms`);
  const ev = call('ConsoleToolset.LogsToolset.GetLogEntries', { pattern: 'JAMEVENT|JAMSIM|JAMCART|JAMLOOK|LogHokej', maxEntries: 0, maxCharsPerEntry: 400 });
  const entries = ev.body?.structuredContent?.returnValue ?? [];
  // keep only the lines of this PIE session: everything after the last "JAMCART loaded" line
  let start = 0;
  entries.forEach((l, i) => { if (l.includes('JAMCART loaded')) start = i; });
  writeFileSync(join(run, 'events.txt'), entries.slice(start).join('\n') + '\n');
  step(ev.ok, 'events', `${entries.length - start} lines`);
  const an = spawnSync(process.execPath, [join(HERE, 'analyze.mjs'), run], { encoding: 'utf8', env });
  process.stdout.write(an.stdout);
  const err = lastStatus?.err;
  finish(err ? 1 : 0, { error: err, frames: lastStatus?.sim_frame_last, captures, wall_s: Math.round((Date.now() - t0) / 1000) });
})();
