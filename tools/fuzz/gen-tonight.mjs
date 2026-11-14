#!/usr/bin/env node
// Corpora for lane A159 (track S3, evolution 4, items 3 and 4): the keys and switches added since the last fuzz, and the cuts the operator clicks.
//
//   node tools/fuzz/gen-tonight.mjs keys  --out tools/fuzz/out/a159-keys  [--per-class 200] [--seed 159]
//   node tools/fuzz/gen-tonight.mjs cuts  --out tools/fuzz/out/a159-cuts  [--seeds 20]
//
// Same manifest format as gen.mjs (tj.fuzz/1), so tools/fuzz/run.mjs runs it with the oracles of Bar.Fuzz.Cartridges unchanged (validator parity, brain-vs-brain twice
// with equal hashes, the playtester twice, no non-finite state, nothing beyond the boards, the long run that must end, the hard-stop oracle).
//
// keys: classes of schema keys (named below; the key lists are checked against the schema, a renamed key throws). Per class, `per-class` matches: first every key of the class
//   alone at both ends (so each limit is played), then random cartridges whose keys of the class are each at an end (40 %) or uniform inside the range (60 %), on a random base
//   cut (the regular cut, the arenas, the twelve modes, the season's takes) with a random seed. The levels (ai.difficulty) and five-a-side teams are classes of their own.
// cuts: the twelve mode cuts and the eight takes (Cartridges/modes, Cartridges/ladder take-N), each at each level (ai.difficulty) for N seeds.
import { readFileSync, writeFileSync, mkdirSync, readdirSync } from 'node:fs';
import { dirname, resolve, join } from 'node:path';
import { fileURLToPath } from 'node:url';
import { rng } from './gen.mjs';
import { schemaLeaves, setPath } from './keys.mjs';
import { validateCartridgeText } from '../../schema/validate.mjs';

const HERE = dirname(fileURLToPath(import.meta.url));
const CART_DIR = resolve(HERE, '..', '..', 'Game', 'Content', 'Cartridges');
const clone = (v) => JSON.parse(JSON.stringify(v));
const LEVELS = ['rookie', 'pro', 'all_star'];

export const CLASSES = {
  stick: ['stick.passive_take', 'stick.body_shield', 'stick.take_grace_frames', 'stick.poke_assist_deg', 'stick.miss_recovery_frames'],
  skater: ['skater.reverse_brake_decel', 'skater.burst.frames', 'skater.burst.accel_mult', 'skater.burst.own_stop', 'skater.burst.rest_frames', 'skater.burst.rest_kept_mps', 'skater.spin.exposed_frames'],
  contact: ['contact.board_bonus', 'contact.release_grace_frames', 'contact.check_reach_m'],
  brain: ['ai.yield_margin_m', 'ai.yield_radius_m', 'ai.hold_radius_m', 'ai.speed_scale', 'ai.red_skill', 'ai.react_frames', 'ai.body_check', 'ai.sprint'],
  level: ['ai.difficulty'],
  'five-a-side': [],
};

function bases() {
  const out = [];
  for (const d of [CART_DIR, join(CART_DIR, 'modes'), join(CART_DIR, 'ladder')]) {
    for (const f of readdirSync(d).filter((x) => x.endsWith('.cartridge.json')).sort()) {
      if (/^hockey\.test\./.test(f) || /fallback/.test(f)) continue;
      out.push({ name: f.replace('.cartridge.json', ''), json: JSON.parse(readFileSync(join(d, f), 'utf8')) });
    }
  }
  return out;
}

const verdict = (cart) => validateCartridgeText(JSON.stringify(cart));

function leafOf(leaves, p) {
  const l = leaves.find((x) => x.path === p);
  if (!l) throw new Error(`gen-tonight.mjs names ${p}, which is not a key of game.schema.json: rename it there`);
  return l;
}
const ends = (l) => (l.type === 'enum' ? l.values : [l.values[0], l.values[1]]);
function inside(l, r) {
  if (l.type === 'enum' || l.type === 'boolean') return r.pick(l.values);
  const [lo, hi] = l.values;
  if (l.type === 'integer') return r.int(lo, hi);
  return Number((lo + r.next() * (hi - lo)).toFixed(3));
}

// grow or cut a team to n skaters (a copy of its last skater, on a grid of spawns; the validator decides if it stands)
function setTeamSize(cart, n) {
  const L = cart.game.rink?.length_m ?? 90, W = cart.game.rink?.width_m ?? 45;
  cart.game.teams.forEach((t, ti) => {
    const side = ti === 0 ? -1 : 1;
    while (t.skaters.length > n) t.skaters.pop();
    while (t.skaters.length < n) t.skaters.push({ role: 'chaser', spawn: [0, 0] });
    t.skaters.forEach((s, i) => { s.spawn = [Number((side * L * (0.08 + 0.06 * Math.floor(i / 2))).toFixed(2)), Number(((i % 2 ? 1 : -1) * W * 0.12 * (1 + Math.floor(i / 2) * 0.6)).toFixed(2))]; });
    if (t.skaters.length && !t.skaters.some((s) => s.role === 'player') && ti === 0) t.skaters[0].role = 'player';
    if (t.skaters.length && !t.skaters.some((s) => s.role === 'goalie') && t.skaters.length > 1 && t.skaters.some((s) => s.role === 'goalie') === false) { /* the base had none: keep it so */ }
  });
}

export function genKeys({ out, perClass = 200, seed = 159 }) {
  mkdirSync(out, { recursive: true });
  const leaves = schemaLeaves();
  const pool = bases();
  const r = rng(seed);
  const manifest = { schema: 'tj.fuzz/1', seed, cases: [], classes: {} };
  let n = 0;
  const skipped = {};
  for (const [cls, keys] of Object.entries(CLASSES)) {
    const ls = keys.map((k) => leafOf(leaves, k));
    let made = 0, tries = 0;
    const add = (name, cart, mode) => {
      cart.id = `a159-${cls}-${String(made + 1).padStart(3, '0')}`;
      cart.title = `A159 ${cls} ${made + 1}`.slice(0, 60);
      const v = verdict(cart);
      if (v.errors.length) { skipped[cls] = (skipped[cls] ?? 0) + 1; return false; }
      n++; made++;
      const file = `c${String(n).padStart(5, '0')}.json`;
      writeFileSync(join(out, file), JSON.stringify(cart, null, 1));
      manifest.cases.push({ name: `${cls}/${name}`, file, kind: 'valid', profile: `a159/${cls}/${mode}`, ok: true, pointers: [] });
      return true;
    };
    const rand = () => { const b = r.pick(pool); const c = clone(b.json); c.seed = r.int(1, 2 ** 31 - 2); return [b, c]; };
    // 1. every key of the class alone at both ends (a level: every level on every shipped base would be item 4's; here the regular cut and a random base)
    for (const l of ls) for (const v of ends(l)) {
      if (made >= perClass) break;
      const [b, c] = rand();
      setPath(c.game, l.path, v);
      add(`${l.path}=${v}@${b.name}`, c, 'limit');
    }
    // 2. random cartridges
    while (made < perClass && tries++ < perClass * 6) {
      const [b, c] = rand();
      const parts = [];
      if (cls === 'five-a-side') {
        setTeamSize(c, 5);
        if (r.coin(0.5)) setTeamSize(c, r.pick([1, 2, 3, 4, 5]));
        parts.push(`teams=${c.game.teams.map((t) => t.skaters.length).join('v')}`);
        // keys of tonight at their ends on top: the five-a-side game is where they were played last
        for (const k of [...CLASSES.stick, ...CLASSES.contact]) if (r.coin(0.2)) { const l = leafOf(leaves, k); const v = r.coin(0.5) ? r.pick(ends(l)) : inside(l, r); setPath(c.game, k, v); parts.push(`${k}=${v}`); }
      } else {
        for (const l of ls) {
          if (!r.coin(0.6) && ls.length > 1) continue;
          const v = r.coin(0.4) ? r.pick(ends(l)) : inside(l, r);
          setPath(c.game, l.path, v);
          parts.push(`${l.path}=${v}`);
        }
        if (!parts.length) continue;
        if (cls !== 'level' && r.coin(0.4)) { const lv = r.pick(LEVELS); setPath(c.game, 'ai.difficulty', lv); parts.push(`ai.difficulty=${lv}`); }
      }
      add(`${parts.join(',').slice(0, 150)}@${b.name}`, c, 'random');
    }
    manifest.classes[cls] = { keys, cases: made, refused_by_validator: skipped[cls] ?? 0 };
  }
  manifest.summary = { total: n, classes: manifest.classes };
  writeFileSync(join(out, 'manifest.json'), JSON.stringify(manifest, null, 1));
  return manifest.summary;
}

export function genCuts({ out, seeds = 20 }) {
  mkdirSync(out, { recursive: true });
  const manifest = { schema: 'tj.fuzz/1', seed: 0, cases: [] };
  const all = bases();
  const modes = all.filter((b) => /^hockey\.mode\./.test(b.name));
  const takes = all.filter((b) => /^take-[1-8]$/.test(b.name));
  if (modes.length !== 12) throw new Error(`expected 12 mode cuts, found ${modes.length}`);
  if (takes.length !== 8) throw new Error(`expected 8 takes, found ${takes.length}`);
  let n = 0;
  const rejected = [];
  for (const b of [...modes, ...takes]) for (const lv of LEVELS) for (let s = 1; s <= seeds; s++) {
    const c = clone(b.json);
    c.seed = 1000 * s + 7;
    setPath(c.game, 'ai.difficulty', lv);
    c.id = `${b.name.replace(/\./g, '-')}-${lv.replace('_', '-')}-s${String(s).padStart(2, '0')}`.slice(0, 60);
    c.title = c.id;
    const v = verdict(c);
    if (v.errors.length) { rejected.push(`${c.id}: ${v.errors[0]}`); continue; }
    n++;
    const file = `c${String(n).padStart(5, '0')}.json`;
    writeFileSync(join(out, file), JSON.stringify(c, null, 1));
    manifest.cases.push({ name: c.id, file, kind: 'valid', profile: `a159/cuts/${b.name}/${lv}`, ok: true, pointers: [] });
  }
  manifest.summary = { total: n, cuts: modes.length + takes.length, levels: LEVELS, seeds, rejected };
  writeFileSync(join(out, 'manifest.json'), JSON.stringify(manifest, null, 1));
  return manifest.summary;
}

if (process.argv[1] && resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
  const [cmd] = process.argv.slice(2);
  const arg = (k, d) => { const i = process.argv.indexOf(`--${k}`); return i >= 0 ? process.argv[i + 1] : d; };
  const out = resolve(arg('out', join(HERE, 'out', `a159-${cmd}`)));
  const s = cmd === 'keys' ? genKeys({ out, perClass: Number(arg('per-class', '200')), seed: Number(arg('seed', '159')) }) : cmd === 'cuts' ? genCuts({ out, seeds: Number(arg('seeds', '20')) }) : null;
  if (!s) { console.log('usage: gen-tonight.mjs keys|cuts --out <dir>'); process.exit(2); }
  console.log(JSON.stringify(s, null, 1));
}
