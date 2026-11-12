#!/usr/bin/env node
// Lane A32: the PIE proof of the commentator through the editor's MCP server (brief A32 job 5).
//   node tools/commentary/proof.mjs [--url http://127.0.0.1:8018/mcp] [--seconds 60] [--name <run>] [--say goal.blue.01,save.big.02,...]
// Steps: jam.ScriptHuman 1 (console) -> StartPIE (the Rink map is the editor's startup map) -> wait --seconds of wall time -> jam.say_line on five ids
// -> read the log (JAMVO, JAMVO_CUT, JAMBARK, JAMEVENT goal/faceoff/win/lose/period/the seven) -> StopPIE -> tools/commentary/out/<run>/log.txt and
// the analysis: JAMVO starts at least 150 sim frames apart, stoppage lines only in a pause (between a goal / period / win and the next drop), no JAMVO
// start inside the 150 frames after a JAMBARK, no missing VO_Commentator_* line. Exit 0 every check passed, 1 a check failed, 2 could not run.
import { spawnSync } from 'node:child_process';
import { mkdirSync, writeFileSync, readFileSync } from 'node:fs';
import { join, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const HERE = dirname(fileURLToPath(import.meta.url));
const ROOT = resolve(HERE, '..', '..');
const argv = process.argv.slice(2);
function opt(name, dflt) { const i = argv.indexOf('--' + name); if (i < 0) return dflt; const v = argv[i + 1]; argv.splice(i, 2); return v; }
const url = opt('url', process.env.UE_MCP_URL || 'http://127.0.0.1:8018/mcp');
const seconds = Number(opt('seconds', '60'));
const name = opt('name', `proof-${new Date().toISOString().slice(11, 19).replace(/:/g, '')}`);
const say = opt('say', 'goal.blue.01,save.big.02,breakaway.human.01,period.take_1.01,win.shutout.01').split(',');
const run = join(HERE, 'out', name);
mkdirSync(run, { recursive: true });
const env = { ...process.env, MSYS_NO_PATHCONV: '1' };

const STOPPAGE_EVENTS = ['period', 'period_end', 'season_take', 'win', 'lose'];
const GATE = 150;

function call(tool, args, timeout = 120) {
  const argFile = join(run, '_args.json');
  writeFileSync(argFile, JSON.stringify(args));
  const r = spawnSync(process.execPath, [join(ROOT, 'tools', 'mcp', 'mcp.mjs'), 'call', tool, '@' + argFile, '--url', url, '--timeout', String(timeout)], { encoding: 'utf8', env });
  const lines = r.stdout.trim().split('\n');
  let verdict = { verdict: 'unparsed' };
  try { verdict = JSON.parse(lines.at(-1)); } catch {}
  let body = null;
  try { body = JSON.parse(lines.slice(0, -1).join('\n')); } catch {}
  return { ok: r.status === 0 && verdict.verdict === 'pass', verdict, body, raw: r.stdout + r.stderr };
}
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
const checks = [];
function check(ok, label, detail) { checks.push({ label, ok, detail }); console.log(`${ok ? 'PASS' : 'FAIL'} ${label}${detail ? ': ' + detail : ''}`); return ok; }
function finish(code, extra = {}) {
  console.log(JSON.stringify({ tool: 'thirdjam.commentary.proof/1', run, verdict: code === 0 ? 'pass' : code === 1 ? 'fail' : 'could_not_run', checks, ...extra }));
  process.exit(code);
}

// Pulls the frame= of a log line.
const frameOf = (l) => { const m = l.match(/frame=(\d+)/); return m ? Number(m[1]) : null; };

export function analyze(lines) {
  // Keep this PIE session only: everything after the last "JAMCART loaded".
  let start = 0;
  lines.forEach((l, i) => { if (l.includes('JAMCART loaded')) start = i; });
  const L = lines.slice(start);
  // A JAMVO line is forced when a "jam.say_line <id>" console line precedes it in the log (jam.say_line plays regardless of the rules, by design).
  const forcedIds = [];
  const vo = [];
  for (const l of L) {
    const m = l.match(/jam\.say_line (\S+)$/);
    if (m) { forcedIds.push(m[1]); continue; }
    if (!/JAMVO id=/.test(l)) continue;
    const id = l.match(/id=(\S+)/)[1];
    const fi = forcedIds.indexOf(id);
    const forced = fi >= 0;
    if (forced) forcedIds.splice(fi, 1);
    vo.push({ line: l, id, event: l.match(/event=(\S+)/)[1], frame: frameOf(l), dur: Number((l.match(/dur=(\d+)/) || [0, 0])[1]), forced });
  }
  // Barks addressed to the human (the booth yields to these): every bark but goal_against and the faceoff taunt (goal_for by an opponent is a complaint
  // about a bot teammate's goal and does not count either; the log has no speaker team, so goal_for is counted as at the human: the stricter reading).
  const barks = L.filter((l) => /JAMBARK who=/.test(l) && !/situation=(goal_against|faceoff) /.test(l)).map(frameOf).filter((f) => f !== null);
  const ev = (re) => L.filter((l) => new RegExp(`JAMEVENT ${re} `).test(l)).map(frameOf);
  const goals = ev('goal'), drops = L.filter((l) => /JAMEVENT faceoff .*kind=drop/.test(l)).map(frameOf), ends = [...ev('win'), ...ev('lose')], periods = ev('period');
  // Pause windows: [period or goal or end frame, next drop frame) (an end lasts to the end of the log).
  const windows = [];
  for (const f of [...goals, ...periods]) { const next = drops.find((d) => d > f); windows.push([f, next ?? Infinity]); }
  for (const f of ends) windows.push([f, Infinity]);
  const inPause = (f) => windows.some(([a, b]) => f >= a && f < b);
  const results = { lines: vo.length, barks: barks.length, goals: goals.length, drops: drops.length, spacingMin: null, spacingBad: [], stoppageBad: [], overlapBad: [], missing: L.filter((l) => /JAMVO missing/.test(l)).length };
  const auto = vo.filter((v) => !v.forced);
  for (let i = 1; i < auto.length; i++) {
    const gap = auto[i].frame - auto[i - 1].frame;
    results.spacingMin = results.spacingMin === null ? gap : Math.min(results.spacingMin, gap);
    if (gap < GATE) results.spacingBad.push(`${auto[i - 1].id}@${auto[i - 1].frame} -> ${auto[i].id}@${auto[i].frame} (${gap})`);
    if (auto[i].frame < auto[i - 1].frame + auto[i - 1].dur) results.overlapBad.push(`${auto[i].id}@${auto[i].frame} starts inside ${auto[i - 1].id} (ends ${auto[i - 1].frame + auto[i - 1].dur})`);
  }
  for (const v of auto) {
    const stoppage = STOPPAGE_EVENTS.includes(v.event) || v.id.startsWith('goal.pause.');
    if (stoppage && !inPause(v.frame)) results.stoppageBad.push(`${v.id}@${v.frame}`);
    for (const b of barks) if (v.frame > b && v.frame - b < GATE) results.overlapBad.push(`${v.id}@${v.frame} inside the hold of a bark at ${b}`);
  }
  return { results, vo, windows, session: L };
}

if (process.argv[1] && /proof\.mjs$/.test(process.argv[1].replace(/\\/g, '/'))) {
  (async () => {
    const t0 = Date.now();
    const w = call('ConsoleToolset.ConsoleToolset.RunConsoleCommand', { command: 'jam.ScriptHuman 1' });
    if (!check(w.ok, 'jam.ScriptHuman 1', w.verdict.error)) finish(2);
    call('ConsoleToolset.LogsToolset.SetVerbosity', { logCategory: 'LogJamCommentary', verbosity: 'Verbose' });
    const s = call('EditorToolset.EditorAppToolset.StartPIE', {}, 180);
    if (!check(s.ok, 'StartPIE', `${s.verdict.ms} ms`)) finish(2, { error: s.verdict.error });
    console.log(`INFO playing ${seconds} s of wall time`);
    await sleep(seconds * 1000);
    const said = [];
    for (const id of say) {
      const r = call('ConsoleToolset.ConsoleToolset.RunConsoleCommand', { command: `jam.say_line ${id}` });
      said.push({ id, ok: r.ok });
      await sleep(2600);
    }
    check(said.every((x) => x.ok), 'jam.say_line x5', said.map((x) => x.id).join(', '));
    const ev = call('ConsoleToolset.LogsToolset.GetLogEntries', { pattern: 'JAMVO|JAMBARK|JAMCART loaded|JAMEVENT (goal|faceoff|win|lose|period|save|post|breakaway|carry|period_end|clock|season_take) |JAMFX missing|jam\\.say_line|JAMSIM', maxEntries: 0, maxCharsPerEntry: 400 }, 180);
    const entries = ev.body?.structuredContent?.returnValue ?? [];
    const sp = call('EditorToolset.EditorAppToolset.StopPIE', {}, 180);
    check(sp.ok, 'StopPIE', `${sp.verdict.ms} ms`);
    const { results, vo, session } = analyze(entries);
    writeFileSync(join(run, 'log.txt'), session.join('\n') + '\n');
    writeFileSync(join(run, 'lines.txt'), vo.map((v) => v.line).join('\n') + '\n');
    check(ev.ok && session.length > 0, 'log read', `${session.length} lines of this session -> ${join(run, 'log.txt')}`);
    check(vo.length >= 3, 'lines played', `${vo.length} JAMVO lines (${results.goals} goals, ${results.drops} drops, ${results.barks} barks)`);
    check(results.spacingBad.length === 0, 'spacing >= 150 frames', results.spacingBad.length ? results.spacingBad.join('; ') : `min gap ${results.spacingMin} frames`);
    check(results.overlapBad.length === 0, 'one line at a time, none inside a bark hold', results.overlapBad.join('; '));
    check(results.stoppageBad.length === 0, 'stoppage lines only in a pause', results.stoppageBad.join('; '));
    check(results.missing === 0, 'no missing VO_Commentator_*', `${results.missing} missing`);
    const forced = vo.filter((v) => v.forced).length;
    for (const v of vo) console.log(`  | ${v.line.replace(/^.*?JAMVO/, 'JAMVO')}`);
    finish(checks.every((c) => c.ok) ? 0 : 1, { ...results, forced, wall_s: Math.round((Date.now() - t0) / 1000) });
  })();
}
