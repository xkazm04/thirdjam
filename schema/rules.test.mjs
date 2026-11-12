// Lane A15 (C-0005): game.rules.hooks, game.rules.variants, game.rink.zones, obstacles bumper / goal_shield, and the Designer's
// frame-600 guard (DesignerRulesRoundTrip). Run: node --test schema/rules.test.mjs
import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { validateCartridge, validateDesignerOutput, parseDesignerReply, loadLimits, expandVariants, hookEndsMatchEarly, DESIGNER_GAME_EDITABLE } from './validate.mjs';

const HERE = dirname(fileURLToPath(import.meta.url));
const REPO = resolve(HERE, '..');
const read = (p) => JSON.parse(readFileSync(resolve(REPO, p), 'utf8'));
const regular = () => read('Game/Content/Cartridges/hockey.regular.cartridge.json');
const chaos = () => read('Game/Content/Cartridges/hockey.chaos.cartridge.json');
const has = (errors, pointer, fragment) => errors.some((e) => e.startsWith(`at ${pointer}:`) && e.includes(fragment));
const errorsOf = (c) => validateCartridge(c).errors;
const designer = (out) => validateDesignerOutput(regular(), out).errors;

const lastGoalDouble = () => ({ id: 'last-goal-double', when: 'on_goal', team: 'scoring', score: { at_least: 3 }, then: [{ do: 'add_score', value: 1 }, { do: 'announce', text: 'LAST GOAL COUNTS DOUBLE' }] });
const shrinkEveryGoal = () => ({ id: 'shrink-every-goal', when: 'on_goal', team: 'any', then: [{ do: 'shrink_rink', value: 0.1 }, { do: 'announce', text: 'THE RINK SHRINKS 10%' }] });
const slowCentre = () => ({ id: 'centre-slow', shape: 'circle', x: 0, z: 0, r: 6, kind: 'slow_ice', strength: 0.6, visible: true });

test('hockey.chaos validates: sudden death, shrink per goal, two bumpers, one slow circle', () => {
  const c = chaos();
  assert.deepEqual(errorsOf(c), []);
  assert.deepEqual(c.game.rules.variants, { sudden_death: true, shrink_per_goal: 0.1 });
  assert.deepEqual(c.game.rink.obstacles.map((o) => o.kind), ['bumper', 'bumper']);
  assert.deepEqual(c.game.rink.zones.map((z) => [z.kind, z.shape]), [['slow_ice', 'circle']]);
});

test('the three worked examples of the Designer prompt validate', () => {
  const a = regular(); a.game.rules.hooks = [lastGoalDouble()];
  assert.deepEqual(errorsOf(a), []);
  const b = regular(); b.game.rules.variants = { shrink_per_goal: 0.1 };
  assert.deepEqual(errorsOf(b), []);
  const c = regular(); c.game.rink.zones = [slowCentre()];
  assert.deepEqual(errorsOf(c), []);
  // and as Designer output over hockey.regular
  assert.deepEqual(designer(a), []); assert.deepEqual(designer(b), []); assert.deepEqual(designer(c), []);
  assert.ok(['rules.hooks', 'rules.variants', 'rink.zones'].every((p) => DESIGNER_GAME_EDITABLE.includes(p)));
});

test('limits: 8 hooks, 8 zones, 4 actions per hook, 12 obstacles', () => {
  const lim = loadLimits().game_ranges;
  assert.equal(lim['rules.hooks[]'], '0..8'); assert.equal(lim['rink.zones[]'], '0..8'); assert.equal(lim['rules.hooks[].then[]'], '1..4');
  const h = regular(); h.game.rules.hooks = Array.from({ length: 9 }, (_, i) => ({ ...lastGoalDouble(), id: `h-${i}` }));
  assert.ok(has(errorsOf(h), '/game/rules/hooks', 'the limit is 8'));
  const z = regular(); z.game.rink.zones = Array.from({ length: 9 }, (_, i) => ({ ...slowCentre(), id: `z-${i}` }));
  assert.ok(has(errorsOf(z), '/game/rink/zones', 'the limit is 8'));
  const t = regular(); t.game.rules.hooks = [{ ...lastGoalDouble(), then: Array.from({ length: 5 }, () => ({ do: 'swap_nets' })) }];
  assert.ok(has(errorsOf(t), '/game/rules/hooks/0/then', 'the limit is 4'));
  const e = regular(); e.game.rules.hooks = [{ ...lastGoalDouble(), then: [] }];
  assert.ok(has(errorsOf(e), '/game/rules/hooks/0/then', 'at least 1'));
});

test('zones: inside the rink, shape needs its size, ids unique, enums exact', () => {
  const out = regular(); out.game.rink.zones = [{ ...slowCentre(), x: 60 }];
  assert.ok(has(errorsOf(out), '/game/rink/zones/0', 'outside the rink'));
  const rect = regular(); rect.game.rink.zones = [{ id: 'lane', shape: 'rect', x: 0, z: 0, kind: 'fast_ice' }];
  assert.ok(has(errorsOf(rect), '/game/rink/zones/0/w', 'rect zone needs w'));
  assert.ok(has(errorsOf(rect), '/game/rink/zones/0/d', 'rect zone needs d'));
  const circ = regular(); circ.game.rink.zones = [{ id: 'c', shape: 'circle', x: 0, z: 0, kind: 'no_shoot', w: 3 }];
  assert.ok(has(errorsOf(circ), '/game/rink/zones/0/r', 'circle zone needs r'));
  assert.ok(has(errorsOf(circ), '/game/rink/zones/0', 'no w or d'));
  const dup = regular(); dup.game.rink.zones = [slowCentre(), { ...slowCentre(), x: 10 }];
  assert.ok(has(errorsOf(dup), '/game/rink/zones/1/id', 'duplicate zone id'));
  const kind = regular(); kind.game.rink.zones = [{ ...slowCentre(), kind: 'lava' }];
  assert.ok(has(errorsOf(kind), '/game/rink/zones/0/kind', '"slow_ice", "fast_ice", "bumper", "no_shoot", "boost_only"'));
  const nul = regular(); nul.game.rink.zones = [{ ...slowCentre(), strength: null }];
  assert.ok(has(errorsOf(nul), '/game/rink/zones/0/strength', 'null'));
  const extra = regular(); extra.game.rink.zones = [{ ...slowCentre(), colour: '#ff0000' }];
  assert.ok(has(errorsOf(extra), '/game/rink/zones/0/colour', 'unknown key'));
});

test('obstacles: bumper and goal_shield are kinds, inside the rink as before', () => {
  const c = regular(); c.game.rink.obstacles = [{ kind: 'bumper', x: 10, z: 5, size: 2 }, { kind: 'goal_shield', x: 38, z: 0, size: 3 }];
  assert.deepEqual(errorsOf(c), []);
  const bad = regular(); bad.game.rink.obstacles = [{ kind: 'wall', x: 0, z: 0, size: 2 }];
  assert.ok(has(errorsOf(bad), '/game/rink/obstacles/0/kind', '"box", "pillar", "bumper", "goal_shield"'));
});

test('hooks: triggers and actions need their fields; ranges hold; ids unique', () => {
  const noEvery = regular(); noEvery.game.rules.hooks = [{ id: 'tick', when: 'on_frame', then: [{ do: 'swap_nets' }] }];
  assert.ok(has(errorsOf(noEvery), '/game/rules/hooks/0/every_frames', 'needs every_frames'));
  const stray = regular(); stray.game.rules.hooks = [{ ...lastGoalDouble(), every_frames: 600 }];
  assert.ok(has(errorsOf(stray), '/game/rules/hooks/0/every_frames', 'only applies to when "on_frame"'));
  const reached = regular(); reached.game.rules.hooks = [{ id: 'r', when: 'on_score_reached', then: [{ do: 'swap_nets' }] }];
  assert.ok(has(errorsOf(reached), '/game/rules/hooks/0/score', 'needs score.at_least'));
  for (const [act, missing] of [['add_score', 'value'], ['set_param', 'param'], ['announce', 'text'], ['spawn_pickup', 'kind'], ['apply_modifier', 'frames'], ['shrink_rink', 'value']]) {
    const c = regular(); c.game.rules.hooks = [{ id: 'x', when: 'on_goal', then: [{ do: act }] }];
    assert.ok(has(errorsOf(c), `/game/rules/hooks/0/then/0/${missing}`, `needs ${missing}`), `${act} needs ${missing}`);
  }
  const frac = regular(); frac.game.rules.hooks = [{ id: 'x', when: 'on_goal', then: [{ do: 'shrink_rink', value: 0.9 }] }];
  assert.ok(has(errorsOf(frac), '/game/rules/hooks/0/then/0/value', 'fraction'));
  const half = regular(); half.game.rules.hooks = [{ id: 'x', when: 'on_goal', then: [{ do: 'add_score', value: 1.5 }] }];
  assert.ok(has(errorsOf(half), '/game/rules/hooks/0/then/0/value', 'whole number'));
  const mult = regular(); mult.game.rules.hooks = [{ id: 'x', when: 'on_goal', then: [{ do: 'set_score_multiplier', value: 7 }] }];
  assert.ok(has(errorsOf(mult), '/game/rules/hooks/0/then/0/value', '1 to 5'));
  const dup = regular(); dup.game.rules.hooks = [lastGoalDouble(), lastGoalDouble()];
  assert.ok(has(errorsOf(dup), '/game/rules/hooks/1/id', 'duplicate hook id'));
  const when = regular(); when.game.rules.hooks = [{ ...lastGoalDouble(), when: 'on_whistle' }];
  assert.ok(has(errorsOf(when), '/game/rules/hooks/0/when', '"on_goal"'));
  const secs = regular(); secs.game.rules.hooks = [{ id: 't', when: 'on_frame', every_seconds: 10, then: [{ do: 'swap_nets' }] }];
  assert.ok(has(errorsOf(secs), '/game/rules/hooks/0/every_seconds', 'unknown key'));
  const range = regular(); range.game.rules.hooks = [{ id: 't', when: 'on_frame', every_frames: 30, then: [{ do: 'swap_nets' }] }];
  assert.ok(has(errorsOf(range), '/game/rules/hooks/0/every_frames', 'below the minimum 60'));
  const text = regular(); text.game.rules.hooks = [{ id: 't', when: 'on_goal', then: [{ do: 'announce', text: 'x'.repeat(61) }] }];
  assert.ok(has(errorsOf(text), '/game/rules/hooks/0/then/0/text', 'limit is 60'));
});

test('variants: shapes and ranges; expansion mirrors the C++ loader (hockey.chaos gives two hooks, sudden death armed at 600)', () => {
  const bad = regular(); bad.game.rules.variants = { mercy_goals: 1 };
  assert.ok(has(errorsOf(bad), '/game/rules/variants/mercy_goals', 'below the minimum 2'));
  const unk = regular(); unk.game.rules.variants = { golden_goal: true };
  assert.ok(has(errorsOf(unk), '/game/rules/variants/golden_goal', 'unknown key'));
  const hooks = expandVariants(chaos().game);
  assert.deepEqual(hooks.map((h) => h.id), ['variant-sudden-death', 'variant-shrink-per-goal']);
  assert.equal(hooks[0].after_frame, 600);
  assert.deepEqual(hooks[1].then[0], { do: 'shrink_rink', value: 0.1 });
  assert.equal(hooks[1].then[1].text, 'THE RINK SHRINKS 10%');
  // every expanded hook is itself a valid hook, and none of them ends the match early
  for (const h of hooks) { const c = regular(); c.game.rules.hooks = [h]; assert.deepEqual(errorsOf(c), [], h.id); assert.equal(hookEndsMatchEarly(h), false, h.id); }
  const all = regular(); all.game.rules = { mode: 'timer', timer_frames: 9000, variants: { sudden_death: true, last_goal_double: true, mercy_goals: 3, period_count: 3, shrink_per_goal: 0.1 } };
  const ex = expandVariants(all.game);
  assert.deepEqual(ex.map((h) => h.id), ['variant-sudden-death', 'variant-mercy', 'variant-periods', 'variant-shrink-per-goal']); // last_goal_double needs first_to
  assert.equal(ex.find((h) => h.id === 'variant-periods').every_frames, 3000);
  const ft = regular(); ft.game.rules.variants = { last_goal_double: true };
  assert.deepEqual(expandVariants(ft.game)[0].score, { at_least: 3 });
});

// Ten sentences a player might type and the hooks / zones / variants the Designer should write for them.
const MOCK = [
  ['the last goal counts double', (g) => { g.rules.hooks = [lastGoalDouble()]; }],
  ['the rink shrinks by 10 percent every goal', (g) => { g.rules.variants = { shrink_per_goal: 0.1 }; }],
  ['put a slow-ice circle at centre', (g) => { g.rink.zones = [slowCentre()]; }],
  ['sudden death after the first minute', (g) => { g.rules.hooks = [{ id: 'sudden-death', when: 'on_goal', after_frame: 3600, then: [{ do: 'end_match', team: 'scoring' }, { do: 'announce', text: 'SUDDEN DEATH' }] }]; }],
  ['mercy rule at a four goal lead', (g) => { g.rules.variants = { mercy_goals: 4 }; }],
  ['swap the nets every minute', (g) => { g.rules.hooks = [{ id: 'swap-minute', when: 'on_frame', every_frames: 3600, team: 'any', then: [{ do: 'swap_nets' }, { do: 'announce', text: 'NETS SWAPPED' }] }]; }],
  ['make the ice slippery when someone gets knocked down', (g) => { g.rules.hooks = [{ id: 'slick-on-knockdown', when: 'on_knockdown', then: [{ do: 'set_param', param: 'ice_friction', value: 0.005 }, { do: 'announce', text: 'GLASS ICE' }] }]; }],
  ['a fast lane down the middle and no shooting from the corners', (g) => { g.rink.zones = [{ id: 'fast-lane', shape: 'rect', x: 0, z: 0, w: 60, d: 6, kind: 'fast_ice' }, { id: 'no-shoot-left', shape: 'rect', x: -35, z: 0, w: 14, d: 40, kind: 'no_shoot', visible: false }]; }],
  ['bumpers in the corners and a speed boost for whoever concedes', (g) => { g.rink.obstacles = [{ kind: 'bumper', x: -30, z: -12, size: 2 }, { kind: 'bumper', x: 30, z: 12, size: 2 }]; g.rules.hooks = [{ id: 'catch-up', when: 'on_goal', team: 'conceding', then: [{ do: 'apply_modifier', kind: 'speed', frames: 600 }] }]; }],
  ['goals are worth two points once a team leads by two', (g) => { g.rules.hooks = [{ id: 'double-points', when: 'on_goal', score: { lead_at_least: 2 }, then: [{ do: 'set_score_multiplier', team: 'conceding', value: 2 }, { do: 'announce', text: 'COMEBACK: goals count double' }] }]; }],
];

test('DesignerRulesRoundTrip: ten mock sentences give valid Designer output over hockey.regular', () => {
  for (const [sentence, edit] of MOCK) {
    const out = regular(); edit(out.game);
    const r = parseDesignerReply(JSON.stringify(out), regular());
    assert.equal(r.kind, 'cartridge', `${sentence}: ${JSON.stringify(r.errors)}`);
  }
});

test('DesignerRulesRoundTrip: a hook that ends the match before frame 600 is rejected, the guarded forms pass', () => {
  const early = regular(); early.game.rules.hooks = [{ id: 'instant', when: 'on_goal', then: [{ do: 'end_match', team: 'scoring' }] }];
  assert.deepEqual(errorsOf(early), [], 'a human-authored cartridge may do this');
  assert.ok(has(designer(early), '/game/rules/hooks/0', 'before frame 600'));
  const tick = regular(); tick.game.rules.hooks = [{ id: 'tick', when: 'on_frame', every_frames: 300, then: [{ do: 'end_match' }] }];
  assert.ok(has(designer(tick), '/game/rules/hooks/0', 'before frame 600'));
  const armed = regular(); armed.game.rules.hooks = [{ id: 'instant', when: 'on_goal', after_frame: 600, then: [{ do: 'end_match', team: 'scoring' }] }];
  assert.deepEqual(designer(armed), []);
  const second = regular(); second.game.rules.hooks = [{ id: 'two', when: 'on_goal', score: { at_least: 2 }, then: [{ do: 'end_match', team: 'scoring' }] }];
  assert.deepEqual(designer(second), []);
  const period = regular(); period.game.rules.hooks = [{ id: 'p', when: 'on_period_end', then: [{ do: 'end_match' }] }];
  assert.deepEqual(designer(period), []);
  // the variants are always safe: sudden_death is armed at 600 by the loader
  const sd = regular(); sd.game.rules.variants = { sudden_death: true };
  assert.deepEqual(designer(sd), []);
  assert.equal(hookEndsMatchEarly({ id: 'x', when: 'on_goal', then: [{ do: 'announce', text: 'hi' }] }), false);
});

test('the Designer may still not touch roles, spawns or the camera while editing hooks', () => {
  const out = regular(); out.game.rules.hooks = [lastGoalDouble()]; out.game.camera.mode = 'arena';
  assert.ok(has(designer(out), '/game/camera/mode', 'may not change'));
});
