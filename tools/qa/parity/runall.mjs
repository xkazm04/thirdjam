#!/usr/bin/env node
// Lane A57: the five-seed parity sweep, one game at a time: for each seed a Shipping run and a Development run (idle human: the only way to play the same
// match in both, see PARITY.md), then a Development repeat of seeds 1 and 2 (the control: same build, same seed, a second launch).
//   node tools/qa/parity/runall.mjs --ship <...\Windows> --dev <...\Windows> --out <dir> [--seeds 1,2,3,4,5]
import { spawnSync } from 'node:child_process';
import { join, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';
const here = dirname(fileURLToPath(import.meta.url));
const argv = process.argv.slice(2);
const opt = (k, d) => { const i = argv.indexOf('--' + k); return i >= 0 ? argv[i + 1] : d; };
const ship = opt('ship'), dev = opt('dev'), out = opt('out'), seeds = opt('seeds', '1,2,3,4,5').split(',');
const run = (kind, pkg, seed, tag, extra = '') => spawnSync('node', [join(here, 'run.mjs'), '--pkg', pkg, '--kind', kind, '--seed', seed, '--out', join(out, `${tag}-s${seed}`), '--args', extra, '--seconds', '100'], { stdio: 'inherit' });
for (const s of seeds) { run('ship', ship, s, 'ship'); run('dev', dev, s, 'dev'); }
for (const s of seeds.slice(0, 2)) run('dev', dev, s, 'dev2');
console.log('parity sweep done');
