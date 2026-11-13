// Lane A83: bot-vs-bot (playerbot) matches on a cartridge path through the JamTournament commandlet; prints a table row. node tour.mjs <label> <cart> <seeds> [difficulty]
import { spawnSync } from 'node:child_process';
import { join, resolve } from 'node:path';
import { writeFileSync } from 'node:fs';
import { engineRoot } from '../../../tournament/run.mjs';
const [,, label, cart, seeds, diff = 'pro', tune = ''] = process.argv;   // tune: -aitune keys (DifficultyProfile.h), e.g. mate.pass_to_human=0.6
const up = resolve('../../../../Game/ThirdJam.uproject');
const r = spawnSync(join(engineRoot(), 'Engine/Binaries/Win64/UnrealEditor-Cmd.exe'), [up, '-run=JamTournament', '-sim=jam', `-cartridge=${cart}`, `-difficulty=${diff}`, `-seeds=${seeds}`, '-maxframes=36000', ...(tune ? [`-aitune=${tune}`] : []), '-nullrhi', '-unattended', '-nopause', '-nosplash', '-stdout', '-FullStdOutLogOutput'], { encoding: 'utf8', maxBuffer: 1 << 28 });
const ms = r.stdout.split(/\r?\n/).map((l) => l.indexOf('{"tool":"thirdjam.tournament/1"')).map((i, k, a) => i).length && r.stdout.split(/\r?\n/).filter((l) => l.includes('{"tool":"thirdjam.tournament/1"')).map((l) => JSON.parse(l.slice(l.indexOf('{'))));
writeFileSync(`tour-${label}.jsonl`, ms.map((m) => JSON.stringify(m)).join('\n') + '\n');
const avg = (a) => a.reduce((s, x) => s + x, 0) / (a.length || 1); const r1 = (x) => Math.round(x * 10) / 10;
const pm = (k) => r1(avg(ms.map((m) => m[k] / (m.play_frames / 3600))));
console.log(`| ${label} | ${ms.length} | ${r1(avg(ms.map((m) => m.frames)) / 60)} | ${r1(avg(ms.map((m) => m.goals[0] + m.goals[1])))} | ${r1(avg(ms.map((m) => m.stalls)))} (${pm('stalls')}/min) | ${r1(avg(ms.map((m) => m.puck_path_m / (m.play_frames / 60))))} | ${ms.filter((m) => !m.over).length} | ${Object.keys(ms[0]).filter((k) => Array.isArray(ms[0][k]) && k !== 'goals').join(',')} |`);
