// plan generator: node mkplan.mjs a|idle|modes|takes > plan-x.json   (uses Game/Content/Cartridges of the worktree)
import { readdirSync, readFileSync } from 'node:fs';
const kind = process.argv[2]; const plan = [];
const LV = ['rookie', 'pro', 'all_star'], CL = ['novice', 'regular', 'good'];
const dir = new URL('../../../Game/Content/Cartridges/', import.meta.url);
const levelOf = (rel) => JSON.parse(readFileSync(new URL(rel, dir), 'utf8')).game?.ai?.difficulty || 'pro';
if (kind === 'a') for (const rink of ['regular', 'large']) for (const lvl of LV) for (const cls of CL)
  plan.push({ label: `a-${rink}-${cls}-${lvl}`, cls, lvl, cart: rink === 'large' ? 'hockey.arena.large' : 'hockey.regular', seeds: rink === 'large' ? '1-2000' : 'regularband:2000' });
if (kind === 'idle') for (const lvl of LV) plan.push({ label: `idle-regular-${lvl}`, cls: 'regular', lvl, cart: 'hockey.regular', seeds: 'regularband:100', trace: true });
if (kind === 'modes') {
  const mods = readdirSync(new URL('modes/', dir)).filter((f) => f.endsWith('.cartridge.json'));
  const lv = new Set();
  for (const f of mods) { const slug = f.replace(/^hockey\.mode\./, '').replace('.cartridge.json', ''); const lvl = levelOf(`modes/${f}`); lv.add(lvl);
    plan.push({ label: `m-${slug}`, cls: 'regular', lvl, cart: `modes/${f.replace('.cartridge.json', '')}`, seeds: '1-200' });
    if (slug === 'marathon') plan.push({ label: `m-${slug}-sprint`, cls: 'regular', lvl, cart: `modes/${f.replace('.cartridge.json', '')}`, seeds: '1-200', extra: ['-playersprint'] }); }
  for (const l of lv) plan.push({ label: `m-regular-${l}`, cls: 'regular', lvl: l, cart: 'hockey.regular', seeds: '1-200' });
  plan.push({ label: 'm-regular-sprint', cls: 'regular', lvl: levelOf('modes/hockey.mode.marathon.cartridge.json'), cart: 'hockey.regular', seeds: '1-200', extra: ['-playersprint'] });
}
if (kind === 'takes') for (const f of readdirSync(new URL('ladder/', dir)).filter((f) => /^take-.*\.cartridge\.json$/.test(f)))
  plan.push({ label: `t-${f.replace('.cartridge.json', '')}`, cls: 'regular', lvl: levelOf(`ladder/${f}`), cart: `ladder/${f.replace('.cartridge.json', '')}`, seeds: '1-200' });
console.log(JSON.stringify(plan, null, 1));
