import { spawnSync } from 'node:child_process';
import { writeFileSync } from 'node:fs';
import { join, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
const HERE = dirname(fileURLToPath(import.meta.url)), ROOT = resolve(HERE, '..', '..');
const url = 'http://127.0.0.1:8034/mcp', dir = join(ROOT, 'Game/Saved/a69'), env = { ...process.env, MSYS_NO_PATHCONV: '1' };
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
function call(tool, args) { const f = join(dir, '_a.json'); writeFileSync(f, JSON.stringify(args)); const r = spawnSync(process.execPath, [join(ROOT, 'tools/mcp/mcp.mjs'), 'call', tool, '@' + f, '--url', url, '--timeout', '120'], { encoding: 'utf8', env, maxBuffer: 1 << 26 }); const l = r.stdout.trim().split('\n'); let b = null; try { b = JSON.parse(l.slice(0, -1).join('\n')); } catch {} return b?.structuredContent?.returnValue ?? b; }
function py(mode) { const r = spawnSync(process.execPath, [join(HERE, 'py.mjs'), join(HERE, 'a69floor.py'), '--url', url, '--var', 'MODE=' + mode], { encoding: 'utf8', env }); const i = r.stdout.indexOf('"structuredContent"'); return i < 0 ? r.stdout.slice(0, 300) : r.stdout.slice(i, i + 400); }
(async () => {
  call('EditorToolset.EditorAppToolset.StopPIE', {});
  call('ConsoleToolset.ConsoleToolset.RunConsoleCommand', { command: 'jam.ScriptHuman 1' });
  call('ConsoleToolset.ConsoleToolset.RunConsoleCommand', { command: 'jam.Cut ' + join(dir, 'a69.clay.cartridge.json').split(String.fromCharCode(92)).join('/') });
  call('EditorToolset.EditorAppToolset.StartPIE', { options: { playMode: 'PlayMode_InEditorFloating' } });
  await sleep(15000);
  console.log('info', py('info'));
  const res = { on: [], off: [] };
  for (let r = 0; r < 3; r++) for (const k of ['on', 'off']) { py(k); await sleep(2500); for (let i = 0; i < 20; i++) res[k].push(call('PerfToolset.StatsToolset.GetFrameTimings', {}).gpuMs); }
  call('EditorToolset.EditorAppToolset.StopPIE', {});
  const mean = (v) => (v.reduce((s, x) => s + x, 0) / v.length).toFixed(3);
  console.log('gpuMs floor shadow on', mean(res.on), 'off', mean(res.off), 'n', res.on.length);
})();
