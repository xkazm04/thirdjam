// Key sweep (lane A141, track S3, evolution 3): every scalar key of game.schema.json at both ends of its range, alone and in pairs with the keys it interacts with.
//
// The list of keys is DERIVED from schema/game.schema.json (object properties down to the scalars: numbers, integers, booleans, enums), so a key added tomorrow is
// swept without an edit here. Arrays (obstacles, zones, hooks, pickups, abilities, effects, teams) and free strings are the structured builders' job in gen.mjs.
// PAIRS names keys that interact; a name that is no longer in the schema throws (a renamed key must be fixed here, not silently dropped).
// The hook combinations (HOOK_COMBOS) put the keys the match-flow hooks touch under the hooks that end a period, shrink the rink or call set_param.
//
//   sweepCases(ctx) -> [{ name, cart, profile }]   ctx = { base, validate(cart) -> bool, repair(game), setParams: [[param, lo, hi]] }
import { readFileSync } from 'node:fs';
import { dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const HERE = dirname(fileURLToPath(import.meta.url));
const SCHEMA = JSON.parse(readFileSync(resolve(HERE, '..', '..', 'schema', 'game.schema.json'), 'utf8'));
const clone = (v) => JSON.parse(JSON.stringify(v));

// [{ path: 'contact.board_bonus', values: [lo, hi] }]: the two ends (enum: every member; boolean: false and true).
export function schemaLeaves(schema = SCHEMA) {
  const out = [];
  const walk = (node, prefix) => {
    for (const [k, v] of Object.entries(node.properties ?? {})) {
      const path = prefix ? `${prefix}.${k}` : k;
      if (v.type === 'object' && v.properties) { walk(v, path); continue; }
      const t = Array.isArray(v.type) ? v.type[0] : v.type;
      if (v.enum) out.push({ path, type: 'enum', values: [...v.enum] });
      else if (t === 'boolean') out.push({ path, type: 'boolean', values: [false, true] });
      else if ((t === 'number' || t === 'integer') && v.minimum !== undefined && v.maximum !== undefined) out.push({ path, type: t, values: [v.minimum, v.maximum] });
    }
  };
  walk(schema, '');
  return out.filter((l) => !['rules.offside'].includes(l.path));
}

export function setPath(game, path, value) {
  const parts = path.split('.');
  let cur = game;
  for (const p of parts.slice(0, -1)) { if (typeof cur[p] !== 'object' || cur[p] === null) cur[p] = {}; cur = cur[p]; }
  cur[parts[parts.length - 1]] = value;
}

// Keys that act on each other; each pair runs at all four corners.
export const PAIRS = [
  ['contact.release_grace_frames', 'stick.foul_grace_frames'],
  ['contact.release_grace_frames', 'stick.take_grace_frames'],
  ['stick.take_grace_frames', 'stick.foul_grace_frames'],
  ['stick.take_grace_frames', 'puck.count'],
  ['stick.passive_take', 'stick.take_grace_frames'],
  ['stick.passive_take', 'stick.body_shield'],
  ['stick.body_shield', 'stick.sweep.shield_frames'],
  ['stick.body_shield', 'contact.carrier_can_hit'],
  ['contact.board_bonus', 'rink.board_restitution'],
  ['contact.board_bonus', 'contact.stability_mps'],
  ['contact.board_bonus', 'rink.width_m'],
  ['contact.board_bonus', 'skater.speed'],
  ['skater.reverse_brake_decel', 'skater.brake_decel'],
  ['skater.reverse_brake_decel', 'skater.accel'],
  ['skater.reverse_brake_decel', 'skater.speed'],
  ['skater.burst.own_stop', 'skater.burst.frames'],
  ['skater.burst.own_stop', 'skater.burst.accel_mult'],
  ['skater.burst.frames', 'skater.brake_decel'],
  ['contact.cross_reach_m', 'stick.reach_m'],
  ['contact.down_frames_min', 'contact.down_frames_max'],
  ['shot.tap_frames', 'shot.charge_frames_max'],
  ['rules.variants.shrink_per_goal', 'rink.length_m'],
  ['rules.timer_frames', 'rules.variants.period_count'],
  ['rules.goal_pause_frames', 'rules.variants.period_count'],
];

const leafOf = (leaves, path) => {
  const l = leaves.find((x) => x.path === path);
  if (!l) throw new Error(`keys.mjs PAIRS names ${path}, which is not a key of game.schema.json: rename it there`);
  return l;
};
const ends = (l) => (l.type === 'enum' ? l.values : [l.values[0], l.values[1]]);

// Hooks that end a period, shrink the rink, or set a parameter, stacked on the keys they interact with.
const MATCH_FLOW = [
  ['frame60-end-period', { when: 'on_frame', every_frames: 60 }, { do: 'end_period' }],
  ['faceoff-end-period', { when: 'on_faceoff' }, { do: 'end_period' }],
  ['period-end-swap', { when: 'on_period_end' }, { do: 'swap_nets' }],
  ['period-start-end-match', { when: 'on_period_start', after_frame: 120 }, { do: 'end_match' }],
];
const SHRINKERS = [
  ['frame60-shrink', (g) => { g.rules.hooks = [{ id: 'h1', when: 'on_frame', every_frames: 60, then: [{ do: 'shrink_rink', value: 0.5 }] }]; }],
  ['frame60-shrink-grow', (g) => { g.rules.hooks = [{ id: 'h1', when: 'on_frame', every_frames: 60, then: [{ do: 'shrink_rink', value: 0.5 }, { do: 'grow_rink', value: 0.05 }] }]; }],
  ['per-goal-shrink', (g) => { g.rules.mode = 'first_to'; g.rules.score_to_win = 9; g.rules.variants = { shrink_per_goal: 0.3 }; delete g.rules.timer_frames; }],
  ['goal-shrink-hook', (g) => { g.rules.hooks = [{ id: 'h1', when: 'on_goal', then: [{ do: 'shrink_rink', value: 0.5 }, { do: 'swap_nets' }] }]; }],
];

export function sweepCases({ base, validate, repair, setParams }) {
  const leaves = schemaLeaves();
  const cases = [];
  const skipped = [];
  const mk = (name, profile, mutate) => {
    const cart = clone(base);
    cart.id = `sweep-${String(cases.length + skipped.length + 1).padStart(4, '0')}`;   // the descriptive name is the case name in the manifest
    cart.title = `Sweep ${name}`.slice(0, 60);
    cart.game.rules ??= { mode: 'first_to', score_to_win: 3 };
    mutate(cart.game, cart);
    // a key that belongs to one rules mode puts the cartridge in that mode
    const r = cart.game.rules;
    if (r.timer_frames !== undefined && r.mode !== 'timer') r.mode = 'timer';
    if (r.mode === 'timer' && r.timer_frames === undefined) r.timer_frames = 3600;
    if (r.mode === 'first_to') { if (r.score_to_win === undefined) r.score_to_win = 3; delete r.timer_frames; }
    if (r.mode === 'timer') delete r.score_to_win;
    repair(cart.game);
    if (!validate(cart)) { skipped.push(name); return; }
    cases.push({ name, cart, profile });
  };
  const keyTag = (p, v) => `${p}=${typeof v === 'string' ? v : String(v)}`;
  // 1. every key at both ends, alone
  for (const l of leaves) for (const v of ends(l)) mk(`key-${keyTag(l.path, v)}`, `sweep/${l.path}`, (g) => setPath(g, l.path, v));
  // 2. the pairs, four corners
  for (const [a, b] of PAIRS) {
    const la = leafOf(leaves, a), lb = leafOf(leaves, b);
    for (const va of ends(la)) for (const vb of ends(lb)) mk(`pair-${keyTag(a, va)}-${keyTag(b, vb)}`, `pair/${a}+${b}`, (g) => { setPath(g, a, va); setPath(g, b, vb); });
  }
  // 3. the graces against the hooks that end a period (every combination of the three graces at their ends), on a 600-frame clock and a three-period one
  const graces = ['contact.release_grace_frames', 'stick.foul_grace_frames', 'stick.take_grace_frames'].map((p) => leafOf(leaves, p));
  for (const [hn, when, act] of MATCH_FLOW) {
    for (let mask = 0; mask < 8; mask++) {
      const periods = mask & 1 ? 3 : 1;
      mk(`graces${mask}-${hn}-p${periods}`, `combo/graces+${hn}`, (g) => {
        graces.forEach((l, i) => setPath(g, l.path, ends(l)[(mask >> i) & 1]));
        g.rules = { mode: 'timer', timer_frames: periods === 3 ? 1800 : 600, goal_pause_frames: mask & 2 ? 30 : 600, variants: { period_count: periods },
          hooks: [{ id: 'h1', ...when, then: [act] }] };
      });
    }
  }
  // 4. the board bonus against a shrinking rink (smallest rink, dead and perfect boards)
  const bonus = leafOf(leaves, 'contact.board_bonus');
  for (const [sn, fn] of SHRINKERS) for (const b of ends(bonus)) for (const rest of [0, 1]) {
    mk(`bonus${b}-${sn}-rest${rest}`, `combo/board_bonus+${sn}`, (g) => {
      setPath(g, 'contact.board_bonus', b); g.rink.length_m = 40; g.rink.width_m = 20; g.rink.board_restitution = rest;
      g.rules = { mode: 'timer', timer_frames: 3600 };
      fn(g);
    });
  }
  // 5. the take grace (and passive take) against set_param on every tunable at both ends of its field's range
  const take = leafOf(leaves, 'stick.take_grace_frames');
  for (const [param, lo, hi] of setParams) for (const t of ends(take)) for (const v of [lo, hi]) {
    mk(`take${t}-setparam-${param}=${v}`, `combo/take_grace+set_param`, (g) => {
      setPath(g, 'stick.take_grace_frames', t); setPath(g, 'stick.passive_take', t > 0);
      g.rules = { mode: 'timer', timer_frames: 1200, hooks: [{ id: 'h1', when: 'on_frame', every_frames: 60, then: [{ do: 'set_param', param, value: v }] }] };
    });
  }
  // 6. the burst and the reverse brake against set_param skater_speed / skater_turn_rate at their ends
  for (const own of [false, true]) for (const [param, lo, hi] of setParams.filter((s) => /^skater_/.test(s[0]))) for (const v of [lo, hi]) {
    mk(`burst-own${own}-setparam-${param}=${v}`, 'combo/burst+set_param', (g) => {
      setPath(g, 'skater.burst.own_stop', own); setPath(g, 'skater.burst.frames', 60); setPath(g, 'skater.reverse_brake_decel', 40);
      g.rules = { mode: 'timer', timer_frames: 1200, hooks: [{ id: 'h1', when: 'on_frame', every_frames: 60, then: [{ do: 'set_param', param, value: v }] }] };
    });
  }
  return { cases, skipped, leaves };
}

// 10,000 cases of chained hooks (item 5): hooks that trigger each other. A hook whose action is end_period / end_match / add_score / swap_nets fires the period and goal
// hooks, so a ring of them can chain; the sim caps the depth at 8 (JamSim.cpp kMaxRuleDepth). chain(r, i) builds one cartridge from the base.
export function chainCase(r, i, base) {
  const cart = clone(base);
  const n = r.int(2, 8);
  const whens = ['on_goal', 'on_period_start', 'on_period_end', 'on_faceoff', 'on_frame', 'on_score_reached', 'on_check', 'on_knockdown'];
  const acts = [{ do: 'end_period' }, { do: 'add_score', team: 'scoring', value: 1 }, { do: 'add_score', team: 'any', value: -1 }, { do: 'swap_nets' }, { do: 'shrink_rink', value: 0.05 },
    { do: 'grow_rink', value: 0.05 }, { do: 'end_period' }, { do: 'apply_modifier', kind: 'speed', frames: 60 }, { do: 'spawn_pickup', kind: 'speed' }];
  const hooks = [];
  for (let k = 0; k < n; k++) {
    const when = k === 0 ? r.pick(['on_goal', 'on_faceoff', 'on_period_start']) : r.pick(whens);
    const h = { id: `h${k + 1}`, when, then: [] };
    if (when === 'on_frame') h.every_frames = r.pick([60, 120]);
    if (when === 'on_score_reached') h.score = { at_least: r.int(0, 3) };
    const m = r.int(1, 4);
    for (let j = 0; j < m; j++) h.then.push(clone(r.pick(acts)));
    hooks.push(h);
  }
  cart.id = `chain-${String(i).padStart(5, '0')}`;
  cart.title = `Chain ${i}`;
  cart.game.rules = { mode: r.pick(['timer', 'first_to']), goal_pause_frames: r.pick([30, 120]), hooks, variants: r.coin(0.5) ? { period_count: r.int(1, 3) } : undefined };
  if (cart.game.rules.mode === 'timer') cart.game.rules.timer_frames = r.pick([600, 1800, 3600]);
  else cart.game.rules.score_to_win = r.pick([1, 3, 9]);
  if (cart.game.rules.variants === undefined) delete cart.game.rules.variants;
  return cart;
}
