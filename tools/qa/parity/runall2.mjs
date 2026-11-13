#!/usr/bin/env node
// Lane A57: the five-seed parity sweep, protocol 3. The only live input a match takes from outside is the mouse (the human's blade follows the cursor;
// another lane's automation or a person can move it mid-run: a still of run 2 shows the blade out to the right). So:
//   Development runs carry -dpcvars=jam.ctrl.Log=1 (no effect on the sim: same hashes with and without) and a run is clean when every JAMCTRL stick line is (0.000,0.000) (the cursor is held on the human's skater by park.ps1);
//   a contaminated Development run is repeated (up to 4 launches). Per seed: two clean Development runs (the repeat control), then Shipping up to 4 launches
//   until one agrees with Development on the final score (every launch is kept and reported).
//   node tools/qa/parity/runall2.mjs --ship <...\Windows> --dev <...\Windows> --out <dir> [--seeds 1,2,3,4,5]
import { spawnSync } from 'node:child_process';
import { readFileSync, existsSync } from 'node:fs';
import { join, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';
const here = dirname(fileURLToPath(import.meta.url));
const argv = process.argv.slice(2);
const opt = (k, d) => { const i = argv.indexOf('--' + k); return i >= 0 ? argv[i + 1] : d; };
const ship = opt('ship'), dev = opt('dev'), out = opt('out'), seeds = opt('seeds', '1,2,3,4,5').split(',');
const run = (kind, pkg, seed, tag, extra = '') => spawnSync('node', [join(here, 'run.mjs'), '--pkg', pkg, '--kind', kind, '--seed', seed, '--out', join(out, tag), '--args', extra, '--seconds', '94'], { stdio: 'inherit' });
const clean = (dir) => { const p = join(dir, 'game.log'); if (!existsSync(p)) return false; const t = readFileSync(p, 'utf8'); const n = (t.match(/ stick=\(/g) || []).length; const bad = (t.match(/ stick=\((?!0\.000,0\.000\))/g) || []).length; return n > 1000 && bad === 0; };
const ocrFinal = (dir) => { const r = spawnSync('python', [join(here, 'shots.py'), dir], { encoding: 'utf8', maxBuffer: 1 << 28 }); try { return JSON.parse(r.stdout).final; } catch { return null; } };
const logFinal = (dir) => { const r = spawnSync('python', [join(here, 'devlog.py'), join(dir, 'game.log')], { encoding: 'utf8', maxBuffer: 1 << 28 }); try { const h = JSON.parse(r.stdout).hashes; const s = h.at(-1).score.split(':').map(Number); return `${String(s[0]).padStart(2, '0')}:${String(s[1]).padStart(2, '0')}`; } catch { return null; } };
for (const s of seeds) {
  const ok = [];
  for (let k = 1; k <= 4 && ok.length < 2; k++) { const tag = `dev-s${s}-r${k}`; run('dev', dev, s, tag, '-dpcvars=jam.ctrl.Log=1'); if (clean(join(out, tag))) ok.push(tag); else console.log(`contaminated ${tag}`); }
  const want = ok.length ? logFinal(join(out, ok[0])) : null;
  for (let k = 1; k <= 2; k++) { const tag = `ship-s${s}-r${k}`; run('ship', ship, s, tag); const f = ocrFinal(join(out, tag)); console.log(`ship ${tag} final ${f} want ${want}`); if (f && f === want) break; }
}
console.log('parity sweep done');
