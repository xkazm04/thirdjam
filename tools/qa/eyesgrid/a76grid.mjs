// Lane A76 proof driver: A54's shoot.ps1 with A54's non-dev timeline (card, faceoff, farnet, pause, call) on a Development exe (B = 6 s later),
// the game log kept, optional resolution and a bad cartridge. Stills -> docs/experiments/img/a76/<cut>-<tier>-<moment>[-720].jpg
//   node a76grid.mjs <worktree> <exe> <cell...>   cell = cut:tier[:720] | bad
import { spawnSync, spawn } from 'node:child_process';
import { dirname } from 'node:path';
import { fileURLToPath } from 'node:url';
import { existsSync, mkdirSync, readFileSync, writeFileSync } from 'node:fs';
import { join } from 'node:path';
const [wt, exe, ...cells] = process.argv.slice(2);
const B = 6;
const out = join(wt, 'tools', 'qa', 'out', 'a76');
const img = join(wt, 'docs', 'experiments', 'img', 'a76');
mkdirSync(out, { recursive: true }); mkdirSync(img, { recursive: true });
const cutDir = join(wt, 'Game', 'Content', 'Cartridges');
for (const c of cells) {
  const [cut, tier, res] = c.split(':');
  const tag = (cut === 'bad' ? 'bad-cut' : `${cut.split('/').pop().replace('hockey.mode.', 'mode.')}-${tier.split('_')[0]}${res ? '-' + res : ''}`) + (process.env.A76_TAG ?? '');
  const dir = join(out, tag); mkdirSync(dir, { recursive: true });
  let cart, len = 90;
  if (cut === 'bad') {
    cart = join(dir, 'hockey.bad.cartridge.json');
    const j = JSON.parse(readFileSync(join(cutDir, 'hockey.chaos.cartridge.json'), 'utf8')); j.look = { tier: 'l1_day' };   // look at the top level: rejected (unknown key)
    writeFileSync(cart, JSON.stringify(j, null, 2));
  } else {
    const j = JSON.parse(readFileSync(join(cutDir, cut + '.cartridge.json'), 'utf8'));
    len = j.game?.rink?.length_m ?? 90;
    j.game = { ...(j.game ?? {}), look: { ...(j.game?.look ?? {}), tier } };
    cart = join(dir, `${cut.split('/').pop()}.${tier}.cartridge.json`); writeFileSync(cart, JSON.stringify(j, null, 2));
  }
  const arrive = 19.2 + (len / 2 + 3) / 7.3;
  const steps = [
    { at: B + 12, keys: '{ENTER}' }, { at: B + 13.7, shot: 'card' }, { at: B + 19, shot: 'faceoff' }, { at: B + 19.2, down: 'W' },
    { at: B + arrive - 0.3, shot: 'farnet' }, { at: B + arrive + 0.5, up: 'W' }, { at: B + arrive + 1.5, keys: '{ESC}' }, { at: B + arrive + 3, shot: 'pause' },
    { at: B + arrive + 3.5, keys: '{DOWN}' }, { at: B + arrive + 4.1, keys: '{ENTER}' }, { at: B + arrive + 6, shot: 'call' },
  ];
  const plan = { seconds: B + arrive + 8, steps };
  if (cut === 'bad') { plan.steps = steps.slice(0, 3); plan.seconds = B + 20; }
  const pf = join(dir, 'plan.json'); writeFileSync(pf, JSON.stringify(plan));
  const args = ['-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', join(wt, 'tools', 'qa', 'eyesgrid', 'shoot.ps1'), '-Exe', exe, '-PlanFile', pf, '-OutDir', dir,
    '-GameArgs', `-cut=${cart.split(String.fromCharCode(92)).join('/')} ${process.env.A76_EXTRA ?? ''}`, '-Log', join(dir, 'game.log')];
  if (res === '720') args.push('-ResX', '1280', '-ResY', '720');
  // Lane A76: the cursor is shown and locked to the viewport while the game has the foreground (A49), so the real mouse aims the stick and turns the
  // skater; A54's h19 run never had the foreground. The cursor is parked up-ice (top centre of the game's client area) for the whole launch.
  const cur = spawn('powershell', ['-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', join(dirname(fileURLToPath(import.meta.url)), 'cursor.ps1'), '-X', res === '720' ? '680' : '1000', '-Y', res === '720' ? '260' : '330', '-Seconds', String(Math.ceil(plan.seconds + 25))], { stdio: 'ignore' });
  const r = spawnSync('powershell', args, { encoding: 'utf8', timeout: 300000 });
  try { cur.kill(); } catch {}
  for (const m of ['card', 'faceoff', 'farnet', 'pause', 'call']) {
    const p = join(dir, m + '.png');
    if (existsSync(p)) spawnSync('ffmpeg', ['-loglevel', 'error', '-y', '-i', p, '-q:v', '3', join(img, `${tag}-${m}.jpg`)]);
  }
  console.log('cell', tag, r.status);
}
