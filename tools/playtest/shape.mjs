#!/usr/bin/env node
// Lane A94 (R-0045 law 2): the shape of a match from its per-frame log. Runs the playtester through the tournament commandlet with -trace=1 (the brain's
// TRACE line per frame: puck, every skater's team, role letter and position, the carrier and the last toucher) and reads the lines as they stream, or reads
// a saved log (--log). One thirdjam.shape/1 JSON line per match and a summary line last. It measures; it changes nothing.
//
//   node tools/playtest/shape.mjs [--seeds 1-60] [--level pro] [--player regular] [--cartridge hockey.regular] [--workers 6] [--chunk 10]
//        [--tune "opp.hk_on=0"] [--max-frames 36000] [--uproject <path>] [--out <file.jsonl>] [--log <saved log>]
//
// Per match, over the frames in play (not frozen, not the faceoff countdown):
//   spread_m     mean distance between two teammates (every pair of each team, the human included)
//   pile_pct     share of frames with three or more skaters of one team within 6 m of the puck
//   bot_passes_min  completed bot passes per minute of play: the last toucher changes from a bot to a teammate (the human included as the receiver) and the
//                puck has travelled 4 m or more from where the passer last carried it (a scrum's touches between teammates are not passes)
//   carries      bot possessions of 20 frames or more of carrying (a bot carrier of record from the first frame it carries until another skater touches or play stops), ended by
//                pass (a teammate touches next), shot (the release flew at the net: its line crosses the goal line within 3 m of the post, or a goal
//                followed), lost (an opponent touches next); carry_s the mean length in seconds
//   goals        from the match's thirdjam.playtest/1 line; touches_per_min and faceoffs_without_touch of the human likewise
// Exit 0 ran, 1 a chunk failed, 2 could not run.
import { existsSync, createReadStream, writeFileSync, mkdirSync } from 'node:fs';
import { spawn, execFileSync } from 'node:child_process';
import { createInterface } from 'node:readline';
import { join, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import os from 'node:os';

const here = dirname(fileURLToPath(import.meta.url));
const repo = resolve(here, '..', '..');
function arg(name, dflt) { const i = process.argv.indexOf(`--${name}`); return i > 0 && i + 1 < process.argv.length ? process.argv[i + 1] : dflt; }

const GOAL_X = 41.0;          // hockey.regular 90 x 45: |x| of the goal lines (length / 2 - 4)
const PILE_M = 6.0;

// One match's running counters.
export function newMatch() {
  return { frames: 0, spreadSum: 0, pile: 0, passes: 0, carries: { pass: 0, shot: 0, lost: 0, stop: 0 }, carryFrames: 0, poss: null, prev: null,
    lastLt: -1, release: null, teamOf: [], human: -1, touches: 0, had: [] };
}

const RE_SK = /(\d)([HPCSB])\((-?[\d.]+),(-?[\d.]+)\)/g;
export function parseTrace(line) {
  const i = line.indexOf('TRACE f=');
  if (i < 0) return null;
  const s = line.slice(i);
  const f = Number(/f=(\d+)/.exec(s)[1]);
  const frozen = s.includes(' frozen ');
  const pm = /puck\((-?[\d.]+),(-?[\d.]+) v([\d.]+)\)/.exec(s);
  const sk = [];
  let m;
  RE_SK.lastIndex = pm ? pm.index + pm[0].length : 0;
  while ((m = RE_SK.exec(s))) { if (s[m.index - 1] === ' ') sk.push({ team: Number(m[1]), role: m[2], x: Number(m[3]), y: Number(m[4]) }); }
  const car = /car=(-?\d+)/.exec(s), lt = /lt=(-?\d+)/.exec(s), fo = /fo=(\d)/.exec(s);
  return { f, frozen, px: pm ? Number(pm[1]) : 0, py: pm ? Number(pm[2]) : 0, pv: pm ? Number(pm[3]) : 0, sk,
    car: car ? Number(car[1]) : -1, lt: lt ? Number(lt[1]) : -1, fo: fo ? fo[1] === '1' : false };
}

const MIN_CARRY = 20;         // frames of carrying before a possession counts as a carry (a scrum's touches do not)
const MIN_PASS_M = 4.0;       // a pass travels at least this far from where the passer last had the puck
function endPoss(M, how) {
  if (M.poss) { if (M.poss.frames >= MIN_CARRY) { M.carries[how]++; M.carryFrames += M.poss.frames; } else { M.touches++; } M.poss = null; }
}

// Did a release from (x0,y0) to (x1,y1) (one frame apart) fly at the net of the team that attacks towards sign(gx)?
function atNet(x0, y0, x1, y1, gx) {
  const dx = x1 - x0, dy = y1 - y0;
  if (dx * gx <= 0) return false;
  const t = (gx - x0) / dx;
  const y = y0 + dy * t;
  return Math.abs(y) < 2.0 + 3.0;
}

export function step(M, T) {
  if (!T || !T.sk.length) return;
  if (M.human < 0) { M.human = T.sk.findIndex((k) => k.role === 'H'); M.teamOf = T.sk.map((k) => k.team); }
  if (T.frozen) {
    // play stops: a possession running into a goal pause ended in a shot that went in (the release was seen) or a stop
    if (M.poss) endPoss(M, M.release && M.release.who === M.poss.who && M.release.shot ? 'shot' : 'stop');
    M.prev = null; M.release = null; M.lastLt = -1;
    return;
  }
  if (T.fo) { M.prev = null; M.lastLt = T.lt; return; }
  M.frames++;
  // spread and pile
  let sum = 0, n = 0;
  const near = [0, 0];
  for (let a = 0; a < T.sk.length; a++) {
    const A = T.sk[a];
    if (Math.hypot(A.x - T.px, A.y - T.py) < PILE_M) near[A.team]++;
    for (let b = a + 1; b < T.sk.length; b++) { const B = T.sk[b]; if (A.team === B.team) { sum += Math.hypot(A.x - B.x, A.y - B.y); n++; } }
  }
  M.spreadSum += n ? sum / n : 0;
  if (near[0] >= 3 || near[1] >= 3) M.pile++;
  // releases: the carrier of record lets the puck go (car i -> -1 with lt still i); keep the release's flight
  if (M.prev && M.prev.car >= 0 && M.prev.car !== M.human && T.car < 0 && T.lt === M.prev.car) {
    const who = M.prev.car;
    const gx = M.teamOf[who] === 0 ? GOAL_X : -GOAL_X;
    M.release = { who, shot: T.pv > 12 && atNet(M.prev.px, M.prev.py, T.px, T.py, gx) };
  }
  // possession of a bot
  if (T.car >= 0 && T.car !== M.human && (!M.poss || M.poss.who !== T.car)) {
    if (M.poss) endPoss(M, M.teamOf[T.car] === M.teamOf[M.poss.who] ? 'pass' : 'lost');
    M.poss = { who: T.car, frames: 0 };
    M.release = null;
  }
  if (M.poss && T.car === M.poss.who) M.poss.frames++;
  if (T.car >= 0) M.had[T.car] = [T.px, T.py];
  // the last toucher changes
  if (T.lt !== M.lastLt && T.lt >= 0) {
    const from = M.lastLt;
    if (from >= 0 && from !== M.human && M.teamOf[from] === M.teamOf[T.lt]) {
      const shotRel = M.release && M.release.who === from && M.release.shot;
      const h = M.had[from];
      if (!shotRel && h && Math.hypot(T.px - h[0], T.py - h[1]) >= MIN_PASS_M) M.passes++;
    }
    if (M.poss && T.lt !== M.poss.who) {
      const shotRel = M.release && M.release.who === M.poss.who && M.release.shot;
      endPoss(M, shotRel ? 'shot' : (M.teamOf[T.lt] === M.teamOf[M.poss.who] ? 'pass' : 'lost'));
    }
    M.lastLt = T.lt;
  }
  M.prev = T;
}

export function finish(M, pl) {
  const min = M.frames / 3600;
  const c = M.carries;
  const nc = c.pass + c.shot + c.lost;
  return { tool: 'thirdjam.shape/1', seed: pl ? pl.seed : undefined, play_s: Math.round(M.frames / 60), spread_m: +(M.spreadSum / Math.max(1, M.frames)).toFixed(2),
    pile_pct: +((100 * M.pile) / Math.max(1, M.frames)).toFixed(1), bot_passes: M.passes, bot_passes_min: +(M.passes / Math.max(1e-9, min)).toFixed(2),
    carries: nc, carry_pass: c.pass, carry_shot: c.shot, carry_lost: c.lost, carry_stop: c.stop, carry_s: +(M.carryFrames / 60 / Math.max(1, nc + c.stop)).toFixed(2),
    goals: pl ? pl.goals[0] + pl.goals[1] : undefined, result: pl ? pl.result : undefined, touches_per_min: pl ? pl.touches_per_min : undefined,
    faceoffs: pl ? pl.faceoffs : undefined, faceoffs_without_touch: pl ? pl.faceoffs_without_touch : undefined };
}

// Feeds lines; calls onMatch(row) at every thirdjam.playtest/1 line (the match's end).
export function makeReader(onMatch) {
  let M = newMatch();
  return (line) => {
    const j = line.indexOf('{"tool":"thirdjam.playtest/1"');
    if (j >= 0) {
      let pl = null; try { pl = JSON.parse(line.slice(j)); } catch { /* truncated */ }
      onMatch(finish(M, pl));
      M = newMatch();
      return;
    }
    if (line.includes('TRACE f=')) step(M, parseTrace(line));
  };
}

const avg = (a) => (a.length ? a.reduce((s, x) => s + x, 0) / a.length : 0);
export function summarize(rows) {
  const sum = (k) => rows.reduce((s, r) => s + (r[k] || 0), 0);
  const nc = sum('carries');
  const playMin = sum('play_s') / 60;
  return { matches: rows.length, spread_m: +avg(rows.map((r) => r.spread_m)).toFixed(2), pile_pct: +(rows.reduce((s, r) => s + r.pile_pct * r.play_s, 0) / Math.max(1, sum('play_s'))).toFixed(1),
    bot_passes_min: +(sum('bot_passes') / Math.max(1e-9, playMin)).toFixed(2), carries_per_match: +(nc / Math.max(1, rows.length)).toFixed(1),
    carry_pass_pct: +((100 * sum('carry_pass')) / Math.max(1, nc)).toFixed(1), carry_shot_pct: +((100 * sum('carry_shot')) / Math.max(1, nc)).toFixed(1),
    carry_lost_pct: +((100 * sum('carry_lost')) / Math.max(1, nc)).toFixed(1), carry_s: +avg(rows.map((r) => r.carry_s)).toFixed(2),
    goals: +avg(rows.map((r) => r.goals || 0)).toFixed(2), touches_per_min: +avg(rows.map((r) => r.touches_per_min || 0)).toFixed(2),
    faceoffs_without_touch: sum('faceoffs_without_touch'), faceoffs: sum('faceoffs'), play_s: +avg(rows.map((r) => r.play_s)).toFixed(0) };
}

function engineRoot() {
  for (const c of [process.env.UE_ROOT, 'C:/Program Files/Epic Games/UE_5.9']) if (c && existsSync(join(c, 'Engine', 'Binaries', 'Win64', 'UnrealEditor-Cmd.exe'))) return c;
  return null;
}
function freeGB() {
  try { return Number(execFileSync('powershell', ['-NoProfile', '-Command', '(Get-CimInstance Win32_OperatingSystem).FreePhysicalMemory'], { encoding: 'utf8' }).trim()) / 1048576; }
  catch { return os.freemem() / 2 ** 30; }
}

function runChunk(o, lo, hi, onMatch) {
  const exe = join(engineRoot(), 'Engine', 'Binaries', 'Win64', 'UnrealEditor-Cmd.exe');
  const args = [o.uproject, '-run=JamTournament', '-sim=jam', `-cartridge=${o.cartridge}`, `-difficulty=${o.level}`, `-player=${o.player}`, `-seeds=${lo}-${hi}`,
    `-maxframes=${o.maxFrames}`, '-trace=1', '-nullrhi', '-unattended', '-nopause', '-nosplash', '-stdout', '-FullStdOutLogOutput', '-LogCmds=LogJamAI Display'];
  if (o.tune) args.push(`-aitune=${o.tune}`);
  return new Promise((res) => {
    const p = spawn(exe, args, { windowsHide: true });
    const feed = makeReader(onMatch);
    let n = 0;
    const rl = createInterface({ input: p.stdout });
    rl.on('line', (l) => { if (l.includes('thirdjam.playtest/1')) n++; feed(l); });
    p.stderr.resume();
    p.on('close', (code) => res({ code, n }));
    p.on('error', () => res({ code: -1, n }));
  });
}

async function main() {
  const o = { seeds: arg('seeds', '1-60'), level: arg('level', 'pro'), player: arg('player', 'regular'), cartridge: arg('cartridge', 'hockey.regular'),
    workers: Number(arg('workers', '6')), chunk: Number(arg('chunk', '10')), tune: arg('tune', ''), maxFrames: Number(arg('max-frames', '36000')),
    uproject: arg('uproject', join(repo, 'Game', 'ThirdJam.uproject')), out: arg('out', ''), log: arg('log', '') };
  const rows = [];
  const onMatch = (r) => { rows.push(r); if (o.out) writeFileSync(o.out, rows.map((x) => JSON.stringify(x)).join('\n') + '\n'); };
  if (o.log) {
    const rl = createInterface({ input: createReadStream(o.log) });
    const feed = makeReader(onMatch);
    for await (const l of rl) feed(l);
  } else {
    if (!engineRoot()) { console.log('FAIL shape: Unreal Engine not found (set UE_ROOT)'); process.exit(2); }
    if (o.out) mkdirSync(dirname(resolve(o.out)), { recursive: true });
    const [lo, hi] = o.seeds.split('-').map(Number);
    const chunks = [];
    for (let s = lo; s <= (hi || lo); s += o.chunk) chunks.push([s, Math.min(hi || lo, s + o.chunk - 1)]);
    let failed = 0;
    const worker = async () => {
      while (chunks.length) {
        if (freeGB() < 12) { await new Promise((r) => setTimeout(r, 20000)); continue; }
        const [a, b] = chunks.shift();
        const r = await runChunk(o, a, b, onMatch);
        if (r.n !== b - a + 1) { failed++; console.log(`FAIL shape chunk ${a}-${b}: exit ${r.code}, ${r.n} matches`); }
        else console.log(`INFO shape chunk ${a}-${b}: ${r.n} matches; ${rows.length} so far`);
      }
    };
    await Promise.all(Array.from({ length: o.workers }, worker));
    if (failed) process.exitCode = 1;
  }
  rows.sort((a, b) => (a.seed || 0) - (b.seed || 0));
  const s = summarize(rows);
  console.log(JSON.stringify({ tool: 'thirdjam.shape/1', verdict: 'pass', level: o.level, player: o.player, cartridge: o.cartridge, tune: o.tune || undefined, ...s }));
}
if (process.argv[1] && resolve(process.argv[1]) === fileURLToPath(import.meta.url)) main();
