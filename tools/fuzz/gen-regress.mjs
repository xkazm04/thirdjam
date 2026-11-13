// Lane A118 (track S3, evolution 2): the regression corpus of the seven fault classes, built from the cartridges lane A109's fuzz found (tools/fuzz/found/).
// Every found cartridge goes in as it is, with Node's verdict and pointers (the C++ validator and loader must agree). A found cartridge the new set_param rule
// refuses (the five never-ending ones) also goes in as a "-valid" variant with only those set_param actions removed, so the end_period / swap_nets loop that
// never ended still runs. Jam.Core.FuzzRegress runs the corpus with the long run (the match must end) on every valid case.
//   node tools/fuzz/gen-regress.mjs
import { readFileSync, readdirSync, writeFileSync, mkdirSync, rmSync } from 'node:fs';
import { dirname, join, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { validateCartridgeText, validateCartridge, setParamRange } from '../../schema/validate.mjs';

const HERE = dirname(fileURLToPath(import.meta.url));
const FOUND = join(HERE, 'found');
const OUT = join(HERE, 'regress');
rmSync(OUT, { recursive: true, force: true });
mkdirSync(OUT, { recursive: true });
const pointers = (errors) => [...new Set(errors.map((e) => e.match(/^at (\S+?):/)?.[1] ?? '?'))].sort();
const cases = [];
const add = (name, text, kind) => {
  const file = `${name}.json`;
  writeFileSync(join(OUT, file), text);
  const v = validateCartridgeText(text);
  cases.push({ name, file, kind, ok: v.ok, pointers: pointers(v.errors) });
};
const legal = (a) => {
  if (a?.do !== 'set_param') return true;
  const r = setParamRange(a.param);
  return !!r && typeof a.value === 'number' && a.value >= r[0] && a.value <= r[1];
};
for (const f of readdirSync(FOUND).filter((f) => f.endsWith('.json')).sort()) {
  const text = readFileSync(join(FOUND, f), 'utf8');
  const name = f.replace(/\.json$/, '');
  add(name, text, 'found');
  if (!name.startsWith('never-ends')) continue;
  const c = JSON.parse(text);
  for (const h of c.game?.rules?.hooks ?? []) {
    h.then = h.then.filter(legal);
    if (!h.then.length) h.then.push({ do: 'announce', text: 'GO' });
  }
  if (c.game?.rules?.timer_frames > 28800) c.game.rules.timer_frames = 28800;   // lane A141: the clock limit is 28,800 (R-0052 point 6)
  const t = JSON.stringify(c, null, 1);
  if (!validateCartridge(c).ok) throw new Error(`${name}: the stripped variant is still invalid`);
  add(`${name}-valid`, t, 'found-stripped');
}
writeFileSync(join(OUT, 'manifest.json'), JSON.stringify({ tool: 'gen-regress', cases }, null, 1));
console.log(`PASS gen-regress ${cases.length} cases, ${cases.filter((c) => c.ok).length} valid`);
for (const c of cases) console.log(`  ${c.ok ? 'ok ' : 'bad'} ${c.name} ${c.pointers.join(' ')}`);
