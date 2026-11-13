#!/usr/bin/env node
// Lane A69: (1) frame cost of the clay zone rings: GetFrameTimings sampled in PIE on a six-zone clay cut, rings on (the new clay) vs rings off (decal quads, glass and rail
// drawn = what clay was before A69), interleaved, 3 rounds; (2) three live switches (jam.livecut night / clay) with the component count after each.
//   node tools/qa/a69run.mjs --url http://127.0.0.1:8034/mcp --dir <Game/Saved/a69>
import { spawnSync } from 'node:child_process';
import { writeFileSync } from 'node:fs';
import { join, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
const HERE = dirname(fileURLToPath(import.meta.url)), ROOT = resolve(HERE, '..', '..');
const a = process.argv.slice(2);
const opt = (n, d) => { const i = a.indexOf('--' + n); return i < 0 ? d : a[i + 1]; };
const url = opt('url', 'http://127.0.0.1:8034/mcp'), dir = resolve(opt('dir', join(ROOT, 'Game/Saved/a69')));
const env = { ...process.env, MSYS_NO_PATHCONV: '1' };
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
function call(tool, args) {
  const f = join(dir, '_a.json'); writeFileSync(f, JSON.stringify(args));
  const r = spawnSync(process.execPath, [join(ROOT, 'tools/mcp/mcp.mjs'), 'call', tool, '@' + f, '--url', url, '--timeout', '120'], { encoding: 'utf8', env, maxBuffer: 1 << 26 });
  const lines = r.stdout.trim().split('\n'); let body = null; try { body = JSON.parse(lines.slice(0, -1).join('\n')); } catch {}
  return body?.structuredContent?.returnValue ?? body;
}
const con = (c) => call('ConsoleToolset.ConsoleToolset.RunConsoleCommand', { command: c });
function py(tier) {
  const src = join(HERE, 'a69set.py'); const r = spawnSync(process.execPath, [join(HERE, 'py.mjs'), src, '--url', url, '--var', 'TIER=' + tier], { encoding: 'utf8', env });
  const m = r.stdout.match(/"returnValue":\s*"((?:[^"\\]|\\.)*)"/); try { return JSON.parse(JSON.parse('"' + m[1] + '"')); } catch { return r.stdout.slice(0, 200); }
}
const clay = join(dir, 'a69.clay.cartridge.json').split('\\').join('/'), night = join(dir, 'a69.night.cartridge.json').split('\\').join('/');
(async () => {
  call('EditorToolset.EditorAppToolset.StopPIE', {});
  con('jam.ScriptHuman 1'); con(`jam.Cut ${clay}`);
  call('EditorToolset.EditorAppToolset.StartPIE', { options: { playMode: 'PlayMode_InEditorFloating' } });
  await sleep(15000);
  const res = { on: [], off: [], switches: [] };
  const sample = (key, n = 20) => { for (let i = 0; i < n; i++) { const t = call('PerfToolset.StatsToolset.GetFrameTimings', {}); res[key].push(t); } };
  for (let round = 0; round < 3; round++) {
    for (const [key, tier] of [['off', 'x'], ['on', 'p3_clay']]) { py(tier); await sleep(2500); sample(key); }
  }
  py('p3_clay');
  res.start = py('count');
  for (let i = 1; i <= 3; i++) {
    con(`jam.livecut ${night}`); await sleep(5000); const n = py('count');
    con(`jam.livecut ${clay}`); await sleep(5000); const c = py('count');
    res.switches.push({ i, night: n, clay: c });
  }
  call('EditorToolset.EditorAppToolset.StopPIE', {});
  writeFileSync(join(dir, 'a69-cost.json'), JSON.stringify(res, null, 1));
  const m = (k, f) => { const v = res[k].map((t) => t?.[f]).filter((x) => typeof x === 'number'); return v.length ? (v.reduce((s, x) => s + x, 0) / v.length).toFixed(3) + ' (n=' + v.length + ')' : 'n/a'; };
  console.log('sample keys', JSON.stringify(res.on[0]));
  for (const f of Object.keys(res.on[0] || {})) if (typeof res.on[0][f] === 'number') console.log(f, 'off', m('off', f), 'on', m('on', f));
  console.log(JSON.stringify(res.start), JSON.stringify(res.switches));
})();
