#!/usr/bin/env node
// Lane A153 (REVIEW-2 item 10): the human's mates' releases, from a log traced frame by frame (-trace=1).
//   node tools/perf/results/a153/releases.mjs <log...>
// A release: the puck's speed goes above 14 m/s on a frame whose previous frame had a mate of the human as the carrier of record (TRACE car=) or as the
// last toucher with the puck within 2 m of it. Counted a match: every release; those from inside 15 m of the net the mates attack; of those, the ones that
// head for the net (the puck's first two frames of flight point inside the mouth widened by a metre a side: a shot). Also the human's passes received a
// minute of play (the playtest line) and the mates' goals share is not known to the trace: goals for a match are printed from the playtest line.
import { createReadStream } from 'node:fs';
import { createInterface } from 'node:readline';
const GX = { 6: 56, 10: 41 };
for (const file of process.argv.slice(2)) {
  let matches = 0, rel = 0, rel15 = 0, shot15 = 0, shotAll = 0, pr = 0, playFrames = 0, gf = 0, shotsMates = 0;
  let prev = null, pend = null;
  const rl = createInterface({ input: createReadStream(file) });
  for await (const l of rl) {
    const j = l.indexOf('{"tool":"thirdjam.playtest/1"');
    if (j >= 0) { try { const p = JSON.parse(l.slice(j)); matches++; pr += +p.passes_received || 0; playFrames += +p.play_frames || 0; gf += p.goals?.[0] ?? 0; shotsMates += +p.shots_mates || 0; } catch {} prev = null; pend = null; continue; }
    const i = l.indexOf('TRACE f='); if (i < 0) continue;
    const t = l.slice(i);
    const m = /f=(\d+)( frozen)? stall=(\d) puck\(([-\d.]+),([-\d.]+) v([\d.]+)\)/.exec(t); if (!m) continue;
    const s = /since=(\d+) ht=(\d) car=(-?\d+) lt=(-?\d+) fo=(\d)/.exec(t); if (!s) continue;
    const sk = [...t.matchAll(/ (\d)([PCSBH])\(([-\d.]+),([-\d.]+)\)/g)].map((x) => ({ team: +x[1], role: x[2], x: +x[3], y: +x[4] }));
    const hi = sk.findIndex((k) => k.role === 'H'); if (hi < 0) { prev = null; continue; }
    const cur = { f: +m[1], frozen: !!m[2], px: +m[4], py: +m[5], pv: +m[6], car: +s[3], lt: +s[4], sk, hi };
    if (pend && cur.f === pend.f + 1 && !cur.frozen) {
      // the second frame of flight: does it head for the net?
      const dx = cur.px - pend.px0, dy = cur.py - pend.py0;
      if (Math.abs(dx) > 1e-6 && Math.sign(dx) === pend.sg) { const yAt = pend.py0 + dy * ((pend.gx - pend.px0) / dx); if (Math.abs(yAt) < 3.0) { shotAll++; if (pend.in15) shot15++; } }
      pend = null;
    } else if (pend && cur.f > pend.f + 1) pend = null;
    if (prev && !cur.frozen && !prev.frozen && cur.f === prev.f + 1 && cur.pv > 14 && prev.pv <= 14) {
      const team = sk[hi].team;
      let who = prev.car;
      if (who < 0 && prev.lt >= 0 && prev.sk[prev.lt] && Math.hypot(prev.sk[prev.lt].x - prev.px, prev.sk[prev.lt].y - prev.py) < 2.0) who = prev.lt;
      if (who >= 0 && who !== hi && prev.sk[who] && prev.sk[who].team === team) {
        const sg = team === 0 ? 1 : -1, gx = sg * (GX[sk.length] || 41);
        const d = Math.hypot(gx - prev.px, prev.py);
        rel++; if (d < 15) rel15++;
        pend = { f: cur.f, px0: prev.px, py0: prev.py, gx, sg, in15: d < 15 };
      }
    }
    prev = cur;
  }
  const n = matches || 1;
  console.log(`${file.split(/[\/]/).pop()} matches=${matches} mates: releases/match=${(rel / n).toFixed(2)} from_inside_15m=${(rel15 / n).toFixed(2)} at_the_net=${(shotAll / n).toFixed(2)} at_the_net_from_inside_15m=${(shot15 / n).toFixed(2)} shots_mates(playtest)=${(shotsMates / n).toFixed(2)} | human: passes_received/min=${(pr / (playFrames / 3600 || 1)).toFixed(2)} goals_for=${(gf / n).toFixed(2)}`);
}
