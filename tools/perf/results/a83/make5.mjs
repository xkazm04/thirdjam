// Lane A83: five-a-side copies of a cartridge (3 F + 2 D a side). node make5.mjs <in.json> <out.json> [length width]
import { readFileSync, writeFileSync } from 'node:fs';
import { validateCartridge } from '../../../../schema/validate.mjs';
const [,, inp, out, L, Wd] = process.argv;
const c = JSON.parse(readFileSync(inp, 'utf8'));
const g = c.game;
if (L) { g.rink.length_m = Number(L); g.rink.width_m = Number(Wd); }
const k = g.rink.length_m / 90;   // spawns scale with the rink (take-2 and take-7 do the same)
const r = (x) => Math.round(x * k * 100) / 100;
const blue = [['player', -7.5, 0], ['chaser', -7.5, -7.5], ['support', -7.5, 7.5], ['back', -22.5, -5], ['back', -22.5, 5]];
const orange = [['chaser', 7.5, 0], ['support', 7.5, 7.5], ['support', 7.5, -7.5], ['back', 22.5, 5], ['back', 22.5, -5]];
const mk = (a) => a.map(([role, x, y]) => ({ role, spawn: [r(x), r(y)] }));
g.teams[0].skaters = mk(blue);
g.teams[1].skaters = mk(orange);
const res = validateCartridge(c);
if (res.errors.length) { console.log('FAIL', res.errors.join(' | ')); process.exit(1); }
writeFileSync(out, `${JSON.stringify(c, null, 2)}\n`);
console.log('PASS', out);
