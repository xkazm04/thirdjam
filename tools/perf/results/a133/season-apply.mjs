#!/usr/bin/env node
// Lane A133 evolution 3 (R-0057): write each take's level and one ai.red_skill into the take and its fallback (text edit, formatting kept).
//   node season-apply.mjs 4=rookie 5=pro:0.3 ...      (a take without ":red" carries no red_skill)
import { readFileSync, writeFileSync, existsSync } from 'node:fs';
import { join, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { validateCartridge } from '../../../../schema/validate.mjs';
const dir = join(resolve(dirname(fileURLToPath(import.meta.url)), '..', '..', '..', '..'), 'Game', 'Content', 'Cartridges', 'ladder');
for (const arg of process.argv.slice(2)) {
  const [t, spec] = arg.split('='); const [level, red] = spec.split(':');
  for (const name of [`take-${t}`, `take-${t}-fallback`]) {
    const f = join(dir, `${name}.cartridge.json`);
    if (!existsSync(f)) continue;
    let s = readFileSync(f, 'utf8');
    const re = /"ai": \{\r?\n(\s*)"difficulty": "[a-z_]+"(,\r?\n\s*"red_skill": [0-9.]+)?\r?\n/;
    const m = s.match(re); if (!m) { console.log(`FAIL ${name}: no ai block`); process.exit(1); }
    const nl = s.includes('\r\n') ? '\r\n' : '\n';
    s = s.replace(re, `"ai": {${nl}${m[1]}"difficulty": "${level}"${red ? `,${nl}${m[1]}"red_skill": ${red}` : ''}${nl}`);
    const res = validateCartridge(JSON.parse(s)); if (res.errors.length) { console.log(`FAIL ${name}: ${res.errors.join(' | ')}`); process.exit(1); }
    writeFileSync(f, s); console.log(`${name}: ${level}${red ? ' red ' + red : ''}`);
  }
}
