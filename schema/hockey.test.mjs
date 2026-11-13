import { test } from 'node:test';
import assert from 'node:assert/strict';
import { readFileSync, readdirSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';
import { validateCartridge, validateDesignerOutput, loadLimits, DESIGNER_GAME_EDITABLE } from './validate.mjs';

const HERE = dirname(fileURLToPath(import.meta.url));
const CART_DIR = resolve(HERE, '..', 'Game', 'Content', 'Cartridges');
const read = (f) => JSON.parse(readFileSync(resolve(CART_DIR, f), 'utf8'));
const regular = () => read('hockey.regular.cartridge.json');
// R-0010: the old 120 x 60 regular, kept as hockey.arena.large (geometry tests below were written for it)
const large = () => read('hockey.arena.large.cartridge.json');
const PRESETS = [
  'hockey.test.visual-b2.cartridge.json',
  'hockey.test.visual-a26.cartridge.json',   // lane A26: the regular cut with game.skater.visual toy_skeletal (the proof cartridge)
  'hockey.test.figure-toy.cartridge.json', 'hockey.test.figure-b1.cartridge.json', 'hockey.test.figure-b2.cartridge.json',   // lane A30: the regular cut with each bake-off figure
  'hockey.regular.cartridge.json', 'hockey.raw.cartridge.json', 'hockey.boost.cartridge.json',
  'hockey.arena.large.cartridge.json', 'hockey.arena.wide.cartridge.json',
  'hockey.chaos.cartridge.json',   // lane A15 (C-0005): sudden death, shrink per goal, two bumpers, one slow circle
];

const has = (errors, pointer, fragment) => errors.some((e) => e.startsWith(`at ${pointer}:`) && e.includes(fragment));
const errorsOf = (c) => validateCartridge(c).errors;

test('the preset folder holds exactly the six presets', () => {
  assert.deepEqual(readdirSync(CART_DIR).filter((f) => f.endsWith('.json')).sort(), [...PRESETS].sort());
});

for (const f of PRESETS) {
  test(`preset ${f} validates`, () => {
    const r = validateCartridge(read(f));
    assert.deepEqual(r.errors, []);
  });
}

test('preset ids are unique and kebab-case; tunable defaults equal the game fields they dial', () => {
  const ids = new Set();
  for (const f of PRESETS) {
    const c = read(f);
    assert.ok(!ids.has(c.id)); ids.add(c.id);
    const get = { skater_speed: c.game.skater.speed, skater_turn_rate: c.game.skater.turn_rate, shot_speed: c.game.shot.speed,
      puck_max_speed: c.game.puck.max_speed, ice_friction: c.game.rink.ice_friction, board_restitution: c.game.rink.board_restitution,
      // absent ai numbers come from the difficulty profile; pro (and DEFAULTS.md) is 1 for both
      ai_speed_scale: c.game.ai?.speed_scale ?? 1, ai_red_skill: c.game.ai?.red_skill ?? 1, goal_pause_frames: c.game.rules.goal_pause_frames };
    for (const t of c.tunables) assert.equal(t.default, get[t.key], `${f} ${t.key}`);
  }
});

test('preset numbers match the brief', () => {
  const r = regular().game;
  assert.deepEqual([r.rink.length_m, r.rink.width_m], [90, 45]);   // R-0010
  assert.deepEqual(r.skater, { speed: 11, accel: 12, turn_rate: 5, lateral_grip: 3, mass: 80, glide_drag: 0.147, brake_decel: 6, check_enabled: true });   // R-0013: the body check is in every cut
  assert.deepEqual(r.puck, { radius_m: 0.12, max_speed: 30, drag: 0.5, colour_hex: '#111111' });   // R-0011: carrying needs drag
  assert.deepEqual(r.shot, { reach_m: 1.5, speed: 18, cooldown_frames: 24, aim_cone_deg: 8 });
  assert.equal(r.rules.mode, 'first_to'); assert.equal(r.rules.score_to_win, 3); assert.equal(r.rules.goal_pause_frames, 120);
  assert.equal(r.look.tier, 'l2_night'); assert.equal(r.pickups, undefined);
  assert.deepEqual(r.teams.map((t) => [t.id, t.colour_hex]), [['blue', '#1D4ED8'], ['orange', '#E8651A']]);
  assert.deepEqual(r.teams[0].skaters.map((s) => s.role), ['player', 'chaser', 'back']);
  assert.deepEqual(r.teams[1].skaters.map((s) => s.role), ['chaser', 'support', 'back']);
  const raw = read('hockey.raw.cartridge.json').game.skater;
  assert.deepEqual([raw.check_enabled, raw.check_speed_min, raw.knockdown_frames], [true, 5, 150]);   // R-0013: raw lengthens knockdowns
  assert.equal(read('hockey.boost.cartridge.json').game.pickups.length, 3);
  // R-0010: regular is 90 x 45 at speed 11; the old 120 x 60 at speed 8 is hockey.arena.large
  const s = regular().game;
  assert.deepEqual([s.rink.length_m, s.rink.width_m, s.skater.speed], [90, 45, 11]);
  const l = read('hockey.arena.large.cartridge.json').game;
  assert.deepEqual([l.rink.length_m, l.rink.width_m, l.skater.speed], [120, 60, 8]);
  assert.deepEqual([l.rules.mode, l.rules.timer_frames, l.rules.score_to_win], ['timer', 10800, undefined]);   // R-0011 point 4
  for (const f of PRESETS) assert.equal(read(f).game.puck.drag, 0.5, f);
  const w = read('hockey.arena.wide.cartridge.json').game.rink;
  assert.deepEqual([w.length_m, w.width_m], [140, 70]);
});

// Each enum rejects a wrong value, with the pointer and the allowed list.
const ENUMS = [
  ['rules.mode', (g) => { g.rules.mode = 'sudden_death'; }, '/game/rules/mode', '"first_to", "timer"'],
  ['teams[].skaters[].role', (g) => { g.teams[0].skaters[1].role = 'goalie'; }, '/game/teams/0/skaters/1/role', '"player", "chaser", "support", "back"'],
  ['pickups[].kind', (g) => { g.pickups = [{ kind: 'rocket', spawn_every_frames: 600, duration_frames: 60 }]; }, '/game/pickups/0/kind', '"speed", "precision", "heavy_puck", "big_puck", "reverse_other"'],
  ['look.tier', (g) => { g.look.tier = 'l4_ultra'; }, '/game/look/tier', '"l1_day", "l2_night", "l3_neon", "p3_clay", "p4_broadcast"'],
  ['camera.mode', (g) => { g.camera.mode = 'first_person'; }, '/game/camera/mode', '"third_person", "arena"'],
  ['rink.obstacles[].kind', (g) => { g.rink.obstacles = [{ kind: 'wall', x: 0, z: 0, size: 2 }]; }, '/game/rink/obstacles/0/kind', '"box", "pillar"'],
];
for (const [name, mutate, pointer, allowed] of ENUMS) {
  test(`enum ${name} rejects a wrong value with a pointer`, () => {
    const c = regular();
    mutate(c.game);
    const errors = errorsOf(c);
    assert.ok(has(errors, pointer, 'is not allowed'), errors.join('\n'));
    assert.ok(has(errors, pointer, allowed), errors.join('\n'));
  });
}

test('every enum value the brief lists is accepted', () => {
  for (const mode of ['first_to', 'timer']) {
    const c = regular(); c.game.rules.mode = mode; c.game.rules.timer_frames = 3600;
    assert.deepEqual(errorsOf(c), []);
  }
  for (const tier of ['l1_day', 'l2_night', 'l3_neon', 'p3_clay', 'p4_broadcast']) { const c = regular(); c.game.look.tier = tier; assert.deepEqual(errorsOf(c), []); }
  // B-to-A-0018 point 5: look.trim_emissive 0..100, absent = the tier's default.
  { const c = regular(); c.game.look.trim_emissive = 12; assert.deepEqual(errorsOf(c), []); }
  { const c = regular(); c.game.look.trim_emissive = 101; assert.ok(errorsOf(c).some((e) => e.includes('/game/look/trim_emissive'))); }
  for (const mode of ['third_person', 'arena']) { const c = regular(); c.game.camera.mode = mode; assert.deepEqual(errorsOf(c), []); }
  for (const kind of ['speed', 'precision', 'heavy_puck', 'big_puck', 'reverse_other']) {
    const c = regular(); c.game.pickups = [{ kind, spawn_every_frames: 600, duration_frames: 60 }]; assert.deepEqual(errorsOf(c), []);
  }
  for (const role of ['chaser', 'support', 'back']) { const c = regular(); c.game.teams[0].skaters[1].role = role; assert.deepEqual(errorsOf(c), []); }
});

test('skater.visual: valid values pass, a wrong value fails with a pointer, the Designer may not change it', () => {
  for (const v of ['toy', 'humanoid_b1', 'generated_b2']) {
    const c = regular(); c.game.skater.visual = v;
    assert.deepEqual(errorsOf(c), []);
  }
  const bad = regular(); bad.game.skater.visual = 'clay';
  const errors = errorsOf(bad);
  assert.ok(has(errors, '/game/skater/visual', 'is not allowed'), errors.join('\n'));
  assert.ok(has(errors, '/game/skater/visual', '"toy", "humanoid_b1", "generated_b2"'));
  const changed = regular(); changed.game.skater.visual = 'generated_b2';
  assert.ok(has(validateDesignerOutput(regular(), changed).errors, '/game/skater/visual', 'may not change'));
  assert.equal(regular().game.skater.visual, undefined);
  const b2 = read('hockey.test.visual-b2.cartridge.json');
  assert.equal(b2.id, 'hockey-test-visual-b2');
  assert.equal(b2.game.skater.visual, 'generated_b2');
  const { visual, ...rest } = b2.game.skater;
  assert.deepEqual(rest, large().game.skater);
});

test('exactly one player: zero and two are both rejected', () => {
  const none = regular(); none.game.teams[0].skaters[0].role = 'chaser';
  assert.ok(has(errorsOf(none), '/game/teams', 'exactly one skater across both teams must have role "player", found 0'));
  const two = regular(); two.game.teams[1].skaters[0].role = 'player';
  const errors = errorsOf(two);
  assert.ok(has(errors, '/game/teams', 'found 2'));
  assert.ok(errors.some((e) => e.includes('/game/teams/0/skaters/0/role') && e.includes('/game/teams/1/skaters/0/role')));
});

test('a team count other than two is rejected', () => {
  const c = regular(); c.game.teams.pop();
  assert.ok(has(errorsOf(c), '/game/teams', 'needs at least 2'));
});

test('spawns outside the rink fail, including the rounded corners and the boards margin', () => {
  const c = large(); c.game.teams[1].skaters[2].spawn = [70, 0];
  assert.ok(has(errorsOf(c), '/game/teams/1/skaters/2/spawn', 'outside the rink'));
  const z = large(); z.game.teams[0].skaters[1].spawn = [-10, 30.2];
  assert.ok(has(errorsOf(z), '/game/teams/0/skaters/1/spawn', 'outside the rink'));
  const corner = large(); corner.game.teams[0].skaters[1].spawn = [-59, -29]; // inside the box, outside the round corner
  assert.ok(has(errorsOf(corner), '/game/teams/0/skaters/1/spawn', 'outside the rink'));
  const edge = large(); edge.game.teams[0].skaters[1].spawn = [-59.4, 0];
  assert.deepEqual(errorsOf(edge), []);
});

test('a rink shrunk by the Designer leaves spawns outside and is caught', () => {
  const c = regular(); c.game.rink.length_m = 40;
  assert.ok(errorsOf(c).some((e) => e.includes('/spawn:') && e.includes('outside the rink')));
});

test('two skaters may not share a spawn', () => {
  const c = regular(); c.game.teams[0].skaters[1].spawn = [...c.game.teams[0].skaters[0].spawn];
  assert.ok(errorsOf(c).some((e) => e.includes('less than 1 m')));
});

test('pickup duration_frames must be above 0 and a whole number', () => {
  const c = regular(); c.game.pickups = [{ kind: 'speed', spawn_every_frames: 600, duration_frames: 0 }];
  const errors = errorsOf(c);
  assert.ok(has(errors, '/game/pickups/0/duration_frames', 'at least 1 frame'));
  assert.ok(has(errors, '/game/pickups/0/duration_frames', 'below the minimum 1'));
  const f = regular(); f.game.pickups = [{ kind: 'speed', spawn_every_frames: 600, duration_frames: 10.5 }];
  assert.ok(has(errorsOf(f), '/game/pickups/0/duration_frames', 'expected an integer'));
});

test('unknown keys, nulls and wall-clock names are errors inside game', () => {
  const a = regular(); a.game.rink.floor = 'ice';
  assert.ok(has(errorsOf(a), '/game/rink/floor', 'unknown key "floor"'));
  const b = regular(); b.game.camera.fov_deg = null;
  assert.ok(has(errorsOf(b), '/game/camera/fov_deg', 'null'));
  const c = regular(); c.game.rules.timer_seconds = 120;
  assert.ok(has(errorsOf(c), '/game/rules/timer_seconds', 'unknown key'));
  const d = regular(); delete d.game.rink;
  assert.ok(has(errorsOf(d), '/game/rink', 'required key "rink" is missing'));
});

test('ranges: each limit rejects one step outside', () => {
  const lim = loadLimits().game_ranges;
  const g = () => regular().game;
  const paths = {
    'rink.length_m': (x) => (o) => { o.rink.length_m = x; }, 'rink.width_m': (x) => (o) => { o.rink.width_m = x; },
    'skater.speed': (x) => (o) => { o.skater.speed = x; }, 'rules.score_to_win': (x) => (o) => { o.rules.score_to_win = x; },
    'shot.speed': (x) => (o) => { o.shot.speed = x; }, 'camera.fov_deg': (x) => (o) => { o.camera.fov_deg = x; },
  };
  for (const [path, set] of Object.entries(paths)) {
    const [lo, hi] = lim[path].split('..').map(Number);
    for (const [v, word] of [[lo - 1, 'below'], [hi + 1, 'above']]) {
      const c = regular(); set(v)(c.game);
      assert.ok(errorsOf(c).some((e) => e.includes(`/game/${path.replace('.', '/')}:`) && e.includes(word)), `${path} ${v}`);
    }
  }
  const t = regular(); t.game.rules.mode = 'timer'; t.game.rules.timer_frames = 599;
  assert.ok(has(errorsOf(t), '/game/rules/timer_frames', 'below the minimum 600'));
  t.game.rules.timer_frames = 36001;
  assert.ok(has(errorsOf(t), '/game/rules/timer_frames', 'above the maximum 36000'));
  const s = regular(); s.game.teams[0].skaters.push(...Array.from({ length: 5 }, (_, i) => ({ role: 'back', spawn: [-20, i * 2 - 4] })));
  assert.ok(has(errorsOf(s), '/game/teams/0/skaters', 'the limit is 5'));
  assert.ok(g());
});

test('limits.json game_ranges mirror the schema exactly', () => {
  const schema = JSON.parse(readFileSync(resolve(HERE, 'game.schema.json'), 'utf8'));
  const mirror = {};
  const walk = (s, path) => {
    if (s.properties) { for (const [k, v] of Object.entries(s.properties)) walk(v, path ? `${path}.${k}` : k); return; }
    if (s.type === 'array') {
      mirror[`${path}[]`] = `${s.minItems ?? 0}..${s.maxItems}`;
      if (s.items?.properties) for (const [k, v] of Object.entries(s.items.properties)) walk(v, `${path}[].${k}`);
      return;
    }
    if (s.enum) mirror[path] = s.enum.join('|');
    else if (s.minimum !== undefined) mirror[path] = `${s.minimum}..${s.maximum}`;
  };
  walk(schema, '');
  assert.deepEqual(loadLimits().game_ranges, mirror);
});

test('mode needs its end condition; check_speed_min cannot exceed top speed; corner radius within half the width', () => {
  const a = regular(); delete a.game.rules.score_to_win;
  assert.ok(has(errorsOf(a), '/game/rules/score_to_win', 'needs score_to_win'));
  const b = regular(); b.game.rules.mode = 'timer';
  assert.ok(has(errorsOf(b), '/game/rules/timer_frames', 'needs timer_frames'));
  const c = read('hockey.raw.cartridge.json'); c.game.skater.check_speed_min = 9;
  assert.ok(has(errorsOf(c), '/game/skater/check_speed_min', 'no check could ever land'));
  const d = regular(); d.game.rink.width_m = 20; d.game.rink.corner_radius_m = 12;
  assert.ok(has(errorsOf(d), '/game/rink/corner_radius_m', 'more than half'));
});

test('camera, ai, puck, shot and pickups may be left out (the engine applies DEFAULTS.md)', () => {
  const c = regular();
  for (const k of ['camera', 'ai', 'puck', 'shot', 'pickups']) delete c.game[k];
  assert.deepEqual(errorsOf(c), []);
});

test('Designer: may change the listed game paths, not the roles or the camera mode', () => {
  const base = regular();
  const ok = regular();
  ok.game.rules.mode = 'timer'; ok.game.rules.timer_frames = 3600; ok.game.rules.score_to_win = 5;
  ok.game.look.tier = 'l3_neon'; ok.game.rink.ice_friction = 0.02; ok.game.rink.obstacles = [{ kind: 'box', x: 0, z: 10, size: 3 }];
  ok.game.pickups = [{ kind: 'speed', spawn_every_frames: 600, duration_frames: 120 }];
  ok.values = { skater_speed: 9 };
  assert.deepEqual(validateDesignerOutput(base, ok).errors, []);
  const role = regular(); role.game.teams[0].skaters[1].role = 'support';
  assert.ok(has(validateDesignerOutput(base, role).errors, '/game/teams/0/skaters/1/role', 'may not change'));
  const cam = regular(); cam.game.camera.mode = 'arena';
  assert.ok(has(validateDesignerOutput(base, cam).errors, '/game/camera/mode', 'may not change'));
  const spd = regular(); spd.game.skater.speed = 9;
  assert.ok(has(validateDesignerOutput(base, spd).errors, '/game/skater/speed', 'may not change'));
  const tun = regular(); tun.tunables[0].max = 12;
  assert.ok(has(validateDesignerOutput(base, tun).errors, '/tunables', 'may not change'));
  // lane A44: the three chase-camera numbers are editable; camera.mode and every role stay the host's
  assert.ok(DESIGNER_GAME_EDITABLE.includes('look.tier') && !DESIGNER_GAME_EDITABLE.some((p) => p === 'camera' || p === 'camera.mode' || p.includes('role')));
  for (const p of ['shot.one_timer', 'skater.breakaway', 'stick.deke', 'stick.foul_grace_frames', 'commentary', 'look.trim_emissive', 'camera.boards_margin_m', 'camera.end_height_m', 'camera.far_net_bias']) assert.ok(DESIGNER_GAME_EDITABLE.includes(p), p);
  const fov = regular(); fov.game.camera.fov_deg = 90;
  assert.ok(has(validateDesignerOutput(base, fov).errors, '/game/camera/fov_deg', 'may not change'));
  const payoffs = regular(); payoffs.game.shot.one_timer = { window_frames: 24, cone_bonus_deg: 16 }; payoffs.game.skater.breakaway = { speed_bonus: 0.4, frames: 120 };
  payoffs.game.stick = { deke: { offset_m: 1.5, commit_frames: 30 }, foul_grace_frames: 60 }; payoffs.game.commentary = { intensity: 'quiet' };
  payoffs.game.look.trim_emissive = 60; payoffs.game.camera.end_height_m = 12;
  assert.deepEqual(validateDesignerOutput(base, payoffs).errors, []);
});

test('ai.difficulty: rookie | pro | all_star, absent = pro; every preset names it; the Designer may change it', () => {
  for (const f of PRESETS) assert.equal(read(f).game.ai.difficulty, 'pro', f);
  for (const d of ['rookie', 'pro', 'all_star']) { const c = regular(); c.game.ai.difficulty = d; assert.deepEqual(errorsOf(c), [], d); }
  const bad = regular(); bad.game.ai.difficulty = 'hard';
  assert.ok(has(errorsOf(bad), '/game/ai/difficulty', 'is not allowed'));
  const cap = regular(); cap.game.ai.difficulty = 'All_Star';
  assert.ok(has(errorsOf(cap), '/game/ai/difficulty', 'is not allowed'));
  const nul = regular(); nul.game.ai.difficulty = null;
  assert.ok(has(errorsOf(nul), '/game/ai/difficulty', 'null'));
  const absent = regular(); delete absent.game.ai.difficulty;
  assert.deepEqual(errorsOf(absent), []);
  // explicit numbers sit next to the level (they override its profile)
  const mixed = regular(); mixed.game.ai = { difficulty: 'rookie', speed_scale: 0.8 };
  assert.deepEqual(errorsOf(mixed), []);
  assert.ok(DESIGNER_GAME_EDITABLE.includes('ai.difficulty'));
  const base = regular();
  for (const d of ['rookie', 'all_star']) {
    const easier = regular(); easier.game.ai.difficulty = d;
    assert.deepEqual(validateDesignerOutput(base, easier).errors, [], d);
  }
  const dropped = regular(); delete dropped.game.ai;
  assert.deepEqual(validateDesignerOutput(base, dropped).errors, []);
  const noAi = regular(); delete noAi.game.ai;
  const added = regular(); added.game.ai = { difficulty: 'rookie' };
  assert.deepEqual(validateDesignerOutput(noAi, added).errors, []);
  // the level is editable, the numbers next to it are not
  const nums = regular(); nums.game.ai.hold_radius_m = 9;
  assert.ok(has(validateDesignerOutput(base, nums).errors, '/game/ai', 'may not change'));
});

test('barks: enabled and intensity quiet | normal | loud; absent = enabled normal; unknown keys and values fail; the Designer may change it', () => {
  for (const intensity of ['quiet', 'normal', 'loud']) { const c = regular(); c.game.barks = { enabled: true, intensity }; assert.deepEqual(errorsOf(c), [], intensity); }
  const off = regular(); off.game.barks = { enabled: false }; assert.deepEqual(errorsOf(off), []);
  const empty = regular(); empty.game.barks = {}; assert.deepEqual(errorsOf(empty), []);
  const bad = regular(); bad.game.barks = { intensity: 'screaming' };
  assert.ok(has(errorsOf(bad), '/game/barks/intensity', 'is not allowed'));
  const cap = regular(); cap.game.barks = { intensity: 'Loud' };
  assert.ok(has(errorsOf(cap), '/game/barks/intensity', 'is not allowed'));
  const extra = regular(); extra.game.barks = { volume: 3 };
  assert.ok(has(errorsOf(extra), '/game/barks/volume', 'unknown key'));
  const nul = regular(); nul.game.barks = { enabled: null };
  assert.ok(has(errorsOf(nul), '/game/barks/enabled', 'null'));
  const base = regular();
  const loud = regular(); loud.game.barks = { enabled: true, intensity: 'loud' };
  assert.deepEqual(validateDesignerOutput(base, loud).errors, []);
  assert.ok(DESIGNER_GAME_EDITABLE.includes('barks'));
});

// Lane A84 (R-0041 point 3): game.abilities[], the human's ability on F.
test('abilities: the four kinds, mode or pickup; one mode at most; a pickup ability needs pickups; unknown keys and ranges fail', () => {
  const ok = regular();
  ok.game.pickups = [{ kind: 'speed', spawn_every_frames: 600, duration_frames: 300 }];
  ok.game.abilities = [
    { kind: 'shotgun', trigger: 'mode', cooldown_frames: 1200 },
    { kind: 'hip_check', trigger: 'pickup', magnitude: 1.5 },
    { kind: 'shield', trigger: 'pickup', duration_frames: 360, charges: 3 },
    { kind: 'slow_puck', trigger: 'pickup', duration_frames: 120, magnitude: 0.5 },
  ];
  assert.deepEqual(errorsOf(ok), []);
  const two = regular(); two.game.abilities = [{ kind: 'shield', trigger: 'mode' }, { kind: 'shotgun', trigger: 'mode' }];
  assert.ok(has(errorsOf(two), '/game/abilities/1/trigger', 'only one ability may have trigger "mode"'));
  const lone = regular(); lone.game.abilities = [{ kind: 'slow_puck', trigger: 'pickup' }];
  assert.ok(has(errorsOf(lone), '/game/abilities/0/trigger', 'game.pickups is empty'));
  const unk = regular(); unk.game.abilities = [{ kind: 'shield', trigger: 'mode', colour: 'red' }];
  assert.ok(errorsOf(unk).some((e) => e.startsWith('at /game/abilities/0')));
  const kind = regular(); kind.game.abilities = [{ kind: 'flamethrower', trigger: 'mode' }];
  assert.ok(errorsOf(kind).some((e) => e.startsWith('at /game/abilities/0/kind')));
  const low = regular(); low.game.abilities = [{ kind: 'hip_check', trigger: 'mode', cooldown_frames: 59 }];
  assert.ok(has(errorsOf(low), '/game/abilities/0/cooldown_frames', 'below'));
  const many = regular(); many.game.abilities = Array.from({ length: 5 }, () => ({ kind: 'shield', trigger: 'mode', charges: 1 }));
  assert.ok(errorsOf(many).some((e) => e.startsWith('at /game/abilities')));
});
