#!/usr/bin/env node
// Lane A49: read a packaged game's log (a49-prove.ps1) and print the five proofs.
//   node tools/qa/ctrl/a49-analyze.mjs tools/qa/out/ctrl/bindings/game.log
// PASS/FAIL lines and one JSON line last.
import { readFileSync } from 'node:fs';

const file = process.argv[2];
if (!file) { console.error('usage: a49-analyze.mjs <game.log>'); process.exit(2); }
const lines = readFileSync(file, 'utf8').split(/\r?\n/);
const out = { tool: 'thirdjam.a49/1', file };
let fails = 0;
const pass = (name, ok, detail) => { console.log(`${ok ? 'PASS' : 'FAIL'} ${name} ${detail}`); if (!ok) fails++; out[name] = { ok, detail }; };

// 1. The mouse reaches the stick: JAMCTRL stick lines by source.
const src = {};
let stickMoved = 0, prevStick = null, stickLines = 0, mouseNonZero = 0;
const ctrl = lines.filter((l) => l.includes('JAMCTRL frame='));
for (const l of ctrl) {
  const m = l.match(/stick=\(([-\d.]+),([-\d.]+)\) src=(\w+)/);
  if (!m) continue;
  stickLines++;
  src[m[3]] = (src[m[3]] || 0) + 1;
  const v = [parseFloat(m[1]), parseFloat(m[2])];
  if (m[3] === 'mouse' && Math.hypot(v[0], v[1]) > 0.1) mouseNonZero++;
  if (prevStick && (Math.abs(v[0] - prevStick[0]) > 0.01 || Math.abs(v[1] - prevStick[1]) > 0.01)) stickMoved++;
  prevStick = v;
}
pass('mouse_reaches_stick', (src.mouse || 0) > 60 && mouseNonZero > 60 && stickMoved > 30,
  `stick lines ${stickLines}; by source ${JSON.stringify(src)}; mouse-aimed non-zero frames ${mouseNonZero}; frames where the stick changed ${stickMoved}`);
const modeLine = lines.find((l) => l.includes('JAMCTRL input mode set'));
pass('match_input_mode', !!modeLine, modeLine ? modeLine.replace(/^.*JAMCTRL/, 'JAMCTRL') : 'no input-mode line');

// 2. Every binding once.
const btn = {};
for (const l of lines) { const m = l.match(/JAMCTRL frame=(\d+) btn=(\w+)(?: held=(\d))?/); if (m) { const k = m[2] + (m[3] !== undefined ? `_${m[3]}` : ''); btn[k] = (btn[k] || 0) + 1; } }
const need = ['shoot_1', 'shoot_0', 'check', 'lift', 'deke', 'poke', 'pass', 'sprint_1', 'sprint_0', 'brake_1', 'brake_0'];
const missing = need.filter((k) => !btn[k]);
const signals = lines.filter((l) => /JAMSIGNAL|JAMSIG/.test(l)).length;
const paused = lines.filter((l) => l.includes('JAMCALL paused')).length;
pass('every_binding_once', missing.length === 0, `edges ${JSON.stringify(btn)}; missing [${missing.join(', ')}]; signal lines ${signals}; pauses ${paused}`);
const moves = ctrl.filter((l) => /move=\((?!0\.00,0\.00)/.test(l)).length;
pass('wasd_moves', moves > 60, `frames with a non-zero move ${moves}`);

// 3. The sent aim never jumps: max |d| over the JAMAIM lines (10 s = 600 frames at least).
let maxD = 0, aimLines = 0, over20 = 0;
for (const l of lines) { const m = l.match(/JAMAIM frame=\S+ raw=([-\d.]+) sent=([-\d.]+) d=([-\d.]+)/); if (m) { aimLines++; const d = Math.abs(parseFloat(m[3])); if (d > maxD) maxD = d; if (d > 20) over20++; } }
pass('aim_no_blink', aimLines >= 600 && over20 === 0, `JAMAIM lines ${aimLines}; max |d| per frame ${maxD.toFixed(1)} deg; frames over 20 deg ${over20}`);

// 4. The human stays in the middle third over the strip.
let camLines = 0, inThird = 0, maxX = 0, maxY = 0;
for (const l of lines) { const m = l.match(/JAMCAM frame=\S+ x=([-\d.]+) y=([-\d.]+) third=(\d)/); if (m) { camLines++; if (m[3] === '1') inThird++; maxX = Math.max(maxX, Math.abs(parseFloat(m[1]))); maxY = Math.max(maxY, Math.abs(parseFloat(m[2]))); } }
pass('camera_middle_third', camLines >= 1200 && inThird === camLines, `JAMCAM lines ${camLines} (${(camLines / 60).toFixed(1)} s); in the middle third ${inThird}; max |x| ${maxX.toFixed(3)} max |y| ${maxY.toFixed(3)} (edge 0.333)`);

// 5. The drill (when run): the summary line.
const drill = lines.filter((l) => l.includes('JAMDRILL summary')).map((l) => l.replace(/^.*JAMDRILL/, 'JAMDRILL'));
if (drill.length) {
  for (const d of drill) {
    const m = d.match(/mode=(\d) attempts=(\d+) landed=(\d+) stripped=(\d+) knockdowns=(\d+) victim_visual_down=(\d+) lift_clean=(\d+)/);
    if (!m) continue;
    const mode = m[1];
    if (mode === '1') pass('tackle_strips', m[3] === m[2] && m[4] === m[2] && m[6] === m[2], d);
    else pass('crosscheck_lifts', m[7] === m[2], d);
  }
} else {
  console.log('INFO no JAMDRILL summary in this log (the drill runs with -dpcvars=jam.ctrl.Drill=1 or 2)');
}
const hashes = lines.filter((l) => l.includes('JAMSIM frame=')).map((l) => l.replace(/^.*JAMSIM/, 'JAMSIM'));
if (hashes.length) console.log('INFO ' + hashes[hashes.length - 1]);
out.verdict = fails ? 'fail' : 'pass';
console.log(JSON.stringify(out));
process.exit(fails ? 1 : 0);
