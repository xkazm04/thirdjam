#!/usr/bin/env node
// Lane A87 (R-0043 point 1): the acceptance run's verdict. Reads the game log of a jam.ctrl.Drill 20 run (and the driver's events.log) and prints the step
// table, pass or fail per verb, with the facts; the input lines that reached the game (JAMCTRL) and the shot read-outs (JAMSHOT release) are attached so a
// failure says whether the press arrived and what the sim saw. The last line is {"tool":"thirdjam.accept/1",...}. Exit 0 every verb passed, 1 a verb failed
// or never reported, 2 could not run (no log, the drill never began).
//   node tools/qa/accept/accept.mjs <game.log> [events.log]
import { readFileSync, existsSync } from 'node:fs';

const [logPath, evPath] = process.argv.slice(2);
const STEPS = {
  1: { name: 'Skate, sprint (W, Shift)', verbs: ['skate', 'sprint', 'sprint_carry'] },
  2: { name: 'Aim (the cursor at four points)', verbs: ['aim'] },
  3: { name: 'Carry (sweeps at full reach)', verbs: ['carry'] },
  4: { name: 'Shoot, blade extended (tap, 8, 30 frames)', verbs: ['shoot_tap', 'shoot_half', 'shoot_full'] },
  5: { name: 'Pass, blade extended (tap, held = saucer)', verbs: ['pass', 'saucer'] },
  6: { name: 'Receive and one-timer', verbs: ['onetimer'] },
  7: { name: 'Check (RMB), cross-check (MMB)', verbs: ['check', 'crosscheck'] },
  8: { name: 'Be checked', verbs: ['checked'] },
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
for (const l of lines) {
  let m;
  if ((m = l.match(/JAMACCEPT step=(\d+) verb=(\w+) ok=(\d) frame=(\d+) ?(.*)$/))) verdicts.push({ step: +m[1], verb: m[2], ok: m[3] === '1', frame: +m[4], facts: m[5].trim() });
  else if ((m = l.match(/JAMACCEPT act step=(\d+) k=(\d+) act=(\w+).*frame=(\d+)/))) acts.push({ step: +m[1], k: +m[2], act: m[3], frame: +m[4] });
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
const table = [];
let allOk = true;
for (let s = from; s <= to; s++) {
  const def = STEPS[s];
  const rows = def.verbs.map((verb, i) => {
    const v = verdicts.find((x) => x.step === s && x.verb === verb);
    if (!v) { allOk = false; return { verb, ok: false, facts: 'no verdict line (the run never reached it: see events.log)' }; }
    if (!v.ok) allOk = false;
    const io = inputs(s, i, v);
    let extra = '';
    if (io.btn.length) extra += ` input=[${[...new Set(io.btn)].join(',')}]`;
    if (io.rel.length) extra += ` releases=[${io.rel.map((r) => `charge_frames ${r.charge_frames} puck_dist ${r.puck_dist} reach ${r.reach} carrier ${r.carrier}`).join('; ')}]`;
    return { verb, ok: v.ok, facts: v.facts + extra };
  });
  const ok = rows.every((r) => r.ok);
  table.push({ step: s, name: def.name, ok, verbs: rows });
}
const pk = ev.match(/pass key resolved: (\w+)/) || ev.match(/pass key unresolved/);
const warns = [...new Set((ev.match(/WARN [^\r\n]*shared OS cursor[^\r\n]*/g) || []).map((w) => w.replace(/\d+:[A-Za-z]:[^ ]*/g, '<other>')))];
console.log(`acceptance run: ${logPath}`);
for (const w of warns) console.log(`environment: ${w} (a verdict under these is not a clean one)`);
console.log(`begin: human ${b.human} mate ${b.mate} opp ${b.opp}; shot_reach ${b.shot_reach} carry_reach ${b.carry_reach} charge_max ${b.charge_max} saucer_half ${b.saucer_half} onetimer_window ${b.onetimer_window} knock_frames ${b.knock_frames}; pass key ${pk ? (pk[1] ?? 'unresolved') : 'not probed'}`);
console.log('');
console.log('| step | verb | result | facts |');
console.log('|---|---|---|---|');
for (const t of table) for (const r of t.verbs) console.log(`| ${t.step} ${t.name} | ${r.verb} | ${r.ok ? 'PASS' : 'FAIL'} | ${r.facts.replace(/\|/g, '/')} |`);
console.log('');
for (const t of table) console.log(`${t.ok ? 'PASS' : 'FAIL'} step ${t.step} ${t.name}: ${t.verbs.map((r) => `${r.verb} ${r.ok ? 'ok' : 'FAIL'}`).join(', ')}`);
console.log(`summary line: ${summary ?? 'none (the run did not finish)'}`);
const code = allOk ? 0 : 1;
out({ verdict: allOk ? 'pass' : 'fail', log: logPath, warnings: warns, pass_key: pk ? (pk[1] ?? 'unresolved') : '', steps: table.map((t) => ({ step: t.step, ok: t.ok, verbs: t.verbs.map((r) => ({ verb: r.verb, ok: r.ok, facts: r.facts })) })) }, code);
