#!/usr/bin/env node
// Lane A54: the eyes regression grid. One launch per (cut x look tier) plus one per menu screen, each driven by shoot.ps1;
// stills are named <cut>-<tier>-<moment>.png (1080p PNG in tools/qa/out/eyesgrid/<run>/<cell>/, JPEG q88 copy in docs/experiments/img/eyesgrid/<run>/).
//   node tools/qa/eyesgrid/grid.mjs shoot [--run <name>] [--pkg <packaged Windows dir or exe>] [--only <substr>] [--dev]
//   node tools/qa/eyesgrid/grid.mjs eyes  [--run <name>] [--preset <p>] [--force]   (bug-hunt on every still, readability on game-camera stills, hud on HUD stills)
// Shipping has no log and no console: moments are reached by timed keys (ENTER = play, W = skate to the far net, ESC = pause, DOWN ENTER = Director's Call).
// --dev: a Development package (log + console): the same cells plus the goal still, triggered by the JAMEVENT goal line with jam.ScriptHuman 1.
import { spawnSync } from 'node:child_process';
import { existsSync, mkdirSync, readFileSync, writeFileSync, readdirSync } from 'node:fs';
import { join, resolve, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';

const here = dirname(fileURLToPath(import.meta.url));
const root = resolve(here, '..', '..', '..');
const argv = process.argv.slice(2);
const cmd = argv[0];
const opt = (n, d) => { const i = argv.indexOf('--' + n); return i < 0 ? d : (argv[i + 1] && !argv[i + 1].startsWith('--') ? argv[i + 1] : true); };
const run = opt('run', 'h19');
const pkg = opt('pkg', 'C:\\Users\\kazda\\kiro\\thirdjam\\Game\\Packaged\\Hokej_Hokej_H19\\Windows');
const dev = !!opt('dev', false);
const B = dev ? 6 : 0;   // a Development exe needs about 6 s longer to reach the menu
const outDir = join(root, 'tools', 'qa', 'out', 'eyesgrid', run);
const imgDir = join(root, 'docs', 'experiments', 'img', 'eyesgrid', run);
const jsonDir = join(root, 'docs', 'experiments', 'eyes');
mkdirSync(outDir, { recursive: true }); mkdirSync(imgDir, { recursive: true });

const cutDir = join(root, 'Game', 'Content', 'Cartridges');
const allCuts = readdirSync(cutDir).filter((f) => f.endsWith('.cartridge.json') && !f.includes('.test.')).map((f) => f.replace('.cartridge.json', ''));
const tiers = ['l1_day', 'l2_night', 'l3_neon', 'p3_clay', 'p4_broadcast'];
const tierCuts = ['hockey.regular', 'hockey.boost'];   // photographed at every tier; the other cuts at their own tier
const cells = [];
for (const c of allCuts) {
  const own = JSON.parse(readFileSync(join(cutDir, c + '.cartridge.json'), 'utf8')).game?.look?.tier ?? 'l2_night';
  const len = JSON.parse(readFileSync(join(cutDir, c + '.cartridge.json'), 'utf8')).game?.rink?.length_m ?? 90;
  cells.push({ cut: c, tier: own, len, own: true });
  if (tierCuts.includes(c)) for (const t of tiers) if (t !== own) cells.push({ cut: c, tier: t, len });
}
const shortTier = (t) => t.split('_')[0];   // l1 l2 l3 p3 p4

function cartFor(cell) {
  const src = join(cutDir, cell.cut + '.cartridge.json');
  const dst = join(outDir, 'cuts', `${cell.cut}.${cell.tier}.cartridge.json`);
  mkdirSync(dirname(dst), { recursive: true });
  const j = JSON.parse(readFileSync(src, 'utf8'));
  j.game = { ...(j.game ?? {}), look: { ...(j.game?.look ?? {}), tier: cell.tier } };   // look lives under game (an unknown top-level key rejects the cut and falls back to regular)
  writeFileSync(dst, JSON.stringify(j, null, 2));
  return dst.replace(/\\/g, '/');
}

function launch(name, plan, gameArgs, exe, log) {
  const dir = join(outDir, name);
  mkdirSync(dir, { recursive: true });
  const pf = join(dir, 'plan.json');
  writeFileSync(pf, JSON.stringify(plan));
  const args = ['-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', join(here, 'shoot.ps1'), '-Exe', exe, '-PlanFile', pf, '-OutDir', dir, '-GameArgs', gameArgs];
  if (log) args.push('-Log', log);
  const r = spawnSync('powershell', args, { encoding: 'utf8', timeout: 300000 });
  return { dir, ok: r.status === 0 };
}

function jpeg(png, jpg) { spawnSync('ffmpeg', ['-loglevel', 'error', '-y', '-i', png, '-q:v', '3', jpg]); }

if (cmd === 'shoot') {
  const sh = join(pkg, 'ThirdJam', 'Binaries', 'Win64', 'ThirdJam-Win64-Shipping.exe');
  const exe = pkg.endsWith('.exe') ? pkg : (existsSync(sh) ? sh : join(pkg, 'ThirdJam', 'Binaries', 'Win64', 'ThirdJam.exe'));
  const only = opt('only', '');
  const manifest = [];
  const keep = (dir, from, to, cut, tier) => {
    const p = join(dir, from + '.png');
    if (!existsSync(p)) { console.log('MISSING', to); return; }
    jpeg(p, join(imgDir, to + '.jpg'));
    manifest.push({ still: to + '.jpg', moment: from, cut, tier });
  };
  // menu screens: one launch each (ESC does not reliably return to the main list), DOWN n then ENTER
  const screens = opt('own', false) ? [] : [['main', 0], ['cuts', 1], ['direct', 2], ['friend', 3], ['settings', 4], ['credits', 5]];
  for (const [s, n] of screens) {
    if (only && !('menu-' + s).includes(only)) continue;
    const steps = [];
    for (let i = 0; i < n; i++) steps.push({ at: B + 11 + i * 0.5, keys: '{DOWN}' });
    if (n) steps.push({ at: B + 11.2 + n * 0.5, keys: '{ENTER}' });
    steps.push({ at: B + 13.5 + n * 0.5, shot: 'screen' });
    const r = launch(`menu-${s}`, { seconds: B + 15.5 + n * 0.5, steps }, '', exe);
    keep(r.dir, 'screen', `menu-${s}-screen`, 'menu', s);
    console.log('menu', s, r.ok);
  }
  for (const cell of cells) {
    const tag = `${cell.cut}-${shortTier(cell.tier)}`;
    if (only && !tag.includes(only)) continue;
    if (opt('own', false) && !cell.own) continue;   // --own: only each cut at its own look tier
    const cart = cartFor(cell);
    const arrive = 19.2 + (cell.len / 2 + 3) / 7.3;   // measured (PostMessage-held W, no foreground): about 7.3 m/s over the first 40 m; the far net stands len/2 + 4 m from the faceoff dot
    const steps = [
      { at: B + 12, keys: '{ENTER}' }, { at: B + 15, shot: 'card' }, { at: B + 19, shot: 'faceoff' }, { at: B + 19.2, down: 'W' },
      { at: B + arrive - 0.3, shot: 'farnet' }, { at: B + arrive + 0.5, up: 'W' }, { at: B + arrive + 1.5, keys: '{ESC}' }, { at: B + arrive + 3, shot: 'pause' },
      { at: B + arrive + 3.5, keys: '{DOWN}' }, { at: B + arrive + 4.1, keys: '{ENTER}' }, { at: B + arrive + 6, shot: 'call' },
    ];
    let seconds = B + arrive + 8, args = `-cut=${cart}`, log, triggers;
    if (dev) {
      // Development: scripted human plays itself; a goal anywhere in the first 150 s is photographed 0.8 s after the JAMEVENT line.
      log = join(outDir, tag, 'game.log');
      args += ' -dpcvars=jam.ScriptHuman=1';
      steps.length = 0;
      steps.push({ at: B + 12, keys: '{ENTER}' }, { at: B + 15, shot: 'card' }, { at: B + 19, shot: 'faceoff' });
      seconds = 150; triggers = [{ on: 'JAMEVENT goal ', after: 0.8, shot: 'goal', max: 1, skip: 0 }];
    }
    const r = launch(tag, { seconds, steps, triggers }, args, exe, log);
    for (const m of ['card', 'faceoff', 'farnet', 'goal', 'pause', 'call']) if (existsSync(join(r.dir, m + '.png'))) keep(r.dir, m, `${cell.cut}-${shortTier(cell.tier)}-${m}`, cell.cut, shortTier(cell.tier));
    console.log('cell', tag, r.ok);
  }
  const mf = join(outDir, 'manifest.json');
  const prev = existsSync(mf) ? JSON.parse(readFileSync(mf, 'utf8')) : [];
  const names = new Set(manifest.map((m) => m.still));
  writeFileSync(mf, JSON.stringify([...prev.filter((m) => !names.has(m.still)), ...manifest], null, 1));
  console.log('stills', manifest.length);
}

else if (cmd === 'eyes') {
  const man = JSON.parse(readFileSync(join(outDir, 'manifest.json'), 'utf8'));
  const exist = man.filter((m) => existsSync(join(imgDir, m.still)));
  const game = exist.filter((m) => ['faceoff', 'farnet', 'goal'].includes(m.moment));
  const hud = exist.filter((m) => ['card', 'faceoff', 'farnet', 'goal', 'pause', 'call'].includes(m.moment));
  const jobs = [['bug-hunt', exist, []], ['readability', game, ['--distance', '6']], ['hud', hud, []]];
  const only = opt('preset', '');
  let calls = 0;
  for (const [preset, list, extra] of jobs) {
    if (only && only !== preset) continue;
    for (let i = 0; i < list.length; i += 6) {
      const batch = list.slice(i, i + 6);
      const jf = join(jsonDir, `a54-${run}-${preset}-${String(i / 6 + 1).padStart(2, '0')}.json`);
      if (existsSync(jf) && !opt('force', false)) continue;
      const a = [join(root, 'tools', 'eyes', 'eyes.mjs')];
      for (const m of batch) a.push('--still', join(imgDir, m.still));
      a.push('--ask', preset, '--no-think', '--json', jf, ...extra);
      const r = spawnSync('node', a, { encoding: 'utf8', timeout: 300000 });
      calls++;
      console.log(preset, i / 6 + 1, 'exit', r.status);
    }
  }
  console.log('gemini calls', calls);
}
