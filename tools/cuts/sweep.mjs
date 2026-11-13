#!/usr/bin/env node
// Lane A37 (R-0028 point 3): every shipped cut plays. Runs the 30 offline Designer cuts (Game/Content/Designer/cached-cuts.json), the
// season takes and their fallbacks (Game/Content/Cartridges/ladder/) and the shipped presets (hockey.*.cartridge.json) through the
// JamCutSweep commandlet (real UJamSim with rules, zones and effects wired as the game mode wires them; the real brain at the cut's
// own level, the scripted human), N seeds each, and writes a verdict per cut.
//
//   node tools/cuts/sweep.mjs [--seeds 1-5] [--max-frames 36000] [--jobs 3] [--only <substring>] [--group cuts,ladder,presets]
//                             [--difficulty pro] [--uproject <path>] [--out tools/cuts/out]
//
// Verdicts: plays | ends late (a seed ran past --late-frames, default 18000 = 5 min, or ended in some seeds only) | no human touch (a faceoff
// left the scripted human without a touch for 600 frames, or the human averaged under 3 touches) | never ends (no seed ended inside the
// window) | loader rejects (UJamCartridge::LoadFromFile said no) | no goals (ended by its clock with no goal in any seed: the bots did not score).
// Writes <out>/sweep.jsonl (every match line), <out>/sweep.json (per-cut rows + counts) and <out>/sweep.md (the table).
// Exit: 0 every cut plays, 1 a cut does not play (the table says which), 2 could not run.
import { readFileSync, existsSync, mkdirSync, writeFileSync, readdirSync } from 'node:fs';
import { spawn } from 'node:child_process';
import { join, dirname, resolve, basename } from 'node:path';
import { fileURLToPath } from 'node:url';

const here = dirname(fileURLToPath(import.meta.url));
const repo = resolve(here, '..', '..');
const cartDir = join(repo, 'Game', 'Content', 'Cartridges');
const cutsFile = join(repo, 'Game', 'Content', 'Designer', 'cached-cuts.json');

function arg(name, dflt) {
  const i = process.argv.indexOf(`--${name}`);
  return i > 0 && i + 1 < process.argv.length ? process.argv[i + 1] : dflt;
}
function die(code, msg) {
  console.log(`FAIL cuts ${msg}`);
  console.log(JSON.stringify({ tool: 'thirdjam.cuts/1', verdict: code === 2 ? 'could_not_run' : 'fail', exit: code, error: msg }));
  process.exit(code);
}
export const slug = (s) => s.toLowerCase().replace(/[^a-z0-9]+/g, '-').replace(/^-|-$/g, '');

export function engineRoot() {
  if (process.env.UE_ROOT && existsSync(process.env.UE_ROOT)) return process.env.UE_ROOT;
  const dflt = 'C:/Program Files/Epic Games/UE_5.9';
  return existsSync(dflt) ? dflt : null;
}

// Every shipped cut as { id, group, name, file, cartridge }: the Designer cuts are written out of the cache so the loader reads a file.
export function collectCuts(outDir, groups) {
  const list = [];
  if (groups.includes('cuts')) {
    const cache = JSON.parse(readFileSync(cutsFile, 'utf8'));
    const dir = join(outDir, 'cuts');
    mkdirSync(dir, { recursive: true });
    for (const c of cache.cuts) {
      const file = join(dir, `${slug(c.name)}.cartridge.json`);
      writeFileSync(file, `${JSON.stringify(c.cartridge, null, 2)}\n`);
      list.push({ id: slug(c.name), group: 'cuts', name: c.name, sentence: c.sentence, file, cartridge: c.cartridge });
    }
  }
  if (groups.includes('ladder')) {
    const ladder = JSON.parse(readFileSync(join(cartDir, 'ladder', 'season.ladder.json'), 'utf8')).ladder.takes;
    for (const t of ladder) {
      for (const id of [t.cartridge, t.fallback].filter(Boolean)) {
        const file = join(cartDir, 'ladder', `${id}.cartridge.json`);
        const cartridge = JSON.parse(readFileSync(file, 'utf8'));
        list.push({ id, group: id.endsWith('-fallback') ? 'ladder-fallback' : 'ladder', name: cartridge.title, sentence: t.twist_text, file, cartridge });
      }
    }
  }
  if (groups.includes('presets')) {
    for (const f of readdirSync(cartDir).filter((f) => /^hockey\..*\.cartridge\.json$/.test(f)).sort()) {
      const file = join(cartDir, f);
      const cartridge = JSON.parse(readFileSync(file, 'utf8'));
      list.push({ id: f.replace(/\.cartridge\.json$/, ''), group: f.includes('.test.') ? 'preset-test' : 'presets', name: cartridge.title, sentence: '', file, cartridge });
    }
  }
  return list;
}

function parseLines(stdout) {
  const out = [];
  for (const l of (stdout || '').split(/\r?\n/)) {
    const i = l.indexOf('{"tool":"thirdjam.tournament/1"');
    if (i < 0) continue;
    try { out.push(JSON.parse(l.slice(i))); } catch { /* a truncated log line */ }
  }
  return out;
}

function runCommandlet({ uproject, file, seeds, maxFrames, difficulty, timeoutMs }) {
  const ue = engineRoot();
  if (!ue) die(2, 'Unreal Engine not found (set UE_ROOT)');
  const cmd = join(ue, 'Engine', 'Binaries', 'Win64', 'UnrealEditor-Cmd.exe');
  const args = [uproject, '-run=JamCutSweep', `-cartridge=${file.split('\\').join('/')}`, `-seeds=${seeds}`, `-maxframes=${maxFrames}`,
    '-nullrhi', '-Multiprocess', '-unattended', '-nopause', '-nosplash', '-stdout', '-FullStdOutLogOutput'];
  if (difficulty) args.push(`-difficulty=${difficulty}`);
  return new Promise((res) => {
    const p = spawn(cmd, args, { windowsHide: true });
    let out = '', err = '';
    const timer = setTimeout(() => { p.kill(); }, timeoutMs);   // only the process this sweep started
    p.stdout.on('data', (d) => { out += d; });
    p.stderr.on('data', (d) => { err += d; });
    p.on('close', (code) => { clearTimeout(timer); res({ code, lines: parseLines(out), err: err.slice(-1000) }); });
    p.on('error', (e) => { clearTimeout(timer); res({ code: -1, lines: [], err: e.message }); });
  });
}

const mean = (a) => (a.length ? a.reduce((s, x) => s + x, 0) / a.length : 0);
const f1 = (x) => (Math.round(x * 10) / 10).toFixed(1);

// The human's team from the cartridge (role "player"); the stats' goals[] are per team in cartridge order.
export function humanTeam(cartridge) {
  const i = (cartridge.game?.teams || []).findIndex((t) => t.skaters.some((s) => s.role === 'player'));
  return i < 0 ? 0 : i;
}

// One row per cut from its match lines. lateFrames: a match longer than this is "late".
export function judge(cut, lines, { maxFrames, lateFrames }) {
  const row = { id: cut.id, group: cut.group, name: cut.name, sentence: cut.sentence, file: cut.file.split('\\').join('/') };
  if (lines.length === 0) { return { ...row, verdict: 'loader rejects', detail: 'the commandlet printed no match line (crash or timeout)' }; }
  if (lines[0].load_error) { return { ...row, verdict: 'loader rejects', detail: lines[0].load_error }; }
  const ht = humanTeam(cut.cartridge);
  const sw = lines[0].sweep || {};
  const over = lines.filter((m) => m.over).length;
  const wins = lines.filter((m) => m.over && m.winner === ht).length;
  const losses = lines.filter((m) => m.over && m.winner === 1 - ht).length;
  const draws = lines.filter((m) => m.over && m.winner < 0).length;
  const goals = lines.map((m) => m.goals[0] + m.goals[1]);
  const touches = lines.map((m) => m.touches[m.player] ?? 0);
  const frames = lines.map((m) => m.frames);
  const noTouchFaceoffs = lines.reduce((s, m) => s + m.faceoffs_without_player_touch, 0);
  const pickupsTaken = lines.reduce((s, m) => s + (m.sweep?.pickups_taken ?? 0), 0);
  const humanPickups = lines.reduce((s, m) => s + (m.sweep?.human_pickups ?? 0), 0);
  const ruleFires = lines.reduce((s, m) => s + (m.sweep?.rule_fires ?? 0), 0);
  const periods = Math.max(...lines.map((m) => m.sweep?.periods ?? 1));
  const causes = [...new Set(lines.map((m) => m.sweep?.end_cause || 'none'))].join('/');
  const outside = lines.filter((m) => m.max_board_distance > 0.01).length;
  const effectErrors = lines[0].sweep?.effect_errors || '';
  let verdict = 'plays';
  const why = [];
  // Late: a seed that never ended; a first_to cut whose mean runs past lateFrames or whose worst seed runs past 1.5 x lateFrames; a timer cut whose
  // worst seed runs past 1.5 x its clock (frames count goal pauses and faceoffs, so a 3:00 clock is about 11,900 frames). "long" is a note only.
  const worst = Math.max(...frames);
  const isTimer = sw.rules_mode === 'timer' && sw.timer_frames > 0;
  const late = over < lines.length || (isTimer ? worst > sw.timer_frames * 1.5 : mean(frames) > lateFrames || worst > lateFrames * 1.5);
  if (over === 0) { verdict = 'never ends'; why.push(`no seed ended inside ${maxFrames} frames`); }
  else if (late) { verdict = 'ends late'; why.push(`${lines.length - over} of ${lines.length} did not end; mean ${Math.round(mean(frames))}, longest ${worst} frames`); }
  else if (worst > lateFrames) why.push(`long: worst seed ${worst} frames`);
  if (noTouchFaceoffs > 0 || mean(touches) < 3) { if (verdict === 'plays') verdict = 'no human touch'; why.push(`${noTouchFaceoffs} faceoffs without a human touch in 600 frames; human touches ${f1(mean(touches))} per match`); }
  if (goals.every((g) => g === 0)) { if (verdict === 'plays') verdict = 'no goals'; why.push('no goal in any seed'); }
  if (effectErrors) why.push(`effect system: ${effectErrors}`);
  if (outside) why.push(`${outside} matches had a skater outside the boards`);
  return {
    ...row, verdict, detail: why.join('; '), level: sw.level, rules_mode: sw.rules_mode, score_to_win: sw.score_to_win, timer_frames: sw.timer_frames,
    seeds: lines.length, over, wins, losses, draws, goals_mean: +f1(mean(goals)), goals_human_mean: +f1(mean(lines.map((m) => m.goals[ht]))),
    frames_mean: Math.round(mean(frames)), frames_max: Math.max(...frames), human_touches_mean: +f1(mean(touches)), human_shots_mean: +f1(mean(lines.map((m) => m.shots[m.player] ?? 0))),
    faceoffs: lines.reduce((s, m) => s + m.faceoffs, 0), faceoffs_without_touch: noTouchFaceoffs, max_faceoff_to_touch: Math.max(...lines.map((m) => m.max_faceoff_to_touch)),
    pickups_spawned: lines.reduce((s, m) => s + (m.sweep?.pickups_spawned ?? 0), 0), pickups_taken: pickupsTaken, human_pickups: humanPickups,
    effects_started: lines.reduce((s, m) => s + (m.sweep?.effects_started ?? 0), 0), hooks: sw.hooks ?? 0, rule_fires: ruleFires,
    end_match_actions: lines.reduce((s, m) => s + (m.sweep?.end_match_actions ?? 0), 0), zones: sw.zones ?? 0, zone_enters: lines.reduce((s, m) => s + (m.sweep?.zone_enters ?? 0), 0),
    periods, end_cause: causes, checks: lines.reduce((s, m) => s + (m.sweep?.checks ?? 0), 0), shots_blocked: lines.reduce((s, m) => s + (m.sweep?.shots_blocked ?? 0), 0),
    rink_m: lines[lines.length - 1].sweep?.rink_m, outside_rink: outside, effect_errors: effectErrors, hashes: lines.map((m) => m.hash),
  };
}

export function table(rows) {
  const head = ['cut', 'group', 'level', 'rules', 'seeds', 'W-L-D', 'ended', 'end by', 'frames mean/max', 'goals', 'human touches', 'no-touch faceoffs', 'pickups taken (human)', 'hooks/fires', 'zones/enters', 'periods', 'verdict', 'detail'];
  const body = rows.map((r) => r.verdict === 'loader rejects'
    ? [r.name, r.group, '', '', '', '', '', '', '', '', '', '', '', '', '', '', r.verdict, r.detail]
    : [r.name, r.group, r.level, r.rules_mode === 'timer' ? `timer ${r.timer_frames}` : `first to ${r.score_to_win}`, String(r.seeds), `${r.wins}-${r.losses}-${r.draws}`, `${r.over}/${r.seeds}`, r.end_cause,
      `${r.frames_mean}/${r.frames_max}`, String(r.goals_mean), String(r.human_touches_mean), String(r.faceoffs_without_touch), `${r.pickups_taken} (${r.human_pickups})`, `${r.hooks}/${r.rule_fires}`, `${r.zones}/${r.zone_enters}`, String(r.periods), r.verdict, r.detail]);
  const line = (cells) => `| ${cells.map((c) => String(c).replace(/\|/g, '\\|')).join(' | ')} |`;
  return [line(head), `|${head.map(() => '---').join('|')}|`, ...body.map(line)].join('\n');
}

export function counts(rows) {
  const c = { cuts: rows.length, plays: 0, 'ends late': 0, 'no human touch': 0, 'never ends': 0, 'loader rejects': 0, 'no goals': 0 };
  for (const r of rows) c[r.verdict] = (c[r.verdict] || 0) + 1;
  return c;
}

async function main() {
  const seeds = arg('seeds', '1-5');
  const maxFrames = Number(arg('max-frames', '36000'));
  const lateFrames = Number(arg('late-frames', '18000'));
  const jobs = Math.max(1, Number(arg('jobs', '3')));
  const only = arg('only', null);
  const groups = arg('group', 'cuts,ladder,presets').split(',');
  const difficulty = arg('difficulty', null);
  const uproject = arg('uproject', join(repo, 'Game', 'ThirdJam.uproject'));
  const outDir = resolve(arg('out', join(here, 'out')));
  if (!/^\d+(-\d+)?$/.test(seeds)) die(2, `bad --seeds ${seeds}`);
  if (!existsSync(uproject)) die(2, `no uproject ${uproject}`);
  mkdirSync(outDir, { recursive: true });
  // --file a.json[,b.json]: sweep these cartridge files instead (how a hand fix is measured before it goes into the library).
  const files = arg('file', null);
  let cuts = files
    ? files.split(',').map((f) => { const file = resolve(f); const cartridge = JSON.parse(readFileSync(file, 'utf8')); return { id: basename(file).replace(/\.cartridge\.json$/, ''), group: 'file', name: cartridge.title, sentence: '', file, cartridge }; })
    : collectCuts(outDir, groups);
  if (only) cuts = cuts.filter((c) => c.id.includes(only) || c.name.toLowerCase().includes(only.toLowerCase()));
  if (cuts.length === 0) die(2, 'no cut matched');
  console.log(`INFO cuts ${cuts.length} cuts, seeds ${seeds}, ${maxFrames} frames max, ${jobs} commandlets at a time`);
  const t0 = Date.now();
  const rows = new Array(cuts.length);
  const all = [];
  let next = 0;
  async function worker() {
    while (next < cuts.length) {
      const k = next++;
      const cut = cuts[k];
      const s0 = Date.now();
      const r = await runCommandlet({ uproject, file: cut.file, seeds, maxFrames, difficulty, timeoutMs: 40 * 60 * 1000 });
      for (const m of r.lines) all.push({ cut: cut.id, group: cut.group, ...m });
      rows[k] = judge(cut, r.lines, { maxFrames, lateFrames });
      if (r.code !== 0 && r.lines.length === 0) rows[k].detail += ` (exit ${r.code}: ${r.err.trim().split('\n').pop() || 'no stderr'})`;
      console.log(`${rows[k].verdict === 'plays' ? 'PASS' : 'FAIL'} cut ${cut.name} [${cut.group}]: ${rows[k].verdict}${rows[k].detail ? ` (${rows[k].detail})` : ''} in ${Math.round((Date.now() - s0) / 1000)} s`);
    }
  }
  await Promise.all(Array.from({ length: jobs }, worker));
  const c = counts(rows);
  const md = table(rows);
  writeFileSync(join(outDir, 'sweep.jsonl'), all.map((m) => JSON.stringify(m)).join('\n') + '\n');
  writeFileSync(join(outDir, 'sweep.json'), JSON.stringify({ tool: 'thirdjam.cuts/1', seeds, max_frames: maxFrames, late_frames: lateFrames, difficulty: difficulty || 'the cut\'s own', counts: c, rows }, null, 2) + '\n');
  writeFileSync(join(outDir, 'sweep.md'), `${md}\n`);
  console.log('');
  console.log(md);
  const bad = rows.filter((r) => r.verdict !== 'plays').length;
  console.log(`${bad ? 'FAIL' : 'PASS'} cuts ${c.plays} of ${rows.length} play (${Object.entries(c).filter(([k, v]) => k !== 'cuts' && k !== 'plays' && v).map(([k, v]) => `${v} ${k}`).join(', ') || 'nothing else'}) in ${Math.round((Date.now() - t0) / 1000)} s`);
  console.log(JSON.stringify({ tool: 'thirdjam.cuts/1', verdict: bad ? 'fail' : 'pass', exit: bad ? 1 : 0, seeds, counts: c, out: outDir.split('\\').join('/') }));
  process.exit(bad ? 1 : 0);
}

if (process.argv[1] && resolve(process.argv[1]) === fileURLToPath(import.meta.url)) main();
