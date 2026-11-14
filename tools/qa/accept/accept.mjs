#!/usr/bin/env node
// Lane A87 (R-0043 point 1): the acceptance run's verdict. Reads the game log of a jam.ctrl.Drill 20 run (and the driver's events.log) and prints the step
// table, pass or fail per verb, with the facts; the input lines that reached the game (JAMCTRL) and the shot read-outs (JAMSHOT release) are attached so a
// failure says whether the press arrived and what the sim saw. The last line is {"tool":"thirdjam.accept/1",...}. Exit 0 every verb passed, 1 a verb failed
// or never reported, 2 could not run (no log, the drill never began), 3 machine (every failing row has a machine reason: see A131 in ACCEPT.md).
//   node tools/qa/accept/accept.mjs <game.log> [events.log]
import { readFileSync, existsSync } from 'node:fs';

const [logPath, evPath] = process.argv.slice(2);
const STEPS = {
  1: { name: 'Skate, sprint, brake (W, Shift, S)', verbs: ['skate', 'sprint', 'sprint_carry', 'brake'] },
  2: { name: 'Aim (the cursor at four points)', verbs: ['aim'] },
  3: { name: 'Carry (sweeps at full reach)', verbs: ['carry'] },
  4: { name: 'Shoot (LMB with the puck), blade extended (tap, 8, 30 frames)', verbs: ['shoot_tap', 'shoot_half', 'shoot_full'] },
  5: { name: 'Pass (RMB with the puck), blade extended (tap, held = saucer)', verbs: ['pass', 'saucer'] },
  6: { name: 'Receive and one-timer', verbs: ['onetimer'] },
  7: { name: 'Check (RMB without the puck) with closing speed, cross-check (LMB without the puck) from the front: hits by momentum (A92)', verbs: ['check', 'crosscheck'] },
  8: { name: 'Be checked at full speed while standing: down for the frames the hit earned (A92)', verbs: ['checked'] },
  9: { name: 'Deke (wheel), spin (E), sweep', verbs: ['deke', 'spin', 'sweep'] },
  10: { name: 'Pause and resume (Esc), then aim', verbs: ['resume_aim'] },
};
function out(o, code) { console.log(JSON.stringify({ tool: 'thirdjam.accept/1', ...o, exit: code })); process.exit(code); }
if (!logPath || !existsSync(logPath)) { console.log(`FAIL no game log ${logPath ?? ''}`); out({ verdict: 'error', reason: 'no game log' }, 2); }
const lines = readFileSync(logPath, 'utf8').split(/\r?\n/);
const ev = evPath && existsSync(evPath) ? readFileSync(evPath, 'utf8') : '';
const begin = lines.find((l) => l.includes('JAMACCEPT begin'));
if (!begin) { console.log('FAIL the drill never began (no JAMACCEPT begin line)'); out({ verdict: 'error', reason: 'no JAMACCEPT begin' }, 2); }
const kv = (s) => Object.fromEntries([...s.matchAll(/(\w+)=(\([^)]*\)|\S+)/g)].map((m) => [m[1], m[2]]));
const b = kv(begin.slice(begin.indexOf('JAMACCEPT begin') + 15));
const from = +b.from || 1, to = +b.to || 10;

const verdicts = [];   // { step, verb, ok, frame, facts }
const acts = [];       // { step, k, act, frame }
const ctrl = [];       // { frame, btn }
const releases = [];   // JAMSHOT release
const shots = [];      // JAMSHOT (fired)
const passes = [];     // JAMPASS
let summary = null;
const tod = (l) => { const t = l.match(/^\[\d{4}\.\d\d\.\d\d-(\d\d)\.(\d\d)\.(\d\d):(\d\d\d)\]/); return t ? ((+t[1] * 60 + +t[2]) * 60 + +t[3]) * 1000 + +t[4] : NaN; };   // the game logs UTC; only differences are used
for (const l of lines) {
  let m;
  if ((m = l.match(/JAMACCEPT step=(\d+) verb=(\w+) ok=(\d) frame=(\d+) ?(.*)$/))) verdicts.push({ step: +m[1], verb: m[2], ok: m[3] === '1', frame: +m[4], facts: m[5].trim(), ms: tod(l) });
  else if ((m = l.match(/JAMACCEPT act step=(\d+) k=(\d+) act=(\w+).*frame=(\d+)/))) acts.push({ step: +m[1], k: +m[2], act: m[3], frame: +m[4], ms: tod(l) });
  else if ((m = l.match(/JAMACCEPT summary (.*)$/))) summary = m[1];
  else if ((m = l.match(/JAMCTRL frame=(\d+) btn=(\w+)(?: held=(\d))?/))) ctrl.push({ frame: +m[1], btn: m[2] + (m[3] !== undefined ? `:${m[3]}` : '') });
  else if ((m = l.match(/JAMSHOT release frame=(\d+) charge_frames=(\d+).*puck_dist=([\d.]+) reach=([\d.]+).*carrier=(-?\d+)/))) releases.push({ frame: +m[1], charge_frames: +m[2], puck_dist: +m[3], reach: +m[4], carrier: +m[5] });
  else if ((m = l.match(/JAMSHOT frame=(\d+) skater=(\d+) .* human one_timer=(\d)/))) shots.push({ frame: +m[1], one_timer: +m[3] });
  else if ((m = l.match(/JAMPASS frame=(\d+) skater=(\d+) .* human saucer=(\d)/))) passes.push({ frame: +m[1], saucer: +m[3] });
}
// The input that reached the game between a verb's act line and its verdict.
function inputs(step, verbIndex, v) {
  const a = acts.filter((x) => x.step === step);
  const start = a.length ? Math.max(...a.filter((x) => x.frame <= v.frame).map((x) => x.frame), 0) : 0;
  const lo = start || v.frame - 400;
  const btn = ctrl.filter((c) => c.frame >= lo - 2 && c.frame <= v.frame).map((c) => c.btn);
  const rel = releases.filter((r) => r.frame >= lo - 2 && r.frame <= v.frame);
  return { btn, rel };
}
// A131: what the machine did to the hand, from the driver's events.log: LAG (an act's distance from its log line, ms), HOLD (a held input, asked and real ms), MACHINE (a first act
// more than 1 s late: the driver aborts the run). A failing row whose step had a late hand, or any failing row beside a foreign game on the cursor, is a `machine` row, not a `fail`.
const lagOver = { rmb: 100, mmb: 100, lmb: 250, pass: 250, keys: 400, key: 400, sweep: 400, wheel: 400, pause_resume: 800, lmb_onetimer: 250, probe_pass: 800, none: 1e9 };   // ms: step 7's cue is 10 frames (167 ms) before the contact
const lateByStep = {};   // step -> [reasons]
for (const m of ev.matchAll(/LAG step=(\d+) k=(\d+) act=(\w+) lag_ms=(\d+)/g)) { const lim = lagOver[m[3]] ?? 400; if (+m[4] > lim) (lateByStep[+m[1]] ??= []).push(`${m[3]} k${m[2]} ${m[4]} ms behind its log line (limit ${lim})`); }
for (const m of ev.matchAll(/HOLD step=(\d+) k=(\d+) want_ms=(\d+) held_ms=(\d+)/g)) { const over = +m[4] - +m[3]; if (over > (+m[3] < 40 ? 25 : 60)) (lateByStep[+m[1]] ??= []).push(`input k${m[2]} held ${m[4]} ms for ${m[3]} asked`); }
// The press that reached the game late: step 7's cue is 10 frames (RMB) / about 6 frames (MMB, at cross_reach_m) before the contact. The sim stamps the cue (act line) and the press
// (JAMCTRL btn=check / lift), both in sim frames: a press more than 8 (RMB) / 4 (MMB) frames behind its cue came after the hit, whatever the driver did (a game stalled by the machine).
for (const k of [1, 2]) {
  const cue = acts.find((x) => x.step === 7 && x.k === k && (x.act === 'rmb' || x.act === 'mmb'));
  const press = cue && ctrl.find((c) => c.frame >= cue.frame && /^(check|lift)/.test(c.btn));
  if (cue && press && press.frame - cue.frame > (k === 1 ? 8 : 4)) (lateByStep[7] ??= []).push(`${cue.act} reached the game ${press.frame - cue.frame} sim frames after its cue (limit ${k === 1 ? 8 : 4})`);
}
// A held input is read in sim frames by the game and held in wall time by the hand: when the sim ran slower than 52 frames per wall second during a shot's hold (the JAMCTRL press and
// release frames against the driver's own HOLD ms), a charge band missed on the low side is the machine's.
for (const m of ev.matchAll(/HOLD step=4 k=(\d+) want_ms=\d+ held_ms=(\d+)/g)) {
  const cue = acts.find((x) => x.step === 4 && x.k === +m[1]); if (!cue) continue;
  const dn = ctrl.find((c) => c.frame >= cue.frame && c.btn === 'shoot:1'); const up = dn && ctrl.find((c) => c.frame >= dn.frame && c.btn === 'shoot:0');
  if (dn && up && +m[2] >= 100) { const hz = (up.frame - dn.frame) * 1000 / +m[2]; if (hz < 52) (lateByStep[4] ??= []).push(`the sim ran ${hz.toFixed(0)} frames per second during hold k${m[1]} (${up.frame - dn.frame} sim frames in ${m[2]} ms)`); }
}
// The sim's own pace between a step's first act and its verdict (sim frames per wall second, from the game log's time stamps): the sim is fixed at 60 Hz, so a step that ran above 100
// (the game fast-forwarding after a stall: outnumbered's first step ran 301 frames in 0.47 s, before the hand's W could matter) or below 40 was lived on a machine that was not keeping up.
const paceNote = {};
for (const v of verdicts) {
  if (v.step === 10) continue;   // the pause stops the sim for 1.2 s of wall time on purpose
  const a0 = acts.find((x) => x.step === v.step); if (!a0 || !Number.isFinite(a0.ms) || !Number.isFinite(v.ms) || v.frame <= a0.frame) continue;
  const dt = ((v.ms - a0.ms) + 86400000) % 86400000; if (dt <= 0) continue; const hz = (v.frame - a0.frame) * 1000 / dt;
  if (hz > 100 || hz < 40) (paceNote[v.step] ??= new Set()).add(`the sim ran ${hz.toFixed(0)} frames per second from the first act to ${v.verb}'s verdict (${v.frame - a0.frame} frames in ${dt} ms)`);
}
for (const [st, set] of Object.entries(paceNote)) (lateByStep[+st] ??= []).push(...set);
const machineAbort = (ev.match(/MACHINE: [^\r\n]*/) || [])[0];
const warnsEarly = ev.match(/WARN [^\r\n]*shared OS cursor/g) || [];
const table = [];
let allOk = true;
let anyReal = false, anyMachine = false;
for (let s = from; s <= to; s++) {
  const def = STEPS[s];
  const rows = def.verbs.map((verb, i) => {
    const v = verdicts.find((x) => x.step === s && x.verb === verb);
    if (!v) { allOk = false; if (machineAbort) anyMachine = true; else anyReal = true; return { verb, ok: false, machine: machineAbort ? 'run aborted' : '', facts: 'no verdict line (the run never reached it: see events.log)' }; }
    const skipped = !v.ok && /skipped=(no_mate|no_opp)/.test(v.facts);   // A179: the cut has no mate / opponent for the step: not run, not a fail
    if (!v.ok && !skipped) allOk = false;
    const io = inputs(s, i, v);
    let extra = '';
    if (io.btn.length) extra += ` input=[${[...new Set(io.btn)].join(',')}]`;
    if (io.rel.length) extra += ` releases=[${io.rel.map((r) => `charge_frames ${r.charge_frames} puck_dist ${r.puck_dist} reach ${r.reach} carrier ${r.carrier}`).join('; ')}]`;
    let machine = '';
    if (!v.ok && !skipped) { machine = (lateByStep[s] || []).join('; ') || (warnsEarly.length && !['skate', 'sprint', 'sprint_carry', 'brake'].includes(verb) ? 'a foreign game shared the cursor' : ''); if (machine) anyMachine = true; else anyReal = true; }
    return { verb, ok: v.ok || skipped, skipped, machine, facts: v.facts + extra + (machine ? ` MACHINE[${machine}]` : '') };
  });
  const ok = rows.every((r) => r.ok);
  table.push({ step: s, name: def.name, ok, verbs: rows });
}
// A155 (PIXEL ROW): the driver photographs the live window before it quits it (PIXEL lit= mean=). A frame whose lit share is under 0.20 is a black world with its HUD on top: a real fail.
const px = ev.match(/PIXEL lit=([0-9.]+) mean=([0-9.]+)/);
if (px) {
  const lit = +px[1], mean = +px[2], pok = lit >= 0.2;
  if (!pok) { allOk = false; anyReal = true; }
  table.push({ step: 'px', name: 'The frame is not black (one still of the window before the quit)', ok: pok, verbs: [{ verb: 'pixels', ok: pok, machine: '', facts: `lit_share=${lit} mean_luminance=${mean} (a rendered rink reads 0.99 / 148; the black world with its HUD 0.01 / 1; the menu card 0.38)${pok ? '' : ' BLACK FRAME'}` }] });
} else console.log('WARN no PIXEL line in events.log: the frame was not photographed');
const pk = ev.match(/pass key resolved: (\w+)/) || ev.match(/pass key unresolved/);
const warns = [...new Set((ev.match(/WARN [^\r\n]*shared OS cursor[^\r\n]*/g) || []).map((w) => w.replace(/\d+:[A-Za-z]:[^ ]*/g, '<other>')))];
console.log(`acceptance run: ${logPath}`);
for (const w of warns) console.log(`environment: ${w} (a verdict under these is not a clean one)`);
console.log(`begin: human ${b.human} mate ${b.mate} opp ${b.opp}; shot_reach ${b.shot_reach} carry_reach ${b.carry_reach} charge_max ${b.charge_max} saucer_half ${b.saucer_half} onetimer_window ${b.onetimer_window} knock_frames ${b.knock_frames}; pass key ${pk ? (pk[1] ?? 'unresolved') : 'not probed'}`);
console.log('');
console.log('| step | verb | result | facts |');
console.log('|---|---|---|---|');
for (const t of table) for (const r of t.verbs) console.log(`| ${t.step} ${t.name} | ${r.verb} | ${r.skipped ? 'SKIP' : r.ok ? 'PASS' : (r.machine ? 'MACHINE' : 'FAIL')} | ${r.facts.replace(/\|/g, '/')} |`);
console.log('');
for (const t of table) console.log(`${t.ok ? 'PASS' : 'FAIL'} step ${t.step} ${t.name}: ${t.verbs.map((r) => `${r.verb} ${r.skipped ? 'SKIP' : r.ok ? 'ok' : 'FAIL'}`).join(', ')}`);
console.log(`summary line: ${summary ?? 'none (the run did not finish)'}`);
const verdict = allOk ? 'pass' : (anyReal ? 'fail' : 'machine');
if (verdict === 'machine') console.log(`MACHINE: no row failed on the game's account; the machine did: ${machineAbort ?? Object.entries(lateByStep).map(([k, v]) => `step ${k}: ${v.join(', ')}`).join(' | ') ?? ''}${warnsEarly.length ? ' foreign game on the cursor' : ''} -> repeat the run`);
const code = allOk ? 0 : (verdict === 'machine' ? 3 : 1);
out({ verdict, log: logPath, warnings: warns, pass_key: pk ? (pk[1] ?? 'unresolved') : '', steps: table.map((t) => ({ step: t.step, ok: t.ok, verbs: t.verbs.map((r) => ({ verb: r.verb, ok: r.ok, machine: r.machine || undefined, facts: r.facts })) })) }, code);
