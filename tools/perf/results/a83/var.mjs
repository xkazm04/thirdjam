// Lane A83: a variant of the 5 v 5 copy. node var.mjs <name> '<js body on g (game)>'
import { readFileSync, writeFileSync } from 'node:fs';
import { validateCartridge } from '../../../../schema/validate.mjs';
const [,, name, body] = process.argv;
const c = JSON.parse(readFileSync('carts/hockey.regular5.cartridge.json', 'utf8'));
new Function('g', body)(c.game);
const res = validateCartridge(c);
if (res.errors.length) { console.log('FAIL', res.errors.join(' | ')); process.exit(1); }
writeFileSync(`carts/x.${name}.cartridge.json`, `${JSON.stringify(c, null, 2)}\n`);
