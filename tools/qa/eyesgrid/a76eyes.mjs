// Lane A76: hud + locate on every a76 still of a column (before | after | cardmove), readability (6 m) on faceoff/farnet; 6 stills per call, --no-think.
// JSON -> docs/experiments/eyes/a76-<preset>-<column>-NN.json.   node a76eyes.mjs <worktree> <column>
import { spawnSync } from 'node:child_process';
import { readdirSync, existsSync } from 'node:fs';
import { join } from 'node:path';
const wt = process.argv[2], col = process.argv[3] ?? 'after';
const img = join(wt, 'docs', 'experiments', 'img', 'a76');
const all = readdirSync(img).filter((f) => f.endsWith('.jpg') && f.includes(`-${col}-`)).sort();
const game = all.filter((f) => /-(card|faceoff|farnet)\.jpg$/.test(f));
const jobs = [['hud', game, []], ['locate', game, []], ['readability', game.filter((f) => !f.endsWith('-card.jpg')), ['--distance', '6']]];
for (const [preset, list, extra] of jobs) for (let i = 0; i < list.length; i += 6) {
  const jf = join(wt, 'docs', 'experiments', 'eyes', `a76-${preset}-${col}-${String(i / 6 + 1).padStart(2, '0')}.json`);
  if (existsSync(jf)) continue;
  const a = [join(wt, 'tools', 'eyes', 'eyes.mjs')];
  for (const s of list.slice(i, i + 6)) a.push('--still', join(img, s));
  a.push('--ask', preset, '--no-think', '--json', jf, ...extra);
  const r = spawnSync('node', a, { encoding: 'utf8', timeout: 300000, cwd: wt });
  console.log(col, preset, i / 6 + 1, 'exit', r.status, (r.stderr || '').slice(0, 200));
}
