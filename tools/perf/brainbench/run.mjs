// Lane A106: builds brainbench.cpp with the tournament driver's sources (clang++ -O2, as tools/tournament/run.mjs) and runs it on hockey.regular's
// five-a-side teams and on the 3 v 3 preset. -DJAMCORE_API= because JamAICore.h now exports four functions with the engine's macro. Usage: node tools/perf/brainbench/run.mjs [seeds] [frames]
import { spawnSync } from 'node:child_process'; import { existsSync, readFileSync, mkdirSync } from 'node:fs'; import { join, dirname } from 'node:path'; import { fileURLToPath } from 'node:url';
const here = dirname(fileURLToPath(import.meta.url)); const wt = join(here, '..', '..', '..'); const src = join(wt, 'Game', 'Source', 'JamCore', 'AI');
const out = join(here, 'out'); mkdirSync(out, { recursive: true }); const exe = join(out, 'brainbench.exe');
const cxx = process.env.CXX || ['C:/Program Files/LLVM/bin/clang++.exe'].find(existsSync) || 'clang++';
const b = spawnSync(cxx, ['-std=c++17', '-O2', '-DJAMCORE_API=', `-I${src}`, join(here, 'brainbench.cpp'), ...['JamAICore', 'JamBotMatch', 'JamBotRefSim', 'DifficultyProfile'].map(n => join(src, n + '.cpp')), '-o', exe], { encoding: 'utf8' });
if (b.status !== 0) { console.error(b.stderr); process.exit(2); }
const g = JSON.parse(readFileSync(join(wt, 'Game', 'Content', 'Cartridges', 'hockey.regular.cartridge.json'), 'utf8')).game;
const five = g.teams.flatMap((t, ti) => t.skaters.map(s => `sk=${ti},${s.role},${s.spawn[0]},${s.spawn[1]}`)).join(';');
const [seeds = '3', frames = '7200'] = process.argv.slice(2);
for (const [name, cfg] of [['3v3 preset', ''], ['5v5 hockey.regular teams', `rink.length_m=90;rink.width_m=45;skater.speed=11;${five}`]]) {
  const r = spawnSync(exe, [cfg, seeds, frames], { encoding: 'utf8' });
  console.log(name, (r.stdout || '').trim(), (r.stderr || '').trim());
}
