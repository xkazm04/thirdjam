// Fuzz corpus generator (lane A109, track S3). Zero dependencies, Node 24.
//
//   node tools/fuzz/gen.mjs --out tools/fuzz/out/run1 --count 5000 --seed 1 [--invalid 1000]
//
// Writes <out>/manifest.json and one file per cartridge (c00001.json ...). The manifest lists, per case: name, file, kind, ok (Node validator's
// verdict), pointers (the sorted set of JSON pointers of its errors, as Jam.Core.CartridgeParity compares them) and the profile that made it.
//
// Two families:
//   valid   schema-valid cartridges built from schema/game.schema.json and schema/limits.json: every scalar key at a limit, at mid-range, uniform, or one
//           of each by key; plus named adversarial scenarios (smallest rink with five a side and the most pickups, the shortest timer, every ability,
//           contact keys at their extremes, a tap longer than the charge, the smallest reach, ...). Cross-field rules the validator enforces are
//           repaired on the way, so the valid family is valid by construction; whatever the validator still refuses is kept as a case (family "rejected").
//   invalid schema-invalid cartridges: wrong type, unknown key, missing required key, number out of range, null, fractional frames, and text-level
//           corruption (truncated, BOM, comments, trailing comma, 1e999, duplicate keys, deep nesting). The loader must refuse each with the same pointer set.
// Same --seed gives the same corpus byte for byte.
import { readFileSync, writeFileSync, mkdirSync, readdirSync } from 'node:fs';
import { dirname, resolve, join } from 'node:path';
import { fileURLToPath } from 'node:url';
import { validateCartridge } from '../../schema/validate.mjs';

const HERE = dirname(fileURLToPath(import.meta.url));
const REPO = resolve(HERE, '..', '..');
const GAME_SCHEMA = JSON.parse(readFileSync(resolve(REPO, 'schema', 'game.schema.json'), 'utf8'));
const LIMITS = JSON.parse(readFileSync(resolve(REPO, 'schema', 'limits.json'), 'utf8'));
const CART_DIR = resolve(REPO, 'Game', 'Content', 'Cartridges');

// ---------- rng ----------
export function rng(seed) {
  let a = (seed >>> 0) || 1;
  const next = () => {
    a = (a + 0x6d2b79f5) >>> 0;
    let t = a;
    t = Math.imul(t ^ (t >>> 15), t | 1);
    t ^= t + Math.imul(t ^ (t >>> 7), t | 61);
    return ((t ^ (t >>> 14)) >>> 0) / 4294967296;
  };
  const r = {
    next,
    int: (lo, hi) => lo + Math.floor(next() * (hi - lo + 1)),
    pick: (arr) => arr[Math.floor(next() * arr.length)],
    coin: (p = 0.5) => next() < p,
    shuffle: (arr) => { const o = [...arr]; for (let i = o.length - 1; i > 0; i--) { const j = Math.floor(next() * (i + 1)); [o[i], o[j]] = [o[j], o[i]]; } return o; },
  };
  return r;
}

const clone = (v) => JSON.parse(JSON.stringify(v));
const isObj = (v) => v !== null && typeof v === 'object' && !Array.isArray(v);
const round = (v, d = 4) => Number(v.toFixed(d));

// ---------- scalar leaves from the schema ----------
const SKIP_TOP = new Set(['rules', 'teams', 'pickups', 'abilities', 'effects', 'meta']);
const SKIP_PATH = new Set(['rink.obstacles', 'rink.zones', 'puck.colour_hex', 'look.trim_colour_hex']);

function leaf(node, mode, r) {
  const t = Array.isArray(node.type) ? node.type[0] : node.type;
  if (node.enum) return r.pick(node.enum);
  if (t === 'boolean') return r.coin();
  if (t === 'number' || t === 'integer') {
    const lo = node.minimum, hi = node.maximum;
    if (lo === undefined || hi === undefined) return undefined;
    let m = mode;
    if (m === 'mixed') m = r.pick(['min', 'max', 'mid', 'rand', 'nearmin', 'nearmax']);
    if (m === 'limits') m = r.coin() ? 'min' : 'max';
    let v;
    switch (m) {
      case 'min': v = lo; break;
      case 'max': v = hi; break;
      case 'mid': v = (lo + hi) / 2; break;
      case 'nearmin': v = lo + (hi - lo) * 0.001; break;
      case 'nearmax': v = hi - (hi - lo) * 0.001; break;
      default: v = lo + (hi - lo) * r.next();
    }
    if (t === 'integer') return Math.min(hi, Math.max(lo, Math.round(v)));
    return Math.min(hi, Math.max(lo, round(v, 5)));
  }
  return undefined;
}

function fillObject(node, mode, presence, r, path = '') {
  const out = {};
  for (const [k, v] of Object.entries(node.properties ?? {})) {
    const p = path ? `${path}.${k}` : k;
    if (!path && SKIP_TOP.has(k)) continue;
    if (SKIP_PATH.has(p)) continue;
    const required = (node.required ?? []).includes(k);
    if (!required && !r.coin(presence)) continue;
    if (v.type === 'object' && v.properties) {
      const sub = fillObject(v, mode, presence, r, p);
      if (Object.keys(sub).length || required) out[k] = sub;
    } else if (v.type === 'array') {
      // handled by the structured builders
    } else {
      const val = leaf(v, mode, r);
      if (val !== undefined) out[k] = val;
    }
  }
  return out;
}

// ---------- geometry ----------
const MARGIN = 0.5;
function insideRink(x, z, L, W, corner) {
  const hx = L / 2 - MARGIN, hz = W / 2 - MARGIN;
  if (Math.abs(x) > hx || Math.abs(z) > hz) return false;
  const rr = Math.max(0, Math.min(corner - MARGIN, hx, hz));
  const dx = Math.abs(x) - (hx - rr), dz = Math.abs(z) - (hz - rr);
  return !(dx > 0 && dz > 0 && dx * dx + dz * dz > rr * rr);
}

function randomPoint(L, W, corner, r, taken) {
  for (let i = 0; i < 400; i++) {
    const x = round((r.next() * 2 - 1) * (L / 2 - MARGIN), 2), z = round((r.next() * 2 - 1) * (W / 2 - MARGIN), 2);
    if (insideRink(x, z, L, W, corner) && taken.every(([a, b]) => Math.hypot(a - x, b - z) >= 1.2)) return [x, z];
  }
  return null;
}

function gridSpawn(team, i, L, W, corner) {
  // a tight grid around centre ice: always inside and 1 m apart, whatever the rink
  const sign = team === 0 ? -1 : 1;
  const x = sign * (1.5 + 2.2 * i), z = (i % 2 ? 3 : -3) + (team ? 0.5 : 0);
  return insideRink(x, z, L, W, corner) ? [round(x, 2), round(z, 2)] : [round(sign * (1.5 + 2.2 * i), 2), 0];
}

function buildTeams(g, r, sizeA, sizeB, scatter, playerTeam) {
  const L = g.rink.length_m, W = g.rink.width_m, corner = typeof g.rink.corner_radius_m === 'number' ? g.rink.corner_radius_m : 5;
  const taken = [];
  const teams = [];
  const colours = [['#1D4ED8', '#E8651A'], ['#00FF00', '#FF00FF'], ['#000000', '#FFFFFF'], ['#123456', '#654321']];
  const cc = r.pick(colours);
  [sizeA, sizeB].forEach((n, t) => {
    const skaters = [];
    for (let i = 0; i < n; i++) {
      let spawn = null;
      if (scatter) spawn = randomPoint(L, W, corner, r, taken);
      if (!spawn) spawn = gridSpawn(t, i, L, W, corner);
      // grid fallback may collide when scatter mixes with grid: nudge
      let guard = 0;
      while (taken.some(([a, b]) => Math.hypot(a - spawn[0], b - spawn[1]) < 1.2) && guard++ < 50) spawn = [round(spawn[0] + 1.3, 2), spawn[1]];
      taken.push(spawn);
      const role = i === 0 && t === playerTeam ? 'player' : r.pick(['chaser', 'support', 'back']);
      skaters.push({ role, spawn });
    }
    teams.push({ id: t === 0 ? 'blue' : 'orange', colour_hex: cc[t], skaters });
  });
  return teams;
}

// ---------- structured parts ----------
const BUILTIN = ['speed', 'precision', 'heavy_puck', 'big_puck', 'reverse_other'];
const STATS = Object.keys(JSON.parse(JSON.stringify({
  skater_speed: 1, skater_accel: 1, skater_turn_rate: 1, skater_grip: 1, skater_mass: 1, shot_speed: 1, shot_aim_cone_deg: 1, shot_reach: 1, check_speed_min: 1,
  knockdown_frames: 1, puck_drag: 1, puck_max_speed: 1, puck_radius: 1, ice_friction: 1, board_restitution: 1,
})));
const STAT_FIELD = {
  skater_speed: 'skater.speed', skater_accel: 'skater.accel', skater_turn_rate: 'skater.turn_rate', skater_grip: 'skater.lateral_grip', skater_mass: 'skater.mass',
  shot_speed: 'shot.speed', shot_aim_cone_deg: null, shot_reach: 'shot.reach_m', check_speed_min: 'skater.check_speed_min', knockdown_frames: 'skater.knockdown_frames',
  puck_drag: 'puck.drag', puck_max_speed: 'puck.max_speed', puck_radius: 'puck.radius_m', ice_friction: 'rink.ice_friction', board_restitution: 'rink.board_restitution',
};
function statWidth(stat) {
  const f = STAT_FIELD[stat];
  if (!f) return 45;
  const [lo, hi] = String(LIMITS.game_ranges[f]).split('..').map(Number);
  return hi - lo;
}

function buildZones(g, r, n) {
  const L = g.rink.length_m, W = g.rink.width_m, corner = typeof g.rink.corner_radius_m === 'number' ? g.rink.corner_radius_m : 5;
  const kinds = ['slow_ice', 'fast_ice', 'bumper', 'no_shoot', 'boost_only'];
  const out = [];
  for (let i = 0; i < n; i++) {
    const p = randomPoint(L, W, corner, r, []);
    if (!p) break;
    const circle = r.coin();
    const z = { id: `zone-${i + 1}`, shape: circle ? 'circle' : 'rect', x: p[0], z: p[1], kind: r.pick(kinds) };
    if (circle) z.r = round(0.5 + r.next() * 39.5, 2); else { z.w = round(1 + r.next() * 159, 2); z.d = round(1 + r.next() * 79, 2); }
    if (r.coin()) z.strength = r.pick([0, 1, round(r.next(), 3)]);
    if (r.coin(0.3)) z.visible = r.coin();
    out.push(z);
  }
  return out;
}

function buildObstacles(g, r, n) {
  if (SAFE) return [];
  const L = g.rink.length_m, W = g.rink.width_m, corner = typeof g.rink.corner_radius_m === 'number' ? g.rink.corner_radius_m : 5;
  const out = [];
  for (let i = 0; i < n; i++) {
    const p = r.coin(0.3) ? [0, 0] : randomPoint(L, W, corner, r, []);   // [0,0] sits on the faceoff dot: adversarial
    if (!p) break;
    out.push({ kind: r.pick(['box', 'pillar', 'bumper', 'goal_shield']), x: p[0], z: p[1], size: round(0.5 + r.next() * 9.5, 2) });
  }
  return out;
}

function buildEffects(r, zoneIds, n) {
  const triggers = ['pickup', 'goal_for', 'goal_against', 'check_landed', 'check_received', 'period_start', 'zone_enter', 'zone_exit', 'director'];
  const targets = ['self', 'own_team', 'other_team', 'everyone', 'puck', 'rink'];
  const out = [];
  for (let i = 0; i < n; i++) {
    const trigger = r.pick(triggers.filter((t) => zoneIds.length || !t.startsWith('zone_')));
    const e = { id: `fx-${i + 1}`, target: r.pick(targets), duration_frames: r.pick([60, 7200, r.int(60, 7200)]), trigger, stats: [] };
    const ns = r.int(1, 4);
    for (let j = 0; j < ns; j++) {
      const stat = r.pick(STATS);
      const st = { stat };
      if (r.coin(0.7)) st.multiply = r.pick([0.25, 4, round(0.25 + r.next() * 3.75, 3)]);
      if (r.coin(0.5) || st.multiply === undefined) { const w = statWidth(stat); st.add = r.pick([-w, w, 0, round((r.next() * 2 - 1) * w, 3)]); }
      e.stats.push(st);
    }
    if (r.coin(0.5)) e.kind = r.pick(['speed', 'precision', 'heavy_puck', 'big_puck', 'reverse_other', 'custom']);
    if (zoneIds.length && (trigger === 'zone_enter' || trigger === 'zone_exit') && r.coin(0.8)) e.zone = r.pick(zoneIds);
    if (r.coin(0.5)) e.stack = r.pick(['refresh', 'extend', 'ignore']);
    if (r.coin(0.3)) e.icon = r.pick(['bolt', 'crosshair', 'weight', 'circle', 'swirl', 'snail', 'shield', 'star', 'flame', 'snowflake', 'magnet', 'anchor']);
    out.push(e);
  }
  return out;
}

const SET_PARAMS = ['skater_speed', 'skater_accel', 'shot_speed', 'puck_max_speed', 'puck_radius', 'ice_friction', 'board_restitution', 'skater_mass', 'puck_drag',
  'warp_factor', 'x', 'goal_width_m', 'rink_length_m', 'knockdown_frames'];
function buildHook(r, i, kindIds, maxFrames) {
  const when = r.pick(['on_goal', 'on_period_start', 'on_period_end', 'on_check', 'on_knockdown', 'on_pickup_take', 'on_faceoff', 'on_frame', 'on_score_reached']);
  const h = { id: `hook-${i + 1}`, when, then: [] };
  if (when === 'on_frame') h.every_frames = r.pick([60, 600, 18000, r.int(60, 18000)]);
  if (when === 'on_score_reached') h.score = { at_least: r.int(0, 20) };
  if (r.coin(0.3)) h.after_frame = r.pick([0, 599, 600, r.int(0, 18000)]);
  if (r.coin(0.4)) h.team = r.pick(['scoring', 'conceding', 'any']);
  if (when !== 'on_score_reached' && r.coin(0.3)) {
    h.score = {};
    if (r.coin()) h.score.at_least = r.int(0, 10);
    if (r.coin()) h.score.at_most = r.int(h.score.at_least ?? 0, 20);
    if (r.coin(0.4)) h.score.lead_at_least = r.int(1, 20);
    if (!Object.keys(h.score).length) delete h.score;
  }
  const n = r.int(1, 4);
  for (let j = 0; j < n; j++) {
    const d = r.pick(['add_score', 'set_score_multiplier', 'end_period', 'end_match', 'shrink_rink', 'grow_rink', 'set_param', 'spawn_pickup', 'swap_nets', 'announce', 'apply_modifier']);
    if (SAFE && (d === "set_param" || d === "shrink_rink" || d === "swap_nets" || d === "end_period")) continue;
    const a = { do: d };
    if (r.coin(0.5)) a.team = r.pick(['scoring', 'conceding', 'any']);
    if (d === 'add_score') a.value = r.int(-20, 20);
    if (d === 'set_score_multiplier') a.value = r.pick([1, 5, r.int(1, 5)]);
    if (d === 'shrink_rink' || d === 'grow_rink') a.value = r.pick([0.05, 0.5, round(0.05 + r.next() * 0.45, 3)]);
    if (d === 'set_param') { a.param = r.pick(SET_PARAMS); a.value = r.pick([-20, 20, 0, round((r.next() * 2 - 1) * 20, 3)]); }
    if (d === 'spawn_pickup') a.kind = r.pick(kindIds);
    if (d === 'apply_modifier') { a.kind = r.pick(kindIds); a.frames = r.pick([1, 3600, r.int(1, 3600)]); }
    if (d === 'announce') a.text = r.pick(['GO', 'x'.repeat(60), 'quote " and \\ slash', 'tab\there']);
    if (r.coin(0.2) && d !== 'announce') a.frames = r.int(1, 3600);
    h.then.push(a);
  }
  if (!h.then.length) h.then.push({ do: "announce", text: "GO" });
  return h;
}

// ---------- the base cartridge ----------
const bases = [];
function loadBases() {
  if (bases.length) return bases;
  const dirs = [CART_DIR, join(CART_DIR, 'modes'), join(CART_DIR, 'ladder')];
  for (const d of dirs) {
    for (const f of readdirSync(d).filter((f) => f.endsWith('.cartridge.json')).sort()) bases.push({ name: f, json: JSON.parse(readFileSync(join(d, f), 'utf8')) });
  }
  return bases;
}

function newGame(r, mode, presence) {
  const g = fillObject(GAME_SCHEMA, mode, presence, r);
  g.rink ??= {};
  if (g.rink.length_m === undefined) g.rink.length_m = 90;
  if (g.rink.width_m === undefined) g.rink.width_m = 45;
  g.skater ??= {};
  if (g.skater.speed === undefined) g.skater.speed = 11;
  g.look ??= { tier: 'p3_clay' };
  if (g.look.tier === undefined) g.look.tier = 'p3_clay';
  return g;
}

function repair(g) {
  const L = g.rink.length_m, W = g.rink.width_m;
  if (typeof g.rink.corner_radius_m === 'number' && g.rink.corner_radius_m > Math.min(L, W) / 2) g.rink.corner_radius_m = Math.min(L, W) / 2;
  if (typeof g.rink.goal_width_m === 'number' && g.rink.goal_width_m >= W) g.rink.goal_width_m = round(W - 0.5, 2);
  const sk = g.skater;
  if (typeof sk.check_speed_min === 'number' && sk.check_speed_min > sk.speed) sk.check_speed_min = sk.speed;
}

// scenarios: [name, mutate(game, r)] applied after random fill; each is "adversarial combination" from the brief
const SCENARIOS = {
  'smallest-rink-5v5-most-pickups': (g, r) => {
    g.rink.length_m = 40; g.rink.width_m = 20; g.rink.corner_radius_m = r.pick([0, 10, 15]); g.rink.goal_width_m = r.pick([2, 10]);
    g._sizes = [5, 5];
    g.pickups = Array.from({ length: 8 }, (_, i) => ({ kind: BUILTIN[i % 5], spawn_every_frames: 120, duration_frames: r.pick([1, 3600]), magnitude: r.pick([0.1, 4]), max_on_ice: 4 }));
    g.puck ??= {}; g.puck.count = 3;
  },
  'shortest-timer': (g, r) => { g.rules = { mode: 'timer', timer_frames: 600, goal_pause_frames: r.pick([30, 600]) }; g.rules.variants = r.coin() ? { period_count: 3 } : undefined; g._sizes = [r.int(2, 5), r.int(2, 5)]; },
  'every-ability': (g, r) => {
    g.pickups = [{ kind: 'speed', spawn_every_frames: 120, duration_frames: 600, max_on_ice: 4 }];
    g.abilities = ['shotgun', 'hip_check', 'shield', 'slow_puck'].map((kind, i) => ({ kind, trigger: i === 0 ? 'mode' : 'pickup', charges: r.pick([1, 9]), cooldown_frames: r.pick([60, 3600]), duration_frames: r.pick([1, 600]), magnitude: r.pick([0.1, 4]) }));
    g._sizes = [5, 5];
  },
  'contact-extremes-lo': (g) => { g.contact = { momentum: true, carrier_can_hit: true, lunge_mps: 0, thrust_mps: 0, cross_reach_m: 0, stability_mps: 1, stand_speed_mps: 0, stand_brace: 1, speed_stability: 0.3, sprint_stability: 0.3, spin_stability: 0.3, unseen_stability: 0.3, bump_ratio: 0, loose_ratio: 0, down_ratio: 0.2, down_full_ratio: 1, down_frames_min: 300, down_frames_max: 15, stagger_frames: 60 }; g._sizes = [5, 5]; },
  'contact-extremes-hi': (g) => { g.contact = { momentum: true, carrier_can_hit: true, lunge_mps: 6, thrust_mps: 10, cross_reach_m: 1.5, stability_mps: 30, stand_speed_mps: 5, stand_brace: 3, speed_stability: 1, sprint_stability: 1, spin_stability: 1, unseen_stability: 1, bump_ratio: 1, loose_ratio: 3, down_ratio: 5, down_full_ratio: 10, down_frames_min: 15, down_frames_max: 300, stagger_frames: 0 }; g.skater = { ...g.skater, mass: 160, speed: 16, check_speed_min: 1 }; g._sizes = [5, 5]; },
  'tap-longer-than-charge': (g) => { g.shot = { ...g.shot, tap_frames: 30, charge_frames_max: 5 }; },
  'tap-with-no-charge': (g) => { g.shot = { ...g.shot, tap_frames: 30, charge_frames_max: 0, cooldown_frames: 6 }; },
  'smallest-reach': (g) => { g.stick = { ...g.stick, reach_m: 0.6, carry_reach_mult: 0.5, reception_m: 0, lift_range_m: 0.5 }; g.shot = { ...g.shot, reach_m: 0.5 }; },
  'largest-reach': (g) => { g.stick = { ...g.stick, reach_m: 1.6, carry_reach_mult: 1, reception_m: 3, lift_range_m: 2.5, lift_cone_deg: 180 }; g.shot = { ...g.shot, reach_m: 4 }; },
  'frictionless-boards-dead': (g) => { g.rink.ice_friction = 0; g.rink.board_restitution = 1; g.puck = { ...g.puck, drag: 0, ice_restitution: 1, max_speed: 60 }; g.skater = { ...g.skater, glide_drag: 0 }; },
  'sticky-ice': (g) => { g.rink.ice_friction = 0.1; g.rink.board_restitution = 0; g.puck = { ...g.puck, drag: 2 }; g.skater = { ...g.skater, glide_drag: 1, accel: 4, speed: 3 }; },
  'fast-skaters-small-rink': (g) => { g.rink.length_m = 40; g.rink.width_m = 20; g.skater = { ...g.skater, speed: 16, accel: 40, turn_rate: 12, lateral_grip: 12 }; g.shot = { ...g.shot, speed: 40 }; g._sizes = [5, 5]; },
  'max-lists': (g, r) => {
    g.rink.obstacles = buildObstacles(g, r, 12); g.rink.zones = buildZones(g, r, 8);
  },
  'puck-tiny-and-huge': (g, r) => { g.puck = { ...g.puck, radius_m: r.pick([0.06, 0.4]), count: 3, gravity_mps2: r.pick([0, 30]) }; },
  'goal-extremes': (g, r) => { g.rink.goal_width_m = r.pick([2, 10]); g.rink.goal_height_m = r.pick([0.3, 1.5]); },
  'every-hook-8': (g, r) => { g._hooks = 8; },
  'no-penalties-no-timer-win9': (g) => { g.rules = { mode: 'first_to', score_to_win: 9, penalties: false, goal_pause_frames: 30 }; },
  'mass-and-speed-limits': (g) => { g.skater = { ...g.skater, mass: 40, speed: 16 }; g.skater.check_speed_min = Math.min(g.skater.check_speed_min ?? 12, 16); },
  'two-v-two': (g) => { g._sizes = [2, 2]; },
  'five-v-two': (g) => { g._sizes = [5, 2]; },
  'ai-extremes': (g, r) => { g.ai = { difficulty: r.pick(['rookie', 'pro', 'all_star']), yield_margin_m: r.pick([0, 6]), yield_radius_m: r.pick([0, 40]), hold_radius_m: r.pick([0, 15]), speed_scale: r.pick([0.3, 1.5]), red_skill: r.pick([0, 1]), react_frames: r.pick([0, 60]), body_check: r.pick([0, 1]), sprint: r.pick([0, 1]) }; },
};

function buildRules(g, r, forced) {
  if (forced) return forced;
  const rules = {};
  rules.mode = r.pick(['first_to', 'timer']);
  if (rules.mode === 'first_to') rules.score_to_win = r.pick([1, 9, r.int(1, 9)]);
  else rules.timer_frames = r.pick([600, 36000, r.int(600, 36000)]);
  if (r.coin(0.6)) rules.goal_pause_frames = r.pick([30, 600, r.int(30, 600)]);
  if (r.coin(0.4)) rules.penalties = r.coin();
  if (r.coin(0.3)) rules.penalty_frames = r.pick([60, 600, r.int(60, 600)]);
  if (r.coin(0.2)) rules.offside = false;
  if (r.coin(0.4)) {
    const v = {};
    if (r.coin()) v.sudden_death = r.coin();
    if (r.coin()) v.last_goal_double = r.coin();
    if (r.coin()) v.mercy_goals = r.pick([2, 9, r.int(2, 9)]);
    if (r.coin()) v.period_count = r.int(1, 3);
    if (r.coin() && !SAFE) v.shrink_per_goal = r.pick([0.02, 0.3, round(0.02 + r.next() * 0.28, 3)]);
    if (Object.keys(v).length) rules.variants = v;
  }
  return rules;
}

function buildValid(index, r, profile) {
  const bases_ = loadBases();
  const base = clone(r.pick(bases_.filter((b) => !b.name.includes('fallback') || r.coin(0.1))).json);
  const mode = profile.mode;
  const g = newGame(r, mode, profile.presence);
  const scenarioName = profile.scenario;
  if (scenarioName) SCENARIOS[scenarioName](g, r);
  g.rules = buildRules(g, r, g.rules);
  if (g.rules && g.rules.variants === undefined) delete g.rules.variants;
  repair(g);
  const sizes = g._sizes ?? [r.int(2, 5), r.int(2, 5)];
  delete g._sizes;
  const hookN = g._hooks;
  delete g._hooks;
  // zones and obstacles
  if (g.rink.zones === undefined && r.coin(0.35)) g.rink.zones = buildZones(g, r, r.pick([1, 3, 8]));
  if (g.rink.obstacles === undefined && r.coin(0.35)) g.rink.obstacles = buildObstacles(g, r, r.pick([1, 4, 12]));
  const zoneIds = (g.rink.zones ?? []).map((z) => z.id);
  // effects, pickups, abilities
  let effects = [];
  if (r.coin(0.35) || scenarioName === 'max-effects') effects = buildEffects(r, zoneIds, r.pick([1, 3, 12]));
  if (effects.length) g.effects = effects;
  const kindIds = [...BUILTIN, ...effects.map((e) => e.id)];
  if (g.pickups === undefined && r.coin(0.35)) {
    g.pickups = Array.from({ length: r.pick([1, 4, 8]) }, () => {
      const p = { kind: r.pick(kindIds), spawn_every_frames: r.pick([120, 7200, r.int(120, 7200)]), duration_frames: r.pick([1, 3600, r.int(1, 3600)]) };
      if (r.coin()) p.magnitude = r.pick([0.1, 4, round(0.1 + r.next() * 3.9, 3)]);
      if (r.coin()) p.max_on_ice = r.int(1, 4);
      if (zoneIds.length && r.coin(0.3)) p.zone = r.pick(zoneIds);
      return p;
    });
  }
  if (g.abilities === undefined && r.coin(0.3)) {
    let modeUsed = false;
    g.abilities = Array.from({ length: r.int(1, 4) }, () => {
      const trig = !modeUsed && (r.coin() || !g.pickups) ? 'mode' : (g.pickups ? 'pickup' : null);
      if (trig === 'mode') modeUsed = true;
      return trig ? { kind: r.pick(['shotgun', 'hip_check', 'shield', 'slow_puck']), trigger: trig, charges: r.int(1, 9), cooldown_frames: r.pick([60, 3600, r.int(60, 3600)]), duration_frames: r.pick([1, 600, r.int(1, 600)]), magnitude: r.pick([0.1, 4]) } : null;
    }).filter(Boolean);
    if (!g.abilities.length) delete g.abilities;
  }
  // hooks
  const nh = hookN ?? (r.coin(0.4) ? r.int(1, 8) : 0);
  if (nh) g.rules.hooks = Array.from({ length: nh }, (_, i) => buildHook(r, i, kindIds));
  // teams last: they read the final rink
  g.teams = buildTeams(g, r, sizes[0], sizes[1], r.coin(0.5), r.pick([0, 0, 0, 1]));
  // misc colours and optional objects
  if (r.coin(0.5)) { g.puck ??= {}; g.puck.colour_hex = r.pick(['#111111', '#FFFFFF', '#abcdef']); }
  if (r.coin(0.4)) g.look.trim_colour_hex = r.pick(['#38BDF8', '#000000', '#ffffff']);
  if (r.coin(0.2)) g.meta = { pitch: 'fuzz', facts: Array.from({ length: r.int(1, 3) }, (_, i) => `fact ${i}`) };
  const c = base;
  c.id = `fuzz-${String(index).padStart(5, '0')}`;
  c.title = `Fuzz ${index}`;
  c.seed = r.pick([0, 1, 4294967295, r.int(0, 2 ** 31)]);
  c.game = g;
  // tunables: a few values inside their declared range
  c.values = {};
  for (const t of c.tunables ?? []) {
    if (!r.coin(0.25)) continue;
    if (t.type === 'integer') c.values[t.key] = r.pick([t.min, t.max, r.int(t.min, t.max)]);
    else if (t.type === 'number') c.values[t.key] = r.pick([t.min, t.max, round(t.min + (t.max - t.min) * r.next(), 4)]);
  }
  return c;
}

// ---------- invalid ----------
function paths(v, p = '', out = []) {
  if (Array.isArray(v)) v.forEach((x, i) => { out.push({ p: `${p}/${i}`, v: x }); paths(x, `${p}/${i}`, out); });
  else if (isObj(v)) for (const [k, x] of Object.entries(v)) { out.push({ p: `${p}/${k}`, v: x }); paths(x, `${p}/${k}`, out); }
  return out;
}
function getParent(root, ptr) {
  const parts = ptr.split('/').slice(1);
  let cur = root;
  for (let i = 0; i < parts.length - 1; i++) cur = cur[parts[i]];
  return [cur, parts[parts.length - 1]];
}

let SKIP_TEXT = [];
let SAFE = false;   // smoke corpus: leave out what the findings in docs/experiments/FUZZ.md show to fail (set_param extremes, shrink_rink, obstacles on spawns), so the gate test guards the rest
function buildInvalid(index, r, valid) {
  const kinds = ['wrong-type', 'unknown-key', 'missing-required', 'out-of-range-hi', 'out-of-range-lo', 'null', 'fractional-int', 'empty-list-needed', 'text'];
  const kind = r.pick(kinds);
  const c = clone(valid);
  c.id = `fuzz-bad-${String(index).padStart(5, '0')}`;
  const all = paths(c.game);
  let text = null;
  const numeric = all.filter((e) => typeof e.v === 'number');
  const ints = numeric.filter((e) => Number.isInteger(e.v));
  const objs = [{ p: '/game', v: c.game }, ...all.filter((e) => isObj(e.v))];
  const lim = (ptr) => { const key = ptr.replace(/^\/game\//, '').replace(/\/\d+(?=\/|$)/g, '[]').replace(/\//g, '.'); const m = LIMITS.game_ranges[key]; return m && /^-?[\d.]+\.\.-?[\d.]+$/.test(m) ? m.split('..').map(Number) : null; };
  switch (kind) {
    case 'wrong-type': {
      const t = r.pick(all);
      const [par, key] = getParent(c.game, t.p.replace(/^\/game/, ''));
      par[key] = typeof t.v === 'string' ? 12 : r.pick(['a string', true, [], {}]);
      break;
    }
    case 'unknown-key': { const o = r.pick(objs); (o.v)[r.pick(['foo', 'speedd', 'Speed', 'x_y', 'a'.repeat(50)])] = r.pick([1, 'x', true, {}]); break; }
    case 'missing-required': {
      const o = r.pick(['/game/rink/length_m', '/game/rink/width_m', '/game/skater/speed', '/game/rules/mode', '/game/teams', '/game/look/tier', '/game/skater', '/game/rink', '/game/look', '/game/rules', '/game', '/seed', '/id', '/theme', '/tunables', '/values', '/entities', '/ai', '/rules', '/title', '/schema']);
      const [par, key] = getParent(c, o);
      delete par[key];
      break;
    }
    case 'out-of-range-hi':
    case 'out-of-range-lo': {
      const cand = numeric.map((e) => ({ e, l: lim(e.p) })).filter((x) => x.l);
      if (!cand.length) return null;
      const x = r.pick(cand);
      const [par, key] = getParent(c.game, x.e.p.replace(/^\/game/, ''));
      const hi = kind === 'out-of-range-hi';
      par[key] = Number.isInteger(x.e.v) ? (hi ? x.l[1] + 1 : x.l[0] - 1) : round(hi ? x.l[1] + Math.max(0.001, Math.abs(x.l[1]) * 0.01) : x.l[0] - Math.max(0.001, Math.abs(x.l[0]) * 0.01), 5);
      break;
    }
    case 'null': { const t = r.pick(all); const [par, key] = getParent(c.game, t.p.replace(/^\/game/, '')); par[key] = null; break; }
    case 'fractional-int': { if (!ints.length) return null; const x = r.pick(ints.filter((e) => /_frames|count|charges|score|knock|period|max_on_ice/.test(e.p))); if (!x) return null; const [par, key] = getParent(c.game, x.p.replace(/^\/game/, '')); par[key] = x.v + 0.5; break; }
    case 'empty-list-needed': { c.game.teams = []; break; }
    case 'text': {
      const s = JSON.stringify(c, null, 2);
      const m = r.pick(['truncate', 'bom', 'comment', 'trailing-comma', 'huge-exp', 'neg-huge-exp', 'tiny-exp', 'dup-key', 'deep', 'empty', 'whitespace', 'array-root', 'string-root', 'number-root', 'null-root', 'nan', 'single-quote', 'nul-char', 'big-int', 'neg-zero', 'deep-obj', 'deep-huge', 'long-string', 'many-keys'].filter((x) => !SKIP_TEXT.includes(x)));
      switch (m) {
        case 'truncate': text = s.slice(0, r.int(1, s.length - 2)); break;
        case 'bom': text = '﻿' + s; break;
        case 'comment': text = s.replace('{', '{ // comment\n'); break;
        case 'trailing-comma': text = s.replace(/\n}$/, ',\n}'); break;
        case 'huge-exp': text = s.replace(/"length_m": [\d.]+/, '"length_m": 1e999'); break;
        case 'neg-huge-exp': text = s.replace(/"speed": [\d.]+/, '"speed": -1e999'); break;
        case 'tiny-exp': text = s.replace(/"speed": [\d.]+/, '"speed": 1e-999'); break;
        case 'dup-key': text = s.replace('"game": {', '"game": {"rink": {"length_m": 90, "width_m": 45},'); break;
        case 'deep': text = '['.repeat(4000) + ']'.repeat(4000); break;
        case 'deep-obj': text = '{"a":'.repeat(20000) + '1' + '}'.repeat(20000); break;
        case 'deep-huge': text = '['.repeat(300000) + ']'.repeat(300000); break;
        case 'long-string': text = s.replace(/"title": "[^"]*"/, `"title": "${'x'.repeat(5000000)}"`); break;
        case 'many-keys': text = s.replace('"game": {', '"game": {' + Array.from({ length: 20000 }, (_, i) => `"k${i}": 1`).join(',') + ','); break;
        case 'empty': text = ''; break;
        case 'whitespace': text = '   \n\t '; break;
        case 'array-root': text = '[' + s + ']'; break;
        case 'string-root': text = '"hello"'; break;
        case 'number-root': text = '12'; break;
        case 'null-root': text = 'null'; break;
        case 'nan': text = s.replace(/"speed": [\d.]+/, '"speed": NaN'); break;
        case 'single-quote': text = s.replace(/"id": "[^"]*"/, "'id': 'x'"); break;
        case 'nul-char': text = s.replace(/"title": "[^"]*"/, '"title": "a\\u0000b"'); break;
        case 'big-int': text = s.replace(/"seed": \d+/, '"seed": 99999999999999999999'); break;
        case 'neg-zero': text = s.replace(/"seed": \d+/, '"seed": -0'); break;
      }
      return { name: `${c.id}`, text, profile: `text:${m}` };
    }
  }
  return { name: `fuzz-bad-${String(index).padStart(5, '0')}`, text: JSON.stringify(c), profile: kind };
}

const everyFrame = (g, doing) => { g.rules.hooks = [{ id: 'h1', when: 'on_frame', every_frames: 60, then: [doing] }]; };
const setParam = (param, value) => (g) => everyFrame(g, { do: 'set_param', param, value });
const MINIMAL = {
  'setparam-skater-mass-neg': setParam('skater_mass', -20), 'setparam-skater-mass-zero': setParam('skater_mass', 0), 'setparam-ice-friction-20': setParam('ice_friction', 20),
  'setparam-ice-friction-neg': setParam('ice_friction', -20), 'setparam-puck-radius-neg': setParam('puck_radius', -20), 'setparam-puck-radius-20': setParam('puck_radius', 20),
  'setparam-puck-max-speed-neg': setParam('puck_max_speed', -20), 'setparam-puck-max-speed-zero': setParam('puck_max_speed', 0), 'setparam-skater-speed-neg': setParam('skater_speed', -20),
  'setparam-skater-accel-20': setParam('skater_accel', 20), 'setparam-board-restitution-20': setParam('board_restitution', 20), 'setparam-puck-drag-20': setParam('puck_drag', 20),
  'setparam-knockdown-neg': setParam('knockdown_frames', -20), 'setparam-shot-speed-neg': setParam('shot_speed', -20), 'setparam-unknown': setParam('warp_factor', 5),
  'setparam-rink-length-neg': setParam('rink_length_m', -20), 'setparam-goal-width-neg': setParam('goal_width_m', -20),
  'spawn-in-big-box-at-boards': (g) => { g.rink.obstacles = [{ kind: 'box', x: -43, z: 0, size: 10 }]; g.teams[0].skaters[1].spawn = [-43, 0]; },
  'spawn-in-pillar-centre': (g) => { g.rink.obstacles = [{ kind: 'pillar', x: -7.5, z: 0, size: 10 }]; },
  'spawn-in-goal-shield': (g) => { g.rink.obstacles = [{ kind: 'goal_shield', x: -43, z: 0, size: 10 }]; g.teams[0].skaters[1].spawn = [-43, 0]; },
  'obstacle-on-faceoff-dot': (g) => { g.rink.obstacles = [{ kind: 'pillar', x: 0, z: 0, size: 10 }]; },
  'timer-600': (g) => { g.rules = { mode: 'timer', timer_frames: 600 }; },
  'faceoff-add-score': (g) => { g.rules = { mode: 'timer', timer_frames: 15724, hooks: [{ id: 'h1', when: 'on_faceoff', then: [{ do: 'add_score', team: 'any', value: -20 }] }] }; },
  'goal-add-score-minus': (g) => { g.rules.hooks = [{ id: 'h1', when: 'on_goal', then: [{ do: 'add_score', team: 'scoring', value: -20 }] }]; },
  'end-period-every-60': (g) => { g.rules = { mode: 'timer', timer_frames: 6000, hooks: [{ id: 'h1', when: 'on_frame', every_frames: 60, then: [{ do: 'end_period' }] }] }; },
  'shrink-every-60': (g) => { g.rules.hooks = [{ id: 'h1', when: 'on_frame', every_frames: 60, then: [{ do: 'shrink_rink', value: 0.5 }] }]; },
  'swap-nets-every-60': (g) => { g.rules.hooks = [{ id: 'h1', when: 'on_frame', every_frames: 60, then: [{ do: 'swap_nets' }] }]; },
  'end-match-at-0': (g) => { g.rules.hooks = [{ id: 'h1', when: 'on_faceoff', then: [{ do: 'end_match' }] }]; },
};

// ---------- main ----------
function verdict(text) {
  let errors;
  try { errors = validateCartridge(JSON.parse(text)).errors; } catch { errors = ['at /: not json']; }
  const pointers = [...new Set(errors.map((e) => e.match(/^at (\S+?):/)?.[1] ?? '?'))].sort();
  return { ok: errors.length === 0, pointers, first: errors[0] };
}

export function generate({ count, invalid, seed, out, skipText = [], safe = false }) {
  SKIP_TEXT = skipText;
  SAFE = safe;
  const r = rng(seed);
  mkdirSync(out, { recursive: true });
  const manifest = { schema: 'tj.fuzz/1', seed, cases: [] };
  const scenarioNames = Object.keys(SCENARIOS);
  const modes = ['limits', 'mid', 'rand', 'mixed'];
  let n = 0, rejected = 0;
  const rejectionReasons = {};
  const add = (name, text, kind, profile) => {
    n++;
    const file = `c${String(n).padStart(5, '0')}.json`;
    writeFileSync(join(out, file), text);
    const v = verdict(text);
    manifest.cases.push({ name, file, kind, profile, ok: v.ok, pointers: v.pointers });
    return v;
  };
  // 1. every scenario at least five times, then random profiles
  const validPool = [];
  for (let i = 0; i < count; i++) {
    const profile = i < scenarioNames.length * 5
      ? { scenario: scenarioNames[i % scenarioNames.length], mode: r.pick(modes), presence: r.pick([0.5, 0.9, 1]) }
      : { scenario: r.coin(0.35) ? r.pick(scenarioNames) : null, mode: modes[i % modes.length], presence: r.pick([0.3, 0.6, 0.9, 1]) };
    const c = buildValid(i + 1, r, profile);
    const text = JSON.stringify(c, null, 1);
    const label = `${profile.scenario ?? 'random'}/${profile.mode}/${profile.presence}`;
    const v = add(c.id, text, 'valid', label);
    if (!v.ok) { rejected++; const key = (v.first ?? '').replace(/[-\d.]+/g, '#').slice(0, 110); rejectionReasons[key] = (rejectionReasons[key] ?? 0) + 1; manifest.cases[manifest.cases.length - 1].kind = 'rejected'; }
    else validPool.push(c);
  }
  // 1b. hand-minimal cartridges for the faults the first runs showed (hockey.regular plus one change each)
  for (const [name, mutate] of SAFE ? [] : Object.entries(MINIMAL)) {
    const c = JSON.parse(readFileSync(join(CART_DIR, 'hockey.regular.cartridge.json'), 'utf8'));
    c.id = 'min-' + name; mutate(c.game, c);
    add(c.id, JSON.stringify(c, null, 1), 'valid', 'minimal/' + name);
  }
  // 2. invalid
  const base = validPool.length ? validPool : [JSON.parse(readFileSync(join(CART_DIR, 'hockey.regular.cartridge.json'), 'utf8'))];
  let produced = 0, guard = 0;
  while (produced < invalid && guard++ < invalid * 20) {
    const res = buildInvalid(produced + 1, r, r.pick(base));
    if (!res) continue;
    const v = add(res.name, res.text, 'invalid', res.profile);
    if (v.ok) { manifest.cases[manifest.cases.length - 1].kind = 'invalid-but-accepted'; }
    produced++;
  }
  manifest.summary = { total: n, valid: manifest.cases.filter((c) => c.kind === 'valid').length, rejected, invalid: produced, rejectionReasons };
  manifest.summary.invalidAccepted = manifest.cases.filter((c) => c.kind === 'invalid-but-accepted').length;
  writeFileSync(join(out, 'manifest.json'), JSON.stringify(manifest, null, 1));
  return manifest.summary;
}

if (import.meta.url === `file:///${process.argv[1].replace(/\\/g, '/')}` || process.argv[1]?.endsWith('gen.mjs')) {
  const argv_has = (k) => process.argv.includes(`--${k}`);
  const arg = (k, d) => { const i = process.argv.indexOf(`--${k}`); return i >= 0 ? process.argv[i + 1] : d; };
  const summary = generate({ count: Number(arg('count', '5000')), invalid: Number(arg('invalid', '1000')), seed: Number(arg('seed', '1')), safe: argv_has("safe"), skipText: (arg('skip-text', '') || '').split(',').filter(Boolean), out: resolve(arg('out', join(HERE, 'out', 'run1'))) });
  console.log(JSON.stringify(summary, null, 1));
}
