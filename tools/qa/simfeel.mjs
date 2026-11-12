#!/usr/bin/env node
// Lane A10: offline port of UJamSim::StepSkater (Game/Source/JamCore/Private/JamSim.cpp) for what-if feel numbers.
// It is the sim's skater formula, frame by frame at 60 Hz, nothing else: no puck, no boards, no bots.
//   node tools/qa/simfeel.mjs                 baseline (hockey.regular) and the variants below
//   node tools/qa/simfeel.mjs --json          machine-readable
const DT = 1 / 60;
const BASE = { speed: 8, accel: 12, turn_rate: 5, lateral_grip: 3, glide_drag: 0.147, brake_decel: 6, ice_friction: 0.015 };

function wrapPi(a) { while (a > Math.PI) a -= 2 * Math.PI; while (a < -Math.PI) a += 2 * Math.PI; return a; }
function rotateToward(yaw, target, step) { const d = wrapPi(target - yaw); return Math.abs(d) <= step ? target : yaw + Math.sign(d) * step; }
function moveToward(a, b, d) { return Math.abs(b - a) <= d ? b : a + Math.sign(b - a) * d; }

function stepSkater(s, c, move, brake) {
  const glide = c.glide_drag * (c.ice_friction / 0.015);
  let mag = Math.hypot(move[0], move[1]);
  if (mag > 1) { move = [move[0] / mag, move[1] / mag]; mag = 1; }
  const top = c.speed * mag;
  if (mag > 0.05) s.yaw = rotateToward(s.yaw, Math.atan2(move[1], move[0]), c.turn_rate * DT);
  const f = [Math.cos(s.yaw), Math.sin(s.yaw)];
  let along = s.v[0] * f[0] + s.v[1] * f[1];
  let side = [s.v[0] - f[0] * along, s.v[1] - f[1] * along];
  if (mag > 0.05 && !brake && along < top) along = Math.min(top, along + c.accel * mag * DT);
  along = moveToward(along, 0, (glide + (brake ? c.brake_decel : 0)) * DT);
  const k = Math.max(0, 1 - (c.lateral_grip + (brake ? c.brake_decel * 0.5 : 0)) * DT);
  side = [side[0] * k, side[1] * k];
  s.v = [f[0] * along + side[0], f[1] * along + side[1]];
  s.p = [s.p[0] + s.v[0] * DT, s.p[1] + s.v[1] * DT];
}

function measure(c) {
  const out = {};
  // accel from rest, full stick +x
  let s = { p: [0, 0], v: [0, 0], yaw: 0 };
  let f95 = null, f99 = null, d95 = null;
  for (let i = 1; i <= 300; i++) {
    stepSkater(s, c, [1, 0], false);
    const sp = Math.hypot(...s.v);
    if (f95 === null && sp >= 0.95 * c.speed) { f95 = i; d95 = s.p[0]; }
    if (f99 === null && sp >= 0.99 * c.speed) f99 = i;
  }
  out.accel = { frames_to_95pct: f95, s_to_95pct: +(f95 / 60).toFixed(2), dist_to_95pct_m: +d95.toFixed(1), frames_to_99pct: f99 };
  // glide from top speed, no input
  s = { p: [0, 0], v: [c.speed, 0], yaw: 0 };
  let v5 = null;
  let fStop = null;
  for (let i = 1; i <= 7200; i++) {
    stepSkater(s, c, [0, 0], false);
    if (i === 300) v5 = Math.hypot(...s.v);
    if (fStop === null && Math.hypot(...s.v) < 0.05) { fStop = i; break; }
  }
  out.glide = { v_after_5s_mps: +v5.toFixed(2), s_to_stop: fStop ? +(fStop / 60).toFixed(1) : '>120', stop_distance_m: +s.p[0].toFixed(1) };
  // brake (bBrake, no key bound in M0): from top speed
  s = { p: [0, 0], v: [c.speed, 0], yaw: 0 };
  fStop = null;
  for (let i = 1; i <= 1200; i++) { stepSkater(s, c, [0, 0], true); if (Math.hypot(...s.v) < 0.05) { fStop = i; break; } }
  out.brake_key = { s_to_stop: +(fStop / 60).toFixed(2), stop_distance_m: +s.p[0].toFixed(2) };
  // reverse stick (what a player does to stop): from top speed, stick to -x until velocity along +x <= 0
  s = { p: [0, 0], v: [c.speed, 0], yaw: 0 };
  let fRev = null, xRev = null, yRev = null, minSp = 99;
  for (let i = 1; i <= 1200; i++) {
    stepSkater(s, c, [-1, 0], false);
    minSp = Math.min(minSp, Math.hypot(...s.v));
    if (s.v[0] <= 0) { fRev = i; xRev = s.p[0]; yRev = s.p[1]; break; }
  }
  out.reverse_stop = { frames: fRev, s: +(fRev / 60).toFixed(2), along_distance_m: +xRev.toFixed(2), side_drift_m: +Math.abs(yRev).toFixed(2), min_speed_mps: +minSp.toFixed(2) };
  // 90 degree turn at top speed: stick +y from +x travel; frames until velocity is within 5 deg of +y, lateral slip peak
  s = { p: [0, 0], v: [c.speed, 0], yaw: 0 };
  let f90 = null, slipMax = 0, overshoot = 0;
  for (let i = 1; i <= 600; i++) {
    stepSkater(s, c, [0, 1], false);
    const va = Math.atan2(s.v[1], s.v[0]);
    slipMax = Math.max(slipMax, Math.abs(wrapPi(va - s.yaw)) * 180 / Math.PI);
    overshoot = Math.max(overshoot, s.p[0]);
    if (f90 === null && Math.abs(wrapPi(va - Math.PI / 2)) < 5 * Math.PI / 180) f90 = i;
  }
  out.turn90 = { frames_until_velocity_follows: f90, s: +(f90 / 60).toFixed(2), max_slip_deg: +slipMax.toFixed(0), drift_along_old_heading_m: +overshoot.toFixed(1) };
  // steady circle: stick always 90 deg left of velocity
  s = { p: [0, 0], v: [c.speed, 0], yaw: 0 };
  const pts = [];
  for (let i = 1; i <= 600; i++) {
    const u = Math.hypot(...s.v) > 0.5 ? [s.v[0] / Math.hypot(...s.v), s.v[1] / Math.hypot(...s.v)] : [1, 0];
    stepSkater(s, c, [-u[1], u[0]], false);
    if (i > 300) pts.push({ p: [...s.p], v: [...s.v], yaw: s.yaw });
  }
  const cx = pts.reduce((a, q) => a + q.p[0], 0) / pts.length, cy = pts.reduce((a, q) => a + q.p[1], 0) / pts.length;
  const r = pts.reduce((a, q) => a + Math.hypot(q.p[0] - cx, q.p[1] - cy), 0) / pts.length;
  const vm = pts.reduce((a, q) => a + Math.hypot(...q.v), 0) / pts.length;
  const slip = pts.reduce((a, q) => a + Math.abs(wrapPi(Math.atan2(q.v[1], q.v[0]) - q.yaw)) * 180 / Math.PI, 0) / pts.length;
  out.circle = { radius_m: +r.toFixed(2), speed_mps: +vm.toFixed(2), slip_deg: +slip.toFixed(1), lap_s: +((2 * Math.PI * r) / vm).toFixed(2) };
  // shot flight: puck drag 0, 18 m/s
  out.shot = { s_from_20m: +(20 / 18).toFixed(2), s_from_centre_120: +(56 / 18).toFixed(2), s_from_centre_90: +(41 / 18).toFixed(2), skater_speed_ratio: +(18 / c.speed).toFixed(2) };
  out.rink = { s_end_to_end_120: +(120 / c.speed).toFixed(1), s_end_to_end_90: +(90 / c.speed).toFixed(1) };
  return out;
}

const variants = [
  ['baseline hockey.regular', {}],
  ['arena.small: speed 11', { speed: 11 }],
  ['accel 16', { accel: 16 }],
  ['accel 20', { accel: 20 }],
  ['lateral_grip 2', { lateral_grip: 2 }],
  ['lateral_grip 4.5', { lateral_grip: 4.5 }],
  ['turn_rate 4', { turn_rate: 4 }],
  ['turn_rate 6.5', { turn_rate: 6.5 }],
  ['glide_drag 0.3', { glide_drag: 0.3 }],
  ['glide_drag 0.5', { glide_drag: 0.5 }],
  ['speed 10, accel 16, grip 2.5', { speed: 10, accel: 16, lateral_grip: 2.5 }],
  ['speed 11, accel 18, grip 2.5, glide 0.3', { speed: 11, accel: 18, lateral_grip: 2.5, glide_drag: 0.3 }],
];
const json = process.argv.includes('--json');
const all = {};
for (const [label, over] of variants) {
  const c = { ...BASE, ...over };
  const m = measure(c);
  all[label] = m;
  if (!json) {
    console.log(`\n== ${label}`);
    console.log(`  accel: ${m.accel.s_to_95pct} s / ${m.accel.dist_to_95pct_m} m to 95% top | glide: ${m.glide.v_after_5s_mps} m/s after 5 s, stops after ${m.glide.stop_distance_m} m (${m.glide.s_to_stop} s)`);
    console.log(`  reverse-stick stop: ${m.reverse_stop.s} s, ${m.reverse_stop.along_distance_m} m along, min speed ${m.reverse_stop.min_speed_mps} | brake key: ${m.brake_key.s_to_stop} s, ${m.brake_key.stop_distance_m} m`);
    console.log(`  90 deg turn: velocity follows after ${m.turn90.s} s, slip peak ${m.turn90.max_slip_deg} deg, drifts ${m.turn90.drift_along_old_heading_m} m on | circle r ${m.circle.radius_m} m at ${m.circle.speed_mps} m/s, slip ${m.circle.slip_deg} deg, lap ${m.circle.lap_s} s`);
    console.log(`  shot 18 m/s: ${m.shot.s_from_20m} s from 20 m, ${m.shot.s_from_centre_120} s centre-to-net on 120 m (${m.shot.s_from_centre_90} s on 90 m), ${m.shot.skater_speed_ratio}x skater | end to end ${m.rink.s_end_to_end_120} s / ${m.rink.s_end_to_end_90} s`);
  }
}
if (json) console.log(JSON.stringify(all, null, 2));
