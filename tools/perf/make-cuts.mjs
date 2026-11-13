// Lane A58: one profiling cut per look tier, from hockey.regular. Only look.tier, id, title and the rules change:
// a 200 s timer so a 3-minute bot match never ends early on first_to 3 (the regular cut's rule).
import fs from 'node:fs'; import path from 'node:path'; import { fileURLToPath } from 'node:url';
const here = path.dirname(fileURLToPath(import.meta.url));
const root = path.resolve(here, '../..');
const base = JSON.parse(fs.readFileSync(path.join(root, 'Game/Content/Cartridges/hockey.regular.cartridge.json'), 'utf8'));
const tiers = process.argv.slice(2).length ? process.argv.slice(2) : ['l2_night', 'p3_clay', 'p4_broadcast', 'l1_day', 'l3_neon'];
for (const t of tiers) {
  const c = structuredClone(base);
  c.id = `perf-${t.replace(/_/g, '-')}`; c.title = `Perf ${t}`;
  c.game.look.tier = t;
  c.game.rules = { mode: 'timer', timer_frames: 12000, goal_pause_frames: base.game.rules.goal_pause_frames, offside: false, penalties: base.game.rules.penalties };
  const f = path.join(here, 'cuts', `perf.${t}.cartridge.json`);
  fs.writeFileSync(f, JSON.stringify(c, null, 2) + '\n'); console.log(f);
}
