#!/usr/bin/env node
// Lane A133 evolution 3: a season take as a candidate (level name + one ai.red_skill), written as a cartridge file under raw/carts/; prints its path.
//   node take-variant.mjs <take n> <level> [red]
import { readFileSync, writeFileSync, mkdirSync } from 'node:fs';
import { join, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { validateCartridge } from '../../../../schema/validate.mjs';
const here = dirname(fileURLToPath(import.meta.url));
const repo = resolve(here, '..', '..', '..', '..');
const [t, level, red] = process.argv.slice(2);
const c = JSON.parse(readFileSync(join(repo, 'Game', 'Content', 'Cartridges', 'ladder', `take-${t}.cartridge.json`), 'utf8'));
c.game.ai.difficulty = level;
if (red !== undefined) c.game.ai.red_skill = Number(red); else delete c.game.ai.red_skill;
const res = validateCartridge(c);
if (res.errors.length) { console.error(res.errors.join(' | ')); process.exit(2); }
const dir = join(here, 'raw', 'carts'); mkdirSync(dir, { recursive: true });
const f = join(dir, `take-${t}.${level}${red !== undefined ? '.red' + red : ''}.cartridge.json`);
writeFileSync(f, JSON.stringify(c, null, 2) + '\n');
console.log(f.split(String.fromCharCode(92)).join("/"));
