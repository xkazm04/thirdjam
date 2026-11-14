#!/usr/bin/env node
// Director's choices proof runner (lane A67, R-0036): every mode cartridge in Game/Content/Cartridges/modes/ is validated (schema/validate.mjs) and
// played by the regular playtester at the mode's level for N seeds next to hockey.regular at the same level, on the same binary. Per mode: win %,
// goals, the human's touches, checks, and the one number that shows the mode's character; the mode passes when that number differs from regular's
// by a third or more (|mode / regular - 1| >= 1/3) and every match ended. It measures; it writes nothing outside tools/playtest/out/.
//
//   node tools/playtest/modes.mjs [--modes pond,cage] [--seeds 1-20] [--jobs 3] [--player regular] [--max-frames 36000] [--out tools/playtest/out/modes-<stamp>]
//        [--baseline tools/playtest/out/modes-<earlier>.jsonl]   (reuse the regular cells of an earlier run on the same binary: same seeds, same hash)
//        [--sprint]   (lane A72, ask A3: the playtester presses sprint in every cell, -playersprint)
//        [--dir <folder>]   (lane A122: measure the *.cartridge.json files of another folder, e.g. variants under tools/playtest/out/; the slug is the file name
//                            after "hockey.mode." up to the first dot, so hockey.mode.cage.v2.cartridge.json is measured with Cage's number)
//
// Lane A122 (track M1): a mode may ask for its own run in RUN below (Marathon: the playtester sprints, and its regular baseline sprints too); a timer mode's
// frame cap is its clock plus the sim's 6,000 frames of pauses and sudden death (JamSim.cpp HardStop), so a long clock is never read as unfinished; the table
// carries the ratio, the match length and the human's knockdowns; a win rate outside 15 to 85 % is flagged (a note, not a failure: the level is the ladder's).
//
// Lane A139 (track M1, evolution 2): Marathon's and Power Play's numbers are per play minute (a clock is a length, not a character); numbers in docs/experiments/MODES-4.md.
//
// Lane A149 (track M1, evolution 3): the twelve on the set ladder (docs/experiments/MODES-5.md). The table carries the share of matches with 30 s without a
// human touch, and a novice run (--player novice) is read against the novice's floor (10 % or more) instead of the 15 to 85 % band. Both are notes, not failures.
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
  // lane A139 (track M1, evolution 2): per play minute, not per match: an eight-minute match has 2.2x the minutes of a regular one, so the per-match count read the
  // clock, not the legs (MODES-3.md: 2.29x a match was 1.03x a minute).
  marathon: { label: 'human sprint frames / play min', of: (ms) => perPlayMin(ms, (m) => m.sprint_frames || 0) },
  'marathon-pace': { label: 'minutes per goal', of: (ms) => { const g = ms.reduce((a, m) => a + m.goals[0] + m.goals[1], 0); return g ? ms.reduce((a, m) => a + m.frames, 0) / 3600 / g : 0; } },
  // lane A139: per play minute, as Pond: a four-minute clock against a first-to-three is a length, not a character
  'power-play': { label: 'passes + received / play min', of: (ms) => perPlayMin(ms, (m) => (m.passes || 0) + (m.passes_received || 0)) },
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
  // lane A84 (R-0041 point 3): the abilities on F. Shotgun's character is the bodies on the ice (every knockdown, any skater, any cause, per match: the shotgun drops
  // every skater within a metre of its path); Arsenal's is the tricks used (the human's ability uses per match: regular has none). Both need the playtester's F
  // (JamPlaytester's PlaytesterAbility) and the binary that writes `knockdowns_all` and `abilities_used`.
  // lane A122 (track M1): per play minute, not per match: a Shotgun match is a third shorter than a regular one, so the per-match count read the length, not the blast.
  shotgun: { label: 'knockdowns (all skaters) / play min', of: (ms) => perPlayMin(ms, (m) => m.knockdowns_all || 0) },
  arsenal: { label: 'human abilities used / match', of: (ms) => avg(ms.map((m) => m.abilities_used || 0)) },
};

// Lane A122: what a mode's cell needs beyond the defaults. sprint: the playtester presses sprint (the mode is about the sprint) and the mode is compared with a
// regular cell that sprints too.
export const RUN = {
  marathon: { sprint: true },
};
// The frame cap of one cell: a timer match's hard stop is max(33000, clock + 6000) frames (JamSim.cpp), 600 frames of margin on top.
export const capFor = (cart, maxFrames) => (cart?.game?.rules?.mode === 'timer' ? Math.max(maxFrames, (cart.game.rules.timer_frames || 0) + 6600) : maxFrames);
export const WIN_BAND = [15, 85];
export const NOVICE_FLOOR = 10;   // lane A149: a newcomer who picks a mode off the menu wins one match in ten or more
export const bandFor = (player) => (player === 'novice' ? [NOVICE_FLOOR, 100] : WIN_BAND);

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

export function listModes(dir = modesDir) {
  if (!existsSync(dir)) return [];
  const shipped = resolve(dir) === resolve(modesDir);
  return readdirSync(dir).filter((f) => f.endsWith('.cartridge.json')).sort().map((f) => {
    const id = f.replace(/^hockey\.mode\./, '').replace(/\.cartridge\.json$/, '');
    // a variant file "hockey.mode.cage.v2.cartridge.json" is Cage (slug) under its own row name (id)
    const slug = shipped ? id : id.split('.')[0];
    const cart = JSON.parse(readFileSync(join(dir, f), 'utf8'));
    // the commandlet takes a cartridge name under Content/Cartridges or a file path (forward slashes: it is echoed into the JSON lines)
    const name = shipped ? `modes/${f.replace(/\.cartridge\.json$/, '')}` : join(dir, f).split('\\').join('/');
    return { slug, id, file: join(dir, f), name, title: cart.title, level: cart.game?.ai?.difficulty || 'pro', cart };
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
const winPct = (ms) => (ms.length ? r1((100 * ms.filter((m) => m.result === 'win').length) / ms.length) : 0);
const downs = (ms) => r2(avg(ms.map((m) => m.knockdowns || 0)));
const noTouch30 = (ms) => (ms.length ? r1((100 * ms.filter((m) => (m.max_no_touch || 0) >= 1800).length) / ms.length) : 0);

if (process.argv[1] && resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
  const only = arg('modes', '');
  const seeds = arg('seeds', '1-20');
  const jobs = Number(arg('jobs', '3'));
  const player = arg('player', 'regular');
  const maxFrames = Number(arg('max-frames', '36000'));
  const uproject = arg('uproject', join(repo, 'Game', 'ThirdJam.uproject'));
  const outBase = arg('out', join(here, 'out', `modes-${stamp()}`));
  const baseline = arg('baseline', '');
  const dir = arg('dir', '');
  const sprintAll = process.argv.includes('--sprint');
  if (!/^\d+(-\d+)?$/.test(seeds)) die(2, `bad --seeds ${seeds}`);
  if (!existsSync(uproject)) die(2, `no uproject ${uproject}`);
  if (dir && !existsSync(dir)) die(2, `no --dir ${dir}`);
  let modes = listModes(dir ? resolve(dir) : modesDir);
  if (only) modes = modes.filter((m) => only.split(',').includes(m.slug) || only.split(',').includes(m.id));
  if (!modes.length) die(2, `no mode cartridges in ${dir || modesDir}${only ? ` matching ${only}` : ''}`);
  let failed = 0;
  for (const m of modes) {
    const res = validateCartridge(m.cart);
    if (res.errors.length) { failed++; m.invalid = true; console.log(`FAIL validate ${m.id}: ${res.errors.join(' | ')}`); }
    else console.log(`PASS validate ${m.id}`);
    m.sprint = sprintAll || Boolean(RUN[m.slug]?.sprint);
    m.base = `${m.level}${m.sprint ? '+sprint' : ''}`;
  }
  // One regular cell per level (and per level with the sprint, when a mode runs with it), reused from an earlier run when given and complete.
  const wanted = [...new Set(modes.filter((m) => !m.invalid).map((m) => m.base))];
  const reused = {};
  if (baseline) {
    if (!existsSync(baseline)) die(2, `no baseline ${baseline}`);
    for (const l of readFileSync(baseline, 'utf8').split(/\r?\n/).filter(Boolean)) {
      const m = JSON.parse(l);
      if (m.mode === 'regular' && m.player === player) (reused[`${m.difficulty}${m.sprint ? '+sprint' : ''}`] ??= []).push(m);
    }
  }
  const [lo, hi] = seeds.includes('-') ? seeds.split('-').map(Number) : [Number(seeds), Number(seeds)];
  // a baseline measured on other seeds is not this run's baseline: keep the matches of this run's seeds, and measure again when one is missing
  for (const k of Object.keys(reused)) {
    reused[k] = reused[k].filter((m) => m.seed >= lo && m.seed <= hi);
    if (reused[k].length !== hi - lo + 1) delete reused[k];
  }
  const cells = [
    ...wanted.filter((b) => !reused[b]).map((b) => ({ kind: 'regular', name: 'hockey.regular', level: b.split('+')[0], sprint: b.endsWith('+sprint'), base: b, cap: maxFrames })),
    ...modes.filter((m) => !m.invalid).map((m) => ({ kind: 'mode', name: m.name, level: m.level, sprint: m.sprint, base: m.base, cap: capFor(m.cart, maxFrames), mode: m })),
  ];
  console.log(`INFO modes ${cells.length} cells (${cells.filter((c) => c.kind === 'regular').length} regular baselines measured, ${Object.keys(reused).filter((b) => wanted.includes(b)).length} reused) x seeds ${seeds}, player ${player}, ${jobs} at a time`);
  const t0 = Date.now();
  const done = await pool(cells, jobs, async (c) => {
    try {
      const ms = (await runCell({ uproject, cartridge: c.name, level: c.level, player, seeds, maxFrames: c.cap, sprint: c.sprint })).map((m) => ({ ...m, sprint: c.sprint || undefined }));
      console.log(`INFO cell ${c.kind === 'regular' ? `regular ${c.base}` : c.mode.id} done (${ms.length} matches, ${Math.round((Date.now() - t0) / 1000)} s)`);
      return { ...c, ms, s: summarize(c.kind === 'regular' ? 'regular' : c.mode.slug, c.name, c.level, player, ms) };
    } catch (e) {
      failed++;
      console.log(`FAIL cell ${c.name} ${c.level}: ${e.message}`);
      return null;
    }
  });
  for (const [b, ms] of Object.entries(reused)) if (wanted.includes(b)) done.push({ kind: 'regular', name: 'hockey.regular', level: b.split('+')[0], base: b, ms, s: summarize('regular', 'hockey.regular', b.split('+')[0], player, ms), reused: true });
  const base = Object.fromEntries(done.filter((d) => d && d.kind === 'regular').map((d) => [d.base, d]));
  const results = [];
  const lines = [
    '| mode | level | character number | mode | regular | ratio | a third apart | win % mode / regular | win band | match s mode / regular | human knockdowns / match mode / regular | 30 s without the puck, % of matches mode / regular | unfinished |',
    '|---|---|---|---|---|---|---|---|---|---|---|---|---|',
  ];
  for (const d of done.filter((x) => x && x.kind === 'mode')) {
    const ch = CHARACTER[d.mode.slug] || CHARACTER[d.mode.slug.split('-')[0]] || CHARACTER.sniper;
    const b = base[d.base];
    const mine = ch.of(d.ms);
    const theirs = b ? ch.of(b.ms) : NaN;
    const change = theirs > 0 ? mine / theirs - 1 : (mine > 0 ? Infinity : 0);
    const ok = Boolean(b) && Math.abs(change) >= 1 / 3 && d.s.unfinished === 0;
    if (!ok) failed++;
    const pctChange = Number.isFinite(change) ? Math.round(change * 100) : null;
    const ratio = Number.isFinite(change) ? r2(1 + change) : null;
    const win = winPct(d.ms);
    const band = bandFor(player);
    const inBand = win >= band[0] && win <= band[1];
    const r = { mode: d.mode.id, title: d.mode.title, level: d.level, sprint: d.sprint || undefined, character: ch.label, value: r2(mine), regular_value: r2(theirs), ratio, change_pct: pctChange,
      verdict: ok ? 'pass' : 'fail', win_pct: win, regular_win_pct: b ? winPct(b.ms) : undefined, win_in_band: inBand, match_s: d.s.match_s, regular_match_s: b?.s.match_s,
      human_knockdowns: downs(d.ms), regular_human_knockdowns: b ? downs(b.ms) : undefined, no_touch30_pct: noTouch30(d.ms), regular_no_touch30_pct: b ? noTouch30(b.ms) : undefined, unfinished: d.s.unfinished, mode_cell: cell(d.s), regular_cell: b ? cell(b.s) : undefined };
    results.push(r);
    console.log(`${ok ? 'PASS' : 'FAIL'} mode ${d.mode.id} (${d.base}): ${ch.label} ${r.value} vs regular ${r.regular_value} (${signed(pctChange)}), win ${win} % vs ${r.regular_win_pct} %${inBand ? '' : ` (OUTSIDE ${band[0]}-${band[1]})`}, unfinished ${d.s.unfinished}`);
    lines.push(`| ${d.mode.id} | ${d.base} | ${ch.label} | **${r.value}** | ${r.regular_value} | ${ratio === null ? 'n/a' : ratio} | ${ok ? 'yes' : '**NO**'} | ${win} / ${r.regular_win_pct} | ${inBand ? 'in' : '**out**'} | ${d.s.match_s} / ${b?.s.match_s} | ${r.human_knockdowns} / ${r.regular_human_knockdowns} | ${r.no_touch30_pct} / ${r.regular_no_touch30_pct} | ${d.s.unfinished} |`);
  }
  mkdirSync(dirname(outBase), { recursive: true });
  writeFileSync(`${outBase}.jsonl`, done.filter(Boolean).flatMap((d) => d.ms.map((m) => JSON.stringify({ ...m, mode: d.kind === 'mode' ? d.mode.id : 'regular' }))).join('\n') + '\n');
  writeFileSync(`${outBase}.md`, `${lines.join('\n')}\n`);
  console.log('');
  console.log(lines.join('\n'));
  console.log(`INFO modes ${Math.round((Date.now() - t0) / 1000)} s; lines ${outBase}.jsonl, table ${outBase}.md`);
  console.log(JSON.stringify({ tool: 'thirdjam.modes/1', verdict: failed ? 'fail' : 'pass', exit: failed ? 1 : 0, seeds, player, modes: results, out: `${outBase}.jsonl` }));
  process.exit(failed ? 1 : 0);
}
