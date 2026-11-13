#!/usr/bin/env node
// Lane A71: the idle-stretch measurement from the per-frame log (not the per-match totals of aggregate.mjs).
//
//   node tools/soak/playtest/idle.mjs <log...> [--json out.json] [--min 1800]
//
// Input: UnrealEditor-Cmd logs of `-run=JamTournament -sim=jam -cartridge=hockey.regular -player=<class> -difficulty=<level> -seeds=a-b -trace=30`
// (the trace line is written every 30 frames and on every frame the human touches the puck, with since= / ht= / car= / lt= fields; lane A71 added them to the
// playtester path). A match ends at its thirdjam.playtest/1 line. Per match: the human's touch frames, the gaps between them (start to first touch and last
// touch to the end count), the longest gap in play frames (frozen samples taken out), and for that stretch who had the puck, where it was, what the human was doing.
// Prints a table (p50 / p90 / max of the longest stretch, how many matches reach 30 s) and the make-up of the p80-p95 neighbourhood. Exit 0.
import { readFileSync, writeFileSync } from 'node:fs';

const args = process.argv.slice(2);
const opt = (n, d) => { const i = args.indexOf(`--${n}`); return i >= 0 ? args[i + 1] : d; };
const files = args.filter((a, i) => !a.startsWith('--') && !(i > 0 && args[i - 1].startsWith('--')));
const minGap = Number(opt('min', '1800'));
const STEP = 30;

function parseRow(t) {
  const m = /f=(\d+)( frozen)? stall=(\d) puck\(([-\d.]+),([-\d.]+) v([\d.]+)\)/.exec(t);
  if (!m) return null;
  const sk = [...t.matchAll(/ (\d)([PCSBH])\(([-\d.]+),([-\d.]+)\)>(-?\d+)\[([-\d.]+),([-\d.]+)/g)].map((x) => ({ team: +x[1], role: x[2], x: +x[3], y: +x[4], mx: +x[6], my: +x[7] }));
  const s = /since=(\d+) ht=(\d) car=(-?\d+) lt=(-?\d+) fo=(\d) pk=(\d+)/.exec(t);
  if (!s || !sk.length) return null;
  const hi = sk.findIndex((k) => k.role === 'H');
  return { f: +m[1], frozen: !!m[2], stall: +m[3], px: +m[4], py: +m[5], pv: +m[6], sk, hi, since: +s[1], ht: +s[2] === 1, car: +s[3], lt: +s[4] };
}

// Team 0 attacks +x (OppGoal(0) = +GoalX). The human's team is the human skater's team.
function zone(r, halfLen) {
  const team = r.sk[r.hi].team;
  const x = team === 0 ? r.px : -r.px;
  return x > halfLen / 3 ? 'attack' : x < -halfLen / 3 ? 'defend' : 'neutral';
}

function analyse(rows, result) {
  if (!rows.length || rows[0].hi < 0) return null;
  const halfLen = 56;
  const touches = rows.filter((r) => r.ht).map((r) => r.f);
  const last = rows[rows.length - 1].f;
  const marks = [0, ...touches, last];
  let best = { len: 0, from: 0, to: 0 };
  for (let i = 1; i < marks.length; i++) {
    const a = marks[i - 1], b = marks[i];
    const frozen = rows.filter((r) => r.frozen && r.f > a && r.f <= b && !r.ht).length * STEP;
    const len = Math.max(0, b - a - frozen);
    if (len > best.len) best = { len, from: a, to: b };
  }
  const inside = rows.filter((r) => r.f > best.from && r.f <= best.to && !r.ht);
  const share = { faceoff_pause: 0, human_in_box: 0, opp_carry: 0, mate_carry: 0, loose: 0 };
  const where = { attack: 0, neutral: 0, defend: 0 };
  let n = 0;
  for (const r of inside) {
    const h = r.sk[r.hi];
    n++;
    if (r.frozen) { share.faceoff_pause++; continue; }
    // the penalty box: the human parked at the boards near the centre line, not moving
    const box = Math.abs(h.y) > 21.5 && Math.abs(h.x) < 8 && Math.hypot(h.mx, h.my) < 0.05;
    if (box) { share.human_in_box++; continue; }
    where[zone(r, halfLen)]++;
    if (r.car < 0) share.loose++;
    else if (r.sk[r.car].team !== h.team) share.opp_carry++;
    else share.mate_carry++;
  }
  for (const k of Object.keys(share)) share[k] = n ? share[k] / n : 0;
  const w = where.attack + where.neutral + where.defend || 1;
  const dominant = Object.entries(share).sort((a, b) => b[1] - a[1])[0][0];
  return { longest: best.len, from: best.from, to: best.to, share, dominant, where: { attack: where.attack / w, neutral: where.neutral / w, defend: where.defend / w }, touches: touches.length, frames: last };
}

const matches = [];
for (const file of files) {
  let rows = [];
  for (const line of readFileSync(file, 'utf8').split(/\r?\n/)) {
    const i = line.indexOf('TRACE f=');
    if (i >= 0) { const r = parseRow(line.slice(i)); if (r) rows.push(r); continue; }
    const j = line.indexOf('{"tool":"thirdjam.playtest/1"');
    if (j >= 0) {
      let res; try { res = JSON.parse(line.slice(j)); } catch { rows = []; continue; }
      const a = analyse(rows, res);
      if (a) matches.push({ ...a, player: res.player, level: res.difficulty, seed: res.seed, passes_received: res.passes_received, over: res.over, box_frames: res.box_frames, max_no_touch: res.max_no_touch });
      rows = [];
    }
  }
}
const q = (a, p) => { const s = [...a].sort((x, y) => x - y); return s.length ? s[Math.min(s.length - 1, Math.floor(p * s.length))] : 0; };
const secs = (f) => Math.round(f / 6) / 10;
const L = matches.map((m) => m.longest);
const out = [];
out.push(`matches ${matches.length}; longest no-touch stretch per match (play frames, seconds): p50 ${q(L, 0.5)} (${secs(q(L, 0.5))} s), p90 ${q(L, 0.9)} (${secs(q(L, 0.9))} s), p99 ${q(L, 0.99)}, max ${Math.max(...L)} (${secs(Math.max(...L))} s)`);
out.push(`matches with a stretch >= 15 s: ${L.filter((x) => x >= 900).length}; >= 30 s: ${L.filter((x) => x >= minGap).length} (${Math.round((100 * L.filter((x) => x >= minGap).length) / matches.length)} %)`);
out.push('');
out.push('| class | level | n | p50 s | p90 s | max s | >= 30 s |');
out.push('|---|---|---|---|---|---|---|');
for (const p of ['novice', 'regular', 'good']) for (const l of ['rookie', 'pro', 'all_star']) {
  const x = matches.filter((m) => m.player === p && m.level === l).map((m) => m.longest);
  if (x.length) out.push(`| ${p} | ${l} | ${x.length} | ${secs(q(x, 0.5))} | ${secs(q(x, 0.9))} | ${secs(Math.max(...x))} | ${x.filter((v) => v >= minGap).length} |`);
}
const lo = q(L, 0.8), hi = q(L, 0.95);
const near = matches.filter((m) => m.longest >= lo && m.longest <= hi);
const avg = (a, f) => (a.length ? a.reduce((s, m) => s + f(m), 0) / a.length : 0);
out.push('');
out.push(`the p80-p95 neighbourhood (${near.length} matches, stretches ${secs(lo)}-${secs(hi)} s): mean share of the stretch's frames`);
for (const k of ['opp_carry', 'mate_carry', 'loose', 'faceoff_pause', 'human_in_box']) out.push(`- ${k}: ${(100 * avg(near, (m) => m.share[k])).toFixed(0)} %`);
out.push(`- puck in the human's attack third ${(100 * avg(near, (m) => m.where.attack)).toFixed(0)} %, neutral ${(100 * avg(near, (m) => m.where.neutral)).toFixed(0)} %, own defend third ${(100 * avg(near, (m) => m.where.defend)).toFixed(0)} %`);
const dom = {}; near.forEach((m) => { dom[m.dominant] = (dom[m.dominant] || 0) + 1; });
out.push(`- dominant cause per match: ${Object.entries(dom).map(([k, v]) => `${k} ${v}`).join(', ')}`);
const mateHeavy = near.filter((m) => m.share.mate_carry >= 0.3);
out.push(`- stretches where a teammate had the puck for >= 30 % of the frames: ${mateHeavy.length} of ${near.length}; the human received on average ${avg(near, (m) => m.passes_received || 0).toFixed(1)} teammate passes in these matches (all matches: ${avg(matches, (m) => m.passes_received || 0).toFixed(1)})`);
const allDom = {}; matches.filter((m) => m.longest >= minGap).forEach((m) => { allDom[m.dominant] = (allDom[m.dominant] || 0) + 1; });
out.push(`- all stretches >= 30 s, dominant cause: ${Object.entries(allDom).map(([k, v]) => `${k} ${v}`).join(', ') || 'none'}`);
console.log(out.join('\n'));
const jf = opt('json', '');
if (jf) writeFileSync(jf, JSON.stringify(matches));
console.log(JSON.stringify({ tool: 'thirdjam.idle/1', matches: matches.length, p50: q(L, 0.5), p90: q(L, 0.9), max: Math.max(...L), over30s: L.filter((x) => x >= minGap).length }));
