#!/usr/bin/env node
// Lane A48: measurements from a Development game log (the Shipping exe writes none).
//   node tools/qa/storyboard/measure.mjs <game.log> [--human 0]
// Prints one markdown table and one thirdjam.storyboard/1 JSON line: frames, goals, the human's touches / shots / passes received /
// one-timers / checks landed and taken, barks, commentary lines, step_ms, JAMFX missing, missing usage flag, LoadErrors, ensure / warning lines.
import { readFileSync } from 'node:fs';

const args = process.argv.slice(2);
const file = args.find((a) => !a.startsWith('--'));
if (!file) { console.error('usage: measure.mjs <game.log> [--human N]'); process.exit(2); }
const hi = args.indexOf('--human');
const human = hi >= 0 ? Number(args[hi + 1]) : 0;
const text = readFileSync(file, 'utf8');
const lines = text.split(/\r?\n/);

const ev = /JAMEVENT (\S+) who=(-?\d+) where=\(([^)]*)\) strength=([\d.]+) frame=(\d+) kind=(\S*)/;
const sim = /JAMSIM frame=(\d+) hash=(\w+) score=(\d+):(\d+) phase=(\d+) step_ms mean=([\d.]+) worst=([\d.]+)/;
const counts = {};
const kinds = {};
const m = {
  file, human, cut: '', frames: 0, score: [0, 0], goals: 0, goals_human_team: 0, matches: 0,
  human: { touches: 0, shots: 0, one_timers: 0, passes: 0, passes_received: 0, checks_landed: 0, checks_taken: 0, knockdowns: 0, pickups: 0, pokes_clean: 0, lifts_clean: 0, fouls: 0, breakaways: 0, dekes: 0 },
  all: { shots: 0, checks: 0, knockdowns: 0, pickups: 0, faceoffs: 0, boards: 0, posts: 0, saves: 0 },
  barks: 0, commentary: 0, commentary_cut: 0, commentary_skip: 0,
  step_ms_mean: [], step_ms_worst: [], fps_lines: [],
  jamfx_missing: [], jamvo_missing: [], missing_usage_flag: 0, load_errors: 0, ensures: 0, warnings: 0, errors: 0, fatal: 0,
  events: counts, kinds,
};
let lastPasser = -1, lastPassFrame = -1, lastCarrier = -1;
for (const l of lines) {
  let r;
  if ((r = ev.exec(l))) {
    const [, name, whoS, , , frameS, kind] = r; const who = Number(whoS); const frame = Number(frameS);
    counts[name] = (counts[name] || 0) + 1;
    if (kind) kinds[`${name}:${kind}`] = (kinds[`${name}:${kind}`] || 0) + 1;
    const mine = who === human;
    if (name === 'shot') { m.all.shots++; if (mine) { m.human.shots++; m.human.touches++; if (kind === 'one_timer') m.human.one_timers++; } }
    else if (name === 'pass') { if (mine) { m.human.passes++; m.human.touches++; } lastPasser = who; lastPassFrame = frame; }
    else if (name === 'check') { m.all.checks++; if (mine) m.human.checks_landed++; }
    else if (name === 'knockdown') { m.all.knockdowns++; if (mine) { m.human.checks_taken++; m.human.knockdowns++; } }
    else if (name === 'pickup_take') { m.all.pickups++; if (mine) m.human.pickups++; }
    else if (name === 'goal') { m.goals++; }
    else if (name === 'faceoff' && kind === 'drop') m.all.faceoffs++;
    else if (name === 'board') m.all.boards++;
    else if (name === 'post') m.all.posts++;
    else if (name === 'save') m.all.saves++;
    else if (name === 'poke' && kind === 'clean' && mine) { m.human.pokes_clean++; m.human.touches++; }
    else if (name === 'lift' && kind === 'clean' && mine) { m.human.lifts_clean++; m.human.touches++; }
    else if (name === 'foul' && mine) m.human.fouls++;
    else if (name === 'breakaway' && mine) m.human.breakaways++;
    else if (name === 'deke' && mine) m.human.dekes++;
    else if (name === 'carry' && mine) m.human.touches++;
    // A pass received by the human: the human's shot or pass within 120 frames of a teammate's pass, or a one-timer.
    if (mine && (name === 'shot' || name === 'pass') && lastPasser !== human && lastPasser >= 0 && frame - lastPassFrame <= 120) { m.human.passes_received++; lastPasser = -1; }
    continue;
  }
  if ((r = sim.exec(l))) { m.frames = Number(r[1]); m.score = [Number(r[3]), Number(r[4])]; m.step_ms_mean.push(Number(r[6])); m.step_ms_worst.push(Number(r[7])); continue; }
  if (l.includes('JAMCART loaded')) { m.matches++; const c = /JAMCART loaded (\S+) id=(\S+)/.exec(l); if (c) m.cut = c[2]; continue; }
  if (l.includes('JAMBARK who=')) { m.barks++; continue; }
  if (l.includes('JAMVO id=')) { m.commentary++; continue; }
  if (l.includes('JAMVO_CUT')) { m.commentary_cut++; continue; }
  if (l.includes('JAMVO_SKIP')) { m.commentary_skip++; continue; }
  if (l.includes('JAMFX missing')) { m.jamfx_missing.push(l.replace(/^.*JAMFX missing /, '').trim()); continue; }
  if (l.includes('JAMVO missing')) { m.jamvo_missing.push(l.replace(/^.*JAMVO missing /, '').trim()); continue; }
  if (/missing usage flag/i.test(l)) m.missing_usage_flag++;
  if (/LoadErrors/.test(l)) m.load_errors++;
  if (/Ensure condition failed|ensure\(/i.test(l)) m.ensures++;
  if (/Fatal error|Unhandled Exception|Assertion failed/.test(l)) m.fatal++;
  if (/: ?Warning: /.test(l)) m.warnings++;
  if (/: ?Error: /.test(l)) m.errors++;
  const f = /FPS: ([\d.]+)|Frame: ([\d.]+) ms|MaxTickRate|DeltaSeconds/.exec(l); if (f && (f[1] || f[2])) m.fps_lines.push(l.trim());
}
const avg = (a) => (a.length ? (a.reduce((x, y) => x + y, 0) / a.length) : 0);
const out = {
  ...m,
  seconds: +(m.frames / 60).toFixed(1),
  step_ms_mean_avg: +avg(m.step_ms_mean).toFixed(4), step_ms_worst_max: m.step_ms_worst.length ? Math.max(...m.step_ms_worst) : 0,
};
delete out.step_ms_mean; delete out.step_ms_worst;
const rows = [
  ['cut', out.cut], ['matches built', out.matches], ['frames (last JAMSIM line)', `${out.frames} (${out.seconds} s)`], ['score', out.score.join(':')], ['goals (events)', out.goals],
  ['human touches (shots + passes + clean pokes / lifts + carries)', out.human.touches], ['human shots', out.human.shots], ['human one-timers', out.human.one_timers], ['human passes', out.human.passes],
  ['human passes received', out.human.passes_received], ['human checks landed', out.human.checks_landed], ['human checks taken (knockdowns)', out.human.checks_taken], ['human pickups', out.human.pickups],
  ['all shots / checks / knockdowns / pickups / faceoffs', `${out.all.shots} / ${out.all.checks} / ${out.all.knockdowns} / ${out.all.pickups} / ${out.all.faceoffs}`],
  ['barks (JAMBARK)', out.barks], ['commentary lines (JAMVO) / cut / skipped', `${out.commentary} / ${out.commentary_cut} / ${out.commentary_skip}`],
  ['sim step ms (mean of means / worst)', `${out.step_ms_mean_avg} / ${out.step_ms_worst_max}`],
  ['JAMFX missing', out.jamfx_missing.length ? out.jamfx_missing.join(', ') : 'none'], ['JAMVO missing', out.jamvo_missing.length ? out.jamvo_missing.join(', ') : 'none'],
  ['missing usage flag', out.missing_usage_flag], ['LoadErrors', out.load_errors], ['ensures / fatal', `${out.ensures} / ${out.fatal}`], ['Warning: / Error: lines', `${out.warnings} / ${out.errors}`],
];
console.log('| Measure | Value |\n|---|---|');
for (const [k, v] of rows) console.log(`| ${k} | ${v} |`);
console.log(JSON.stringify({ tool: 'thirdjam.storyboard/1', ...out }));
