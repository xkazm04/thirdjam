// Lane A83: a variant of the shipped (five-a-side) hockey.regular. node var2.mjs <name> '<js body on g (game)>'
import { readFileSync, writeFileSync } from 'node:fs';
import { validateCartridge } from '../../../../schema/validate.mjs';
const [,, name, body] = process.argv;
const c = JSON.parse(readFileSync('../../../../Game/Content/Cartridges/hockey.regular.cartridge.json', 'utf8'));
new Function('g', body)(c.game);
const res = validateCartridge(c);
if (res.errors.length) { console.log('FAIL', res.errors.join(' | ')); process.exit(1); }
writeFileSync(`carts/y.${name}.cartridge.json`, `${JSON.stringify(c, null, 2)}\n`);
