#!/usr/bin/env node
// lane A182: node mkvar.mjs <cartridge> <name> "<a.b=v;...>": a variant cartridge under a182/out/<name>.cartridge.json (validated), path printed
import { readFileSync, writeFileSync, mkdirSync } from 'node:fs';
import { join, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { validateCartridge } from '../../../../schema/validate.mjs';
const here = dirname(fileURLToPath(import.meta.url)), repo = resolve(here, '..', '..', '..', '..');
const [cartridge, name, set] = process.argv.slice(2);
const cart = JSON.parse(readFileSync(join(repo, 'Game', 'Content', 'Cartridges', `${cartridge}.cartridge.json`), 'utf8'));
for (const pair of set.split(';').filter(Boolean)) { const [path, v] = pair.split('='); const parts = path.split('.'); let node = cart.game; for (const p of parts.slice(0, -1)) node = node[p] ??= {}; node[parts.at(-1)] = v === 'true' ? true : v === 'false' ? false : Number(v); }
const res = validateCartridge(cart); if (res.errors.length) { console.error(res.errors.join(' | ')); process.exit(2); }
mkdirSync(join(here, 'out'), { recursive: true });
const file = join(here, 'out', `${name}.cartridge.json`); writeFileSync(file, `${JSON.stringify(cart, null, 2)}\n`); console.log(file.split(String.fromCharCode(92)).join('/'));
