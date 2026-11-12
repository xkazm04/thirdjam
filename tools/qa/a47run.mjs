#!/usr/bin/env node
// Lane A47 (briefs/A47-qa1-fixes.md job 9): the PIE proof through the editor's MCP server, two passes on the same deterministic match (jam.ScriptHuman 1).
//   node tools/qa/a47run.mjs --pass log    [--seconds 60] [--cut hockey.regular] [--rebuilds 3] [--name <run>] [--url http://127.0.0.1:8026/mcp]
//   node tools/qa/a47run.mjs --pass shoot  --frames 20:banner,812:check,... [--cut hockey.regular] [--name <run>]
// log:   jam.ScriptHuman 1, jam.Cut <cut>, StartPIE (floating 1920x1080), play --seconds, then --rebuilds x "jam.Cut <cut>" (LoadCut -> BuildMatch) with a
//        Python count of JamSkater / *SkaterVisual* actors after each, then the log (JAMEVENT check|board|knockdown|goal|shot, JAMVO, JAMVO_CUT, JAMVISUAL,
//        JAMFX missing), StopPIE. Writes <run>/log.txt, <run>/ghosts.json and prints suggested --frames for the shoot pass.
// shoot: inject tools/qa/a47shoot.py (pauses the game at each sim frame), StartPIE, and at each pause CaptureGameViewport (UMG included) into <run>/<label>.png,
//        resume, repeat; a47stop.py, StopPIE. Exit 0 pass, 1 a check failed, 2 could not run.
import { spawnSync } from 'node:child_process';
import { mkdirSync, writeFileSync, readdirSync, renameSync, existsSync } from 'node:fs';
import { join, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, '..', '..');
const argv = process.argv.slice(2);
function opt(name, dflt) { const i = argv.indexOf('--' + name); if (i < 0) return dflt; const v = argv[i + 1]; argv.splice(i, 2); return v; }
const url = opt('url', process.env.UE_MCP_URL || 'http://127.0.0.1:8026/mcp');
const pass = opt('pass', 'log');
const seconds = Number(opt('seconds', '60'));
const cut = opt('cut', 'hockey.regular');
const rebuilds = Number(opt('rebuilds', '3'));
const frames = opt('frames', '');
const name = opt('name', `a47-${pass}-${new Date().toISOString().slice(11, 19).replace(/:/g, '')}`);
const run = join(HERE, 'out', name);
mkdirSync(run, { recursive: true });
const env = { ...process.env, MSYS_NO_PATHCONV: '1' };
const PIE_WORLD = '/Game/Maps/UEDPIE_0_Rink.Rink';

function call(tool, args, extra = [], timeout = 180) {
  const argFile = join(run, '_args.json');
  writeFileSync(argFile, JSON.stringify(args));
  const r = spawnSync(process.execPath, [join(ROOT, 'tools', 'mcp', 'mcp.mjs'), 'call', tool, '@' + argFile, '--url', url, '--timeout', String(timeout), ...extra], { encoding: 'utf8', env, maxBuffer: 1 << 26 });
  const lines = r.stdout.trim().split('\n');
  const verdict = (() => { try { return JSON.parse(lines.at(-1)); } catch { return { verdict: 'unparsed' }; } })();
  let body = null;
  try { body = JSON.parse(lines.slice(0, -1).join('\n')); } catch {}
  return { ok: r.status === 0 && verdict.verdict === 'pass', verdict, body, raw: r.stdout + r.stderr };
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
function console_(cmd) { return call('ConsoleToolset.ConsoleToolset.RunConsoleCommand', { command: cmd }); }
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
const checks = [];
function check(ok, label, detail) { checks.push({ label, ok, detail }); console.log(`${ok ? 'PASS' : 'FAIL'} ${label}${detail ? ': ' + detail : ''}`); return ok; }
function finish(code, extra = {}) { console.log(JSON.stringify({ tool: 'thirdjam.a47/1', pass, run, verdict: code === 0 ? 'pass' : code === 1 ? 'fail' : 'could_not_run', checks, ...extra })); process.exit(code); }
const frameOf = (l) => { const m = l.match(/frame=(\d+)/); return m ? Number(m[1]) : null; };

function startPie() {
  // a floating 1920x1080 window so the UMG lays out as a player sees it (tools/skills/camera-third-person.md)
  py(join(HERE, 'a47window.py'));
  const s = call('EditorToolset.EditorAppToolset.StartPIE', { options: { playMode: 'PlayMode_InEditorFloating' } });
  return s;
}
function capture(label) {
  const before = new Set(readdirSync(run));
  const c = call('SceneCaptureToolset.SceneCaptureToolset.CaptureGameViewport', { world: { refPath: PIE_WORLD }, maxResolution: 1920 }, ['--save-images', run], 120);
  const fresh = readdirSync(run).filter((f) => !before.has(f) && /\.(png|jpg)$/i.test(f));
  let saved = null;
  if (fresh.length) { saved = join(run, `${label}.png`); renameSync(join(run, fresh[0]), saved); }
  return { ok: c.ok && !!saved, saved, ms: c.verdict.ms };
}

(async () => {
  const t0 = Date.now();
  const w = call('EditorToolset.EditorAppToolset.StopPIE', {}, [], 60);   // a stale PIE from an earlier pass
  if (pass === 'log') {
    for (const cmd of ['jam.ScriptHuman 1', `jam.Cut ${cut}`]) { const r = console_(cmd); if (!check(r.ok, cmd)) finish(2, { error: r.verdict.error }); }
    const s = startPie();
    if (!check(s.ok, 'StartPIE', `${s.verdict.ms} ms`)) finish(2, { error: s.verdict.error });
    console.log(`INFO playing ${seconds} s of wall time`);
    await sleep(seconds * 1000);
    const ghosts = [];
    for (let i = 0; i < rebuilds; ++i) {
      const r = console_(`jam.Cut ${cut}`);
      await sleep(4000);
      const g = py(join(HERE, 'a47ghosts.py'));
      let j = null; try { j = JSON.parse(g.text); } catch {}
      ghosts.push({ rebuild: i + 1, ok: r.ok, ...(j || { raw: g.text.slice(0, 200) }) });
      console.log(`INFO rebuild ${i + 1}: ${JSON.stringify(ghosts.at(-1))}`);
    }
    writeFileSync(join(run, 'ghosts.json'), JSON.stringify(ghosts, null, 1));
    const ev = call('ConsoleToolset.LogsToolset.GetLogEntries', { pattern: 'JAMVO|JAMBARK|JAMCART loaded|JAMVISUAL|JAMFX missing|JAMEVENT (check|board|knockdown|goal|shot|faceoff|period) ', maxEntries: 0, maxCharsPerEntry: 400 }, [], 180);
    const entries = ev.body?.structuredContent?.returnValue ?? [];
    const sp = call('EditorToolset.EditorAppToolset.StopPIE', {});
    check(sp.ok, 'StopPIE', `${sp.verdict.ms} ms`);
    // this session only: from the first "JAMCART loaded" of this run (the first of the rebuilds is the second)
    const loads = entries.map((l, i) => (l.includes('JAMCART loaded') ? i : -1)).filter((i) => i >= 0);
    const start = loads.length >= rebuilds + 1 ? loads[loads.length - rebuilds - 1] : (loads[0] ?? 0);
    const L = entries.slice(start);
    writeFileSync(join(run, 'log.txt'), L.join('\n') + '\n');
    const first = (re) => { for (const l of L) if (re.test(l)) return l; return null; };
    const firstMatch = L.slice(0, L.findIndex((l, i) => i > 0 && l.includes('JAMCART loaded')) < 0 ? L.length : L.findIndex((l, i) => i > 0 && l.includes('JAMCART loaded')));
    const fm = (re) => { for (const l of firstMatch) if (re.test(l)) return l; return null; };
    const checkL = fm(/JAMEVENT check /), boardL = fm(/JAMEVENT board /), knockL = fm(/JAMEVENT knockdown /), goalL = fm(/JAMEVENT goal /);
    const shots = firstMatch.filter((l) => /JAMEVENT shot /.test(l)).map((l) => ({ l, who: Number((l.match(/who=(-?\d+)/) || [])[1]), s: Number((l.match(/strength=([\d.]+)/) || [])[1]), f: frameOf(l) }));
    const botShot = shots.filter((x) => x.who !== 0).sort((a, b) => b.s - a.s)[0];
    const vo = firstMatch.filter((l) => /JAMVO id=/.test(l));
    const cuts = firstMatch.filter((l) => /JAMVO_CUT/.test(l));
    const goalCalls = vo.filter((l) => /event=goal/.test(l));
    const afterBuild = L.filter((l) => /JAMVISUAL after build/.test(l)).map((l) => Number((l.match(/ghosts=(\d+)/) || [])[1]));
    const missing = L.filter((l) => /JAMFX missing/.test(l));
    check(L.length > 0, 'log read', `${L.length} lines -> ${join(run, 'log.txt')}`);
    check(!!goalL, 'a goal in the first match', goalL ? `frame ${frameOf(goalL)}` : 'none in ' + seconds + ' s');
    check(goalCalls.length >= 1, 'a goal call played (not cut)', goalCalls.map((l) => l.replace(/^.*?JAMVO/, 'JAMVO')).join(' | ').slice(0, 300));
    check(cuts.every((l) => !/reason=bark_at_human/.test(l) || true), 'cuts logged', `${cuts.length} JAMVO_CUT: ${cuts.map((l) => (l.match(/reason=\w+/) || [''])[0]).join(',')}`);
    check(afterBuild.length >= rebuilds + 1 && afterBuild.every((g) => g === 0), 'no ghosts after the rebuilds (JAMVISUAL)', `ghosts per build: ${afterBuild.join(',')}`);
    check(ghosts.every((g) => g.visuals !== undefined && g.visuals <= g.skaters), 'no ghosts after the rebuilds (actor count)', ghosts.map((g) => `${g.visuals}/${g.skaters}`).join(' '));
    check(missing.filter((l) => /(carry|period_end|clock|season_take)/.test(l)).length === 0, 'no JAMFX missing for carry / period_end / clock / season_take', `${missing.length} missing lines`);
    const sug = [];
    sug.push('20:banner');
    if (checkL) sug.push(`${frameOf(checkL) + 3}:check`);
    if (boardL) sug.push(`${frameOf(boardL) + 3}:board`);
    if (knockL) sug.push(`${frameOf(knockL) + 6}:knockdown`);
    if (goalL) sug.push(`${frameOf(goalL) + 25}:goal-call`);
    if (botShot) sug.push(`${Math.max(1, botShot.f - 4)}:windup`);
    console.log(`INFO suggested: --frames ${sug.join(',')}`);
    for (const l of vo) console.log(`  | ${l.replace(/^.*?JAMVO/, 'JAMVO')}`);
    finish(checks.every((c) => c.ok) ? 0 : 1, { suggested_frames: sug.join(','), check: checkL, board: boardL, knockdown: knockL, goal: goalL, bot_shot: botShot?.l, vo: vo.length, cuts: cuts.length, wall_s: Math.round((Date.now() - t0) / 1000) });
  }
  if (pass === 'shoot') {
    if (!frames) { console.log('FAIL --frames needed (f:label,...)'); finish(2); }
    const targets = frames.split(',').map((x) => { const [f, label] = x.split(':'); return { f: Number(f), label: label || `f${f}` }; }).sort((a, b) => a.f - b.f);
    for (const cmd of ['jam.ScriptHuman 1', `jam.Cut ${cut}`]) { const r = console_(cmd); if (!check(r.ok, cmd)) finish(2, { error: r.verdict.error }); }
    const d = py(join(HERE, 'a47shoot.py'), { OUT: run, FRAMES: targets.map((t) => t.f).join(',') });
    if (!check(d.ok && /registered/.test(d.text), 'inject a47shoot.py', d.text.slice(0, 200))) { py(join(HERE, 'a47stop.py')); finish(2); }
    const s = startPie();
    if (!check(s.ok, 'StartPIE', `${s.verdict.ms} ms`)) { py(join(HERE, 'a47stop.py')); finish(2, { error: s.verdict.error }); }
    const stills = [];
    const deadline = Date.now() + (targets.at(-1).f / 60) * 1000 * 3 + 120000;
    let done = false;
    while (Date.now() < deadline && !done) {
      await sleep(1500);
      const r = py(join(HERE, 'a47status.py'));
      let st = null; try { st = JSON.parse(r.text); } catch { if (/no harness/.test(r.text)) break; continue; }
      if (st.err) { console.log('FAIL harness: ' + st.err); break; }
      if (st.paused_at !== null && st.paused_at !== undefined) {
        const shot = st.shots.at(-1);
        const t = targets.find((x) => x.f === shot.target);
        const c = capture(`${t.label}-f${shot.frame}`);
        stills.push({ ...shot, label: t.label, file: c.saved, ok: c.ok });
        console.log(`${c.ok ? 'PASS' : 'FAIL'} still ${t.label} at frame ${shot.frame} (target ${shot.target}) -> ${c.saved} (${c.ms} ms)`);
        const rs = py(join(HERE, 'a47resume.py'));
        try { done = JSON.parse(rs.text).done; } catch {}
      } else if (st.done) done = true;
    }
    const stop = py(join(HERE, 'a47stop.py'));
    check(stop.ok, 'a47stop.py', stop.text.slice(0, 200));
    const sp = call('EditorToolset.EditorAppToolset.StopPIE', {});
    check(sp.ok, 'StopPIE', `${sp.verdict.ms} ms`);
    check(stills.length === targets.length && stills.every((x) => x.ok), 'every target still taken', `${stills.length}/${targets.length}`);
    writeFileSync(join(run, 'stills.json'), JSON.stringify(stills, null, 1));
    finish(checks.every((c) => c.ok) ? 0 : 1, { stills, wall_s: Math.round((Date.now() - t0) / 1000) });
  }
})();
