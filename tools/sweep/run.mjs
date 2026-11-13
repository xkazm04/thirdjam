#!/usr/bin/env node
// Difficulty sweep (lane A3): every level x every rink x N seeds, the scripted human (player bot) against the AI, on the
// tournament runners of lane A2 (tools/tournament). One table row per level and rink, checked against the target curve.
//
//   node tools/sweep/run.mjs [--runner ue|native] [--seeds 1-20] [--levels rookie,pro,all_star] [--rinks regular,large]
//        [--mode playerbot|brain] [--max-frames 36000] [--tune-<level> "k=v;k=v"] [--set "puck.drag=0.5;skater.accel=16"] [--out file.jsonl]
//
// --set writes a variant of each rink's cartridge (game paths, numbers) to tools/sweep/out/, validates it with schema/validate.mjs,
// and runs that file instead: how feel proposals are measured against the curve.
//
// Rinks (R-0010): regular = hockey.regular (90 x 45, speed 11), large = hockey.arena.large (120 x 60, speed 8), or any cartridge name.
// --tune-<level> adjusts the resolved profile (keys in Game/Source/JamCore/AI/DifficultyProfile.h, ApplyDifficultyTune);
// it is how the profile numbers were found, never a cartridge path.
// Runners: ue = UnrealEditor-Cmd -run=JamTournament on lane A1's UJamSim (the real sim; needs the editor target linked);
// native = the clang build of the same brain on A2's stand-in sim (seconds, for exploring; its numbers are not the gate).
// Exit: 0 ran and the curve holds, 1 ran and the curve misses (or a runner failed), 2 could not run.
import { readFileSync, existsSync, writeFileSync } from 'node:fs';
import { spawnSync } from 'node:child_process';
import { join, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { flatten, buildNative, engineRoot } from '../tournament/run.mjs';
import { validateCartridge } from '../../schema/validate.mjs';
import { mkdirSync } from 'node:fs';

const here = dirname(fileURLToPath(import.meta.url));
const repo = resolve(here, '..', '..');
const cartDir = join(repo, 'Game', 'Content', 'Cartridges');

export const LEVELS = ['rookie', 'pro', 'all_star'];
export const RINKS = { regular: 'hockey.regular', large: 'hockey.arena.large' };
// Brief A3 target curve: human (player bot) win rate, touches by the human per minute of play.
export const TARGET = {
  rookie: { win: 0.85, touchesPerMin: 6 },
  pro: { win: 0.50, touchesPerMin: 0 },   // R-0010: 50 on the 90 x 45 regular (the brief said about 55)
  all_star: { win: 0.25, touchesPerMin: 4 },
};
export const WIN_SLACK = 0.10;   // a level holds when its win rate is within +-10 points of the target

function arg(name, dflt) {
  const i = process.argv.indexOf(`--${name}`);
  return i > 0 && i + 1 < process.argv.length ? process.argv[i + 1] : dflt;
}
function die(code, msg) {
  console.log(`FAIL sweep ${msg}`);
  console.log(JSON.stringify({ tool: 'thirdjam.sweep/1', verdict: code === 2 ? 'could_not_run' : 'fail', exit: code, error: msg }));
  process.exit(code);
}

function parseLines(stdout) {
  const out = [];
  for (const l of (stdout || '').split(/\r?\n/)) {
    const i = l.indexOf('{"tool":"thirdjam.tournament/1"');
    if (i < 0) continue;
    try { out.push(JSON.parse(l.slice(i))); } catch { /* truncated log line */ }
  }
  return out;
}

// Variant cartridge with game paths set ("a.b=v;c.d=v"); returns its absolute path. Exit 2 when it does not validate.
export function writeVariant(cartridge, set) {
  const cart = JSON.parse(readFileSync(join(cartDir, `${cartridge}.cartridge.json`), 'utf8'));
  for (const pair of set.split(';').filter(Boolean)) {
    const [path, v] = pair.split('=');
    const parts = path.split('.');
    let node = cart.game;
    for (const p of parts.slice(0, -1)) node = node[p] ??= {};
    node[parts.at(-1)] = Number(v);
  }
  const res = validateCartridge(cart);
  if (res.errors.length) die(2, `--set makes ${cartridge} invalid: ${res.errors.join(' | ')}`);
  const dir = join(here, 'out');
  mkdirSync(dir, { recursive: true });
  const file = join(dir, `${cartridge}.${set.replace(/[^a-z0-9.]+/gi, '_')}.cartridge.json`);
  writeFileSync(file, `${JSON.stringify(cart, null, 2)}\n`);
  return file;
}

export function runCell({ runner, exe, cartridge, level, tune, seeds, mode, maxFrames }) {
  let r;
  const cartFile = (cartridge.endsWith('.json') ? cartridge : join(cartDir, `${cartridge}.cartridge.json`)).split('\\').join('/');   // forward slashes: the path is echoed into JSON lines
  const label = cartridge.endsWith('.json') ? cartFile.split('/').pop().replace('.cartridge.json', '') : cartridge;
  if (runner === 'native') {
    const cart = JSON.parse(readFileSync(cartFile, 'utf8'));
    let { text } = flatten(cart);
    text = text.split(';').filter((p) => !p.startsWith('ai.difficulty=')).join(';');
    text = `ai.difficulty=${level};${text}`;
    if (tune) text += `;ai.tune=${tune.replace(/=/g, ':').replace(/;/g, ',')}`;
    r = spawnSync(exe, ['--cfg', text, '--cartridge', label, '--seeds', seeds, '--mode', mode, '--max-frames', String(maxFrames)], { encoding: 'utf8', maxBuffer: 64 << 20 });
  } else {
    const ue = engineRoot();
    if (!ue) die(2, 'Unreal Engine not found (set UE_ROOT)');
    const cmd = join(ue, 'Engine', 'Binaries', 'Win64', 'UnrealEditor-Cmd.exe');
    const args = [join(repo, 'Game', 'ThirdJam.uproject'), '-run=JamTournament', '-sim=jam', `-cartridge=${cartFile}`, `-difficulty=${level}`, `-seeds=${seeds}`,
      `-mode=${mode}`, `-maxframes=${maxFrames}`, '-nullrhi', '-Multiprocess', '-unattended', '-nopause', '-nosplash', '-stdout', '-FullStdOutLogOutput'];
    if (tune) args.push(`-aitune=${tune}`);
    r = spawnSync(cmd, args, { encoding: 'utf8', maxBuffer: 256 << 20, timeout: 1800000 });
  }
  if (r.error) die(2, `${runner} runner did not start: ${r.error.message}`);
  const ms = parseLines(r.stdout);
  if (r.status !== 0 || ms.length === 0) die(1, `${runner} runner exit ${r.status}, ${ms.length} matches (${cartridge} ${level})\n${(r.stderr || r.stdout || '').slice(-1500)}`);
  return ms.map((m) => ({ ...m, difficulty: level, tune: tune || undefined }));
}

const avg = (a) => (a.length ? a.reduce((s, x) => s + x, 0) / a.length : 0);

// One row: the human is the player's skater, its team is "us".
export function summarizeCell(rink, level, ms) {
  const pi = ms[0].player;
  const team = (i) => (i < ms[0].touches.length / 2 ? 0 : 1);   // roster order: team 0 first, equal sizes in every preset
  const us = pi >= 0 ? team(pi) : 0;
  const n = ms.length;
  const wins = ms.filter((m) => m.over && m.winner === us).length;
  const losses = ms.filter((m) => m.over && m.winner === 1 - us).length;
  const sumIdx = (arr, pred) => arr.reduce((s, x, i) => s + (pred(i) ? x : 0), 0);
  const playMin = (m) => Math.max(1e-9, m.play_frames / 3600);
  return {
    rink, level, matches: n, wins, losses, unfinished: n - wins - losses,
    win_rate: wins / n,
    goals_per_match: avg(ms.map((m) => m.goals[0] + m.goals[1])),
    goals_for: avg(ms.map((m) => m.goals[us])), goals_against: avg(ms.map((m) => m.goals[1 - us])),
    shots_human: pi >= 0 ? avg(ms.map((m) => m.shots[pi])) : 0,
    shots_mates: avg(ms.map((m) => sumIdx(m.shots, (i) => i !== pi && team(i) === us))),
    shots_opp: avg(ms.map((m) => sumIdx(m.shots, (i) => team(i) !== us))),
    human_touches: pi >= 0 ? avg(ms.map((m) => m.touches[pi])) : 0,
    human_touches_per_min: pi >= 0 ? avg(ms.map((m) => m.touches[pi] / playMin(m))) : 0,
    human_touches_per_min_min: pi >= 0 ? Math.min(...ms.map((m) => m.touches[pi] / playMin(m))) : 0,
    length_s: avg(ms.map((m) => m.frames / 60)),
    length_s_max: Math.max(...ms.map((m) => m.frames / 60)),
    s_per_goal: avg(ms.map((m) => m.play_frames / 60 / Math.max(1, m.goals[0] + m.goals[1]))),
    faceoffs_without_touch: ms.reduce((s, m) => s + m.faceoffs_without_player_touch, 0),
    stalls: avg(ms.map((m) => m.stalls)),
  };
}

export function checkCell(row, mode) {
  if (mode !== 'playerbot') return { ok: row.unfinished === 0, why: row.unfinished ? `${row.unfinished} unfinished` : '' };
  const t = TARGET[row.level];
  if (!t) return { ok: true, why: '' };
  const why = [];
  if (Math.abs(row.win_rate - t.win) > WIN_SLACK) why.push(`win ${(row.win_rate * 100).toFixed(0)}% vs target ${(t.win * 100).toFixed(0)}%`);
  if (row.human_touches_per_min < t.touchesPerMin) why.push(`touches/min ${row.human_touches_per_min.toFixed(1)} < ${t.touchesPerMin}`);
  if (row.faceoffs_without_touch > 0) why.push(`${row.faceoffs_without_touch} faceoffs without a human touch in 600 frames`);
  return { ok: why.length === 0, why: why.join('; ') };
}

export function table(rows) {
  const f1 = (x) => x.toFixed(1);
  const head = ['rink', 'level', 'n', 'W-L-unf', 'win %', 'goals/match', 'GF', 'GA', 'shots human', 'shots mates', 'shots AI opp', 'human touches/min (min)', 'len s (max)', 's/goal', 'no-touch fo', 'curve'];
  const body = rows.map((s) => [s.rink, s.level, s.matches, `${s.wins}-${s.losses}-${s.unfinished}`, (s.win_rate * 100).toFixed(0), f1(s.goals_per_match), f1(s.goals_for), f1(s.goals_against),
    f1(s.shots_human), f1(s.shots_mates), f1(s.shots_opp), `${f1(s.human_touches_per_min)} (${f1(s.human_touches_per_min_min)})`, `${f1(s.length_s)} (${f1(s.length_s_max)})`, f1(s.s_per_goal),
    s.faceoffs_without_touch, s.check.ok ? 'holds' : `MISS ${s.check.why}`].map(String));
  const w = head.map((h, i) => Math.max(h.length, ...body.map((b) => b[i].length)));
  const line = (cells) => '| ' + cells.map((c, i) => c.padEnd(w[i])).join(' | ') + ' |';
  return [line(head), '|' + w.map((x) => '-'.repeat(x + 2)).join('|') + '|', ...body.map(line)].join('\n');
}

if (process.argv[1] && resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
  const runner = arg('runner', 'ue');
  const seeds = arg('seeds', '1-20');
  const levels = arg('levels', LEVELS.join(',')).split(',');
  const rinks = arg('rinks', 'regular,large').split(',');
  const mode = arg('mode', 'playerbot');
  const maxFrames = Number(arg('max-frames', '36000'));
  const out = arg('out', null);
  if (!['ue', 'native'].includes(runner)) die(2, `bad --runner ${runner}`);
  if (!/^\d+(-\d+)?$/.test(seeds)) die(2, `bad --seeds ${seeds}`);
  for (const l of levels) if (!LEVELS.includes(l)) die(2, `bad level ${l} (rookie | pro | all_star)`);
  const exe = runner === 'native' ? buildNative() : null;
  const rows = [];
  const all = [];
  const set = arg('set', '');
  for (const rk of rinks) {
    let cartridge = RINKS[rk] || rk;
    if (!existsSync(join(cartDir, `${cartridge}.cartridge.json`))) die(2, `no cartridge ${cartridge}`);
    const set = arg('set', '');
    if (set) cartridge = writeVariant(cartridge, set);
    for (const level of levels) {
      const tune = arg(`tune-${level}`, '');
      const ms = runCell({ runner, exe, cartridge, level, tune, seeds, mode, maxFrames });
      all.push(...ms);
      const row = summarizeCell(rk, level, ms);
      row.check = checkCell(row, mode);
      if (tune) row.tune = tune;
      if (set) row.set = set;
      rows.push(row);
      console.log(`${row.check.ok ? 'PASS' : 'MISS'} sweep ${rk} ${level}: ${row.wins}-${row.losses}-${row.unfinished}, touches/min ${row.human_touches_per_min.toFixed(1)}${row.check.ok ? '' : ` (${row.check.why})`}`);
    }
  }
  console.log('');
  console.log(table(rows));
  if (out) writeFileSync(out, all.map((m) => JSON.stringify(m)).join('\n') + '\n');
  const ok = rows.every((r) => r.check.ok);
  console.log(JSON.stringify({ tool: 'thirdjam.sweep/1', verdict: ok ? 'pass' : 'fail', exit: ok ? 0 : 1, runner, seeds, mode, rows }));
  process.exit(ok ? 0 : 1);
}
