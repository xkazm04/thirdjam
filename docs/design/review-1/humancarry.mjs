// Lane A100 (track F0, REVIEW-1): what carries look like in a traced match log (TRACE lines + the playtest line per match), the human against the bots.
// node docs/design/review-1/humancarry.mjs <log>      (a log saved from the tournament commandlet with -trace=1; it measures, it changes nothing)
// A carry: the brain's carrier of record (the TRACE line's car=) is the same skater for 20 frames or more, in play (not frozen, not the faceoff countdown).
//   who: h the human, m a team-mate, o an opponent. It ends: lost (an opponent touches next), knocked (the last toucher is cleared: a hit knocked it loose),
//   pass (a team-mate touches next), release (the puck leaves faster than 14 m/s: a shot, a pass in flight, a dump), loose (off the blade and untouched for
//   30 frames), stop (the whistle). pressured: an opponent came inside 2.5 m during it; a crowd start: inside the first 12 frames.
// puck_pct: who is the carrier of record, share of frames in play. speed_mps: displacement over 30 frames (positions print to 0.1 m), sampled every 10 frames.
// human_after_mate_pass: the human's carries whose previous toucher was a team-mate; held_Ns_pct: of those, still carried N seconds later.
import { createReadStream } from 'node:fs';
import { createInterface } from 'node:readline';
import { parseTrace } from '../../../tools/playtest/shape.mjs';
const file = process.argv[2];
const rl = createInterface({ input: createReadStream(file) });
const pct = (a, p) => { if (!a.length) return 0; const s = [...a].sort((x, y) => x - y); return s[Math.min(s.length - 1, Math.floor(p * s.length))]; };
const W = 30;   // frames of the speed window (positions print to 0.1 m)
let M = null; const rows = [];
function fresh() { return { curLt: -1, prevLt: -1, frames: 0, human: -1, team: [], hist: [], spell: null, spells: [], speed: { h: [], m: [], o: [] }, nearPuck: 0, car: { h: 0, m: 0, o: 0, loose: 0 }, stillRun: 0, still: 0, stillSpells: 0 }; }
const cls = (M, i) => (i === M.human ? 'h' : M.team[i] === M.team[M.human] ? 'm' : 'o');
function endSpell(M, how) { if (M.spell) { if (M.spell.frames >= 20) M.spells.push({ who: cls(M, M.spell.who), frames: M.spell.frames, how, toPressure: M.spell.toPressure, dist: M.spell.dist, from: M.spell.from }); M.spell = null; } }
for await (const l of rl) {
  const j = l.indexOf('{"tool":"thirdjam.playtest/1"');
  if (j >= 0) { let pl = null; try { pl = JSON.parse(l.slice(j)); } catch {} if (M) { endSpell(M, 'stop'); rows.push({ M, pl }); } M = null; continue; }
  if (!l.includes('TRACE f=')) continue;
  const T = parseTrace(l); if (!T || !T.sk.length) continue;
  if (!M) M = fresh();
  if (M.human < 0) { M.human = T.sk.findIndex((k) => k.role === 'H'); M.team = T.sk.map((k) => k.team); }
  if (T.frozen || T.fo) { endSpell(M, 'stop'); M.hist = []; continue; }
  M.frames++;
  const H = T.sk[M.human];
  if (Math.hypot(H.x - T.px, H.y - T.py) < 6) M.nearPuck++;
  M.hist.push(T.sk); if (M.hist.length > W + 1) M.hist.shift();
  if (M.hist.length === W + 1 && M.frames % 10 === 0) {
    const A = M.hist[0];
    for (let i = 0; i < T.sk.length; i++) M.speed[cls(M, i)].push(Math.hypot(T.sk[i].x - A[i].x, T.sk[i].y - A[i].y) * 60 / W);
  }
  if (M.hist.length > 20) { const A = M.hist[M.hist.length - 21][M.human]; const still = Math.hypot(H.x - A.x, H.y - A.y) < 0.25; if (still) { M.still++; M.stillRun++; } else { if (M.stillRun >= 30) M.stillSpells++; M.stillRun = 0; } }
  if (T.car >= 0) M.car[cls(M, T.car)]++; else M.car.loose++;
  if (T.lt >= 0 && T.lt !== M.curLt) { M.prevLt = M.curLt; M.curLt = T.lt; }
  if (T.car >= 0) {
    if (M.spell && M.spell.who !== T.car) endSpell(M, M.team[T.car] === M.team[M.spell.who] ? 'pass' : 'lost');
    if (!M.spell) M.spell = { who: T.car, frames: 0, toPressure: -1, x0: T.sk[T.car].x, y0: T.sk[T.car].y, dist: 0, from: (M.prevLt >= 0 && M.prevLt !== T.car) ? cls(M, M.prevLt) + (M.team[M.prevLt] === M.team[T.car] ? "-mate" : "-opp") : "none" };
    M.spell.frames++; M.spell.gap = 0;
    const C = T.sk[T.car];
    M.spell.dist = Math.hypot(C.x - M.spell.x0, C.y - M.spell.y0);
    if (M.spell.toPressure < 0) for (let i = 0; i < T.sk.length; i++) if (M.team[i] !== M.team[T.car] && Math.hypot(T.sk[i].x - C.x, T.sk[i].y - C.y) < 2.5) { M.spell.toPressure = M.spell.frames; break; }
  } else if (M.spell) {
    M.spell.gap = (M.spell.gap || 0) + 1;
    if (T.lt !== M.spell.who && T.lt >= 0) endSpell(M, M.team[T.lt] === M.team[M.spell.who] ? 'pass' : 'lost');
    else if (T.lt < 0) endSpell(M, 'knocked');            // a hit that knocks the puck loose clears the last toucher
    else if (T.pv > 14) endSpell(M, 'release');            // the carrier let it go fast: a shot, a pass in flight or a dump
    else if (M.spell.gap > 30) endSpell(M, 'loose');       // it drifted off the blade and nobody touched it for half a second
  }
}
const all = rows.flatMap((r) => r.M.spells);
const fr = rows.reduce((s, r) => s + r.M.frames, 0);
const sumCar = (k) => rows.reduce((s, r) => s + r.M.car[k], 0);
function carries(k) {
  const a = all.filter((s) => s.who === k); const by = (h) => +(100 * a.filter((s) => s.how === h).length / Math.max(1, a.length)).toFixed(0);
  const tp = a.filter((s) => s.toPressure >= 0).map((s) => s.toPressure);
  return { n_per_match: +(a.length / rows.length).toFixed(1), s_median: +(pct(a.map((s) => s.frames), 0.5) / 60).toFixed(2), s_p90: +(pct(a.map((s) => s.frames), 0.9) / 60).toFixed(2), over_3s_per_match: +(a.filter((s) => s.frames >= 180).length / rows.length).toFixed(1),
    m_median: +pct(a.map((s) => s.dist), 0.5).toFixed(1), lost: by('lost'), knocked: by('knocked'), pass: by('pass'), release: by('release'), loose: by('loose'), stop: by('stop'),
    pressured_pct: +(100 * tp.length / Math.max(1, a.length)).toFixed(0), s_to_pressure_median: +(pct(tp, 0.5) / 60).toFixed(2) };
}
const sp = (k) => ({ p50: +pct(rows.flatMap((r) => r.M.speed[k]), 0.5).toFixed(1), p90: +pct(rows.flatMap((r) => r.M.speed[k]), 0.9).toFixed(1), p99: +pct(rows.flatMap((r) => r.M.speed[k]), 0.99).toFixed(1) });
const pl = (k) => +(rows.reduce((s, r) => s + (r.pl ? r.pl[k] : 0), 0) / rows.length).toFixed(1);
const hp = all.filter((x) => x.who === 'h' && x.from.endsWith('-mate')); const ho = all.filter((x) => x.who === 'h' && !x.from.endsWith('-mate'));
const surv = (a, fr) => +(100 * a.filter((x) => x.frames >= fr).length / Math.max(1, a.length)).toFixed(0);
const pcts = (a) => ({ n_per_match: +(a.length / rows.length).toFixed(1), s_median: +(pct(a.map((x) => x.frames), 0.5) / 60).toFixed(2), s_p75: +(pct(a.map((x) => x.frames), 0.75) / 60).toFixed(2), held_2s_pct: surv(a, 120), lost_or_knocked_pct: +(100 * a.filter((x) => x.how === 'lost' || x.how === 'knocked').length / Math.max(1, a.length)).toFixed(0), released_or_passed_pct: +(100 * a.filter((x) => x.how === 'release' || x.how === 'pass').length / Math.max(1, a.length)).toFixed(0) });
const hAll = all.filter((x) => x.who === 'h'); const oAll = all.filter((x) => x.who === 'o');
const crowd = (x) => x.toPressure >= 0 && x.toPressure <= 12;
console.log(JSON.stringify({ log: file, human_free_start: pcts(hAll.filter((x) => !crowd(x))), human_crowd_start: pcts(hAll.filter(crowd)), opp_free_start: pcts(oAll.filter((x) => !crowd(x))), opp_crowd_start: pcts(oAll.filter(crowd)), human_after_mate_pass: { n_per_match: +(hp.length / rows.length).toFixed(1), s_median: +(pct(hp.map((x) => x.frames), 0.5) / 60).toFixed(2), held_1s_pct: surv(hp, 60), held_2s_pct: surv(hp, 120), lost_pct: +(100 * hp.filter((x) => x.how === 'lost' || x.how === 'knocked').length / Math.max(1, hp.length)).toFixed(0) }, human_won_himself: { n_per_match: +(ho.length / rows.length).toFixed(1), s_median: +(pct(ho.map((x) => x.frames), 0.5) / 60).toFixed(2), held_1s_pct: surv(ho, 60), held_2s_pct: surv(ho, 120) }, matches: rows.length, play_min_per_match: +(fr / 3600 / rows.length).toFixed(2),
  puck_pct: { human: +(100 * sumCar('h') / fr).toFixed(1), mates: +(100 * sumCar('m') / fr).toFixed(1), opp: +(100 * sumCar('o') / fr).toFixed(1), loose: +(100 * sumCar('loose') / fr).toFixed(1) },
  human_within_6m_of_puck_pct: +(100 * rows.reduce((s, r) => s + r.M.nearPuck, 0) / fr).toFixed(1),
  human_carries: carries('h'), mate_carries: carries('m'), opp_carries: carries('o'),
  speed_mps: { human: sp('h'), mates: sp('m'), opp: sp('o') },
  human_still_pct: +(100 * rows.reduce((s, r) => s + r.M.still, 0) / fr).toFixed(1), human_still_spells_per_min: +(rows.reduce((s, r) => s + r.M.stillSpells, 0) / (fr / 3600)).toFixed(2),
  playtest: { win_pct: +(100 * rows.filter((r) => r.pl && r.pl.result === 'win').length / rows.length).toFixed(0), knockdowns: pl('knockdowns'), checks_taken: pl('checks_taken'), checks_landed: pl('checks_landed'), carries_3s: pl('carries_3s'), passes: pl('passes'), passes_received: pl('passes_received'), shots: pl('shots'), goals_by_human: pl('goals_by_human'), cuts: pl('cuts'), rev_brakes: pl('rev_brakes'), bot_rev_brakes: pl('bot_rev_brakes'), carry_sprint_frames: pl('carry_sprint_frames'), bot_carry_sprint_frames: pl('bot_carry_sprint_frames'), dekes: pl('dekes'), one_timers: pl('one_timers'), breakaways: pl('breakaways') } }));
