#!/usr/bin/env node
// Lane A80: read a packaged game's log (a80-prove.ps1, jam.vis.lowlog 1) and print the counts, the figure-against-the-ice table per take and the
// hit-stop offsets (the frame the figure stands versus the sim's release frame).
//   node tools/qa/ctrl/a80-analyze.mjs <game.log> [--json out.json]
// Output: INFO lines and one JSON line last. Exit 0, 2 could not run.
import { readFileSync, writeFileSync } from 'node:fs';

const file = process.argv[2];
if (!file) { console.error('usage: a80-analyze.mjs <game.log> [--json out.json]'); process.exit(2); }
const jIdx = process.argv.indexOf('--json');
const lines = readFileSync(file, 'utf8').split(/\r?\n/);
const num = (re, l) => { const m = l.match(re); return m ? parseFloat(m[1]) : NaN; };
const str = (re, l) => { const m = l.match(re); return m ? m[1] : ''; };

// De-duplicate frames (a re-opened map logs the first frames twice): keep the last occurrence of a (kind, frame, skater).
const seen = new Map();
const counts = { hit: 0, board: 0, pin: 0, stickhit: 0, knockdown: 0, getup: 0, knockdownHuman: 0, hitstop: 0, kick: 0 };
const kd = []; const gu = [];
for (const l of lines) {
  const m = l.match(/JAMVIS (hit|board|pin|stickhit|knockdown|getup) frame=(\d+) skater=(\d+)/);
  if (m) {
    const key = `${m[1]}|${m[2]}|${m[3]}|${/on=0/.test(l)}`;
    if (seen.has(key)) continue; seen.set(key, 1);
    counts[m[1]]++;
    const human = num(/human=(\d)/, l);
    if (m[1] === 'knockdown') { counts.knockdownHuman += human === 1 ? 1 : 0; kd.push({ frame: +m[2], skater: +m[3], human, sim: num(/sim_frames=(\d+)/, l), sent: num(/sent_frames=(\d+)/, l), hs: num(/hitstop=(\d+)/, l), dir: str(/dir=\(([^)]*)\)/, l) }); }
    if (m[1] === 'getup') gu.push({ frame: +m[2], skater: +m[3] });
  }
  if (l.includes('JAMHITSTOP frame=')) counts.hitstop++;
  if (l.includes('JAMKICK frame=')) counts.kick++;
}
const ev = { check: 0, knockdown: 0, foul: 0 };
const evSeen = new Set();
for (const l of lines) {
  const m = l.match(/JAMEVENT (check|knockdown|foul) who=(-?\d+) .*frame=(\d+)/);
  if (!m) continue; const k = `${m[1]}|${m[2]}|${m[3]}`; if (evSeen.has(k)) continue; evSeen.add(k); ev[m[1]]++;
}
console.log(`INFO counts JAMVIS hit ${counts.hit} board ${counts.board} pin ${counts.pin} stickhit ${counts.stickhit} knockdown ${counts.knockdown} (human involved as victim ${counts.knockdownHuman}) getup ${counts.getup}; JAMHITSTOP ${counts.hitstop} JAMKICK ${counts.kick}; JAMEVENT check ${ev.check} knockdown ${ev.knockdown} foul ${ev.foul}`);

// JAMLOW per knockdown.
const low = new Map(); // skater -> [rows]
const lowSeen = new Set();
for (const l of lines) {
  if (!l.includes('JAMLOW frame=')) continue;
  const r = {
    frame: num(/frame=(\d+)/, l), skater: num(/skater=(\d+)/, l), human: num(/human=(\d)/, l), down: num(/down=(\d)/, l), held: num(/held=(\d)/, l), rel: num(/rel=(-?\d+)/, l),
    low: num(/low_cm=(-?[\d.]+)/, l), lowName: str(/ low=(\S+)/, l), pelvis: num(/pelvis_cm=(-?[\d.e+]+)/, l), hand: num(/hand_cm=(-?[\d.e+]+)/, l), head: num(/head_cm=(-?[\d.e+]+)/, l), foot: num(/foot_cm=(-?[\d.e+]+)/, l),
    stand: num(/stand_pelvis_cm=(-?[\d.e+]+)/, l), dir: str(/dir=\(([^)]*)\)/, l), montage: str(/montage=(\S+)/, l),
  };
  const k = `${r.skater}|${r.frame}`; if (lowSeen.has(k)) continue; lowSeen.add(k);
  if (!low.has(r.skater)) low.set(r.skater, []);
  low.get(r.skater).push(r);
}
const clean = (x) => (Math.abs(x) > 1e7 ? NaN : x);
const knocks = [];
for (const k of kd) {
  const nxt = kd.filter((x) => x.skater === k.skater && x.frame > k.frame).map((x) => x.frame).sort((a, b) => a - b)[0] || 1e12;
  const rows = (low.get(k.skater) || []).filter((r) => r.frame >= k.frame && r.frame <= Math.min(k.frame + 180, nxt - 1)).sort((a, b) => a.frame - b.frame);
  const g = gu.find((x) => x.skater === k.skater && x.frame > k.frame);
  const rel = g ? g.frame : k.frame + k.sim;
  const lying = rows.filter((r) => r.frame >= k.frame + 20 && r.frame < rel - 20);   // the lying pose, off the fall and the get-up
  const montages = [...new Set(rows.filter((r) => r.montage !== 'none').map((r) => r.montage))];
  const hsHeld = rows.filter((r) => r.held === 1).length;
  const stand = rows.length ? Math.max(...rows.map((r) => r.stand || 0)) : NaN;
  // The figure stands: the first frame after the lying minimum whose pelvis is back at 90 % of the standing height.
  let standFrame = NaN, stand80 = NaN;
  if (rows.length && stand > 20) {
    const minP = lying.length ? Math.min(...lying.map((r) => clean(r.pelvis)).filter((x) => !isNaN(x))) : 0;
    const after = rows.filter((r) => r.frame > k.frame + 25);
    const lastLying = after.filter((r) => clean(r.pelvis) < 0.6 * stand);
    const lastLy = lastLying.length ? lastLying[lastLying.length - 1].frame : k.frame + 25;
    const up = after.find((r) => r.frame >= lastLy && clean(r.pelvis) >= 0.9 * stand);
    const up8 = after.find((r) => r.frame >= lastLy && clean(r.pelvis) >= 0.8 * stand);
    if (up) standFrame = up.frame; if (up8) stand80 = up8.frame;
    void minP;
  }
  const mn = (f) => { const v = lying.map((r) => clean(r[f])).filter((x) => !isNaN(x)); return v.length ? Math.min(...v) : NaN; };
  const worst = lying.reduce((a, r) => (!a || r.low < a.low ? r : a), null);
  knocks.push({ ...k, release: rel, rows: rows.length, lyingFrames: lying.length, montage: montages[0] || 'none', montageAll: montages.join('+'), hsHeld, lowMin: mn('low'), lowBone: worst ? worst.lowName : '', pelvisMin: mn('pelvis'), handMin: mn('hand'), headMin: mn('head'), footMin: mn('foot'), stand, standFrame, stand80, offset: standFrame - rel, offset80: stand80 - rel });
}
const f1 = (x) => (isNaN(x) ? '-' : x.toFixed(1));
console.log('INFO per knockdown: frame skater human hs  montage  lying-frames  lowest(bone) pelvis hand head foot  stand_pelvis  stand-frame release offset(90%) offset(80%)');
for (const q of knocks) console.log(`  f${q.frame} s${q.skater} h${q.human} hs${q.hs} ${q.montage} n=${q.lyingFrames} low ${f1(q.lowMin)} (${q.lowBone}) pelvis ${f1(q.pelvisMin)} hand ${f1(q.handMin)} head ${f1(q.headMin)} foot ${f1(q.footMin)} stand ${f1(q.stand)} up f${q.standFrame} rel f${q.release} off ${isNaN(q.offset) ? '-' : q.offset} off80 ${isNaN(q.offset80) ? '-' : q.offset80}`);
// Per take table.
const takes = new Map();
for (const q of knocks) { const t = q.montage; if (!takes.has(t)) takes.set(t, []); takes.get(t).push(q); }
console.log('INFO per take (min over the lying window, cm above the ice; negative = under): take n lowest pelvis hand head foot');
for (const [t, qs] of takes) {
  const m = (f) => { const v = qs.map((q) => q[f]).filter((x) => !isNaN(x)); return v.length ? Math.min(...v) : NaN; };
  const md = (f) => { const v = qs.map((q) => q[f]).filter((x) => !isNaN(x)).sort((a, b) => a - b); return v.length ? v[Math.floor(v.length / 2)] : NaN; };
  console.log(`  ${t} n=${qs.length} lowest ${f1(m('lowMin'))} (median ${f1(md('lowMin'))}) pelvis ${f1(m('pelvisMin'))} (median ${f1(md('pelvisMin'))}) hand ${f1(m('handMin'))} head ${f1(m('headMin'))} foot ${f1(m('footMin'))}`);
}
// Offsets: only knockdowns whose hit-stop held (hsHeld > 0) versus the others.
const offs = (qs) => qs.map((q) => q.offset).filter((x) => !isNaN(x));
const stats = (a) => (a.length ? `n=${a.length} min ${Math.min(...a)} max ${Math.max(...a)} mean ${(a.reduce((s, x) => s + x, 0) / a.length).toFixed(1)} [${a.join(',')}]` : 'n=0');
const held = knocks.filter((q) => q.hsHeld > 0), free = knocks.filter((q) => q.hsHeld === 0);
console.log(`INFO offset (frame the pelvis is at 90 % of standing minus the sim release frame; + = the figure is late): held (human involved) ${stats(offs(held))}; not held ${stats(offs(free))}`);
const o8 = (qs) => qs.map((q) => q.offset80).filter((x) => !isNaN(x));
console.log(`INFO offset 80 %: held ${stats(o8(held))}; not held ${stats(o8(free))}`);
const out = { tool: 'thirdjam.a80/1', file, counts, ev, knocks };
console.log(JSON.stringify({ tool: 'thirdjam.a80/1', file, counts, ev, knockdowns: knocks.length, held: held.length }));
if (jIdx > 0) writeFileSync(process.argv[jIdx + 1], JSON.stringify(out, null, 1));
