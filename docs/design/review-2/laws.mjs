// Lane A148 (track F0, REVIEW-2): the four laws of docs/VISION.md read out of a traced match log (-trace=1 of the tournament commandlet with -player=).
// node docs/design/review-2/laws.mjs <log> [goalX=41]        (DBG=1 prints the first knockdowns found.) It measures; it changes nothing.
//
// What the TRACE line carries: every skater's position (0.1 m), facing, move input, the carrier of record, the last toucher, the puck's speed. It carries
// no contact event, no stumble, no sprint flag. So:
//   law 1  A KNOCKDOWN is read from the inputs: the brain sends a downed bot no input (JamAICore.cpp:315) and the playtester sends none for the downed human
//          (JamPlaytester.cpp:232), so a run of 55 to 152 frames of zero move input in play that begins with an opposing skater within 2.2 m is a
//          knockdown and the run's length is the frames down. Closing speed = (attacker's velocity - victim's) along the line between them, from the
//          positions of the 6 frames before (about +-1 m/s); the attacker is the opposing skater within 2.2 m that closes fastest. Checked against the
//          playtest line (knockdowns_all, knockdowns). Bumps and stumbles are not in the trace: the contact tests' census has them.
//   law 2  A SHOT is a jump of the puck's speed by more than 5 m/s to 15.5 m/s or more (a tap leaves at 18, a charged shot faster; a pass at 14) with a
//          last toucher. Where from: the shooter's distance to the middle of the net it attacks and the angle off the rink's long axis. At the net: the
//          puck's line over the next 4 frames crosses the goal line within 5 m of the net's middle. A goal: play freezes within 150 frames with no other
//          toucher between. Zones: slot (15 m or nearer, 40 degrees or less), far (beyond 15 m, 40 degrees or less), wide (40 to 65), corner (over 65
//          degrees, or from behind the goal line).
//   law 3  THE HUMAN PAST A DEFENDER: while the human is the carrier of record, an opponent "in front" (ahead of him towards the net he attacks by 0.5 m
//          or more, within 6 m, within 4 m sideways) that he then leaves 1 m or more behind while still carrying, having come within 4 m of it, is
//          beaten. By what: a stop (his speed fell under 3.5 m/s inside the last 40 frames after being over 6), a cut (his direction of travel turned
//          50 degrees or more inside those frames), else straight; "down" when the defender lay on the ice. An opponent in front and within 3.5 m when the
//          carry ends: robbed (an opponent touches next), hit (the puck knocked loose), moved (he passed or shot).
//   law 4  THE CARRIER'S PACE: his speed over 20 frames, in three bands (under 5, 5 to 10.5, over 10.5 m/s: the carrier's cap is 0.92 x 11 = 10.1 and
//          10.9 sprinting, so the top band is a sprint or speed carried into the carry). Carries lost per carried minute in each band, with the share
//          of those frames that had an opponent inside 3 m. The sprint itself is in the playtest line (carry_sprint_frames).
import { createReadStream } from 'node:fs';
import { createInterface } from 'node:readline';
const file = process.argv[2];
const GOAL_X = Number(process.argv[3] || 41);
const RE = /(\d)([HPCSB])\((-?[\d.]+),(-?[\d.]+)\)>(-?\d+)\[(-?[\d.]+),(-?[\d.]+)( SHOOT)?\]/g;
function parse(line) {
  const i = line.indexOf('TRACE f='); if (i < 0) return null;
  const s = line.slice(i);
  const pm = /puck\((-?[\d.]+),(-?[\d.]+) v([\d.]+)\)/.exec(s); if (!pm) return null;
  const sk = []; let m; RE.lastIndex = pm.index;
  while ((m = RE.exec(s))) sk.push({ team: +m[1], role: m[2], x: +m[3], y: +m[4], face: +m[5], mx: +m[6], my: +m[7] });
  const car = /car=(-?\d+)/.exec(s), lt = /lt=(-?\d+)/.exec(s), fo = /fo=(\d)/.exec(s);
  return { f: +/f=(\d+)/.exec(s)[1], frozen: s.includes(' frozen '), px: +pm[1], py: +pm[2], pv: +pm[3], sk, car: car ? +car[1] : -1, lt: lt ? +lt[1] : -1, fo: fo ? fo[1] === '1' : false };
}
const q = (a, p) => { if (!a.length) return null; const s = [...a].sort((x, y) => x - y); return s[Math.min(s.length - 1, Math.floor(p * s.length))]; };
const r1 = (x) => (x == null ? null : Math.round(x * 10) / 10);
const HN = 64;
const B3 = () => ({ slow: 0, skate: 0, fast: 0 });
function fresh() { return { hist: [], human: -1, team: [], sg: 1, frames: 0, zero: [], downs: [], shots: [], pend: [], carry: null, enc: [], pace: { frames: B3(), near: B3(), lost: B3() }, carries: 0, prev: null, oppPace: { frames: B3(), near: B3(), lost: B3() }, oppCarry: null }; }
const at = (M, back) => M.hist[M.hist.length - 1 - back];
function vel(M, i, backNew, backOld) { const A = at(M, backOld), B = at(M, backNew); if (!A || !B) return null; const dt = (backOld - backNew) / 60; return { x: (B.sk[i].x - A.sk[i].x) / dt, y: (B.sk[i].y - A.sk[i].y) / dt }; }
const paceOf = (v) => (v > 10.5 ? 'fast' : v >= 5 ? 'skate' : 'slow');
const who = (M, i) => (i === M.human ? 'h' : M.team[i] === M.team[M.human] ? 'm' : 'o');
function flushZero(M, i, truncated) {
  const z = M.zero[i];
  if (z && z.info && z.n >= 55 && z.n <= 152) M.downs.push({ ...z.info, frames: z.n, truncated });
  M.zero[i] = null;
}
function endCarry(M, how) {
  const C = M.carry; if (!C) return; M.carry = null;
  if (C.frames < 20) return;
  M.carries++;
  if (how === 'lost' || how === 'knocked') M.pace.lost[C.lastPace]++;
  for (const o of Object.keys(C.st)) {
    const S = C.st[o];
    if (S.state === 'front' && S.lastDist < 3.5) M.enc.push({ out: how === 'lost' ? 'robbed' : how === 'knocked' ? 'hit' : (how === 'pass' || how === 'release') ? 'moved' : 'other' });
  }
}
function endOppCarry(M, how) { const C = M.oppCarry; if (!C) return; M.oppCarry = null; if (C.frames < 20) return; if (how === 'lost' || how === 'knocked') M.oppPace.lost[C.lastPace]++; }
function step(M, T) {
  if (M.human < 0) { M.human = T.sk.findIndex((k) => k.role === 'H'); M.team = T.sk.map((k) => k.team); M.sg = M.team[M.human] === 0 ? 1 : -1; }
  const n = T.sk.length;
  if (T.frozen || T.fo) {
    if (T.frozen) for (const s of M.pend) if (T.f - s.f <= 150) s.goal = true;
    M.pend = [];
    for (let i = 0; i < n; i++) flushZero(M, i, true);
    endCarry(M, 'stop'); endOppCarry(M, 'stop'); M.hist = []; M.prev = null; return;
  }
  M.frames++;
  M.hist.push(T); if (M.hist.length > HN) M.hist.shift();
  // ---- law 1
  for (let i = 0; i < n; i++) {
    const k = T.sk[i];
    if (k.mx === 0 && k.my === 0) {
      if (!M.zero[i]) {
        let info = null;
        // The human's input stops a reaction later than the hit (the playtester's delay: 12, 6 or 3 frames, then its ramp): look back up to 24 frames
        // for the frame an opponent closed on him fastest. A bot's input stops the frame after the hit: look back 2.
        const maxBack = Math.min(i === M.human ? 24 : 2, M.hist.length - 8);
        let best = -1, bc = -99, bd = 0, bb = 1;
        for (let b = 1; b <= maxBack; b++) {
          const P = at(M, b);
          for (let o = 0; o < n; o++) if (M.team[o] !== M.team[i]) {
            const d = Math.hypot(P.sk[o].x - P.sk[i].x, P.sk[o].y - P.sk[i].y); if (d > 2.2) continue;
            const va = vel(M, o, b, b + 6), vv = vel(M, i, b, b + 6), dx = (P.sk[i].x - P.sk[o].x) / Math.max(1e-6, d), dy = (P.sk[i].y - P.sk[o].y) / Math.max(1e-6, d);
            const c = (va.x - vv.x) * dx + (va.y - vv.y) * dy; if (c > bc) { bc = c; best = o; bd = d; bb = b; }
          }
        }
        if (best >= 0) {
          const P = at(M, bb), va = vel(M, best, bb, bb + 6), vv = vel(M, i, bb, bb + 6), sa = Math.hypot(va.x, va.y), sv = Math.hypot(vv.x, vv.y);
          info = { f: T.f, vic: who(M, i), att: who(M, best), closing: bc, attSpeed: sa, vicSpeed: sv, cos: sa > 1 && sv > 1 ? (va.x * vv.x + va.y * vv.y) / (sa * sv) : 0, dist: bd, vicCarried: P.car === i || P.lt === i, pcar: P.car, plt: P.lt, i };
        }
        M.zero[i] = { n: 0, info };
      }
      M.zero[i].n++;
    } else if (M.zero[i]) flushZero(M, i, false);
  }
  // ---- law 2
  const P = M.prev;
  if (P && T.lt >= 0 && T.pv - P.pv > 5 && T.pv >= 15.5) {
    const w = T.lt, s = M.team[w] === 0 ? 1 : -1;
    const X = s * P.sk[w].x, Y = Math.abs(P.sk[w].y), d = Math.hypot(GOAL_X - X, Y), ang = Math.atan2(Y, GOAL_X - X) * 180 / Math.PI;
    if (Math.hypot(P.sk[w].x - P.px, P.sk[w].y - P.py) < 3.0) {
      const sh = { f: T.f, who: who(M, w), lt: w, d, ang, v: T.pv, zone: ang > 65 ? 'corner' : ang > 40 ? 'wide' : d <= 15 ? 'slot' : 'far', goal: false, atNet: null, p0x: P.px, p0y: P.py, gx: s * GOAL_X };
      M.shots.push(sh); M.pend.push(sh);
    }
  }
  M.pend = M.pend.filter((s) => {
    if (T.lt !== s.lt) return false;
    if (s.atNet === null && T.f - s.f >= 4) { const dx = T.px - s.p0x, dy = T.py - s.p0y; s.atNet = dx * s.gx > 0 && Math.abs(s.p0y + dy * ((s.gx - s.p0x) / dx)) < 5.0; }
    return T.f - s.f <= 150;
  });
  // ---- law 3 and 4: the human's carry
  const h = M.human, H = T.sk[h];
  if (T.car === h) {
    if (!M.carry) M.carry = { frames: 0, st: {}, gap: 0, lastPace: 'slow' };
    const C = M.carry; C.frames++; C.gap = 0;
    const v = vel(M, h, 0, 20); const pace = paceOf(v ? Math.hypot(v.x, v.y) : 0); C.lastPace = pace;
    M.pace.frames[pace]++;
    let nearAny = false;
    for (let o = 0; o < n; o++) {
      if (M.team[o] === M.team[h]) continue;
      const O = T.sk[o]; const ax = M.sg * (O.x - H.x), ay = O.y - H.y, d = Math.hypot(ax, ay);
      if (d < 3) nearAny = true;
      let S = C.st[o]; if (!S) S = C.st[o] = { state: 'none', minDist: 99, lastDist: 99 };
      S.lastDist = d;
      if (S.state === 'none') { if (ax > 0.5 && d < 6 && Math.abs(ay) < 4) { S.state = 'front'; S.minDist = d; } }
      else if (S.state === 'front') {
        S.minDist = Math.min(S.minDist, d);
        if (d > 9) S.state = 'none';
        else if (ax < -1.0 && S.minDist < 4) {
          let how = 'straight';
          if (M.zero[o] && M.zero[o].n >= 3) how = 'down';
          else if (M.hist.length >= 47) {
            let vmin = 99, vmaxBefore = 0;
            for (let b = 0; b <= 34; b += 2) { const w = vel(M, h, b, b + 6); vmin = Math.min(vmin, Math.hypot(w.x, w.y)); }
            for (let b = 20; b <= 40; b += 2) { const w = vel(M, h, b, b + 6); vmaxBefore = Math.max(vmaxBefore, Math.hypot(w.x, w.y)); }
            const v0 = vel(M, h, 34, 46), v1 = vel(M, h, 0, 8), a0 = Math.hypot(v0.x, v0.y), a1 = Math.hypot(v1.x, v1.y);
            const turn = a0 > 2 && a1 > 2 ? Math.acos(Math.max(-1, Math.min(1, (v0.x * v1.x + v0.y * v1.y) / (a0 * a1)))) * 180 / Math.PI : 0;
            if (vmin < 3.5 && vmaxBefore > 6) how = 'stop'; else if (turn >= 50) how = 'cut';
          }
          M.enc.push({ out: 'beaten', how, minDist: S.minDist });
          S.state = 'done';
        }
      }
    }
    if (nearAny) M.pace.near[pace]++;
  } else if (M.carry) {
    const C = M.carry;
    if (T.car >= 0) endCarry(M, M.team[T.car] === M.team[h] ? 'pass' : 'lost');
    else { C.gap++; if (T.lt !== h && T.lt >= 0) endCarry(M, M.team[T.lt] === M.team[h] ? 'pass' : 'lost'); else if (T.lt < 0) endCarry(M, 'knocked'); else if (T.pv > 14) endCarry(M, 'release'); else if (C.gap > 30) endCarry(M, 'loose'); }
  }
  // the opponents' carrier
  const oc = T.car >= 0 && M.team[T.car] !== M.team[h] ? T.car : -1;
  if (oc >= 0) {
    if (M.oppCarry && M.oppCarry.who !== oc) endOppCarry(M, 'pass');
    if (!M.oppCarry) M.oppCarry = { who: oc, frames: 0, gap: 0, lastPace: 'slow' };
    const C = M.oppCarry; C.frames++; C.gap = 0;
    const v = vel(M, oc, 0, 20); const pace = paceOf(v ? Math.hypot(v.x, v.y) : 0); C.lastPace = pace; M.oppPace.frames[pace]++;
    let near = false; for (let o = 0; o < n; o++) if (M.team[o] === M.team[h] && Math.hypot(T.sk[o].x - T.sk[oc].x, T.sk[o].y - T.sk[oc].y) < 3) near = true;
    if (near) M.oppPace.near[pace]++;
  } else if (M.oppCarry) {
    const C = M.oppCarry; const w = C.who;
    if (T.car >= 0) endOppCarry(M, 'lost');
    else { C.gap++; if (T.lt !== w && T.lt >= 0) endOppCarry(M, M.team[T.lt] === M.team[w] ? 'pass' : 'lost'); else if (T.lt < 0) endOppCarry(M, 'knocked'); else if (T.pv > 14) endOppCarry(M, 'release'); else if (C.gap > 30) endOppCarry(M, 'loose'); }
  }
  M.prev = T;
}
const rows = []; let M = null;
const rl = createInterface({ input: createReadStream(file) });
for await (const l of rl) {
  const j = l.indexOf('{"tool":"thirdjam.playtest/1"');
  if (j >= 0) { let pl = null; try { pl = JSON.parse(l.slice(j)); } catch {} if (M) { for (let i = 0; i < M.zero.length; i++) flushZero(M, i, true); endCarry(M, 'stop'); rows.push({ M, pl }); } M = null; continue; }
  if (!l.includes('TRACE f=')) continue;
  const T = parse(l); if (!T || !T.sk.length) continue;
  if (!M) M = fresh();
  step(M, T);
}
const N = rows.length, per = (x) => +(x / N).toFixed(2);
const sumPl = (k) => rows.reduce((s, r) => s + (r.pl ? (r.pl[k] || 0) : 0), 0);
const playMin = rows.reduce((s, r) => s + r.M.frames, 0) / 3600;
// law 1
const downs = rows.flatMap((r) => r.M.downs);
if (process.env.DBG) for (const d of downs.slice(0, 30)) console.log(`f=${d.f} vic=${d.vic}#${d.i} att=${d.att} closing=${d.closing.toFixed(1)} attSpeed=${d.attSpeed.toFixed(1)} vicSpeed=${d.vicSpeed.toFixed(1)} cos=${d.cos.toFixed(2)} dist=${d.dist.toFixed(1)} frames=${d.frames} car=${d.pcar} lt=${d.plt}`);
const bands = [[-99, 2], [2, 4], [4, 6], [6, 8], [8, 10], [10, 99]];
const byBand = (set) => bands.map(([a, b]) => { const s = set.filter((d) => d.closing >= a && d.closing < b); return { closing_mps: a < 0 ? 'under 2' : b > 90 ? '10 and up' : `${a} to ${b}`, n: s.length, frames_down_median: q(s.map((d) => d.frames), 0.5), frames_down_p90: q(s.map((d) => d.frames), 0.9) }; });
const full = downs.filter((d) => !d.truncated);
const view = (set) => { const sw = set.filter((d) => d.cos > 0.7 && d.attSpeed > 3 && d.vicSpeed > 3), ho = set.filter((d) => d.cos < -0.5 && d.attSpeed > 3 && d.vicSpeed > 3), st = set.filter((d) => d.vicSpeed <= 1.5);
  return { n: set.length, per_match: per(set.length), closing_mps: { p10: r1(q(set.map((d) => d.closing), 0.1)), median: r1(q(set.map((d) => d.closing), 0.5)), p90: r1(q(set.map((d) => d.closing), 0.9)) },
    frames_down: { p10: q(set.map((d) => d.frames), 0.1), median: q(set.map((d) => d.frames), 0.5), p90: q(set.map((d) => d.frames), 0.9) }, by_closing: byBand(set),
    same_way_both_over_3mps: { n: sw.length, pct: Math.round(100 * sw.length / Math.max(1, set.length)), closing_median: r1(q(sw.map((d) => d.closing), 0.5)), frames_down_median: q(sw.map((d) => d.frames), 0.5) },
    head_on_both_over_3mps: { n: ho.length, closing_median: r1(q(ho.map((d) => d.closing), 0.5)), frames_down_median: q(ho.map((d) => d.frames), 0.5) },
    victim_standing_under_1_5mps: { n: st.length, closing_median: r1(q(st.map((d) => d.closing), 0.5)), frames_down_median: q(st.map((d) => d.frames), 0.5) },
    victim_had_the_puck_pct: Math.round(100 * set.filter((d) => d.vicCarried).length / Math.max(1, set.length)) }; };
const humanDownFrames = downs.filter((d) => d.vic === 'h').reduce((s, d) => s + d.frames, 0);
const law1 = { human_on_the_ice_pct_of_play: +(100 * humanDownFrames / Math.max(1, playMin * 3600)).toFixed(1), human_knockdowns_per_min_trace: +(downs.filter((d) => d.vic === 'h').length / playMin).toFixed(2),
  check: { knockdowns_all_trace: per(downs.length), knockdowns_all_line: per(sumPl('knockdowns_all')), human_trace: per(downs.filter((d) => d.vic === 'h').length), human_line: per(sumPl('knockdowns')) },
  line: { human_stumbles: per(sumPl('stumbles')), human_checks_taken: per(sumPl('checks_taken')), human_checks_landed: per(sumPl('checks_landed')), board_hits: per(sumPl('board_hits')) },
  all: view(full), human_is_victim: view(full.filter((d) => d.vic === 'h')), human_is_attacker: view(full.filter((d) => d.att === 'h')), bot_on_bot: view(full.filter((d) => d.vic !== 'h' && d.att !== 'h')) };
// law 2
const shots = rows.flatMap((r) => r.M.shots);
const zone = (set) => { const o = {}; for (const z of ['slot', 'far', 'wide', 'corner']) { const s = set.filter((x) => x.zone === z); o[z] = { per_match: per(s.length), pct: Math.round(100 * s.length / Math.max(1, set.length)), goals: s.filter((x) => x.goal).length, goal_pct: +(100 * s.filter((x) => x.goal).length / Math.max(1, s.length)).toFixed(1), at_net_pct: Math.round(100 * s.filter((x) => x.atNet).length / Math.max(1, s.length)) }; } return o; };
const side = (w) => { const s = shots.filter((x) => x.who === w); return { per_match_trace: per(s.length), goals_trace: s.filter((x) => x.goal).length, goal_per_shot_pct: +(100 * s.filter((x) => x.goal).length / Math.max(1, s.length)).toFixed(1), at_net_pct: Math.round(100 * s.filter((x) => x.atNet).length / Math.max(1, s.length)), dist_m: { p10: r1(q(s.map((x) => x.d), 0.1)), median: r1(q(s.map((x) => x.d), 0.5)), p90: r1(q(s.map((x) => x.d), 0.9)) }, angle_deg: { median: r1(q(s.map((x) => x.ang), 0.5)), p90: r1(q(s.map((x) => x.ang), 0.9)) }, over_25m_pct: Math.round(100 * s.filter((x) => x.d > 25).length / Math.max(1, s.length)), by_zone: zone(s),
  by_distance: [[0, 10], [10, 15], [15, 25], [25, 41], [41, 999]].map(([a, b]) => { const z = s.filter((x) => x.d >= a && x.d < b); return { m: b > 900 ? 'own half (41 and up)' : `${a} to ${b}`, per_match: per(z.length), pct: Math.round(100 * z.length / Math.max(1, s.length)), at_net_pct: Math.round(100 * z.filter((x) => x.atNet).length / Math.max(1, z.length)), goals: z.filter((x) => x.goal).length }; }) }; };
const hf = rows.reduce((a, r) => { const x = r.pl && r.pl.hold_frames; if (Array.isArray(x)) { a[0] += x[0]; a[1] += x[1]; a[2] += x[2]; } return a; }, [0, 0, 0]);
const law2 = { opp: side('o'), mates: side('m'), human: side('h'), line: { shots_opp: per(sumPl('shots_opp')), shots_mates: per(sumPl('shots_mates')), shots_human: per(sumPl('shots')), goals_against: per(rows.reduce((s, r) => s + (r.pl ? r.pl.goals[1] : 0), 0)), goals_for: per(rows.reduce((s, r) => s + (r.pl ? r.pl.goals[0] : 0), 0)), goals_by_human: per(sumPl('goals_by_human')), passes_received: per(sumPl('passes_received')), passes: per(sumPl('passes')), idle_s: r1(sumPl('idle_frames') / 60 / N), idle_spells: per(sumPl('idle_spells')), max_no_touch_s_mean: r1(sumPl('max_no_touch') / 60 / N), matches_with_30s_dry: rows.filter((r) => r.pl && r.pl.max_no_touch >= 1800).length, hold_frames_mate_opp_none_per_match: hf.map((x) => Math.round(x / N)) } };
// law 3
const enc = rows.flatMap((r) => r.M.enc); const cnt = (f) => enc.filter(f).length;
const beaten = cnt((e) => e.out === 'beaten'), met = enc.length;
const law3 = { human_carries_per_match: per(rows.reduce((s, r) => s + r.M.carries, 0)), defenders_met_per_match: per(met), beaten_per_match: per(beaten), beaten_pct_of_met: Math.round(100 * beaten / Math.max(1, met)),
  beaten_by: { stop: per(cnt((e) => e.how === 'stop')), cut: per(cnt((e) => e.how === 'cut')), straight: per(cnt((e) => e.how === 'straight')), defender_down: per(cnt((e) => e.how === 'down')) },
  not_beaten: { robbed: per(cnt((e) => e.out === 'robbed')), hit_loose: per(cnt((e) => e.out === 'hit')), moved_the_puck: per(cnt((e) => e.out === 'moved')), other: per(cnt((e) => e.out === 'other')) },
  line: { cuts: per(sumPl('cuts')), rev_brakes: per(sumPl('rev_brakes')), dekes: per(sumPl('dekes')), breakaways: per(sumPl('breakaways')), carries_3s: per(sumPl('carries_3s')), bot_rev_brakes: per(sumPl('bot_rev_brakes')), pokes: per(sumPl('pokes')), pokes_clean: per(sumPl('pokes_clean')), lifts: per(sumPl('lifts')) } };
// law 4
const agg = (get) => { const o = {}; for (const p of ['slow', 'skate', 'fast']) { const fr = rows.reduce((s, r) => s + get(r.M).frames[p], 0), near = rows.reduce((s, r) => s + get(r.M).near[p], 0), lost = rows.reduce((s, r) => s + get(r.M).lost[p], 0); o[p] = { carried_s_per_match: r1(fr / 60 / N), opp_within_3m_pct: Math.round(100 * near / Math.max(1, fr)), carries_lost: lost, lost_per_carried_minute: +(lost / Math.max(1e-9, fr / 3600)).toFixed(1) }; } return o; };
const law4 = { human: agg((m) => m.pace), opponents: agg((m) => m.oppPace), line: { carry_sprint_s: r1(sumPl('carry_sprint_frames') / 60 / N), bot_carry_sprint_s: r1(sumPl('bot_carry_sprint_frames') / 60 / N), sprint_s: r1(sumPl('sprint_frames') / 60 / N), bot_sprint_s: r1(sumPl('bot_sprint_frames') / 60 / N) } };
console.log(JSON.stringify({ log: file, matches: N, play_min_per_match: +(playMin / N).toFixed(2), win_pct: Math.round(100 * rows.filter((r) => r.pl && r.pl.result === 'win').length / N), law1, law2, law3, law4 }));
