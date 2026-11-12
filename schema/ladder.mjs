// Ladder validator (tj.ladder/1, C-0005 section 4 game.ladder; lane A17). The C++ mirror is UJamProgression::ParseLadder.
// Run: node schema/ladder.mjs Game/Content/Cartridges/ladder/season.ladder.json   (exit 0 valid, 1 invalid, 2 could not run)
// Checks, in order: the schema (unknown keys are errors), then: take 1 unlocks "always"; cartridges are unique; each
// ladder/<cartridge>.cartridge.json (and <fallback>, R-0014) exists next to the ladder file and its id equals the preset id; the fallback
// must validate, a designed cartridge with a fallback may not validate yet (reported as INFO), one without a fallback must.
import { readFileSync, existsSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { validateAgainstSchema, validateCartridge, loadJsonFile } from './validate.mjs';

const HERE = dirname(fileURLToPath(import.meta.url));
const SCHEMA = resolve(HERE, 'ladder.schema.json');

// obj: the parsed ladder file. dir: the folder holding the take cartridges (absent = skip the file checks).
export function validateLadder(obj, { dir } = {}) {
  const errors = validateAgainstSchema(loadJsonFile(SCHEMA), obj);
  if (errors.length) return { ok: false, errors };
  const takes = obj.ladder.takes;
  const notes = [];
  if (takes[0].unlock !== 'always') errors.push('at /ladder/takes/0/unlock: the first take must unlock "always", or the season can never start.');
  // R-0014: a take with a fallback may ship a designed cartridge that does not validate yet (its primitive has not landed);
  // the fallback must always validate. A take without a fallback must validate itself.
  const checkFile = (id, ptr, mustValidate, notes) => {
    const file = resolve(dir, `${id}.cartridge.json`);
    if (!existsSync(file)) { errors.push(`at ${ptr}: no file ${id}.cartridge.json next to the ladder.`); return; }
    let cart;
    try { cart = JSON.parse(readFileSync(file, 'utf8')); } catch (e) { errors.push(`at ${ptr}: ${id}.cartridge.json is not valid JSON (${e.message}).`); return; }
    const r = validateCartridge(cart);
    if (mustValidate) for (const e of r.errors) errors.push(`at ${ptr}: ${id}.cartridge.json is not a valid cut: ${e}`);
    else if (r.errors.length) notes.push(`${ptr}: ${id} does not validate yet; its fallback plays (${r.errors[0]})`);
    if (cart.id !== id) errors.push(`at ${ptr}: ${id}.cartridge.json has id "${cart.id}"; the ids must match.`);
  };
  const seen = new Map();
  const claim = (id, ptr, i) => {
    if (seen.has(id)) errors.push(`at ${ptr}: "${id}" is already used by take ${seen.get(id) + 1}; each take is its own cut.`);
    else seen.set(id, i);
  };
  takes.forEach((t, i) => {
    claim(t.cartridge, `/ladder/takes/${i}/cartridge`, i);
    if (t.fallback !== undefined) claim(t.fallback, `/ladder/takes/${i}/fallback`, i);
    if (!dir) return;
    checkFile(t.cartridge, `/ladder/takes/${i}/cartridge`, t.fallback === undefined, notes);
    if (t.fallback !== undefined) checkFile(t.fallback, `/ladder/takes/${i}/fallback`, true, notes);
  });
  return { ok: errors.length === 0, errors, notes };
}

export function ladderCli(argv) {
  const file = argv[2];
  if (!file) { console.error('usage: node schema/ladder.mjs season.ladder.json'); return 2; }
  let obj;
  try { obj = JSON.parse(readFileSync(file, 'utf8')); } catch (e) {
    if (e instanceof SyntaxError) { console.log(`FAIL ${file}\n  at /: the file is not valid JSON (${e.message}).`); return 1; }
    console.error(`cannot read input: ${e.message}`); return 2;
  }
  let r;
  try { r = validateLadder(obj, { dir: dirname(resolve(file)) }); } catch (e) { console.error(`validator could not run: ${e.message}`); return 2; }
  for (const n of r.notes ?? []) console.log(`INFO ${n}`);
  if (r.ok) console.log(`PASS ${file}`);
  else { console.log(`FAIL ${file}`); for (const e of r.errors) console.log(`  ${e}`); }
  console.log(JSON.stringify({ tool: 'validate-ladder', file, ok: r.ok, errors: r.errors.length }));
  return r.ok ? 0 : 1;
}

if (process.argv[1] && resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
  process.exitCode = ladderCli(process.argv);
}
