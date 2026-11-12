// Lane A14 (C-0005): game.effects and the extended game.pickups. Run: node --test schema/effects.test.mjs
import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { validateCartridge, validateDesignerOutput, loadLimits, parseDesignerReply, BUILTIN_EFFECTS, EFFECT_STAT_FIELD, DESIGNER_GAME_EDITABLE } from './validate.mjs';
import { runDesigner } from '../tools/designer-eval/loop.mjs';

const HERE = dirname(fileURLToPath(import.meta.url));
const REPO = resolve(HERE, '..');
const read = (p) => JSON.parse(readFileSync(resolve(REPO, p), 'utf8'));
const regular = () => read('Game/Content/Cartridges/hockey.regular.cartridge.json');
const boost = () => read('Game/Content/Cartridges/hockey.boost.cartridge.json');
const has = (errors, pointer, fragment) => errors.some((e) => e.startsWith(`at ${pointer}:`) && e.includes(fragment));
const errorsOf = (c) => validateCartridge(c).errors;
const STATS = Object.keys(EFFECT_STAT_FIELD);

const slowThem = () => ({ id: 'slow-them', kind: 'custom', target: 'other_team', duration_frames: 300, trigger: 'goal_for', stack: 'refresh', icon: 'snail',
  stats: [{ stat: 'skater_speed', multiply: 0.7 }, { stat: 'skater_turn_rate', multiply: 0.8 }] });
const stickyPuck = () => ({ id: 'sticky-puck', target: 'puck', duration_frames: 300, trigger: 'pickup', stats: [{ stat: 'puck_drag', multiply: 3 }, { stat: 'puck_max_speed', multiply: 0.6 }] });

test('the built-in effects file is five effects in the game.effects shape (plus the magnitude notes)', () => {
  const b = read('Game/Content/Designer/builtin-effects.json');
  assert.equal(b.schema, 'tj.effects/1');
  assert.deepEqual(b.effects.map((e) => e.id), BUILTIN_EFFECTS);
  // Each one, under another id, is a valid game.effects entry.
  for (const e of b.effects) {
    const { magnitude_stats, inverse_magnitude_stats, ...def } = e;
    const c = regular(); c.game.effects = [{ ...def, id: `x-${def.id.replace(/_/g, '-')}` }];
    assert.deepEqual(errorsOf(c), [], def.id);
    for (const s of [...(magnitude_stats ?? []), ...(inverse_magnitude_stats ?? [])]) assert.ok(def.stats.some((st) => st.stat === s), `${def.id} ${s}`);
  }
});

test('effects: a full example validates; every stat name and every enum value is accepted', () => {
  const c = regular(); c.game.effects = [slowThem(), stickyPuck()];
  c.game.pickups = [{ kind: 'sticky-puck', spawn_every_frames: 600, duration_frames: 300, max_on_ice: 2 }];
  assert.deepEqual(errorsOf(c), []);
  for (const stat of STATS) { const d = regular(); d.game.effects = [{ ...slowThem(), stats: [{ stat, multiply: 2, add: 0 }] }]; assert.deepEqual(errorsOf(d), [], stat); }
  const lim = loadLimits().game_ranges;
  for (const key of ['effects[].target', 'effects[].trigger', 'effects[].stack', 'effects[].icon', 'effects[].kind']) {
    const field = key.split('.').at(-1);
    for (const v of lim[key].split('|')) {
      const d = regular(); const e = slowThem(); e[field] = v; d.game.effects = [e];
      assert.deepEqual(errorsOf(d), [], `${key} ${v}`);
    }
  }
});

test('effects: limits (12 effects, multiply 0.25-4, duration 60-7200, 1-4 stats, add within the stat range) with pointers', () => {
  const many = regular(); many.game.effects = Array.from({ length: 13 }, (_, i) => ({ ...slowThem(), id: `e-${i}` }));
  assert.ok(has(errorsOf(many), '/game/effects', '12'));
  const m = regular(); m.game.effects = [{ ...slowThem(), stats: [{ stat: 'skater_speed', multiply: 5 }] }];
  assert.ok(has(errorsOf(m), '/game/effects/0/stats/0/multiply', 'above the maximum 4'));
  m.game.effects[0].stats[0].multiply = 0.2;
  assert.ok(has(errorsOf(m), '/game/effects/0/stats/0/multiply', 'below the minimum 0.25'));
  const d = regular(); d.game.effects = [{ ...slowThem(), duration_frames: 59 }];
  assert.ok(has(errorsOf(d), '/game/effects/0/duration_frames', 'below the minimum 60'));
  d.game.effects[0].duration_frames = 7201;
  assert.ok(has(errorsOf(d), '/game/effects/0/duration_frames', 'above the maximum 7200'));
  d.game.effects[0].duration_frames = 300.5;
  assert.ok(has(errorsOf(d), '/game/effects/0/duration_frames', 'expected an integer'));
  const s = regular(); s.game.effects = [{ ...slowThem(), stats: [] }];
  assert.ok(has(errorsOf(s), '/game/effects/0/stats', 'at least 1'));
  const a = regular(); a.game.effects = [{ ...slowThem(), stats: [{ stat: 'skater_mass', add: 200 }] }];
  assert.ok(has(errorsOf(a), '/game/effects/0/stats/0/add', 'width 120'));
  a.game.effects[0].stats[0].add = -120;
  assert.deepEqual(errorsOf(a), []);
});

test('effects: an unknown stat, a built-in id, a duplicate id, a null and an unknown key are rejected with pointers', () => {
  const c = regular(); c.game.effects = [{ ...slowThem(), stats: [{ stat: 'jump_height', multiply: 2 }] }];
  assert.ok(has(errorsOf(c), '/game/effects/0/stats/0/stat', 'is not allowed'));
  assert.ok(has(errorsOf(c), '/game/effects/0/stats/0/stat', '"skater_speed"'));
  const b = regular(); b.game.effects = [{ ...slowThem(), id: 'speed' }];
  assert.ok(has(errorsOf(b), '/game/effects/0/id', 'built-in'));
  const dup = regular(); dup.game.effects = [slowThem(), slowThem()];
  assert.ok(has(errorsOf(dup), '/game/effects/1/id', 'duplicate'));
  const n = regular(); n.game.effects = [{ ...slowThem(), icon: null }];
  assert.ok(has(errorsOf(n), '/game/effects/0/icon', 'null'));
  const u = regular(); u.game.effects = [{ ...slowThem(), seconds: 3 }];
  assert.ok(has(errorsOf(u), '/game/effects/0/seconds', 'unknown key "seconds"'));
  const req = regular(); req.game.effects = [{ id: 'bare' }];
  for (const k of ['target', 'duration_frames', 'stats', 'trigger']) assert.ok(has(errorsOf(req), `/game/effects/0/${k}`, 'required'), k);
});

test('pickups: built-ins and declared effect ids are accepted, anything else is rejected naming both lists; zone and max_on_ice', () => {
  for (const kind of BUILTIN_EFFECTS) { const c = regular(); c.game.pickups = [{ kind, spawn_every_frames: 600, duration_frames: 60 }]; assert.deepEqual(errorsOf(c), [], kind); }
  const c = regular(); c.game.pickups = [{ kind: 'rocket', spawn_every_frames: 600, duration_frames: 60 }];
  const errors = errorsOf(c);
  assert.ok(has(errors, '/game/pickups/0/kind', 'is not allowed'));
  assert.ok(has(errors, '/game/pickups/0/kind', '"speed", "precision", "heavy_puck", "big_puck", "reverse_other"'));
  assert.ok(has(errors, '/game/pickups/0/kind', 'declared: none'));
  c.game.effects = [stickyPuck()];
  assert.ok(has(errorsOf(c), '/game/pickups/0/kind', 'declared: sticky-puck'));
  c.game.pickups[0].kind = 'sticky-puck';
  assert.deepEqual(errorsOf(c), []);
  c.game.pickups[0].max_on_ice = 5;
  assert.ok(has(errorsOf(c), '/game/pickups/0/max_on_ice', 'above the maximum 4'));
  c.game.pickups[0].max_on_ice = 1;
  c.game.pickups[0].zone = 'lane';
  assert.ok(has(errorsOf(c), '/game/pickups/0/zone', 'not the id of any zone'));
  const bad = regular(); bad.game.pickups = [{ kind: 'Speed!', spawn_every_frames: 600, duration_frames: 60 }];
  assert.ok(has(errorsOf(bad), '/game/pickups/0/kind', 'built-in'));
  assert.equal(boost().game.pickups.length, 3);
  assert.deepEqual(errorsOf(boost()), []);
});

test('effects zone: only with zone_enter / zone_exit, and only a declared rink.zones id', () => {
  const c = regular(); c.game.effects = [{ ...slowThem(), trigger: 'zone_enter', zone: 'lane' }];
  assert.ok(has(errorsOf(c), '/game/effects/0/zone', 'not the id of any zone'));
  const p = regular(); p.game.effects = [{ ...slowThem(), trigger: 'pickup', zone: 'lane' }];
  assert.ok(has(errorsOf(p), '/game/effects/0/zone', 'only applies to the triggers "zone_enter" and "zone_exit"'));
});

test('Designer: may add effects and pickups within limits, and DESIGNER_GAME_EDITABLE lists both', () => {
  assert.ok(DESIGNER_GAME_EDITABLE.includes('effects') && DESIGNER_GAME_EDITABLE.includes('pickups'));
  const base = regular();
  const out = regular(); out.game.effects = [slowThem(), stickyPuck()]; out.game.pickups = [{ kind: 'sticky-puck', spawn_every_frames: 600, duration_frames: 300 }];
  assert.deepEqual(validateDesignerOutput(base, out).errors, []);
  const bad = regular(); bad.game.effects = [{ ...slowThem(), stats: [{ stat: 'jump_height', multiply: 2 }] }];
  assert.ok(has(validateDesignerOutput(base, bad).errors, '/game/effects/0/stats/0/stat', 'is not allowed'));
});

// DesignerEffectsRoundTrip: ten sentences through the attempt chain with a mock model that answers with the cut each sentence asks for.
// Every cut must be a valid Designer output over hockey.regular; the eleventh names a stat that does not exist and is rejected with its pointer.
const SENTENCES = [
  ['a boost that slows the other team when I score', (g) => { g.effects = [slowThem()]; }],
  ['sticky puck for 5 s on pickup', (g) => { g.effects = [stickyPuck()]; g.pickups = [{ kind: 'sticky-puck', spawn_every_frames: 900, duration_frames: 300 }]; }],
  ['turbo lane', (g) => { g.effects = [{ id: 'turbo-lane', kind: 'speed', target: 'self', duration_frames: 120, trigger: 'zone_enter', stack: 'refresh', icon: 'bolt', stats: [{ stat: 'skater_speed', multiply: 1.6 }] }]; }],
  ['lots of speed boosts', (g) => { g.pickups = [{ kind: 'speed', spawn_every_frames: 360, duration_frames: 300, magnitude: 1.8, max_on_ice: 3 }]; }],
  ['giant puck after every goal', (g) => { g.effects = [{ id: 'giant-puck', kind: 'big_puck', target: 'puck', duration_frames: 600, trigger: 'goal_for', stats: [{ stat: 'puck_radius', multiply: 2.5 }] }]; }],
  ['the team that concedes gets faster', (g) => { g.effects = [{ id: 'comeback', target: 'own_team', duration_frames: 900, trigger: 'goal_against', stack: 'extend', stats: [{ stat: 'skater_speed', multiply: 1.2 }, { stat: 'skater_accel', multiply: 1.2 }] }]; }],
  ['a check knocks them down for longer', (g) => { g.effects = [{ id: 'hard-hits', target: 'other_team', duration_frames: 600, trigger: 'check_landed', stats: [{ stat: 'knockdown_frames', add: 60 }] }]; }],
  ['rocket shots for the whole take', (g) => { g.effects = [{ id: 'rocket-take', target: 'everyone', duration_frames: 7200, trigger: 'period_start', stats: [{ stat: 'shot_speed', multiply: 1.5 }, { stat: 'puck_max_speed', multiply: 1.4 }] }]; }],
  ['glass ice when the director calls', (g) => { g.effects = [{ id: 'glass-call', target: 'rink', duration_frames: 1200, trigger: 'director', icon: 'snowflake', stats: [{ stat: 'ice_friction', multiply: 0.25 }] }]; }],
  ['heavy pucks and precision pickups', (g) => { g.pickups = [{ kind: 'heavy_puck', spawn_every_frames: 1200, duration_frames: 360 }, { kind: 'precision', spawn_every_frames: 900, duration_frames: 480, magnitude: 2 }]; }],
];
const mockFor = (cartridge) => async () => ({ text: JSON.stringify(cartridge), stop_reason: 'end_turn', usage: { input_tokens: 10, output_tokens: 10 }, ms: 1 });

test('DesignerEffectsRoundTrip: ten sentences through the mock produce valid effects; one bad stat is rejected with a pointer', async () => {
  const base = regular();
  for (const [sentence, edit] of SENTENCES) {
    const cut = regular(); edit(cut.game);
    const r = await runDesigner({ cartridge: base, sentence, callModel: mockFor(cut) });
    assert.equal(r.outcome, 'first_try', `${sentence}: ${JSON.stringify(r.attempts)}`);
    assert.ok(r.cartridge.game.effects || r.cartridge.game.pickups, sentence);
  }
  const bad = regular(); bad.game.effects = [{ id: 'jump', target: 'self', duration_frames: 300, trigger: 'pickup', stats: [{ stat: 'jump_height', multiply: 2 }] }];
  const r = await runDesigner({ cartridge: base, sentence: 'let me jump', callModel: mockFor(bad) });
  assert.equal(r.outcome, 'failed');
  assert.equal(r.attempts.length, 3);
  assert.ok(r.attempts[0].errors.some((e) => e.startsWith('at /game/effects/0/stats/0/stat:') && e.includes('"skater_speed"')), r.attempts[0].errors.join('\n'));
  const parsed = parseDesignerReply(JSON.stringify(bad), base);
  assert.equal(parsed.kind, 'invalid');
});
