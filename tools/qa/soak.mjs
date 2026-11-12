#!/usr/bin/env node
// Lane A10 GPU soak (R-0016 point 6): one editor on port 8010, PIE on the default cut with the scripted human for 60 s,
// jam.playcut hockey.boost at 20 s and hockey.regular at 40 s, stat gpu + ProfileGPU in the log, then grep the editor log.
//   node tools/qa/soak.mjs --log <editor abslog> --name soak-1 [--url http://127.0.0.1:8010/mcp] [--vsm0] [--seconds 60]
// Exit 0 ran (hung or not is in the JSON), 2 could not run.
import { spawnSync } from 'node:child_process';
import { mkdirSync, writeFileSync, readFileSync, existsSync, readdirSync, renameSync } from 'node:fs';
import { join, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, '..', '..');
const argv = process.argv.slice(2);
function opt(name, dflt) { const i = argv.indexOf('--' + name); if (i < 0) return dflt; const v = argv[i + 1]; argv.splice(i, 2); return v; }
function flag(name) { const i = argv.indexOf('--' + name); if (i < 0) return false; argv.splice(i, 1); return true; }
const url = opt('url', 'http://127.0.0.1:8010/mcp');
const logPath = opt('log', '');
const name = opt('name', 'soak-1');
const seconds = Number(opt('seconds', '60'));
const vsm0 = flag('vsm0');
const run = join(HERE, 'out', name);
mkdirSync(join(run, 'shots'), { recursive: true });
const env = { ...process.env, MSYS_NO_PATHCONV: '1' };
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
const t0 = Date.now();
const marks = [];
function mark(label, extra) { const m = { t: +((Date.now() - t0) / 1000).toFixed(1), label, ...extra }; marks.push(m); console.log(`${m.ok === false ? 'FAIL' : 'INFO'} ${m.t}s ${label}${extra?.detail ? ': ' + extra.detail : ''}`); }

function call(tool, args, extra = [], timeout = 60) {
  const argFile = join(run, '_args.json');
  writeFileSync(argFile, JSON.stringify(args));
  const r = spawnSync(process.execPath, [join(ROOT, 'tools', 'mcp', 'mcp.mjs'), 'call', tool, '@' + argFile, '--url', url, '--timeout', String(timeout), ...extra], { encoding: 'utf8', env });
  const lines = r.stdout.trim().split('\n');
  let verdict = {}; try { verdict = JSON.parse(lines.at(-1)); } catch {}
  let body = null; try { body = JSON.parse(lines.slice(0, -1).join('\n')); } catch {}
  return { ok: r.status === 0 && verdict.verdict === 'pass', verdict, body, text: body?.content?.find((c) => c.type === 'text')?.text ?? '' };
}
function console_(cmd) { const r = call('ConsoleToolset.ConsoleToolset.RunConsoleCommand', { command: cmd }); mark('console ' + cmd, { ok: r.ok, detail: r.ok ? '' : r.verdict.error }); return r; }
function logLines(pattern, max = 0) { const r = call('ConsoleToolset.LogsToolset.GetLogEntries', { pattern, maxEntries: max, maxCharsPerEntry: 600 }); return r.body?.structuredContent?.returnValue ?? []; }
function capture(label) {
  const c = call('SceneCaptureToolset.SceneCaptureToolset.CaptureGameViewport', { world: { refPath: '/Game/Maps/UEDPIE_0_Rink.Rink' }, maxResolution: 1280 }, ['--save-images', join(run, 'shots')]);
  if (c.ok) { const newest = readdirSync(join(run, 'shots')).filter((f) => f.startsWith('mcp-image')).sort().at(-1); if (newest) renameSync(join(run, 'shots', newest), join(run, 'shots', `${label}.png`)); }
  mark('capture ' + label, { ok: c.ok, detail: c.ok ? '' : c.verdict.error });
  return c.ok;
}
function alive() { const r = call('ConsoleToolset.LogsToolset.GetLogEntries', { pattern: 'ZZZ_never', maxEntries: 1 }); return r.ok || (r.verdict.verdict !== 'unreachable'); }
function grepEditorLog() {
  if (!logPath || !existsSync(logPath)) return { error: 'no editor log at ' + logPath };
  const text = readFileSync(logPath, 'utf8');
  const lines = text.split('\n');
  const pick = (re) => lines.filter((l) => re.test(l)).map((l) => l.trim().slice(0, 300));
  return {
    crash: pick(/GPU crash|DEVICE_HUNG|Device Removed|DXGI_ERROR|TerminateOnGPUCrash/),
    vsm_warnings: pick(/Warning.*(VirtualShadow|Virtual Shadow)|(VirtualShadow|Virtual Shadow).*Warning/),
    dump: pick(/nv-gpudmp/),
    driver: pick(/LogD3D12RHI:.*(Driver Version|DriverVersion|NVIDIA|Adapter Name)|LogRHI:.*Driver|Found D3D12 adapter/).slice(0, 6),
    jamcart: pick(/JAMCART loaded|JAMLOOK|JAMDESIGNER jam.playcut/),
    profilegpu: pick(/^\s*[\d.]+%\s+[\d.]+ms|ProfileGPU|Total.*ms|ShadowDepths|Lights\b/).filter((l) => /ms/.test(l)).slice(0, 80),
    last: lines.slice(-6).map((l) => l.trim().slice(0, 200)),
  };
}

(async () => {
  if (vsm0) console_('r.Shadow.Virtual.Enable 0');
  console_('jam.ScriptHuman 1');
  const s = call('EditorToolset.EditorAppToolset.StartPIE', {});
  mark('StartPIE', { ok: s.ok, detail: s.ok ? `${s.verdict.ms} ms` : s.verdict.error });
  if (!s.ok) { writeFileSync(join(run, 'soak.json'), JSON.stringify({ marks, log: grepEditorLog() }, null, 2)); process.exit(2); }
  await sleep(3000);
  const cart = logLines('JAMCART loaded|JAMLOOK', 6);
  mark('cartridge', { detail: cart.map((l) => l.replace(/^\[[^\]]*\]\[\s*\d+\]/, '')).join(' | ') });
  console_('stat gpu');
  await sleep(2000);
  capture('t05-stat-gpu');
  console_('ProfileGPU');
  let hung = false;
  const t1 = Date.now();
  const steps = [[20, 'jam.playcut hockey.boost'], [40, 'jam.playcut hockey.regular']];
  let si = 0;
  while ((Date.now() - t1) / 1000 < seconds) {
    await sleep(1000);
    const el = (Date.now() - t1) / 1000;
    if (si < steps.length && el >= steps[si][0]) {
      const r = console_(steps[si][1]);
      if (!r.ok && /unreachable|ECONN/.test(JSON.stringify(r.verdict))) { hung = true; break; }
      await sleep(2500);
      capture(`t${Math.round(el + 2)}-${steps[si][1].split(' ')[1]}`);
      console_('ProfileGPU');
      si++;
    } else if (Math.round(el) % 10 === 0) {
      if (!alive()) { hung = true; mark('editor unreachable', { ok: false }); break; }
    }
  }
  if (!hung) {
    capture('t60-end');
    console_('stat gpu');
    const sp = call('EditorToolset.EditorAppToolset.StopPIE', {});
    mark('StopPIE', { ok: sp.ok, detail: sp.ok ? `${sp.verdict.ms} ms` : sp.verdict.error });
    if (!sp.ok && /unreachable|ECONN/.test(JSON.stringify(sp.verdict))) hung = true;
  }
  await sleep(2000);
  const log = grepEditorLog();
  if (log.crash?.length) hung = true;
  const out = { tool: 'thirdjam.qa.soak/1', name, vsm0, seconds, hung, marks, log };
  writeFileSync(join(run, 'soak.json'), JSON.stringify(out, null, 2));
  console.log(JSON.stringify({ tool: 'thirdjam.qa.soak/1', name, vsm0, hung, crash: log.crash?.slice(0, 3), dump: log.dump?.slice(0, 2), vsm_warnings: log.vsm_warnings?.length, profilegpu_lines: log.profilegpu?.length }));
})();
