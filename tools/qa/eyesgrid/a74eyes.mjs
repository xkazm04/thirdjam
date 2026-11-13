// bug-hunt on every a74 still, hud on every still, readability (6 m) on faceoff/farnet; 6 stills per call, --no-think. JSON -> docs/experiments/eyes/a74-<preset>-NN.json
import { spawnSync } from 'node:child_process';
import { readdirSync, existsSync } from 'node:fs';
import { join } from 'node:path';
const wt = process.argv[2];
const img = join(wt, 'docs', 'experiments', 'img', 'a74');
const sel = process.argv[3] ?? 'on';   // on: marker-on stills; off: the '-off-' stills (marker cvar 0)
const all = readdirSync(img).filter((f) => f.endsWith('.jpg') && (sel === 'off') === f.includes('-off-')).sort();
const game = all.filter((f) => /-(faceoff|farnet)\.jpg$/.test(f));
const jobs = [['bug-hunt', all, []], ['hud', all, []], ['readability', game, ['--distance', '6']]];
let calls = 0;
for (const [preset, list, extra] of jobs) for (let i = 0; i < list.length; i += 6) {
  const jf = join(wt, 'docs', 'experiments', 'eyes', `a74-${preset}-${sel === 'off' ? 'off-' : ''}${String(i / 6 + 1).padStart(2, '0')}.json`);
  if (existsSync(jf)) continue;
  const a = [join(wt, 'tools', 'eyes', 'eyes.mjs')];
  for (const s of list.slice(i, i + 6)) a.push('--still', join(img, s));
  a.push('--ask', preset, '--no-think', '--json', jf, ...extra);
  const r = spawnSync('node', a, { encoding: 'utf8', timeout: 300000, cwd: wt });
  calls++; console.log(preset, i / 6 + 1, 'exit', r.status, (r.stderr || '').slice(0, 200));
}
console.log('calls', calls);
