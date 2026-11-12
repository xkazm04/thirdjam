#!/usr/bin/env node
// tools/p4motion/gate.mjs: the motion gate of lane A29 (brief: R-0025 point 3). Reads one or more clip summaries written by tools/p4motion/clip.py
// (per-frame world positions of pelvis, feet, head and hands, raw and after the root strip) and prints PASS/FAIL per check:
//   loop closure      first and last pose within tolerance (pelvis-relative, every tracked bone), with the best loop cut it found
//   foot contact      no float (the lower foot stays near the ice every frame) and, for in-place clips, no slide beyond the glide the sim gives
//   root motion lint  the stripped clip's pelvis travels no more than ROOT_MAX_M horizontally (the sim owns displacement)
//   contact window    an attack clip names the frame range in which it hits; the gate checks it against the clip and the frame of peak hand speed
// Thresholds live in docs/experiments/P4-MOTION.md section "Gate thresholds" and are repeated here; change both.
//
//   node tools/p4motion/gate.mjs <summary.json> [...] [--jobs tools/p4motion/jobs.json] [--class locomotion|attack|reaction|pose] [--loop|--noloop] [--contact a,b]
//
// Exit 0 every clip passes, 1 a clip fails, 2 could not run. Last stdout line: {"tool":"thirdjam.p4motion-gate/1", ...}
import { readFileSync, existsSync } from 'node:fs';
import { basename } from 'node:path';

const T = {
  loop_pass_m: 0.05,      // max bone error first vs last pose, pelvis-relative: PASS at or below
  loop_warn_m: 0.10,      // WARN up to here, FAIL above
  float_max_m: 0.06,      // the lower foot may be at most this far above the clip's ice level, every frame (locomotion, pose): PASS
  float_warn_m: 0.10,     // WARN up to here (a short hop), FAIL above (the figure floats)
  contact_m: 0.04,        // a foot lower than ice + this counts as planted
  slide_max_mps: 0.15,    // planted-foot world speed on the RAW clip, in-place classes (attack, reaction, pose); locomotion reports its glide instead
  glide_max_mps: 8.0,     // the sim's top speed: a locomotion clip's implied travel may not exceed it
  root_max_m: 0.01,       // horizontal pelvis travel left after the strip
  min_loop_frames: 20,    // a loop cut shorter than this is noise
};

function args(argv) {
  const o = { files: [] };
  for (let i = 2; i < argv.length; i++) {
    const a = argv[i];
    if (!a.startsWith('--')) { o.files.push(a); continue; }
    const k = a.slice(2), v = argv[i + 1];
    if (['loop', 'noloop'].includes(k)) o[k] = true; else { o[k] = v; i++; }
  }
  return o;
}
const dist = (a, b) => Math.hypot(a[0] - b[0], a[1] - b[1], a[2] - b[2]);
const hdist = (a, b) => Math.hypot(a[0] - b[0], a[1] - b[1]);
const rel = (fr, k) => fr[k] && fr.pelvis ? [fr[k][0] - fr.pelvis[0], fr[k][1] - fr.pelvis[1], fr[k][2] - fr.pelvis[2]] : null;
const KEYS = ['foot_l', 'toe_l', 'foot_r', 'toe_r', 'head', 'hand_l', 'hand_r'];
const r3 = (x) => Math.round(x * 1000) / 1000;

function poseErr(a, b) {
  let m = 0;
  for (const k of KEYS) { const p = rel(a, k), q = rel(b, k); if (p && q) m = Math.max(m, dist(p, q)); }
  return m;
}

function gateClip(sum, job) {
  const cls = job.class || 'locomotion';
  const loop = job.loop === true || (job.loop === undefined && cls === 'locomotion');
  const fps = sum.fps || 30;
  // raw = the clip as the vendor made it (root motion intact), restricted to the trimmed window when clip.py cut one; st = after trim/blend/strip
  const st = sum.stripped || sum.raw;
  const raw = sum.trim ? sum.raw.filter((fr) => fr.f >= sum.trim[0] && fr.f <= sum.trim[1]) : sum.raw;
  const checks = [];
  const res = { clip: sum.clip, class: cls, fps, frames: st.length, seconds: r3(st.length / fps), profile: sum.profile, checks };

  // loop closure
  if (loop) {
    const err = poseErr(st[0], st[st.length - 1]);
    // best cut: any start in the first half, any end at least min_loop_frames later, smallest pose error (what --trim should take)
    let best = null;
    for (let i = 0; i < st.length / 2; i++) for (let j = i + T.min_loop_frames; j < st.length; j++) {
      const e = poseErr(st[i], st[j]); if (!best || e < best.err) best = { first_frame: st[i].f, last_frame: st[j].f, err: e };
    }
    const status = err <= T.loop_pass_m ? 'PASS' : err <= T.loop_warn_m ? 'WARN' : 'FAIL';
    checks.push({ check: 'loop_closure', status, err_m: r3(err), best_cut: best ? { first_frame: best.first_frame, last_frame: best.last_frame, err_m: r3(best.err) } : undefined,
      detail: `first vs last pose ${r3(err)} m (pass <= ${T.loop_pass_m}, warn <= ${T.loop_warn_m})` + (best ? `; best cut: frames ${best.first_frame}..${best.last_frame} err ${r3(best.err)} m` : '') });
    res.loop_err_m = r3(err); res.best_cut = best;
  }

  // foot contact: ice level = lowest foot/toe over the clip; float = per-frame lower-foot height above it
  const feetZ = (fr) => [fr.foot_l, fr.toe_l, fr.foot_r, fr.toe_r].filter(Boolean).map((p) => p[2]);
  const ice = Math.min(...raw.flatMap(feetZ));
  let floatMax = 0, floatFrame = null;
  for (const fr of raw) { const z = Math.min(...feetZ(fr)) - ice; if (z > floatMax) { floatMax = z; floatFrame = fr.f; } }
  const floatApplies = cls === 'locomotion' || cls === 'pose';
  checks.push({ check: 'foot_float', status: !floatApplies ? 'INFO' : floatMax <= T.float_max_m ? 'PASS' : floatMax <= T.float_warn_m ? 'WARN' : 'FAIL', max_m: r3(floatMax), frame: floatFrame,
    detail: `lower foot at most ${r3(floatMax)} m above the ice (frame ${floatFrame}; pass <= ${T.float_max_m}, warn <= ${T.float_warn_m}, for locomotion and pose)` });

  // slide: planted-foot world speed on the raw clip (with its root motion), per foot
  const speeds = [];
  for (const side of ['l', 'r']) {
    for (let i = 1; i < raw.length; i++) {
      const a = raw[i - 1], b = raw[i];
      const k = `toe_${side}`, kk = `foot_${side}`;
      const pa = a[k] || a[kk], pb = b[k] || b[kk];
      if (!pa || !pb) continue;
      const planted = Math.min(pa[2], pb[2]) - ice <= T.contact_m;
      if (planted) speeds.push(hdist(pa, pb) * fps);
    }
  }
  speeds.sort((a, b) => a - b);
  const p90 = speeds.length ? speeds[Math.floor(speeds.length * 0.9)] : 0;
  const mean = speeds.length ? speeds.reduce((s, x) => s + x, 0) / speeds.length : 0;
  const travel = sum.root_travel_before_m || sum.root_travel_after_m || [0, 0, 0];
  const implied = Math.hypot(travel[0], travel[1]) / (raw.length / fps);
  if (cls === 'locomotion') {
    checks.push({ check: 'glide', status: implied <= T.glide_max_mps ? 'PASS' : 'FAIL', implied_mps: r3(implied), planted_p90_mps: r3(p90), planted_mean_mps: r3(mean), planted_samples: speeds.length,
      detail: `root travels ${r3(implied)} m/s in the raw clip (the sim gives up to ${T.glide_max_mps}); planted feet glide ${r3(mean)} m/s mean, ${r3(p90)} p90 over ${speeds.length} samples` });
  } else {
    checks.push({ check: 'foot_slide', status: p90 <= T.slide_max_mps ? 'PASS' : 'WARN', planted_p90_mps: r3(p90), planted_mean_mps: r3(mean), planted_samples: speeds.length,
      detail: `planted feet move ${r3(p90)} m/s (p90) in the raw clip; limit ${T.slide_max_mps} for an in-place clip` });
  }

  // root motion lint on the stripped clip
  const p0 = st[0].pelvis, pN = st[st.length - 1].pelvis;
  let hmax = 0;
  for (const fr of st) if (fr.pelvis) hmax = Math.max(hmax, hdist(fr.pelvis, p0));
  // a clip with a real root bone (Manny) is judged on the root: the summary says root_locked when force_root_lock holds it at the origin, and the pelvis may sway
  if (sum.root_locked) {
    checks.push({ check: 'root_motion', status: 'PASS', max_h_m: 0, pelvis_sway_m: r3(hmax), before_m: travel ? travel.map(r3) : undefined,
      detail: `root locked at the origin (force_root_lock); the root track travels ${JSON.stringify(travel.map(r3))} and is never applied; the pelvis sways ${r3(hmax)} m within the clip` });
  } else {
    checks.push({ check: 'root_motion', status: hmax <= T.root_max_m ? 'PASS' : 'FAIL', max_h_m: r3(hmax), before_m: travel ? travel.map(r3) : undefined,
      detail: `pelvis horizontal excursion after the strip ${r3(hmax)} m (limit ${T.root_max_m}); travel before the strip ${JSON.stringify(travel.map(r3))}` });
  }

  // contact window for attack clips: peak hand speed as the suggestion
  if (cls === 'attack') {
    let peak = { f: null, v: 0 };
    for (let i = 1; i < st.length; i++) {
      for (const k of ['hand_l', 'hand_r']) {
        const p = rel(st[i - 1], k), q = rel(st[i], k);
        if (p && q) { const v = dist(p, q) * fps; if (v > peak.v) peak = { f: st[i].f, v }; }
      }
    }
    const w = job.contact_window;
    const ok = Array.isArray(w) && w.length === 2 && w[0] >= st[0].f && w[1] <= st[st.length - 1].f && w[0] <= w[1];
    const inside = ok && peak.f >= w[0] && peak.f <= w[1];
    checks.push({ check: 'contact_window', status: !w ? 'FAIL' : ok ? (inside ? 'PASS' : 'WARN') : 'FAIL', window: w, peak_hand_frame: peak.f, peak_hand_mps: r3(peak.v),
      detail: !w ? 'no contact_window written next to the clip' : `window ${w[0]}..${w[1]} ${ok ? 'inside the clip' : 'outside the clip'}; peak hand speed ${r3(peak.v)} m/s at frame ${peak.f}${inside ? ' (inside the window)' : ''}` });
  }
  res.verdict = checks.some((c) => c.status === 'FAIL') ? 'FAIL' : checks.some((c) => c.status === 'WARN') ? 'WARN' : 'PASS';
  return res;
}

function main() {
  const o = args(process.argv);
  if (!o.files.length) { console.error('usage: gate.mjs <summary.json> [...] [--jobs jobs.json] [--class c] [--loop] [--contact a,b]'); console.log(JSON.stringify({ tool: 'thirdjam.p4motion-gate/1', verdict: 'error', error: 'no summary' })); process.exitCode = 2; return; }
  let jobs = [];
  if (o.jobs && existsSync(o.jobs)) jobs = JSON.parse(readFileSync(o.jobs, 'utf8')).jobs || [];
  const results = [];
  for (const f of o.files) {
    let sum;
    try { sum = JSON.parse(readFileSync(f, 'utf8')); } catch (e) { console.log(`FAIL read ${f}: ${e.message}`); results.push({ clip: basename(f), verdict: 'FAIL', error: String(e.message) }); continue; }
    const job = { ...(jobs.find((j) => j.clip === sum.clip) || {}) };
    if (o.class) job.class = o.class;
    if (o.loop) job.loop = true;
    if (o.noloop) job.loop = false;
    if (o.contact) job.contact_window = o.contact.split(',').map(Number);
    const r = gateClip(sum, job);
    for (const c of r.checks) console.log(`${c.status} ${r.clip} ${c.check}: ${c.detail}`);
    console.log(`${r.verdict} ${r.clip} (${r.frames} frames @ ${r.fps}, ${r.seconds} s, ${r.class})`);
    results.push(r);
  }
  const verdict = results.some((r) => r.verdict === 'FAIL') ? 'fail' : 'pass';
  console.log(JSON.stringify({ tool: 'thirdjam.p4motion-gate/1', verdict, thresholds: T, clips: results }));
  process.exitCode = verdict === 'pass' ? 0 : 1;
}
main();
