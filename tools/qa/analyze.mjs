#!/usr/bin/env node
// Lane A10: numbers from one drive run: node tools/qa/analyze.mjs <run dir> [--speed 8] [--rink 120]
// Reads <run>/rows.jsonl (one row per sim frame, see drive.py), <run>/events.txt (JAMEVENT log lines) and <run>/status.json;
// prints a metrics JSON (also written to <run>/metrics.json). Frames are the clock (60 Hz).
import { readFileSync, writeFileSync, existsSync } from 'node:fs';
import { join } from 'node:path';

const argv = process.argv.slice(2);
function opt(name, dflt) { const i = argv.indexOf('--' + name); if (i < 0) return dflt; const v = argv[i + 1]; argv.splice(i, 2); return v; }
const SPEED = Number(opt('speed', '8'));
const RINK = Number(opt('rink', '120'));
const dir = argv[0];
if (!dir) { console.error('usage: node tools/qa/analyze.mjs <run dir>'); process.exit(2); }

const rows = existsSync(join(dir, 'rows.jsonl')) ? readFileSync(join(dir, 'rows.jsonl'), 'utf8').trim().split('\n').filter(Boolean).map((l) => JSON.parse(l)) : [];
const status = existsSync(join(dir, 'status.json')) ? JSON.parse(readFileSync(join(dir, 'status.json'), 'utf8')) : {};
const events = existsSync(join(dir, 'events.txt')) ? readFileSync(join(dir, 'events.txt'), 'utf8').split('\n').filter((l) => l.includes('JAMEVENT ')) : [];
const ev = events.map((l) => {
  const m = l.match(/JAMEVENT (\w+) who=(-?\d+) where=\(([-\d.]+),([-\d.]+),([-\d.]+)\) strength=([-\d.]+) frame=(\d+) kind=(\w+)/);
  return m ? { name: m[1], who: Number(m[2]), x: Number(m[3]) / 100, y: Number(m[4]) / 100, strength: Number(m[6]), frame: Number(m[7]), kind: m[8] } : null;
}).filter(Boolean);

const speed = (r) => Math.hypot(r.vx, r.vy);
const byPhase = {};
for (const r of rows) (byPhase[r.ph] ??= []).push(r);
const out = { run: dir, plan: status.plan, frames: rows.length, first_frame: rows[0]?.f, last_frame: rows.at(-1)?.f, status };

if (status.plan === 'skate') {
  const acc = byPhase.accel ?? [];
  if (acc.length > 2) {
    const v = acc.map(speed);
    const top = Math.max(...v);
    const f95 = acc.findIndex((r) => speed(r) >= 0.95 * SPEED);
    const f99 = acc.findIndex((r) => speed(r) >= 0.99 * SPEED);
    out.accel = { top_speed_mps: +top.toFixed(3), frames_to_95pct: f95, frames_to_99pct: f99, s_to_95pct: +(f95 / 60).toFixed(2),
      dist_to_95pct_m: f95 > 0 ? +Math.hypot(acc[f95].x - acc[0].x, acc[f95].y - acc[0].y).toFixed(2) : null, start_pos: [acc[0].x, acc[0].y] };
  }
  const gl = byPhase.glide ?? [];
  if (gl.length > 10) {
    const v0 = speed(gl[1]), v1 = speed(gl.at(-1));
    const t = (gl.length - 2) / 60;
    const a = (v0 - v1) / t;
    out.glide = { v_start_mps: +v0.toFixed(3), v_end_mps: +v1.toFixed(3), seconds: +t.toFixed(2), decel_mps2: +a.toFixed(4),
      dist_m: +Math.hypot(gl.at(-1).x - gl[0].x, gl.at(-1).y - gl[0].y).toFixed(2), stop_distance_if_released_at_top_m: a > 0 ? +((SPEED * SPEED) / (2 * a)).toFixed(1) : null,
      seconds_to_stop_from_top: a > 0 ? +(SPEED / a).toFixed(1) : null };
  }
  const rv = byPhase.reverse ?? [];
  if (rv.length > 5) {
    const dir0 = [rv[0].vx, rv[0].vy]; const m0 = Math.hypot(...dir0); const u = [dir0[0] / m0, dir0[1] / m0];
    let stopIdx = rv.findIndex((r) => r.vx * u[0] + r.vy * u[1] <= 0);
    if (stopIdx < 0) stopIdx = rv.length - 1;
    const along = (rv[stopIdx].x - rv[0].x) * u[0] + (rv[stopIdx].y - rv[0].y) * u[1];
    const side = Math.abs(-(rv[stopIdx].x - rv[0].x) * u[1] + (rv[stopIdx].y - rv[0].y) * u[0]);
    const minSpeed = Math.min(...rv.map(speed));
    out.reverse_stop = { v_at_reverse_mps: +m0.toFixed(2), frames_to_stop: stopIdx, s_to_stop: +(stopIdx / 60).toFixed(2), along_distance_m: +along.toFixed(2), side_drift_m: +side.toFixed(2), min_speed_mps: +minSpeed.toFixed(2) };
  }
  const ci = byPhase.circle ?? [];
  if (ci.length > 120) {
    const tail = ci.slice(-120);
    const cx = tail.reduce((a, r) => a + r.x, 0) / tail.length, cy = tail.reduce((a, r) => a + r.y, 0) / tail.length;
    const radii = tail.map((r) => Math.hypot(r.x - cx, r.y - cy));
    const rMean = radii.reduce((a, b) => a + b, 0) / radii.length;
    const vMean = tail.reduce((a, r) => a + speed(r), 0) / tail.length;
    let yawRate = 0, slip = 0, n = 0;
    for (let i = 1; i < tail.length; i++) {
      let dy = tail[i].yaw - tail[i - 1].yaw; while (dy > 180) dy -= 360; while (dy < -180) dy += 360;
      yawRate += Math.abs(dy) * 60; n++;
      const va = Math.atan2(tail[i].vy, tail[i].vx) * 180 / Math.PI; let s = va - tail[i].yaw; while (s > 180) s -= 360; while (s < -180) s += 360; slip += Math.abs(s);
    }
    out.circle = { radius_m: +rMean.toFixed(2), speed_mps: +vMean.toFixed(2), yaw_rate_deg_s: +(yawRate / n).toFixed(1), yaw_rate_rad_s: +(yawRate / n / 180 * Math.PI).toFixed(2), mean_slip_deg: +(slip / n).toFixed(1), path_omega_rad_s: +(vMean / rMean).toFixed(2) };
  }
} else {
  const play = rows.filter((r) => r.sp === 'PLAY');
  const far = play.filter((r) => r.d > 25).length;
  const near = play.filter((r) => r.d < 1.1).length;
  let runs = [], cur = 0;
  for (const r of play) { if (r.d < 1.1) cur++; else { if (cur) runs.push(cur); cur = 0; } }
  if (cur) runs.push(cur);
  const human = status.human_index ?? 0;
  const touches = ev.filter((e) => e.name === 'touch' && e.who === human);
  const shots = ev.filter((e) => e.name === 'shot');
  const myShots = shots.filter((e) => e.who === human);
  const goals = ev.filter((e) => e.name === 'goal');
  const knocks = ev.filter((e) => e.name === 'knockdown');
  const checks = ev.filter((e) => e.name === 'check');
  const faceoffs = ev.filter((e) => e.name === 'faceoff');
  // time of flight: for each human shot, frames until a goal event or until the puck crosses the goal line in the rows
  const flights = myShots.map((s) => {
    const g = goals.find((gg) => gg.frame > s.frame && gg.frame - s.frame < 400);
    const row = rows.find((r) => r.f === s.frame);
    const pv = rows.filter((r) => r.f > s.frame && r.f <= s.frame + 6).map((r) => Math.hypot(r.pvx, r.pvy));
    return { frame: s.frame, dist_to_goal_line_m: row ? +Math.abs((RINK / 2 - 4) - row.px).toFixed(1) : null, puck_speed_after_mps: pv.length ? +Math.max(...pv).toFixed(1) : null, goal_after_frames: g ? g.frame - s.frame : null };
  });
  out.play = {
    first_touch_frame: status.first_touch, first_touch_s: status.first_touch != null ? +(status.first_touch / 60).toFixed(2) : null,
    goal_frames: status.goal_frames, first_goal_s: status.goal_frames?.[0] != null ? +(status.goal_frames[0] / 60).toFixed(1) : null,
    final_score: rows.at(-1)?.sc, play_frames: play.length, far_from_puck_pct: play.length ? +((100 * far) / play.length).toFixed(1) : null,
    on_puck_pct: play.length ? +((100 * near) / play.length).toFixed(1) : null, longest_carry_frames: runs.length ? Math.max(...runs) : 0, carries: runs.length,
    hardturn_frame: status.hardturn, puck_lost_after_hard_turn: status.lost_after_hardturn, shots_pressed: status.shots,
    events: { touches_human: touches.length, shots_all: shots.length, shots_human: myShots.length, goals: goals.length, knockdowns: knocks.length, knockdowns_human: knocks.filter((k) => k.who === human).length, checks: checks.length, checks_human: checks.filter((k) => k.who === human).length, faceoffs: faceoffs.length, names: [...new Set(ev.map((e) => e.name))] },
    flights, teleports: status.teleports,
  };
}
writeFileSync(join(dir, 'metrics.json'), JSON.stringify(out, null, 2));
console.log(JSON.stringify(out, null, 2));
