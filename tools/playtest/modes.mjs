#!/usr/bin/env node
// Director's choices proof runner (lane A67, R-0036): every mode cartridge in Game/Content/Cartridges/modes/ is validated (schema/validate.mjs) and
// played by the regular playtester at the mode's level for N seeds next to hockey.regular at the same level, on the same binary. Per mode: win %,
// goals, the human's touches, checks, and the one number that shows the mode's character; the mode passes when that number differs from regular's
// by a third or more (|mode / regular - 1| >= 1/3) and every match ended. It measures; it writes nothing outside tools/playtest/out/.
//
//   node tools/playtest/modes.mjs [--modes pond,cage] [--seeds 1-20] [--jobs 3] [--player regular] [--max-frames 36000] [--out tools/playtest/out/modes-<stamp>]
//        [--baseline tools/playtest/out/modes-<earlier>.jsonl]   (reuse the regular cells of an earlier run on the same binary: same seeds, same hash)
//        [--sprint]   (lane A72, ask A3: the playtester presses sprint, -playersprint; Marathon's number is the human's sprint frames and needs it)
//
// Exit: 0 every mode passed, 1 a mode failed (invalid, a cell failed, unfinished matches, or its number within a third of regular), 2 could not run.
import { existsSync, mkdirSync, readFileSync, readdirSync, writeFileSync } from 'node:fs';
import { join, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { runCell, summarize } from './run.mjs';
import { validateCartridge } from '../../schema/validate.mjs';

const here = dirname(fileURLToPath(import.meta.url));
const repo = resolve(here, '..', '..');
const modesDir = join(repo, 'Game', 'Content', 'Cartridges', 'modes');

const avg = (a) => (a.length ? a.reduce((s, x) => s + x, 0) / a.length : 0);
const r1 = (x) => Math.round(x * 10) / 10;
const r2 = (x) => Math.round(x * 100) / 100;
const perPlayMin = (ms, f) => { const fr = ms.reduce((a, m) => a + m.play_frames, 0); return fr ? ms.reduce((a, m) => a + f(m), 0) / (fr / 3600) : 0; };

// The character number of each mode, read from the thirdjam.playtest/1 lines of one cell (per match averages).
export const CHARACTER = {
  // touches / min did not move in any Pond variant (22 regular rookie; 24.8, 21.8, 24.9): the pond's character is the passing (MODES.md)
  pond: { label: 'passes + received / play min', of: (ms) => { const f = ms.reduce((a, m) => a + m.play_frames, 0); return f ? ms.reduce((a, m) => a + (m.passes || 0) + (m.passes_received || 0), 0) / (f / 3600) : 0; } },
  // the human's checks landed; knockdowns of the human stay 0 while bots never body check (PLAYTEST.md section 3)
  cage: { label: 'checks landed / match', of: (ms) => avg(ms.map((m) => m.checks_landed || 0)) },
  sniper: { label: 'human goals per human shot', of: (ms) => { const s = ms.reduce((a, m) => a + m.shots, 0); return s ? ms.reduce((a, m) => a + m.goals_by_human, 0) / s : 0; } },
  // lane A72 (asks A1-A3): ten minutes, bots that sprint (ai.sprint) and a playtester that sprints (--sprint): the human's sprint frames per match. Minutes per goal
  // (A67's number) is kept as marathon-pace for the runs without --sprint.
  marathon: { label: 'human sprint frames / match', of: (ms) => avg(ms.map((m) => m.sprint_frames || 0)) },
  'marathon-pace': { label: 'minutes per goal', of: (ms) => { const g = ms.reduce((a, m) => a + m.goals[0] + m.goals[1], 0); return g ? ms.reduce((a, m) => a + m.frames, 0) / 3600 / g : 0; } },
  'power-play': { label: 'passes + received / match', of: (ms) => avg(ms.map((m) => (m.passes || 0) + (m.passes_received || 0))) },
  'blind-side': { label: 'dekes / match', of: (ms) => avg(ms.map((m) => m.dekes || 0)) },
  // lane A81 (modes pass 2): six more characters, each its own number. Per play minute where the mode changes the match length (a timer or one goal).
  'sudden-death': { label: 'match seconds', of: (ms) => avg(ms.map((m) => m.frames)) / 60 },
  outnumbered: { label: 'shots against / play min', of: (ms) => perPlayMin(ms, (m) => m.shots_opp || 0) },
  'glass-house': { label: 'human box seconds / match', of: (ms) => avg(ms.map((m) => m.box_frames || 0)) / 60 },
  // the carry share moved only 17 % (r1); the puck that runs away shows in the human's touches (pinball-b, A81 r2)
  pinball: { label: 'human touches / min', of: (ms) => avg(ms.map((m) => m.touches_per_min)) },
  'tap-in': { label: 'goals (both) / play min', of: (ms) => perPlayMin(ms, (m) => m.goals[0] + m.goals[1]) },
  // R-0041 (aggression is the character): hits = checks the human lands + checks the human takes, per play minute
  bruisers: { label: 'hits (landed + taken) / play min', of: (ms) => perPlayMin(ms, (m) => (m.checks_landed || 0) + (m.checks_taken || 0)) },
  'fog-of-war': { label: 'human breakaways / play min', of: (ms) => perPlayMin(ms, (m) => m.breakaways || 0) },
};

function arg(name, dflt) {
  const i = process.argv.indexOf(`--${name}`);
  return i > 0 && i + 1 < process.argv.length ? process.argv[i + 1] : dflt;
}
function die(code, msg) {
  console.log(`FAIL modes ${msg}`);
  console.log(JSON.stringify({ tool: 'thirdjam.modes/1', verdict: code === 2 ? 'could_not_run' : 'fail', exit: code, error: msg }));
  process.exit(code);
}
const stamp = () => new Date().toISOString().replace(/[-:]/g, '').replace(/\..+/, '').replace('T', '-');

export function listModes() {
  if (!existsSync(modesDir)) return [];
  return readdirSync(modesDir).filter((f) => f.endsWith('.cartridge.json')).sort().map((f) => {
    const slug = f.replace(/^hockey\.mode\./, '').replace(/\.cartridge\.json$/, '');
    const cart = JSON.parse(readFileSync(join(modesDir, f), 'utf8'));
    return { slug, file: join(modesDir, f), name: `modes/${f.replace(/\.cartridge\.json$/, '')}`, title: cart.title, level: cart.game?.ai?.difficulty || 'pro', cart };
  });
}

async function pool(items, jobs, fn) {
  const results = new Array(items.length);
  let next = 0;
  const worker = async () => { while (next < items.length) { const i = next++; results[i] = await fn(items[i], i); } };
  await Promise.all(Array.from({ length: Math.min(jobs, items.length) }, worker));
  return results;
}

function cell(s) {
  return { record: `${s.wins}-${s.losses}-${s.draws}-${s.unfinished}`, win_pct: s.win_pct, goals_for: s.goals_for, goals_against: s.goals_against,
    touches_per_min: s.touches_per_min, checks_landed: s.checks_landed, checks_taken: s.checks_taken, passes: s.passes, passes_received: s.passes_received,
    dekes: s.dekes, breakaways: s.breakaways, one_timers: s.one_timers, shots: s.shots, match_s: s.match_s, unfinished: s.unfinished };
}
const signed = (p) => (p === null ? 'new' : `${p > 0 ? '+' : ''}${p} %`);

if (process.argv[1] && resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
  const only = arg('modes', '');
  const seeds = arg('seeds', '1-20');
  const jobs = Number(arg('jobs', '3'));
  const player = arg('player', 'regular');
  const maxFrames = Number(arg('max-frames', '36000'));
  const uproject = arg('uproject', join(repo, 'Game', 'ThirdJam.uproject'));
  const outBase = arg('out', join(here, 'out', `modes-${stamp()}`));
  const baseline = arg('baseline', '');
  const sprint = process.argv.includes('--sprint');
  if (!/^\d+(-\d+)?$/.test(seeds)) die(2, `bad --seeds ${seeds}`);
  if (!existsSync(uproject)) die(2, `no uproject ${uproject}`);
  let modes = listModes();
  if (only) modes = modes.filter((m) => only.split(',').includes(m.slug));
  if (!modes.length) die(2, `no mode cartridges in ${modesDir}${only ? ` matching ${only}` : ''}`);
  let failed = 0;
  for (const m of modes) {
    const res = validateCartridge(m.cart);
    if (res.errors.length) { failed++; m.invalid = true; console.log(`FAIL validate ${m.name}: ${res.errors.join(' | ')}`); }
    else console.log(`PASS validate ${m.name}`);
  }
  const levels = [...new Set(modes.map((m) => m.level))];
  // Regular cells from an earlier run, when given and complete for a level.
  const reused = {};
  if (baseline) {
    if (!existsSync(baseline)) die(2, `no baseline ${baseline}`);
    for (const l of readFileSync(baseline, 'utf8').split(/\r?\n/).filter(Boolean)) {
      const m = JSON.parse(l);
      if (m.mode === 'regular' && m.player === player) (reused[m.difficulty] ??= []).push(m);
    }
  }
  const cells = [
    ...levels.filter((level) => !reused[level]).map((level) => ({ kind: 'regular', name: 'hockey.regular', level })),
    ...modes.filter((m) => !m.invalid).map((m) => ({ kind: 'mode', name: m.name, level: m.level, mode: m })),
  ];
  console.log(`INFO modes ${cells.length} cells (${levels.length} regular baselines) x seeds ${seeds}, player ${player}, ${jobs} at a time`);
  const t0 = Date.now();
  const done = await pool(cells, jobs, async (c) => {
    try {
      const ms = await runCell({ uproject, cartridge: c.name, level: c.level, player, seeds, maxFrames, sprint });
      return { ...c, ms, s: summarize(c.kind === 'regular' ? 'regular' : c.mode.slug, c.name, c.level, player, ms) };
    } catch (e) {
      failed++;
      console.log(`FAIL cell ${c.name} ${c.level}: ${e.message}`);
      return null;
    }
  });
  for (const [level, ms] of Object.entries(reused)) if (levels.includes(level)) done.push({ kind: 'regular', name: 'hockey.regular', level, ms, s: summarize('regular', 'hockey.regular', level, player, ms), reused: true });
  const base = Object.fromEntries(done.filter((d) => d && d.kind === 'regular').map((d) => [d.level, d]));
  const results = [];
  const lines = [
    '| mode | level | W-L-D-unf | win % (regular) | GF / GA (regular) | touches/min (regular) | checks L/T (regular) | character | mode | regular | change | verdict |',
    '|---|---|---|---|---|---|---|---|---|---|---|---|',
  ];
  for (const d of done.filter((x) => x && x.kind === 'mode')) {
    const ch = (d.mode.slug === 'marathon' && !sprint ? CHARACTER['marathon-pace'] : CHARACTER[d.mode.slug]) || CHARACTER[d.mode.slug.split('-')[0]] || CHARACTER.sniper;
    const b = base[d.level];
    const mine = ch.of(d.ms);
    const theirs = b ? ch.of(b.ms) : NaN;
    const change = theirs > 0 ? mine / theirs - 1 : (mine > 0 ? Infinity : 0);
    const ok = Boolean(b) && Math.abs(change) >= 1 / 3 && d.s.unfinished === 0;
    if (!ok) failed++;
    const pctChange = Number.isFinite(change) ? Math.round(change * 100) : null;
    const r = { mode: d.mode.slug, title: d.mode.title, level: d.level, character: ch.label, value: r2(mine), regular_value: r2(theirs), change_pct: pctChange,
      verdict: ok ? 'pass' : 'fail', mode_cell: cell(d.s), regular_cell: b ? cell(b.s) : undefined };
    results.push(r);
    console.log(`${ok ? 'PASS' : 'FAIL'} mode ${d.mode.slug} (${d.level}): ${ch.label} ${r.value} vs regular ${r.regular_value} (${signed(pctChange)}), win ${d.s.win_pct} % vs ${b?.s.win_pct} %, unfinished ${d.s.unfinished}`);
    lines.push(`| ${d.mode.title} | ${d.level} | ${d.s.wins}-${d.s.losses}-${d.s.draws}-${d.s.unfinished} | ${d.s.win_pct} (${b?.s.win_pct}) | ${d.s.goals_for} / ${d.s.goals_against} (${b?.s.goals_for} / ${b?.s.goals_against}) | ${d.s.touches_per_min} (${b?.s.touches_per_min}) | ${d.s.checks_landed} / ${d.s.checks_taken} (${b?.s.checks_landed} / ${b?.s.checks_taken}) | ${ch.label} | **${r.value}** | ${r.regular_value} | ${signed(pctChange)} | ${r.verdict} |`);
  }
  mkdirSync(dirname(outBase), { recursive: true });
  writeFileSync(`${outBase}.jsonl`, done.filter(Boolean).flatMap((d) => d.ms.map((m) => JSON.stringify({ ...m, mode: d.kind === 'mode' ? d.mode.slug : 'regular' }))).join('\n') + '\n');
  writeFileSync(`${outBase}.md`, `${lines.join('\n')}\n`);
  console.log('');
  console.log(lines.join('\n'));
  console.log(`INFO modes ${Math.round((Date.now() - t0) / 1000)} s; lines ${outBase}.jsonl, table ${outBase}.md`);
  console.log(JSON.stringify({ tool: 'thirdjam.modes/1', verdict: failed ? 'fail' : 'pass', exit: failed ? 1 : 0, seeds, player, modes: results, out: `${outBase}.jsonl` }));
  process.exit(failed ? 1 : 0);
}
