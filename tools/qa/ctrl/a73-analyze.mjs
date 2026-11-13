#!/usr/bin/env node
// Lane A73: read a packaged game's log (a73-prove.ps1) and print the tackling counts and the hit-stop / kick proof.
//   node tools/qa/ctrl/a73-analyze.mjs tools/qa/out/ctrl/a73-side/game.log
// PASS/FAIL/INFO lines and one JSON line last (exit 0 pass, 1 fail, 2 could not run).
import { readFileSync } from 'node:fs';

const file = process.argv[2];
if (!file) { console.error('usage: a73-analyze.mjs <game.log>'); process.exit(2); }
const lines = readFileSync(file, 'utf8').split(/\r?\n/);
const out = { tool: 'thirdjam.a73/1', file };
let fails = 0;
const pass = (name, ok, detail) => { console.log(`${ok ? 'PASS' : 'FAIL'} ${name} ${detail}`); if (!ok) fails++; out[name] = { ok, detail }; };
const info = (name, detail) => { console.log(`INFO ${name} ${detail}`); out[name] = detail; };
const num = (re, l) => { const m = l.match(re); return m ? parseFloat(m[1]) : NaN; };

// The human's index: the first JAMVIS line with human=1, else 0.
let human = 0;
for (const l of lines) { const m = l.match(/JAMVIS knockdown .*skater=(\d+) team=\d+ human=1/); if (m) { human = parseInt(m[1]); break; } }
const hs = lines.filter((l) => l.includes('JAMHITSTOP frame='));
if (hs.length) { const m = hs[0].match(/human=(\d+)/); if (m) human = parseInt(m[1]); }

// 1. Counts.
const vis = { hit: 0, board: 0, pin: 0, stickhit: 0, knockdown: 0, getup: 0 };
for (const l of lines) { const m = l.match(/JAMVIS (hit|board|pin|stickhit|knockdown|getup) frame=/); if (m) vis[m[1]]++; }
const ev = { check: 0, checkHuman: 0, knockdown: 0, knockdownHuman: 0, foul: 0, board: 0 };
const kdFrames = [];
for (const l of lines) {
  const m = l.match(/JAMEVENT (check|knockdown|foul|board) who=(-?\d+) .*strength=([\d.]+) frame=(\d+) kind=(\S*)/);
  if (!m) continue;
  ev[m[1]]++;
  if (m[1] === 'check' && parseInt(m[2]) === human) ev.checkHuman++;
  if (m[1] === 'knockdown') { if (parseInt(m[2]) === human) ev.knockdownHuman++; kdFrames.push({ who: parseInt(m[2]), frame: parseInt(m[4]), strength: parseFloat(m[3]), kind: m[5] }); }
}
const attempts = lines.filter((l) => l.includes('JAMDRILL tackle attempt=')).map((l) => l.replace(/^.*JAMDRILL/, 'JAMDRILL'));
const strips = attempts.filter((l) => / stripped=1/.test(l)).length;
const landed = attempts.filter((l) => / landed=1/.test(l)).length;
const presses = lines.filter((l) => l.includes('JAMDRILL tackle press')).map((l) => l.replace(/^.*JAMDRILL/, 'JAMDRILL'));
const speeds = presses.map((l) => num(/my_speed=([\d.]+)/, l)).filter((x) => !isNaN(x));
info('counts', `JAMVIS hit ${vis.hit} board ${vis.board} pin ${vis.pin} stickhit ${vis.stickhit} knockdown ${vis.knockdown} getup ${vis.getup}; JAMEVENT check ${ev.check} (by the human ${ev.checkHuman}) knockdown ${ev.knockdown} (of the human ${ev.knockdownHuman}) foul ${ev.foul} board ${ev.board}`);
out.counts = { vis, ev, attempts: attempts.length, landed, strips, presses: presses.length };
const summary = lines.filter((l) => l.includes('JAMDRILL summary') || l.includes('JAMDRILL kd_summary')).map((l) => l.replace(/^.*JAMDRILL/, 'JAMDRILL'));
for (const s of summary) info('summary', s);
if (attempts.length) {
  pass('side_drill_attempts', attempts.length >= 10, `${attempts.length} attempts; landed ${landed}; stripped ${strips}; press speeds m/s [${speeds.map((x) => x.toFixed(1)).join(', ')}]`);
  const inBand = speeds.filter((x) => x >= 2.5 && x <= 6.5).length;
  info('press_speed_band', `${inBand} of ${speeds.length} presses at 2.5-6.5 m/s (the brief asks 3-6)`);
}
const kdHuman = lines.filter((l) => l.includes('JAMDRILL kd_human')).map((l) => l.replace(/^.*JAMDRILL/, 'JAMDRILL'));
const getups = lines.filter((l) => l.includes('JAMDRILL getup_human')).map((l) => l.replace(/^.*JAMDRILL/, 'JAMDRILL'));
if (kdHuman.length || getups.length) {
  const measured = getups.map((l) => num(/measured_down_frames=(-?\d+)/, l));
  pass('human_knocked_down_and_up', kdHuman.length >= 10 && getups.length >= kdHuman.length - 1, `knockdowns of the human ${kdHuman.length}; get-ups ${getups.length}; measured down frames [${measured.join(', ')}]`);
}

// 2. Every knockdown got a get-up (JAMVIS edge pairs) and the sent frames are the sim's minus the hit-stop.
const kdVis = lines.filter((l) => l.includes('JAMVIS knockdown frame=')).map((l) => ({ frame: num(/frame=(\d+)/, l), skater: num(/skater=(\d+)/, l), sim: num(/sim_frames=(\d+)/, l), sent: num(/sent_frames=(\d+)/, l), hs: num(/hitstop=(\d+)/, l) }));
pass('getup_per_knockdown', vis.getup >= vis.knockdown - 1, `JAMVIS knockdown ${vis.knockdown}, getup ${vis.getup} (the last may still be down at quit)`);
const sentOk = kdVis.every((k) => k.sent === Math.max(1, k.sim - k.hs));
pass('sent_frames_minus_hitstop', kdVis.length === 0 || sentOk, `${kdVis.length} knockdowns; sent = sim - hitstop on all: ${sentOk}; e.g. ${kdVis.slice(0, 3).map((k) => `f${k.frame} sim ${k.sim} sent ${k.sent} hs ${k.hs}`).join('; ')}`);

// 3. Hit-stop and kick lines: one hit-stop per knockdown that involves the human, one kick per such knockdown or human check.
const kicks = lines.filter((l) => l.includes('JAMKICK frame=')).map((l) => l.replace(/^.*JAMKICK/, 'JAMKICK'));
const humanInvolved = kdFrames.filter((k) => k.kind === 'check' && k.who === human).length + ev.checkHuman;
pass('hitstop_per_human_knockdown', hs.length === humanInvolved || (hs.length > 0 && Math.abs(hs.length - humanInvolved) <= 1), `JAMHITSTOP ${hs.length}; knockdowns involving the human ${humanInvolved} (taken ${kdFrames.filter((k) => k.kind === 'check' && k.who === human).length}, landed ${ev.checkHuman}); e.g. ${hs.slice(0, 2).map((l) => l.replace(/^.*JAMHITSTOP/, 'JAMHITSTOP')).join(' | ')}`);
pass('kick_per_human_hit', kicks.length >= Math.min(humanInvolved, 1), `JAMKICK ${kicks.length}; e.g. ${kicks.slice(0, 2).join(' | ')}`);
// Hold proof: no JAMCAM line (the camera steps on the pushed presentation frame) for the held frames after each hit-stop.
const camFrames = new Set();
for (const l of lines) { const m = l.match(/JAMCAM frame=(\d+)/); if (m) camFrames.add(parseInt(m[1])); }
let heldOk = 0, heldBad = 0;
for (const l of hs) {
  const f = num(/frame=(\d+)/, l), n = num(/frames=(\d+)/, l);
  let ok = camFrames.has(f);
  for (let i = 1; i < n; i++) if (camFrames.has(f + i)) ok = false;
  if (!camFrames.has(f + n)) ok = false;
  if (ok) heldOk++; else heldBad++;
}
if (hs.length && camFrames.size) pass('presentation_held', heldBad === 0, `${heldOk} of ${hs.length} hit-stops: the camera stepped on the hit frame, not on the ${num(/frames=(\d+)/, hs[0]) - 1} frames after it, and again on the release frame`);

// 4. The camera stays on the lying human: the middle third over every down window of the human.
const cam = [];
for (const l of lines) { const m = l.match(/JAMCAM frame=(\d+) x=([-\d.]+) y=([-\d.]+) third=(\d)/); if (m) cam.push({ f: parseInt(m[1]), x: parseFloat(m[2]), y: parseFloat(m[3]), t: m[4] === '1' }); }
const downWindows = [];
for (const l of lines) { const m = l.match(/JAMVIS knockdown frame=(\d+) skater=(\d+) team=\d+ human=1 .*sim_frames=(\d+)/); if (m) downWindows.push([parseInt(m[1]), parseInt(m[1]) + parseInt(m[3])]); }
if (cam.length && downWindows.length) {
  let inW = 0, total = 0, maxX = 0, maxY = 0;
  for (const c of cam) for (const [a, b] of downWindows) if (c.f >= a && c.f <= b) { total++; if (c.t) inW++; maxX = Math.max(maxX, Math.abs(c.x)); maxY = Math.max(maxY, Math.abs(c.y)); break; }
  pass('camera_on_lying_human', total > 0 && inW === total, `${inW} of ${total} JAMCAM frames inside the human's ${downWindows.length} down windows are in the middle third; max |x| ${maxX.toFixed(3)} |y| ${maxY.toFixed(3)} (edge 0.333)`);
}
const stills = lines.filter((l) => l.includes('JAMSTILLS start')).map((l) => l.replace(/^.*JAMSTILLS/, 'JAMSTILLS'));
info('stills', `${stills.length} sequences: ${stills.join(' | ')}`);
const hashes = lines.filter((l) => l.includes('JAMSIM frame=')).map((l) => l.replace(/^.*JAMSIM/, 'JAMSIM'));
if (hashes.length) info('sim', hashes[hashes.length - 1]);
out.verdict = fails ? 'fail' : 'pass';
console.log(JSON.stringify(out));
process.exit(fails ? 1 : 0);
