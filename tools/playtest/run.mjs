#!/usr/bin/env node
// Playtest grid (lane A34, R-0028 point 3): the human-like playtester (novice | regular | good) against the AI at every difficulty on both rinks,
// N seeds per cell, on lane A1's UJamSim through the tournament commandlet (-player=). One thirdjam.playtest/1 JSON line per match, a Markdown
// table of the fun metrics per cell, and one summary JSON line last. It measures; it changes no profile and no sim number.
//
//   node tools/playtest/run.mjs [--players novice,regular,good] [--levels rookie,pro,all_star] [--rinks regular,large] [--seeds 1-20]
//        [--jobs 3] [--max-frames 36000] [--out tools/playtest/out/<stamp>] [--uproject <path>]
//        [--tune "opp.shoot_extra_m=8;speed_scale=0.9"] [--set "rules.penalties=false;puck.drag=0.6"]
//
// --tune adjusts the resolved difficulty profile (-aitune, keys in Game/Source/JamCore/AI/DifficultyProfile.h) and --set writes a variant of each rink's
// cartridge (game paths; numbers, true / false) to tools/playtest/out/, validated with schema/validate.mjs, and plays that file: how a calibration
// proposal is measured against the playtester before anyone changes a profile. Neither touches a shipped file.
//
// Rinks (R-0010): regular = hockey.regular (90 x 45, speed 11, first to 3), large = hockey.arena.large (120 x 60, speed 8, 3-minute timer), or any cartridge name.
// Cells run --jobs at a time, each its own UnrealEditor-Cmd (about 5 s of startup, then 1-3 s per match headless).
// Exit: 0 ran, 1 a cell failed, 2 could not run (bad arguments, no engine).
import { existsSync, mkdirSync, readFileSync, writeFileSync } from 'node:fs';
import { spawn } from 'node:child_process';
import { join, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { engineRoot } from '../tournament/run.mjs';
import { validateCartridge } from '../../schema/validate.mjs';

const here = dirname(fileURLToPath(import.meta.url));
const repo = resolve(here, '..', '..');
export const RINKS = { regular: 'hockey.regular', large: 'hockey.arena.large' };
export const PLAYERS = ['novice', 'regular', 'good'];
export const LEVELS = ['rookie', 'pro', 'all_star'];

function arg(name, dflt) {
  const i = process.argv.indexOf(`--${name}`);
  return i > 0 && i + 1 < process.argv.length ? process.argv[i + 1] : dflt;
}
function die(code, msg) {
  console.log(`FAIL playtest ${msg}`);
  console.log(JSON.stringify({ tool: 'thirdjam.playtest/1', verdict: code === 2 ? 'could_not_run' : 'fail', exit: code, error: msg }));
  process.exit(code);
}
const stamp = () => new Date().toISOString().replace(/[-:]/g, '').replace(/\..+/, '').replace('T', '-');

export function parseLines(stdout) {
  const out = [];
  for (const l of (stdout || '').split(/\r?\n/)) {
    const i = l.indexOf('{"tool":"thirdjam.playtest/1"');
    if (i < 0) continue;
    try { out.push(JSON.parse(l.slice(i))); } catch { /* a truncated log line */ }
  }
  return out;
}

// One cell: one commandlet run. Resolves to its matches; rejects with the tail of the output.
// Variant cartridge with game paths set ("a.b=v;c.d=v"; v a number or true / false); returns its absolute path (forward slashes: it is echoed into JSON lines).
export function writeVariant(cartridge, set) {
  const cart = JSON.parse(readFileSync(join(repo, 'Game', 'Content', 'Cartridges', `${cartridge}.cartridge.json`), 'utf8'));
  for (const pair of set.split(';').filter(Boolean)) {
    const [path, v] = pair.split('=');
    const parts = path.split('.');
    let node = cart.game;
    for (const p of parts.slice(0, -1)) node = node[p] ??= {};
    node[parts.at(-1)] = v === 'true' ? true : v === 'false' ? false : Number(v);
  }
  const res = validateCartridge(cart);
  if (res.errors.length) die(2, `--set makes ${cartridge} invalid: ${res.errors.join(' | ')}`);
  const dir = join(here, 'out');
  mkdirSync(dir, { recursive: true });
  const file = join(dir, `${cartridge}.${set.replace(/[^a-z0-9.]+/gi, '_')}.cartridge.json`);
  writeFileSync(file, `${JSON.stringify(cart, null, 2)}\n`);
  return file.split('\\').join('/');
}

export function runCell({ uproject, cartridge, level, player, seeds, maxFrames, tune, set, sprint }) {
  const ue = engineRoot();
  if (!ue) die(2, 'Unreal Engine not found (set UE_ROOT)');
  const cmd = join(ue, 'Engine', 'Binaries', 'Win64', 'UnrealEditor-Cmd.exe');
  const cartArg = set ? writeVariant(cartridge, set) : cartridge;
  const args = [uproject, '-run=JamTournament', '-sim=jam', `-cartridge=${cartArg}`, `-difficulty=${level}`, `-player=${player}`, `-seeds=${seeds}`,
    `-maxframes=${maxFrames}`, '-nullrhi', '-unattended', '-nopause', '-nosplash', '-stdout', '-FullStdOutLogOutput'];
  if (tune) args.push(`-aitune=${tune}`);
  if (sprint) args.push('-playersprint');   // lane A72 (ask A3): the regular and good players press sprint
  return new Promise((res, rej) => {
    const p = spawn(cmd, args, { windowsHide: true });
    let out = '';
    p.stdout.on('data', (d) => { out += d; });
    p.stderr.on('data', (d) => { out += d; });
    const t = setTimeout(() => { p.kill(); rej(new Error(`timeout: ${cartridge} ${level} ${player}`)); }, 1800000);
    p.on('error', (e) => { clearTimeout(t); rej(e); });
    p.on('close', (code) => {
      clearTimeout(t);
      const ms = parseLines(out).map((m) => ({ ...m, cartridge, tune: tune || undefined, set: set || undefined }));
      if (code !== 0 || ms.length === 0) rej(new Error(`${cartridge} ${level} ${player}: exit ${code}, ${ms.length} matches\n${out.slice(-1500)}`));
      else res(ms);
    });
  });
}

const avg = (a) => (a.length ? a.reduce((s, x) => s + x, 0) / a.length : 0);
const median = (a) => { if (!a.length) return -1; const s = [...a].sort((x, y) => x - y); const m = s.length >> 1; return s.length % 2 ? s[m] : (s[m - 1] + s[m]) / 2; };
const sec = (frames) => (frames < 0 ? -1 : frames / 60);
const r1 = (x) => Math.round(x * 10) / 10;
const pct = (n, d) => (d ? Math.round((100 * n) / d) : 0);

// The fun metrics of one cell (brief A34 point 2), as the table prints them.
export function summarize(rink, cartridge, level, player, ms) {
  const n = ms.length;
  const wins = ms.filter((m) => m.result === 'win').length;
  const losses = ms.filter((m) => m.result === 'loss').length;
  const draws = ms.filter((m) => m.result === 'draw').length;
  const unfinished = ms.filter((m) => !m.over).length;
  const firstGoal = ms.map((m) => m.first_goal_by_human).filter((f) => f >= 0);
  const firstShot = ms.map((m) => m.first_shot).filter((f) => f >= 0);
  const firstTouch = ms.map((m) => m.first_touch).filter((f) => f >= 0);
  const shots = ms.reduce((s, m) => s + m.shots, 0);
  const sot = ms.reduce((s, m) => s + m.shots_on_target, 0);
  return {
    rink, cartridge, level, player, matches: n, wins, losses, draws, unfinished, win_pct: pct(wins, n),
    goals_for: r1(avg(ms.map((m) => m.goals[0]))), goals_against: r1(avg(ms.map((m) => m.goals[1]))), goals_by_human: r1(avg(ms.map((m) => m.goals_by_human))),
    touches_per_min: r1(avg(ms.map((m) => m.touches_per_min))), touches_per_min_min: r1(Math.min(...ms.map((m) => m.touches_per_min))),
    carries_3s: r1(avg(ms.map((m) => m.carries_3s))), carry_pct: pct(ms.reduce((s, m) => s + m.carry_frames, 0), ms.reduce((s, m) => s + m.play_frames, 0)),
    shots: r1(shots / n), shots_on_target_pct: pct(sot, shots), shots_opp: r1(avg(ms.map((m) => m.shots_opp))),
    lead_changes: r1(avg(ms.map((m) => m.lead_changes))),
    first_touch_s: r1(sec(median(firstTouch))), first_shot_s: r1(sec(median(firstShot))), first_goal_s: r1(sec(median(firstGoal))),
    scored: firstGoal.length, scored_2min: firstGoal.filter((f) => f <= 7200).length, no_shot: n - firstShot.length,
    faceoffs: r1(avg(ms.map((m) => m.faceoffs))), faceoffs_without_touch: ms.reduce((s, m) => s + m.faceoffs_without_touch, 0),
    max_faceoff_to_touch_s: r1(sec(Math.max(...ms.map((m) => m.max_faceoff_to_touch)))),
    checks_landed: r1(avg(ms.map((m) => m.checks_landed))), checks_taken: r1(avg(ms.map((m) => m.checks_taken))), fouls: r1(avg(ms.map((m) => m.fouls))),
    fouls_from_behind: r1(avg(ms.map((m) => m.fouls_from_behind || 0))), fouls_no_puck: r1(avg(ms.map((m) => m.fouls_no_puck || 0))),
    box_s: r1(sec(avg(ms.map((m) => m.box_frames)))), pokes: r1(avg(ms.map((m) => m.pokes))), pokes_clean: r1(avg(ms.map((m) => m.pokes_clean))),
    lifts: r1(avg(ms.map((m) => m.lifts))), passes: r1(avg(ms.map((m) => m.passes))),
    // lane A46: teammates' passes seated on the human's blade (lines from before A46 read as 0)
    passes_received: r1(avg(ms.map((m) => m.passes_received || 0))),
    // R-0029 point 4 (lane A39): the payoffs the human used per match and the human's goals within 3 s of one (lines from before A39 read as 0)
    one_timers: r1(avg(ms.map((m) => m.one_timers || 0))), breakaways: r1(avg(ms.map((m) => m.breakaways || 0))), dekes: r1(avg(ms.map((m) => m.dekes || 0))),
    payoff_goals: r1(avg(ms.map((m) => m.payoff_goals || 0))),
    idle_spells: r1(avg(ms.map((m) => m.idle_spells))), idle_s: r1(sec(avg(ms.map((m) => m.idle_frames)))), max_no_touch_s: r1(sec(Math.max(...ms.map((m) => m.max_no_touch)))),
    match_s: r1(sec(avg(ms.map((m) => m.frames)))), play_s: r1(sec(avg(ms.map((m) => m.play_frames)))),
  };
}

export function table(rows) {
  const head = ['rink', 'level', 'player', 'W-L-D-unf', 'win %', 'GF / GA', 'by human', 'touch/min (min)', 'carries 3 s', 'carry %', 'shots', 'SOT %', 'shots opp', 'lead chg',
    'first touch s', 'first shot s', 'first goal s (n / <2 min)', 'fo w/o touch', 'checks L/T', 'fouls behind/no puck (box s)', 'pokes (clean)', 'passes (rcv)', 'payoffs OT/BA/DK (goals)', 'idle spells (s)', 'max no touch s', 'match s'];
  const body = rows.map((s) => [s.rink, s.level, s.player, `${s.wins}-${s.losses}-${s.draws}-${s.unfinished}`, `**${s.win_pct}**`, `${s.goals_for} / ${s.goals_against}`, s.goals_by_human,
    `${s.touches_per_min} (${s.touches_per_min_min})`, s.carries_3s, s.carry_pct, s.shots, s.shots_on_target_pct, s.shots_opp, s.lead_changes, s.first_touch_s, s.first_shot_s,
    `${s.first_goal_s} (${s.scored} / ${s.scored_2min})`, s.faceoffs_without_touch, `${s.checks_landed} / ${s.checks_taken}`, `${s.fouls_from_behind} / ${s.fouls_no_puck} (${s.box_s})`, `${s.pokes} (${s.pokes_clean})`,
    `${s.passes} (${s.passes_received})`, `${s.one_timers} / ${s.breakaways} / ${s.dekes} (${s.payoff_goals})`, `${s.idle_spells} (${s.idle_s})`, s.max_no_touch_s, s.match_s].map(String));
  const line = (cells) => `| ${cells.join(' | ')} |`;
  return [line(head), `|${head.map(() => '---').join('|')}|`, ...body.map(line)].join('\n');
}

async function pool(items, jobs, fn) {
  const results = new Array(items.length);
  let next = 0;
  const worker = async () => { while (next < items.length) { const i = next++; results[i] = await fn(items[i], i); } };
  await Promise.all(Array.from({ length: Math.min(jobs, items.length) }, worker));
  return results;
}

if (process.argv[1] && resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
  const players = arg('players', PLAYERS.join(',')).split(',');
  const levels = arg('levels', LEVELS.join(',')).split(',');
  const rinks = arg('rinks', Object.keys(RINKS).join(',')).split(',');
  const seeds = arg('seeds', '1-20');
  const jobs = Number(arg('jobs', '3'));
  const maxFrames = Number(arg('max-frames', '36000'));
  const uproject = arg('uproject', join(repo, 'Game', 'ThirdJam.uproject'));
  const outBase = arg('out', join(here, 'out', `playtest-${stamp()}`));
  const tune = arg('tune', '');
  const set = arg('set', '');
  if (!/^\d+(-\d+)?$/.test(seeds)) die(2, `bad --seeds ${seeds}`);
  if (!existsSync(uproject)) die(2, `no uproject ${uproject}`);
  for (const p of players) if (!PLAYERS.includes(p)) die(2, `bad player ${p}`);
  for (const l of levels) if (!LEVELS.includes(l)) die(2, `bad level ${l}`);
  const cells = [];
  for (const rink of rinks) for (const level of levels) for (const player of players) cells.push({ rink, cartridge: RINKS[rink] || rink, level, player });
  console.log(`INFO playtest ${cells.length} cells x seeds ${seeds}, ${jobs} at a time, ${uproject}${tune ? ` tune "${tune}"` : ''}${set ? ` set "${set}"` : ''}`);
  const t0 = Date.now();
  let failed = 0;
  const rows = await pool(cells, jobs, async (c) => {
    try {
      const ms = await runCell({ uproject, cartridge: c.cartridge, level: c.level, player: c.player, seeds, maxFrames, tune, set });
      const s = summarize(c.rink, c.cartridge, c.level, c.player, ms);
      console.log(`PASS cell ${c.rink} ${c.level} ${c.player}: ${s.wins}-${s.losses}-${s.draws}-${s.unfinished}, touches/min ${s.touches_per_min}, first goal ${s.first_goal_s} s (${s.scored_2min}/${s.matches} inside 2 min)`);
      return { summary: s, matches: ms };
    } catch (e) {
      failed++;
      console.log(`FAIL cell ${c.rink} ${c.level} ${c.player}: ${e.message}`);
      return null;
    }
  });
  const ok = rows.filter(Boolean);
  mkdirSync(dirname(outBase), { recursive: true });
  const all = ok.flatMap((r) => r.matches);
  writeFileSync(`${outBase}.jsonl`, all.map((m) => JSON.stringify(m)).join('\n') + '\n');
  const md = table(ok.map((r) => r.summary));
  writeFileSync(`${outBase}.md`, `${md}\n`);
  console.log('');
  console.log(md);
  console.log(`INFO playtest ${all.length} matches in ${Math.round((Date.now() - t0) / 1000)} s; lines ${outBase}.jsonl, table ${outBase}.md`);
  console.log(JSON.stringify({ tool: 'thirdjam.playtest/1', verdict: failed ? 'fail' : 'pass', exit: failed ? 1 : 0, seeds, tune: tune || undefined, set: set || undefined, cells: ok.map((r) => r.summary), out: `${outBase}.jsonl` }));
  process.exit(failed ? 1 : 0);
}
