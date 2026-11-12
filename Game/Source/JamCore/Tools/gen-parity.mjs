// Builds the parity table the C++ loader is tested against: every preset, every schema/fixtures file, and a set of
// mutations of hockey.regular, each with Node's verdict and the JSON pointers of its errors. The C++ test
// (Jam.Core.CartridgeParity) must give the same verdict and the same pointer set for every case.
//   node Game/Source/JamCore/Tools/gen-parity.mjs
import { readFileSync, readdirSync, writeFileSync } from 'node:fs';
import { dirname, resolve, basename } from 'node:path';
import { fileURLToPath } from 'node:url';
import { validateCartridge } from '../../../../schema/validate.mjs';

const here = dirname(fileURLToPath(import.meta.url));
const repo = resolve(here, '..', '..', '..', '..');
const cartDir = resolve(repo, 'Game', 'Content', 'Cartridges');
const cases = [];
const pointers = (errors) => [...new Set(errors.map((e) => e.match(/^at (\S+?):/)?.[1] ?? '?'))].sort();
const add = (name, text) => {
  let errors;
  try { errors = validateCartridge(JSON.parse(text)).errors; } catch (e) { errors = ['at /: not json']; }
  cases.push({ name, ok: errors.length === 0, pointers: pointers(errors), text });
};
for (const f of readdirSync(cartDir).filter((f) => f.endsWith('.json')).sort()) add(`preset ${f}`, readFileSync(resolve(cartDir, f), 'utf8'));
for (const dir of ['fixtures', 'fixtures/bad']) {
  for (const f of readdirSync(resolve(repo, 'schema', dir)).filter((f) => f.endsWith('.json')).sort()) add(`${dir}/${f}`, readFileSync(resolve(repo, 'schema', dir, f), 'utf8'));
}
const base = () => JSON.parse(readFileSync(resolve(cartDir, 'hockey.regular.cartridge.json'), 'utf8'));
const mut = {
  'unknown key in rink': (c) => { c.game.rink.foo = 1; },
  'null value': (c) => { c.game.puck.drag = null; },
  'fractional frames': (c) => { c.game.shot.cooldown_frames = 24.5; },
  'bad role': (c) => { c.game.teams[1].skaters[0].role = 'goalie'; },
  'two players': (c) => { c.game.teams[1].skaters[0].role = 'player'; },
  'no player': (c) => { c.game.teams[0].skaters[0].role = 'chaser'; },
  'spawn outside': (c) => { c.game.teams[0].skaters[1].spawn = [-59.8, 29.8]; },
  'spawn in corner': (c) => { c.game.teams[0].skaters[1].spawn = [-59, -29]; },
  'spawns overlap': (c) => { c.game.teams[1].skaters[1].spawn = [10, 0.5]; },
  'corner too large': (c) => { c.game.rink.width_m = 20; c.game.rink.corner_radius_m = 12; c.game.teams.forEach((t) => t.skaters.forEach((s) => { s.spawn[1] = 0; })); c.game.teams[0].skaters[1].spawn = [-12, 0]; c.game.teams[1].skaters[1].spawn = [12, 0]; },
  'goal wider than rink': (c) => { c.game.rink.width_m = 20; c.game.rink.goal_width_m = 10; c.game.teams.forEach((t) => t.skaters.forEach((s) => { s.spawn[1] = 0; })); c.game.teams[0].skaters[1].spawn = [-12, 0]; c.game.teams[1].skaters[1].spawn = [12, 0]; },
  'first_to without score': (c) => { delete c.game.rules.score_to_win; },
  'timer without frames': (c) => { c.game.rules.mode = 'timer'; },
  'timer ok': (c) => { c.game.rules.mode = 'timer'; c.game.rules.timer_frames = 10800; delete c.game.rules.score_to_win; },
  'value not a tunable': (c) => { c.values.warp = 3; },
  'value out of range': (c) => { c.values.skater_speed = 30; },
  'value fractional frames': (c) => { c.values.goal_pause_frames = 90.5; },
  'value ok': (c) => { c.values.skater_speed = 11; c.values.goal_pause_frames = 90; },
  'same colours': (c) => { c.game.teams[1].colour_hex = '#1d4ed8'; },
  'same team ids': (c) => { c.game.teams[1].id = 'blue'; },
  'obstacle outside': (c) => { c.game.rink.obstacles = [{ kind: 'box', x: 70, z: 0, size: 2 }]; },
  'obstacles ok': (c) => { c.game.rink.obstacles = [{ kind: 'box', x: 20, z: 10, size: 2 }, { kind: 'pillar', x: -20, z: -10, size: 1.5 }]; },
  'bad visual': (c) => { c.game.skater.visual = 'robot'; },
  'visual ok': (c) => { c.game.skater.visual = 'generated_b2'; },
  'offside true': (c) => { c.game.rules.offside = true; },
  'bad hex': (c) => { c.game.puck.colour_hex = '#12345'; },
  'negative seed': (c) => { c.seed = -1; },
  'string seed': (c) => { c.seed = '1'; },
  'check min above speed': (c) => { c.game.skater.check_speed_min = 9; },
  'missing rink': (c) => { delete c.game.rink; },
  'three teams': (c) => { c.game.teams.push(JSON.parse(JSON.stringify(c.game.teams[1]))); c.game.teams[2].id = 'green'; c.game.teams[2].colour_hex = '#00FF00'; },
  'one skater team': (c) => { c.game.teams[1].skaters = [c.game.teams[1].skaters[0]]; },
  'title too long': (c) => { c.title = 'x'.repeat(61); },
  'bad id': (c) => { c.id = 'Hockey_Regular'; },
  'unknown top key': (c) => { c.extra = true; },
  'spawn three numbers': (c) => { c.game.teams[0].skaters[0].spawn = [1, 2, 3]; },
  'rink too long': (c) => { c.game.rink.length_m = 200; },
  'pickup ok': (c) => { c.game.pickups = [{ kind: 'speed', spawn_every_frames: 600, duration_frames: 300 }]; },
  'pickup bad kind': (c) => { c.game.pickups = [{ kind: 'jetpack', spawn_every_frames: 600, duration_frames: 300 }]; },
  'camera unknown key': (c) => { c.game.camera.zoom = 2; },
  'look missing tier': (c) => { delete c.game.look.tier; },
};
for (const [name, f] of Object.entries(mut)) { const c = base(); f(c); add(`mutation ${name}`, JSON.stringify(c, null, 2)); }
add('mutation duplicate key (Node accepts, C++ must reject: README rule)', readFileSync(resolve(cartDir, 'hockey.regular.cartridge.json'), 'utf8').replace('"seed": 1,', '"seed": 1,\n  "seed": 2,'));
cases.at(-1).ok = false; cases.at(-1).pointers = ['/seed'];
writeFileSync(resolve(here, 'parity.json'), JSON.stringify({ tool: 'gen-parity', cases }, null, 1));
console.log(`PASS gen-parity ${cases.length} cases, ${cases.filter((c) => c.ok).length} valid`);
for (const c of cases) console.log(`  ${c.ok ? 'ok ' : 'bad'} ${c.name} ${c.pointers.join(' ')}`);
