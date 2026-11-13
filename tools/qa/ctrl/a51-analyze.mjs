#!/usr/bin/env node
// Lane A51 (R-0034): read a packaged game's log from a51-prove.ps1 and print the stick-handling proofs. PASS/FAIL lines and one JSON line last.
//   node tools/qa/ctrl/a51-analyze.mjs tools/qa/out/ctrl/a51-sweep/game.log
//   node tools/qa/ctrl/a51-analyze.mjs tools/qa/out/ctrl/a51-spin/game.log
import { readFileSync } from 'node:fs';

const file = process.argv[2];
if (!file) { console.error('usage: a51-analyze.mjs <game.log>'); process.exit(2); }
const lines = readFileSync(file, 'utf8').split(/\r?\n/);
const out = { tool: 'thirdjam.a51/1', file };
let fails = 0;
const pass = (name, ok, detail) => { console.log(`${ok ? 'PASS' : 'FAIL'} ${name} ${detail}`); if (!ok) fails++; out[name] = { ok, detail }; };

// The real input path: the stick from the mouse, E as the spin edge, F as the pass edge.
const src = {};
for (const l of lines) { const m = l.match(/JAMCTRL frame=\d+ stick=\([^)]*\) src=(\w+)/); if (m) src[m[1]] = (src[m[1]] || 0) + 1; }
pass('mouse_reaches_stick', (src.mouse || 0) > 60, `stick frames by source ${JSON.stringify(src)}`);
const spinBtn = lines.filter((l) => /JAMCTRL frame=\d+ btn=spin \(E \/ B\)/.test(l)).length;
const passBtn = lines.filter((l) => /JAMCTRL frame=\d+ btn=pass \(F \/ X\)/.test(l)).length;

// The sim's own read-outs of the human's sweeps and spins.
const sweeps = lines.filter((l) => /JAMCTRL frame=\d+ sweep side=/.test(l)).length;
const spins = lines.filter((l) => /JAMCTRL frame=\d+ spin frames=/.test(l)).length;
const results = [];
for (const l of lines) {
  const m = l.match(/JAMDRILL stick_result (sweep|spin) n=(\d+) kept=(\d) poked_clean=(\d) knocked=(\d) knock_frames=(\d+)/);
  if (m) results.push({ kind: m[1], n: +m[2], kept: +m[3], poked: +m[4], knocked: +m[5], knockFrames: +m[6] });
}
const sweepRes = results.filter((r) => r.kind === 'sweep');
const spinRes = results.filter((r) => r.kind === 'spin');
const mode = spins > 0 && spinBtn > 0 ? 'spin' : 'sweep';
if (mode === 'sweep') {
  pass('sweeps_seen_by_sim', sweeps >= 10, `JAMCTRL sweep lines ${sweeps} (the mouse crossings the sim counted as sweeps on the carrier)`);
  const kept = sweepRes.filter((r) => r.kept).length;
  pass('sweep_keeps_puck', sweepRes.length >= 10 && kept >= 7, `attempts ${sweepRes.length}, puck kept 30 frames later ${kept}, poked clean ${sweepRes.filter((r) => r.poked).length}`);
} else {
  pass('spin_edges', spinBtn >= 10, `E presses seen as btn=spin ${spinBtn}; pass edges on F ${passBtn}`);
  pass('spins_seen_by_sim', spins >= 10, `JAMCTRL spin lines ${spins} (presses the sim accepted: the bar not empty, the cooldown over)`);
  const kept = spinRes.filter((r) => r.kept).length;
  const knocked = spinRes.filter((r) => r.knocked);
  pass('spin_keeps_puck', spinRes.length >= 10 && kept >= 7, `attempts ${spinRes.length}, puck kept ${kept}, poked clean ${spinRes.filter((r) => r.poked).length}`);
  pass('spin_checked_falls_longer', knocked.length === 0 || knocked.some((r) => r.knockFrames >= 135),
    `knocked mid-spin ${knocked.length}, knock frames ${knocked.map((r) => r.knockFrames).join(',') || '-'} (1.5x = 135)`);
}
const summary = lines.filter((l) => l.includes('JAMDRILL stick_summary')).map((l) => l.replace(/^.*JAMDRILL/, 'JAMDRILL'));
for (const s of summary) console.log('  ' + s);
out.summary = summary;
out.fails = fails;
console.log(JSON.stringify(out));
process.exit(fails ? 1 : 0);
