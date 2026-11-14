// Lane A177 (track F1): a cartridge variant under a SHORT file name. tools/playtest/run.mjs names the file after the whole --set string; with six keys the
// path passes Windows' 260 characters and the write fails (ENOENT). Same steps: read hockey.regular, apply "a.b=1;c.d=true", validate, write.
//   writeVariantShort('hockey.regular', 'contact.x=1;contact.y=true', 'armname') -> out/<armname>.cartridge.json (forward slashes)
import { mkdirSync, readFileSync, writeFileSync } from 'node:fs';
import { join, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { validateCartridge } from '../../../../schema/validate.mjs';
const here = dirname(fileURLToPath(import.meta.url));
const repo = resolve(here, '..', '..', '..', '..');
export function writeVariantShort(cartridge, set, name) {
  const cart = JSON.parse(readFileSync(join(repo, 'Game', 'Content', 'Cartridges', `${cartridge}.cartridge.json`), 'utf8'));
  for (const pair of set.split(';').filter(Boolean)) {
    const [path, v] = pair.split('=');
    const parts = path.split('.');
    let node = cart.game;
    for (const p of parts.slice(0, -1)) node = node[p] ??= {};
    node[parts.at(-1)] = v === 'true' ? true : v === 'false' ? false : Number(v);
  }
  const res = validateCartridge(cart);
  if (res.errors.length) throw new Error(`the set makes ${cartridge} invalid: ${res.errors.join(' | ')}`);
  const dir = join(here, 'out');
  mkdirSync(dir, { recursive: true });
  const file = join(dir, `${cartridge}.${name.replace(/[^a-z0-9]+/gi, '_')}.cartridge.json`);
  writeFileSync(file, `${JSON.stringify(cart, null, 2)}\n`);
  return file.split(String.fromCharCode(92)).join('/');
}
