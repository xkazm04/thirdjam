import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { spawnSync } from 'node:child_process';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import {
  validateCartridge, validateAgainstSchema, validateDesignerOutput, parseDesignerReply, loadLimits,
} from './validate.mjs';

const HERE = dirname(fileURLToPath(import.meta.url));
const FIX = resolve(HERE, 'fixtures');
const good = () => JSON.parse(readFileSync(resolve(FIX, 'test-arena.cartridge.json'), 'utf8'));
const VALIDATE = resolve(HERE, 'validate.mjs');

function cli(file, ...extra) {
  const r = spawnSync(process.execPath, [VALIDATE, file, ...extra], { encoding: 'utf8' });
  return { code: r.status, out: r.stdout, err: r.stderr };
}

test('the fixture cartridge is valid', () => {
  const r = validateCartridge(good());
  assert.deepEqual(r.errors, []);
  assert.equal(r.ok, true);
});

test('CLI exits 0 for the fixture and prints a JSON last line', () => {
  const r = cli(resolve(FIX, 'test-arena.cartridge.json'));
  assert.equal(r.code, 0);
  const last = JSON.parse(r.out.trim().split('\n').at(-1));
  assert.equal(last.tool, 'validate');
  assert.equal(last.ok, true);
});

// Each bad fixture: the file name, and a fragment of the sentence a model will read.
const BAD = {
  'extra-key': ['/difficulty', 'unknown key "difficulty"'],
  'missing-required': ['/theme/name', 'required key "name" is missing'],
  'wrong-type': ['/values/pace', 'must be a number but is a string'],
  'out-of-range': ['/values/obstacle_count', 'above the maximum 40'],
  'null-value': ['/values/assist', 'found null (omit the key'],
  'bad-id': ['/id', 'kebab-case'],
  'duplicate-id': ['/rules/1/id', 'duplicate rule id "basic-score"'],
  'unknown-value-key': ['/values/gravity_flip', 'is not a declared tunable'],
  'wall-clock': ['/rules/1/params/limit_seconds', 'wall-clock time'],
  'game-not-empty': ['/game/mode', 'unknown key "mode"'],
  'bad-reference': ['/ai/0/applies_to/0', 'is not the id of any entity'],
  'bad-hex': ['/theme/palette/0/hex', '#RRGGBB'],
  'bad-rule-kind': ['/rules/0/kind', 'use one of "score", "timer"'],
  'too-many-rules': ['/rules', 'the engine allows at most 24'],
  'fractional-frames': ['/rules/1/params/limit_frames', 'must be a whole number'],
  'tunable-bad-bounds': ['/tunables/0/min', 'is above max'],
  'not-json': ['/', 'not valid JSON'],
};

for (const [name, [pointer, fragment]] of Object.entries(BAD)) {
  test(`bad fixture ${name}: CLI exits 1 with pointer ${pointer}`, () => {
    const r = cli(resolve(FIX, 'bad', `${name}.json`));
    assert.equal(r.code, 1, r.out + r.err);
    const line = r.out.split('\n').find((l) => l.includes(fragment));
    assert.ok(line, `no error sentence containing "${fragment}" in:\n${r.out}`);
    assert.ok(line.includes(`at ${pointer}:`), `sentence should start with the pointer ${pointer}: ${line}`);
  });
}

test('CLI exits 2 when it cannot run', () => {
  assert.equal(cli(resolve(FIX, 'does-not-exist.json')).code, 2);
  const r = spawnSync(process.execPath, [VALIDATE], { encoding: 'utf8' });
  assert.equal(r.status, 2);
});

test('absent values are omitted keys: a value may be left out, and the default is then used', () => {
  const c = good();
  delete c.values.pace;
  assert.equal(validateCartridge(c).ok, true);
});

test('integer tunable rejects a fractional value; enum rejects a non-option; boolean rejects a string', () => {
  const c = good();
  c.values.obstacle_count = 4.5;
  c.values.difficulty_preset = 'impossible';
  c.values.assist = 'yes';
  const { errors } = validateCartridge(c);
  assert.ok(errors.some((e) => e.startsWith('at /values/obstacle_count:') && e.includes('whole number')));
  assert.ok(errors.some((e) => e.startsWith('at /values/difficulty_preset:') && e.includes('use one of calm, normal, wild')));
  assert.ok(errors.some((e) => e.startsWith('at /values/assist:') && e.includes('true or false')));
});

test('a value below the minimum is reported with the allowed range', () => {
  const c = good();
  c.values.pace = 0.1;
  const { errors } = validateCartridge(c);
  assert.ok(errors.some((e) => e.includes('below the minimum 0.5') && e.includes('allowed range 0.5 to 3')));
});

test('a *_frames tunable must be an integer and a *_seconds tunable is refused', () => {
  const c = good();
  c.tunables[3].type = 'number';
  c.tunables.push({ key: 'delay_seconds', type: 'number', min: 0, max: 5, default: 1, description: 'x' });
  const { errors } = validateCartridge(c);
  assert.ok(errors.some((e) => e.startsWith('at /tunables/3/type:') && e.includes('"integer"')));
  assert.ok(errors.some((e) => e.startsWith('at /tunables/7/key:') && e.includes('delay_frames')));
});

test('entity instance total is limited', () => {
  const c = good();
  c.entities[0].count = 1000;
  const { errors } = validateCartridge(c);
  assert.ok(errors.some((e) => e.startsWith('at /entities:') && e.includes('entity instances')));
});

test('oversized cartridge is rejected with a size sentence', () => {
  const c = good();
  const limits = { ...loadLimits(), max_json_bytes: 500 };
  const { errors } = validateCartridge(c, { limits });
  assert.ok(errors.some((e) => e.startsWith('at /:') && e.includes('allows at most 500')));
});

test('blocked_terms from limits.json are enforced on every string', () => {
  const c = good();
  c.theme.name = 'Totally Forbidden Place';
  const limits = { ...loadLimits(), blocked_terms: ['forbidden'] };
  const { errors } = validateCartridge(c, { limits });
  assert.ok(errors.some((e) => e.startsWith('at /theme/name:') && e.includes('"forbidden"')));
  assert.equal(validateCartridge(good(), { limits }).ok, true);
});

test('a pointer escapes ~ and / in key names', () => {
  const schema = { type: 'object', additionalProperties: false, properties: {} };
  const errs = validateAgainstSchema(schema, { 'a/b~c': 1 });
  assert.ok(errs[0].startsWith('at /a~1b~0c:'));
});

test('parameter names must be snake_case', () => {
  const c = good();
  c.entities[0].params.BadName = 1;
  const { errors } = validateCartridge(c);
  assert.ok(errors.some((e) => e.startsWith('at /entities/0/params/BadName:') && e.includes('snake_case')));
});

test('the schema keyword subset: an unsupported keyword is a loud error, not silently ignored', () => {
  assert.throws(() => validateAgainstSchema({ oneOf: [] }, 1), /not in the supported subset/);
});

test('subset keywords work on a small ad hoc schema', () => {
  const schema = {
    type: 'object',
    required: ['n', 'list'],
    additionalProperties: false,
    properties: {
      n: { type: 'integer', minimum: 1, maximum: 3 },
      list: { type: 'array', minItems: 1, maxItems: 2, items: { type: 'string', pattern: '^a' } },
      k: { const: 'x' },
    },
  };
  assert.deepEqual(validateAgainstSchema(schema, { n: 2, list: ['ab'] }), []);
  const errs = validateAgainstSchema(schema, { n: 9, list: ['b', 'ab', 'ac'], k: 'y' });
  assert.ok(errs.some((e) => e.startsWith('at /n:') && e.includes('above the maximum 3')));
  assert.ok(errs.some((e) => e.startsWith('at /list:') && e.includes('the limit is 2')));
  assert.ok(errs.some((e) => e.startsWith('at /list/0:') && e.includes('pattern ^a')));
  assert.ok(errs.some((e) => e.startsWith('at /k:') && e.includes('must be exactly "x"')));
});

// ---- Designer-specific: what may change, and the reply shape ----

test('the Designer may change values, theme and list contents', () => {
  const base = good();
  const out = good();
  out.values.pace = 2;
  out.theme.mood_words = ['bold'];
  out.rules.push({ id: 'extra-score', kind: 'score', params: {} });
  assert.equal(validateDesignerOutput(base, out).ok, true);
});

test('the Designer may not change seed, id, title, tunables or game', () => {
  const base = good();
  const out = good();
  out.seed = 1;
  out.id = 'other-arena';
  out.title = 'Other';
  out.tunables[0].max = 99;
  const { errors } = validateDesignerOutput(base, out);
  for (const k of ['seed', 'id', 'title', 'tunables']) {
    assert.ok(errors.some((e) => e.startsWith(`at /${k}:`) && e.includes(`may not change "${k}"`)), k);
  }
});

test('parseDesignerReply: cartridge, refusal, prose-wrapped, broken, empty', () => {
  const base = good();
  const ok = parseDesignerReply(JSON.stringify(base), base);
  assert.equal(ok.kind, 'cartridge');

  const ref = parseDesignerReply('{"refusal":"I cannot do that."}', base);
  assert.deepEqual(ref, { kind: 'refusal', refusal: 'I cannot do that.' });

  const wrapped = parseDesignerReply(`Sure! Here you go:\n${JSON.stringify(base)}`, base);
  assert.equal(wrapped.kind, 'invalid');
  assert.match(wrapped.errors[0], /text outside the JSON object/);

  const fenced = parseDesignerReply('```json\n{}\n```', base);
  assert.equal(fenced.kind, 'invalid');

  const broken = parseDesignerReply('{"a":', base);
  assert.match(broken.errors[0], /not valid JSON/);

  assert.match(parseDesignerReply('  ', base).errors[0], /empty/);
  assert.match(parseDesignerReply('[1]', base).errors[0], /must be a JSON object/);
});

test('a refusal with extra keys or an empty reason is invalid', () => {
  const base = good();
  assert.match(parseDesignerReply('{"refusal":"no","values":{}}', base).errors[0], /no other keys/);
  assert.match(parseDesignerReply('{"refusal":""}', base).errors[0], /non-empty string/);
});
