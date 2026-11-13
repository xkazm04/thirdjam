// Lane A83: hockey.regular and every ladder take to five a side (3 F + 2 D), spawns scaled by each rink's length / 90 as the takes already do.
// node five-ship.mjs [--check]   (validates each file with schema/validate.mjs; --check writes nothing)
import { readFileSync, writeFileSync, readdirSync } from 'node:fs';
import { join } from 'node:path';
import { validateCartridge } from '../../../../schema/validate.mjs';
const root = '../../../../Game/Content/Cartridges';
const files = ['hockey.regular.cartridge.json', ...readdirSync(join(root, 'ladder')).filter((f) => f.startsWith('take-')).map((f) => `ladder/${f}`)];
const blue = [['player', -7.5, 0], ['chaser', -7.5, -7.5], ['support', -7.5, 7.5], ['back', -22.5, -5], ['back', -22.5, 5]];
const orange = [['chaser', 7.5, 0], ['support', 7.5, 7.5], ['support', 7.5, -7.5], ['back', 22.5, 5], ['back', 22.5, -5]];
let bad = 0;
for (const f of files) {
  const p = join(root, f);
  const c = JSON.parse(readFileSync(p, 'utf8'));
  const g = c.game;
  const k = g.rink.length_m / 90;
  const r = (x) => Math.round(x * k * 100) / 100;
  const mk = (a) => a.map(([role, x, y]) => ({ role, spawn: [r(x), r(y)] }));
  g.teams[0].skaters = mk(blue);
  g.teams[1].skaters = mk(orange);
  const res = validateCartridge(c);
  if (res.errors.length) { bad++; console.log(`FAIL ${f} ${res.errors.join(' | ')}`); continue; }
  if (!process.argv.includes('--check')) writeFileSync(p, `${JSON.stringify(c, null, 2)}\n`);
  console.log(`PASS ${f} ${g.rink.length_m}x${g.rink.width_m} k=${k}`);
}
process.exit(bad ? 1 : 0);
