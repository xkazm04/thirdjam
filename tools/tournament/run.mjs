#!/usr/bin/env node
// Bot tournament (lane A2): N headless bot-vs-bot matches per cartridge and seed range; one JSON line per match,
// then a summary table and one summary JSON line last.
//
//   node tools/tournament/run.mjs --cartridges hockey.regular,hockey.arena.large --seeds 1-20
//        [--mode playerbot|playerbot_ai|brain|idle] [--runner native|ue] [--sim jam|ref] [--max-frames 36000] [--out file.jsonl] [--uproject <path>]
//
// Runners (same engine-free sources, same output):
//   native  clang++ build of tools/tournament/native/main.cpp + Game/Source/JamCore/{AI,Tests} (seconds per 20 matches)
//   ue      UnrealEditor-Cmd <uproject> -run=JamTournament (commandlet in JamCore/AI) -nullrhi; needs the editor target built
// native runs the stand-in sim (JamCore/AI/JamBotRefSim); ue runs UJamSim (-sim=jam, default) or the stand-in (--sim ref).
// Exit: 0 ran, 1 a runner failed, 2 could not run (bad arguments, no compiler, no engine).
import { readFileSync, existsSync, statSync, mkdirSync, writeFileSync } from 'node:fs';
import { spawnSync } from 'node:child_process';
import { join, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const here = dirname(fileURLToPath(import.meta.url));
const repo = resolve(here, '..', '..');
const src = join(repo, 'Game', 'Source', 'JamCore');
const cartDir = join(repo, 'Game', 'Content', 'Cartridges');
const outDir = join(here, 'out');

function arg(name, dflt) {
  const i = process.argv.indexOf(`--${name}`);
  return i > 0 && i + 1 < process.argv.length ? process.argv[i + 1] : dflt;
}
function die(code, msg) {
  console.log(`FAIL tournament ${msg}`);
  console.log(JSON.stringify({ tool: 'thirdjam.tournament/1', verdict: code === 2 ? 'could_not_run' : 'fail', exit: code, error: msg }));
  process.exit(code);
}

const TUNABLE_TO_FIELD = {
  skater_speed: 'skater.speed', skater_turn_rate: 'skater.turn_rate', shot_speed: 'shot.speed', puck_max_speed: 'puck.max_speed',
  ice_friction: 'rink.ice_friction', board_restitution: 'rink.board_restitution', ai_speed_scale: 'ai.speed_scale',
  ai_red_skill: 'ai.red_skill', goal_pause_frames: 'rules.goal_pause_frames',
};
const PASSED = {
  rink: ['length_m', 'width_m', 'corner_radius_m', 'board_restitution', 'ice_friction', 'goal_width_m'],
  skater: ['speed', 'accel', 'turn_rate', 'lateral_grip', 'brake_decel'],
  puck: ['radius_m', 'max_speed', 'drag'],
  shot: ['reach_m', 'speed', 'cooldown_frames', 'aim_cone_deg'],
  rules: ['mode', 'score_to_win', 'timer_frames', 'goal_pause_frames'],
  ai: ['difficulty', 'yield_margin_m', 'yield_radius_m', 'hold_radius_m', 'speed_scale', 'red_skill'],
};

// Cartridge -> flat "k=v;..." for FJamMatchConfig::Parse. Absent keys stay absent (the C++ side holds DEFAULTS.md).
export function flatten(cart) {
  const g = cart.game;
  const kv = new Map();
  const notes = [];
  for (const [obj, keys] of Object.entries(PASSED)) {
    for (const k of keys) if (g[obj] && g[obj][k] !== undefined) kv.set(`${obj}.${k}`, g[obj][k]);
  }
  for (const [k, v] of Object.entries(cart.values || {})) if (TUNABLE_TO_FIELD[k]) kv.set(TUNABLE_TO_FIELD[k], v);
  if (g.rink?.obstacles?.length) notes.push('rink.obstacles ignored by the stand-in sim');
  if (g.pickups?.length) notes.push('pickups ignored by the stand-in sim');
  if (g.skater?.check_enabled) notes.push('checks ignored by the stand-in sim');
  const parts = [...kv].map(([k, v]) => `${k}=${v}`);
  g.teams.forEach((t, ti) => t.skaters.forEach((s) => parts.push(`sk=${ti},${s.role},${s.spawn[0]},${s.spawn[1]}`)));
  return { text: parts.join(';'), notes };
}

function newestMtime(files) { return Math.max(...files.map((f) => statSync(f).mtimeMs)); }

export function buildNative() {
  const exe = join(outDir, 'jam_tournament.exe');
  const sources = [join(here, 'native', 'main.cpp'), join(src, 'AI', 'JamAICore.cpp'), join(src, 'AI', 'JamBotMatch.cpp'), join(src, 'AI', 'JamBotRefSim.cpp'), join(src, 'AI', 'DifficultyProfile.cpp')];
  const headers = [join(src, 'AI', 'JamAICore.h'), join(src, 'AI', 'JamBotMatch.h'), join(src, 'AI', 'JamBotRefSim.h'), join(src, 'AI', 'DifficultyProfile.h')];
  if (existsSync(exe) && statSync(exe).mtimeMs > newestMtime([...sources, ...headers])) return exe;
  const cxx = process.env.CXX || ['C:/Program Files/LLVM/bin/clang++.exe'].find(existsSync) || 'clang++';
  mkdirSync(outDir, { recursive: true });
  const r = spawnSync(cxx, ['-std=c++17', '-O2', '-Wall', '-Wextra', '-Wshadow-all', `-I${join(src, 'AI')}`, ...sources, '-o', exe], { encoding: 'utf8' });
  if (r.error) die(2, `no C++ compiler (${cxx}): ${r.error.message}`);
  if (r.status !== 0) die(1, `native build failed:\n${r.stderr}`);
  console.log(`INFO tournament built ${exe}`);
  return exe;
}

export function engineRoot() {
  if (process.env.UE_ROOT && existsSync(process.env.UE_ROOT)) return process.env.UE_ROOT;
  const dflt = 'C:/Program Files/Epic Games/UE_5.9';
  return existsSync(dflt) ? dflt : null;
}

function runOne(runner, exe, cfg, cartridge, seeds, mode, maxFrames, sim) {
  let r;
  if (runner === 'native') {
    r = spawnSync(exe, ['--cfg', cfg, '--cartridge', cartridge, '--seeds', seeds, '--mode', mode, '--max-frames', String(maxFrames)], { encoding: 'utf8', maxBuffer: 64 << 20 });
  } else {
    const ue = engineRoot();
    if (!ue) die(2, 'Unreal Engine not found (set UE_ROOT)');
    const cmd = join(ue, 'Engine', 'Binaries', 'Win64', 'UnrealEditor-Cmd.exe');
    const uproject = arg('uproject', join(repo, 'Game', 'ThirdJam.uproject'));
    r = spawnSync(cmd, [uproject, '-run=JamTournament', `-sim=${sim}`, `-cfg=${cfg}`, `-cartridge=${cartridge}`, `-seeds=${seeds}`, `-mode=${mode}`, `-maxframes=${maxFrames}`,
      '-nullrhi', '-unattended', '-nopause', '-nosplash', '-stdout', '-FullStdOutLogOutput'], { encoding: 'utf8', maxBuffer: 256 << 20, timeout: 1800000 });
  }
  if (r.error) die(2, `${runner} runner did not start: ${r.error.message}`);
  const lines = (r.stdout || '').split(/\r?\n/).map((l) => { const i = l.indexOf('{"tool":"thirdjam.tournament/1"'); return i >= 0 ? l.slice(i) : null; }).filter(Boolean);
  const matches = [];
  for (const l of lines) { try { matches.push(JSON.parse(l)); } catch { /* a truncated log line */ } }
  if (r.status !== 0 || matches.length === 0) die(1, `${runner} runner exit ${r.status}, ${matches.length} matches\n${(r.stderr || '').slice(-2000)}`);
  return matches;
}

const avg = (a) => (a.length ? a.reduce((s, x) => s + x, 0) / a.length : 0);
const f1 = (x) => x.toFixed(1);

export function summarize(cartridge, mode, ms) {
  const n = ms.length;
  const pi = ms[0].player;
  const team0 = ms[0].touches.map((_, i) => i);
  const wins = ms.filter((m) => m.winner === 0).length;
  const losses = ms.filter((m) => m.winner === 1).length;
  const sum = (m, arr) => arr.reduce((s, x) => s + x, 0);
  return {
    cartridge, mode, matches: n, wins, losses, unfinished: ms.filter((m) => !m.over).length,
    goals_for: avg(ms.map((m) => m.goals[0])), goals_against: avg(ms.map((m) => m.goals[1])),
    shots_player: pi >= 0 ? avg(ms.map((m) => m.shots[pi])) : 0,
    shots_ai: avg(ms.map((m) => sum(m, m.shots) - (pi >= 0 ? m.shots[pi] : 0))),
    touches_per_skater: team0.map((i) => +f1(avg(ms.map((m) => m.touches[i])))),
    shots_per_skater: team0.map((i) => +f1(avg(ms.map((m) => m.shots[i])))),
    puck_path_m: avg(ms.map((m) => m.puck_path_m)), stalls: avg(ms.map((m) => m.stalls)),
    frames: avg(ms.map((m) => m.frames)), frames_max: Math.max(...ms.map((m) => m.frames)),
    frames_per_goal: avg(ms.map((m) => m.play_frames / Math.max(1, m.goals[0] + m.goals[1]))),
    faceoffs_without_player_touch: ms.reduce((s, m) => s + m.faceoffs_without_player_touch, 0),
    max_faceoff_to_touch: Math.max(...ms.map((m) => m.max_faceoff_to_touch)),
    outside_rink: ms.filter((m) => m.max_board_distance > 0.01).length,
  };
}

function table(rows) {
  const head = ['sim', 'cartridge', 'mode', 'n', 'W-L-unf', 'GF', 'GA', 'shots P', 'shots AI', 'touches/skater (team0 | team1)', 'puck m', 'stalls', 'len s', 'max s', 's/goal', 'no-touch fo', 'max fo->touch s'];
  const body = rows.map((s) => [s.sim, s.cartridge, s.mode, s.matches, `${s.wins}-${s.losses}-${s.unfinished}`, f1(s.goals_for), f1(s.goals_against), f1(s.shots_player), f1(s.shots_ai),
    `${s.touches_per_skater.slice(0, 3).join(' ')} | ${s.touches_per_skater.slice(3).join(' ')}`, f1(s.puck_path_m), f1(s.stalls), f1(s.frames / 60), f1(s.frames_max / 60), f1(s.frames_per_goal / 60),
    s.faceoffs_without_player_touch, f1(s.max_faceoff_to_touch / 60)].map(String));
  const w = head.map((h, i) => Math.max(h.length, ...body.map((b) => b[i].length)));
  const line = (cells) => '| ' + cells.map((c, i) => c.padEnd(w[i])).join(' | ') + ' |';
  return [line(head), '|' + w.map((x) => '-'.repeat(x + 2)).join('|') + '|', ...body.map(line)].join('\n');
}

if (process.argv[1] && resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
  const cartridges = arg('cartridges', 'hockey.regular,hockey.arena.large').split(',');
  const seeds = arg('seeds', '1-20');
  const modes = arg('mode', 'playerbot').split(',');
  const runner = arg('runner', 'native');
  const maxFrames = Number(arg('max-frames', '36000'));
  const out = arg('out', null);
  const sim = runner === 'native' ? 'ref' : arg('sim', 'jam');
  if (!/^\d+(-\d+)?$/.test(seeds)) die(2, `bad --seeds ${seeds}`);
  if (!['native', 'ue'].includes(runner)) die(2, `bad --runner ${runner}`);
  const exe = runner === 'native' ? buildNative() : null;
  const all = [];
  const rows = [];
  for (const c of cartridges) {
    const file = join(cartDir, `${c}.cartridge.json`);
    if (!existsSync(file)) die(2, `no cartridge ${file}`);
    const { text, notes } = flatten(JSON.parse(readFileSync(file, 'utf8')));
    for (const n of notes) console.log(`INFO tournament ${c}: ${n}`);
    for (const mode of modes) {
      const ms = runOne(runner, exe, text, c, seeds, mode, maxFrames, sim);
      for (const m of ms) console.log(JSON.stringify(m));
      all.push(...ms);
      rows.push({ sim, ...summarize(c, mode, ms) });
    }
  }
  console.log('');
  console.log(table(rows));
  if (out) writeFileSync(out, all.map((m) => JSON.stringify(m)).join('\n') + '\n');
  console.log(JSON.stringify({ tool: 'thirdjam.tournament/1', verdict: 'pass', exit: 0, runner, seeds, summary: rows }));
}
