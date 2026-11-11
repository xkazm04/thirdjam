// Cartridge validator for Third Jam. Zero dependencies, Node 24.
//
// Usage
//   node schema/validate.mjs file.json                 exit 0 valid, 1 invalid, 2 could not run
//   node schema/validate.mjs out.json --base in.json   also check that only Designer-editable parts changed
//   import { validateCartridge } from './validate.mjs' // -> { ok, errors: string[] }
//
// JSON Schema subset implemented here (anything else in a schema file is a loud error, never ignored):
//   type (a name or a list of names: object array string number integer boolean null)
//   required, properties, additionalProperties (false, true or a schema), items (one schema)
//   enum, const, pattern, minimum, maximum, minLength, maxLength, minItems, maxItems
//   $ref to "#/$defs/name" in the same file, or to a local file "other.json" (optionally "other.json#/$defs/name"),
//   a $ref must stand alone except for annotations
//   annotations that are read and ignored: $schema $id $comment title description default examples $defs
//   `description` on a node that has `pattern` is appended to the pattern error, so keep it a plain-English rule.
// Number keywords apply only to numbers, string keywords only to strings, array keywords only to arrays, so a
// node with type ["number","string"] can carry both minimum and maxLength.
//
// On top of the schema there are engine checks (see checkEngine): list lengths from limits.json, unique ids,
// `values` keys exist in `tunables` and sit inside their declared type/range/options, tunable bounds are sane,
// durations are physics frames (names ending in *_frames, never *_seconds/*_ms), ai.applies_to points at real
// entities, optional blocked_terms. Errors are sentences that start with a JSON pointer, because a model reads
// them to repair its own output.

import { readFileSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const HERE = dirname(fileURLToPath(import.meta.url));
const SCHEMA_FILE = resolve(HERE, 'cartridge.schema.json');
const LIMITS_FILE = resolve(HERE, 'limits.json');

const ANNOTATIONS = new Set(['$schema', '$id', '$comment', 'title', 'description', 'default', 'examples', '$defs']);
const KEYWORDS = new Set([
  'type', 'required', 'properties', 'additionalProperties', 'items', 'enum', 'const', 'pattern',
  'minimum', 'maximum', 'minLength', 'maxLength', 'minItems', 'maxItems', '$ref',
]);

const fileCache = new Map();
export function loadJsonFile(path) {
  const abs = resolve(path);
  if (!fileCache.has(abs)) fileCache.set(abs, JSON.parse(readFileSync(abs, 'utf8')));
  return fileCache.get(abs);
}

export function loadLimits() {
  return loadJsonFile(LIMITS_FILE);
}

// ---------- helpers that build readable text ----------

const esc = (s) => String(s).replace(/~/g, '~0').replace(/\//g, '~1');
const child = (ptr, key) => `${ptr}/${esc(key)}`;
const show = (p) => (p === '' ? '/' : p);

function describe(v) {
  if (v === null) return 'null (omit the key instead of writing null)';
  if (Array.isArray(v)) return `an array of ${v.length} item${v.length === 1 ? '' : 's'}`;
  if (typeof v === 'string') return `a string (${JSON.stringify(v.length > 40 ? v.slice(0, 40) + '...' : v)})`;
  if (typeof v === 'number') return `${Number.isInteger(v) ? 'a whole number' : 'a number'} (${v})`;
  if (typeof v === 'boolean') return `a boolean (${v})`;
  return 'an object';
}

const article = (word) => (word === 'array' || word === 'object' || word === 'integer' ? `an ${word}` : `a ${word}`);

function matchesType(v, t) {
  switch (t) {
    case 'object': return v !== null && typeof v === 'object' && !Array.isArray(v);
    case 'array': return Array.isArray(v);
    case 'string': return typeof v === 'string';
    case 'number': return typeof v === 'number' && Number.isFinite(v);
    case 'integer': return typeof v === 'number' && Number.isInteger(v);
    case 'boolean': return typeof v === 'boolean';
    case 'null': return v === null;
    default: throw new Error(`schema uses unsupported type "${t}"`);
  }
}

function sortKeys(v) {
  if (Array.isArray(v)) return v.map(sortKeys);
  if (v && typeof v === 'object') return Object.fromEntries(Object.keys(v).sort().map((k) => [k, sortKeys(v[k])]));
  return v;
}
function deepEqual(a, b) {
  return JSON.stringify(sortKeys(a)) === JSON.stringify(sortKeys(b));
}

// ---------- the schema walker ----------

function resolveRef(ref, ctx) {
  const [file, frag] = ref.split('#');
  let root = ctx.root;
  let dir = ctx.dir;
  if (file) {
    const abs = resolve(ctx.dir, file);
    root = loadJsonFile(abs);
    dir = dirname(abs);
  }
  let node = root;
  if (frag) {
    for (const part of frag.split('/').slice(1)) {
      node = node?.[part.replace(/~1/g, '/').replace(/~0/g, '~')];
    }
  }
  if (node === undefined) throw new Error(`schema $ref "${ref}" does not resolve`);
  return { schema: node, ctx: { root, dir } };
}

function walk(schema, value, ptr, ctx, errors) {
  if (schema === true) return;
  if (schema === false) { errors.push(`at ${show(ptr)}: no value is allowed here.`); return; }
  for (const k of Object.keys(schema)) {
    if (!KEYWORDS.has(k) && !ANNOTATIONS.has(k)) {
      throw new Error(`schema keyword "${k}" is not in the supported subset (see the top of validate.mjs)`);
    }
  }
  if (schema.$ref !== undefined) {
    for (const k of Object.keys(schema)) {
      if (k !== '$ref' && !ANNOTATIONS.has(k)) throw new Error(`schema node with $ref may only carry annotations, found "${k}"`);
    }
    const r = resolveRef(schema.$ref, ctx);
    walk(r.schema, value, ptr, r.ctx, errors);
    return;
  }
  const at = `at ${show(ptr)}:`;

  if (schema.type !== undefined) {
    const types = Array.isArray(schema.type) ? schema.type : [schema.type];
    if (!types.some((t) => matchesType(value, t))) {
      const names = types.map(article);
      const want = names.length > 1 ? names.slice(0, -1).join(', ') + ' or ' + names[names.length - 1] : names[0];
      errors.push(`${at} expected ${want} but found ${describe(value)}.`);
      return;
    }
  }
  if (schema.const !== undefined && !deepEqual(value, schema.const)) {
    errors.push(`${at} must be exactly ${JSON.stringify(schema.const)} but found ${JSON.stringify(value)}.`);
  }
  if (schema.enum !== undefined && !schema.enum.some((e) => deepEqual(e, value))) {
    errors.push(`${at} ${JSON.stringify(value)} is not allowed; use one of ${schema.enum.map((e) => JSON.stringify(e)).join(', ')}.`);
    return;
  }
  if (typeof value === 'number') {
    if (schema.minimum !== undefined && value < schema.minimum) errors.push(`${at} ${value} is below the minimum ${schema.minimum}.`);
    if (schema.maximum !== undefined && value > schema.maximum) errors.push(`${at} ${value} is above the maximum ${schema.maximum}.`);
  }
  if (typeof value === 'string') {
    if (schema.minLength !== undefined && value.length < schema.minLength) errors.push(`${at} the string is ${value.length} characters long; it needs at least ${schema.minLength}.`);
    if (schema.maxLength !== undefined && value.length > schema.maxLength) errors.push(`${at} the string is ${value.length} characters long; the limit is ${schema.maxLength}.`);
    if (schema.pattern !== undefined && !new RegExp(schema.pattern).test(value)) {
      const why = schema.description ? `: ${schema.description}` : ` (pattern ${schema.pattern})`;
      errors.push(`${at} ${JSON.stringify(value)} is not valid${why}.`);
    }
  }
  if (Array.isArray(value)) {
    if (schema.minItems !== undefined && value.length < schema.minItems) errors.push(`${at} the list has ${value.length} item${value.length === 1 ? '' : 's'}; it needs at least ${schema.minItems}.`);
    if (schema.maxItems !== undefined && value.length > schema.maxItems) errors.push(`${at} the list has ${value.length} items; the limit is ${schema.maxItems}.`);
    if (schema.items !== undefined) value.forEach((item, i) => walk(schema.items, item, child(ptr, i), ctx, errors));
  }
  if (value !== null && typeof value === 'object' && !Array.isArray(value)) {
    const props = schema.properties ?? {};
    for (const req of schema.required ?? []) {
      if (!(req in value)) errors.push(`at ${show(child(ptr, req))}: required key "${req}" is missing.`);
    }
    for (const [k, v] of Object.entries(value)) {
      if (k in props) {
        walk(props[k], v, child(ptr, k), ctx, errors);
      } else if (schema.additionalProperties === false) {
        const allowed = Object.keys(props);
        errors.push(`at ${show(child(ptr, k))}: unknown key "${k}" is not allowed here; remove it.${allowed.length ? ` Allowed keys: ${allowed.join(', ')}.` : ' No keys are allowed in this object.'}`);
      } else if (schema.additionalProperties && schema.additionalProperties !== true) {
        walk(schema.additionalProperties, v, child(ptr, k), ctx, errors);
      }
    }
  }
}

export function validateAgainstSchema(schema, value, { dir = HERE } = {}) {
  const errors = [];
  walk(schema, value, '', { root: schema, dir }, errors);
  return errors;
}

// ---------- engine checks beyond the schema ----------

const isObj = (v) => v !== null && typeof v === 'object' && !Array.isArray(v);
const WALL_CLOCK = /_(seconds?|secs?|ms|millis|milliseconds|minutes?|mins?|hours?)$/;

function checkParams(params, ptr, limits, errors) {
  if (!isObj(params)) return;
  const keys = Object.keys(params);
  if (keys.length > limits.max_param_keys) {
    errors.push(`at ${show(ptr)}: ${keys.length} parameters; the engine allows at most ${limits.max_param_keys}.`);
  }
  for (const k of keys) {
    const p = child(ptr, k);
    if (!/^[a-z][a-z0-9]*(_[a-z0-9]+)*$/.test(k) || k.length > 40) {
      errors.push(`at ${show(p)}: the parameter name ${JSON.stringify(k)} must be snake_case (lowercase letters and digits separated by single underscores, starting with a letter, at most 40 characters).`);
    } else if (WALL_CLOCK.test(k)) {
      errors.push(`at ${show(p)}: "${k}" counts wall-clock time. Time is physics frames (60 per second): rename it to "${k.replace(WALL_CLOCK, '_frames')}" and convert the number.`);
    } else if (k.endsWith('_frames') && !(Number.isInteger(params[k]) && params[k] >= 0)) {
      errors.push(`at ${show(p)}: "${k}" is a duration in frames and must be a whole number of 0 or more, found ${JSON.stringify(params[k])}.`);
    }
  }
}

// Returns a Map id -> index of first use (also reports duplicates).
function checkUniqueIds(list, ptr, what, errors, idKey = 'id') {
  if (!Array.isArray(list)) return undefined;
  const seen = new Map();
  list.forEach((item, i) => {
    const id = isObj(item) ? item[idKey] : undefined;
    if (typeof id !== 'string') return;
    if (seen.has(id)) {
      errors.push(`at ${show(child(child(ptr, i), idKey))}: duplicate ${what} ${idKey} "${id}"; it is already used at ${show(child(child(ptr, seen.get(id)), idKey))}. The ${idKey}s in this list must be unique.`);
    } else seen.set(id, i);
  });
  return seen;
}

function checkCount(list, max, ptr, what, errors) {
  if (Array.isArray(list) && list.length > max) {
    errors.push(`at ${show(ptr)}: ${list.length} ${what}; the engine allows at most ${max}. Remove ${list.length - max}.`);
  }
}

function checkTunables(tunables, limits, errors) {
  checkCount(tunables, limits.max_tunables, '/tunables', 'tunables', errors);
  checkUniqueIds(tunables, '/tunables', 'tunable', errors, 'key');
  if (!Array.isArray(tunables)) return;
  tunables.forEach((t, i) => {
    if (!isObj(t)) return;
    const p = `/tunables/${i}`;
    const name = typeof t.key === 'string' ? t.key : `#${i}`;
    if (typeof t.key === 'string') {
      if (WALL_CLOCK.test(t.key)) {
        errors.push(`at ${show(child(p, 'key'))}: "${t.key}" counts wall-clock time. Time is physics frames (60 per second): name it "${t.key.replace(WALL_CLOCK, '_frames')}".`);
      } else if (t.key.endsWith('_frames') && t.type !== 'integer') {
        errors.push(`at ${show(child(p, 'type'))}: tunable "${t.key}" is a duration in frames, so its type must be "integer".`);
      }
    }
    const has = (k) => t[k] !== undefined;
    if (t.type === 'number' || t.type === 'integer') {
      if (has('options')) errors.push(`at ${show(child(p, 'options'))}: only enum tunables have options; remove it from "${name}".`);
      for (const k of ['min', 'max']) {
        if (!has(k)) errors.push(`at ${show(child(p, k))}: ${t.type} tunable "${name}" must declare "${k}" so its range is bounded.`);
      }
      if (typeof t.min === 'number' && typeof t.max === 'number' && t.min > t.max) {
        errors.push(`at ${show(child(p, 'min'))}: min ${t.min} is above max ${t.max} for tunable "${name}".`);
      }
      if (t.type === 'integer') {
        for (const k of ['min', 'max']) {
          if (typeof t[k] === 'number' && !Number.isInteger(t[k])) errors.push(`at ${show(child(p, k))}: integer tunable "${name}" needs a whole-number ${k}, found ${t[k]}.`);
        }
      }
      if (has('default')) checkValueAgainst(t, t.default, child(p, 'default'), `the default of tunable "${name}"`, errors);
    } else if (t.type === 'boolean') {
      for (const k of ['min', 'max', 'options']) {
        if (has(k)) errors.push(`at ${show(child(p, k))}: boolean tunable "${name}" must not have "${k}"; remove it.`);
      }
      if (has('default') && typeof t.default !== 'boolean') errors.push(`at ${show(child(p, 'default'))}: the default of boolean tunable "${name}" must be true or false, found ${describe(t.default)}.`);
    } else if (t.type === 'enum') {
      for (const k of ['min', 'max']) {
        if (has(k)) errors.push(`at ${show(child(p, k))}: enum tunable "${name}" must not have "${k}"; remove it.`);
      }
      if (!has('options')) errors.push(`at ${show(child(p, 'options'))}: enum tunable "${name}" must list its options.`);
      else if (Array.isArray(t.options)) {
        if (new Set(t.options).size !== t.options.length) errors.push(`at ${show(child(p, 'options'))}: the options of "${name}" contain a duplicate.`);
        if (has('default') && !t.options.includes(t.default)) errors.push(`at ${show(child(p, 'default'))}: the default ${JSON.stringify(t.default)} of "${name}" is not one of its options (${t.options.join(', ')}).`);
      }
    }
  });
}

// Check one value against a tunable declaration. `label` names the value in the sentence.
function checkValueAgainst(t, v, ptr, label, errors) {
  const name = typeof t.key === 'string' ? t.key : '?';
  if (t.type === 'number' || t.type === 'integer') {
    if (typeof v !== 'number' || !Number.isFinite(v)) {
      errors.push(`at ${show(ptr)}: ${label} must be a ${t.type} but is ${describe(v)}.`);
      return;
    }
    if (t.type === 'integer' && !Number.isInteger(v)) {
      errors.push(`at ${show(ptr)}: ${label} must be a whole number but is ${v}.`);
      return;
    }
    if (typeof t.min === 'number' && typeof t.max === 'number' && t.min > t.max) return; // bad bounds are reported on the tunable itself
    const range = typeof t.min === 'number' && typeof t.max === 'number' ? ` (allowed range ${t.min} to ${t.max})` : '';
    if (typeof t.min === 'number' && v < t.min) errors.push(`at ${show(ptr)}: ${label} is ${v}, below the minimum ${t.min}${range}.`);
    else if (typeof t.max === 'number' && v > t.max) errors.push(`at ${show(ptr)}: ${label} is ${v}, above the maximum ${t.max}${range}.`);
  } else if (t.type === 'boolean') {
    if (typeof v !== 'boolean') errors.push(`at ${show(ptr)}: ${label} must be true or false but is ${describe(v)}.`);
  } else if (t.type === 'enum') {
    if (!Array.isArray(t.options)) return;
    if (typeof v !== 'string' || !t.options.includes(v)) errors.push(`at ${show(ptr)}: ${label} is ${JSON.stringify(v)}, which is not an option of "${name}"; use one of ${t.options.join(', ')}.`);
  }
}

function checkValues(obj, errors) {
  const { values, tunables } = obj;
  if (!isObj(values)) return;
  const byKey = new Map();
  if (Array.isArray(tunables)) for (const t of tunables) if (isObj(t) && typeof t.key === 'string') byKey.set(t.key, t);
  for (const [k, v] of Object.entries(values)) {
    const p = child('/values', k);
    const t = byKey.get(k);
    if (v === null) continue; // the schema check already says null is not allowed
    if (!t) {
      errors.push(`at ${show(p)}: "${k}" is not a declared tunable, so it cannot have a value. Tunable keys are: ${[...byKey.keys()].join(', ') || '(none)'}.`);
      continue;
    }
    checkValueAgainst(t, v, p, `the value of tunable "${k}"`, errors);
  }
}

function collectStrings(v, ptr, out) {
  if (typeof v === 'string') out.push([ptr, v]);
  else if (Array.isArray(v)) v.forEach((x, i) => collectStrings(x, child(ptr, i), out));
  else if (isObj(v)) for (const [k, x] of Object.entries(v)) collectStrings(x, child(ptr, k), out);
}

function checkEngine(obj, limits, errors) {
  if (!isObj(obj)) return;
  const size = JSON.stringify(obj).length;
  if (size > limits.max_json_bytes) errors.push(`at /: the cartridge is ${size} characters; the engine allows at most ${limits.max_json_bytes}. Shorten lists and text.`);

  checkTunables(obj.tunables, limits, errors);
  checkValues(obj, errors);

  checkCount(obj.rules, limits.max_rules, '/rules', 'rules', errors);
  checkUniqueIds(obj.rules, '/rules', 'rule', errors);
  if (Array.isArray(obj.rules)) obj.rules.forEach((r, i) => isObj(r) && checkParams(r.params, `/rules/${i}/params`, limits, errors));

  checkCount(obj.entities, limits.max_entities, '/entities', 'entities', errors);
  const entityIds = checkUniqueIds(obj.entities, '/entities', 'entity', errors);
  if (Array.isArray(obj.entities)) {
    let total = 0;
    obj.entities.forEach((e, i) => {
      if (!isObj(e)) return;
      total += Number.isInteger(e.count) ? e.count : 1;
      checkParams(e.params, `/entities/${i}/params`, limits, errors);
    });
    if (total > limits.max_total_entity_instances) {
      errors.push(`at /entities: the counts add up to ${total} entity instances; the engine allows at most ${limits.max_total_entity_instances}. Lower some "count" values or remove entities.`);
    }
  }

  checkCount(obj.ai, limits.max_ai, '/ai', 'ai behaviours', errors);
  checkUniqueIds(obj.ai, '/ai', 'ai behaviour', errors);
  if (Array.isArray(obj.ai)) {
    obj.ai.forEach((a, i) => {
      if (!isObj(a)) return;
      checkParams(a.params, `/ai/${i}/params`, limits, errors);
      if (Array.isArray(a.applies_to) && entityIds) {
        a.applies_to.forEach((ref, j) => {
          if (typeof ref === 'string' && !entityIds.has(ref)) {
            errors.push(`at /ai/${i}/applies_to/${j}: "${ref}" is not the id of any entity. Entity ids are: ${[...entityIds.keys()].join(', ') || '(none)'}.`);
          }
        });
      }
    });
  }

  if (isObj(obj.theme)) {
    checkCount(obj.theme.palette, limits.max_palette, '/theme/palette', 'palette colours', errors);
    checkCount(obj.theme.mood_words, limits.max_mood_words, '/theme/mood_words', 'mood words', errors);
    checkUniqueIds(obj.theme.palette, '/theme/palette', 'palette colour', errors, 'name');
  }

  if (Array.isArray(limits.blocked_terms) && limits.blocked_terms.length) {
    const strings = [];
    collectStrings(obj, '', strings);
    for (const [p, s] of strings) {
      const low = s.toLowerCase();
      const hit = limits.blocked_terms.find((t) => low.includes(String(t).toLowerCase()));
      if (hit) errors.push(`at ${show(p)}: the text contains a term that is not allowed in this game ("${hit}"). Reword it.`);
    }
  }
}

// ---------- public API ----------

export function validateCartridge(obj, opts = {}) {
  const limits = opts.limits ?? loadLimits();
  const schema = opts.schema ?? loadJsonFile(SCHEMA_FILE);
  const errors = validateAgainstSchema(schema, obj, { dir: opts.dir ?? HERE });
  checkEngine(obj, limits, errors);
  return { ok: errors.length === 0, errors: [...new Set(errors)] };
}

// What the Designer must leave exactly as the host gave it.
export const DESIGNER_IMMUTABLE = ['schema', 'id', 'title', 'seed', 'tunables', 'game'];
// What it may edit: values, theme, and the contents of rules, entities and ai.
export const DESIGNER_EDITABLE = ['values', 'theme', 'rules', 'entities', 'ai'];

export function validateDesignerOutput(base, out, opts = {}) {
  const res = validateCartridge(out, opts);
  const errors = [...res.errors];
  if (isObj(base) && isObj(out)) {
    for (const k of DESIGNER_IMMUTABLE) {
      if (k in base && k in out && !deepEqual(base[k], out[k])) {
        errors.push(`at ${show(child('', k))}: the Designer may not change "${k}". Restore it exactly as it was in the current cartridge. Only ${DESIGNER_EDITABLE.join(', ')} may change.`);
      }
    }
  }
  return { ok: errors.length === 0, errors: [...new Set(errors)] };
}

// Turn the model's raw reply text into one of: a cartridge, a refusal, or a list of errors.
// Strict on purpose: JSON only, no prose, no markdown fence.
export function parseDesignerReply(text, base, opts = {}) {
  const limits = opts.limits ?? loadLimits();
  const raw = typeof text === 'string' ? text : '';
  const trimmed = raw.trim();
  if (!trimmed) return { kind: 'invalid', errors: ['at /: the reply is empty. Return a JSON object: the full cartridge, or {"refusal": "<reason>"}.'] };
  let parsed;
  try {
    parsed = JSON.parse(trimmed);
  } catch (e) {
    const head = trimmed.length > 40 ? `${trimmed.slice(0, 40)}...` : trimmed;
    const wrapped = !trimmed.startsWith('{') && trimmed.includes('{');
    return {
      kind: 'invalid',
      errors: [wrapped
        ? `at /: the reply has text outside the JSON object (it starts with ${JSON.stringify(head)}). Return only the JSON object: no sentence before or after it and no markdown fence.`
        : `at /: the reply is not valid JSON (${e.message}). Return only one complete JSON object.`],
    };
  }
  if (!isObj(parsed)) return { kind: 'invalid', errors: [`at /: the reply must be a JSON object but is ${describe(parsed)}.`] };
  if ('refusal' in parsed) {
    const errors = [];
    if (Object.keys(parsed).length !== 1) errors.push('at /: a refusal must be exactly {"refusal": "<reason>"} with no other keys.');
    if (typeof parsed.refusal !== 'string' || parsed.refusal.trim() === '') errors.push('at /refusal: the reason must be a non-empty string.');
    else if (parsed.refusal.length > limits.max_refusal_chars) errors.push(`at /refusal: the reason is ${parsed.refusal.length} characters; the limit is ${limits.max_refusal_chars}.`);
    return errors.length ? { kind: 'invalid', errors } : { kind: 'refusal', refusal: parsed.refusal };
  }
  const res = base ? validateDesignerOutput(base, parsed, opts) : validateCartridge(parsed, opts);
  return res.ok ? { kind: 'cartridge', cartridge: parsed } : { kind: 'invalid', errors: res.errors };
}

// ---------- CLI ----------

function cli(argv) {
  const args = argv.slice(2);
  const baseIdx = args.indexOf('--base');
  let basePath;
  if (baseIdx !== -1) { basePath = args[baseIdx + 1]; args.splice(baseIdx, 2); }
  const file = args[0];
  if (!file) {
    console.error('usage: node schema/validate.mjs file.json [--base current.json]');
    return 2;
  }
  let text;
  let base;
  try {
    text = readFileSync(file, 'utf8');
    if (basePath) base = JSON.parse(readFileSync(basePath, 'utf8'));
  } catch (e) {
    console.error(`cannot read input: ${e.message}`);
    return 2;
  }
  let errors;
  try {
    const parsed = JSON.parse(text);
    errors = (base ? validateDesignerOutput(base, parsed) : validateCartridge(parsed)).errors;
  } catch (e) {
    if (e instanceof SyntaxError) errors = [`at /: the file is not valid JSON (${e.message}).`];
    else { console.error(`validator could not run: ${e.message}`); return 2; }
  }
  if (errors.length === 0) console.log(`PASS ${file}`);
  else { console.log(`FAIL ${file}`); for (const e of errors) console.log(`  ${e}`); }
  console.log(JSON.stringify({ tool: 'validate', file, ok: errors.length === 0, errors: errors.length }));
  return errors.length === 0 ? 0 : 1;
}

if (process.argv[1] && resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
  process.exitCode = cli(process.argv);
}
