// A108: writes the matrix cuts: each of the 12 mode cartridges, and each look tier, as an acceptance cut.
// A mode cut = the mode cartridge with the acceptance run's own overrides (9 to win, first_to, penalties off, rookie not forced: the mode's level stays).
// A tier cut = accept-cut.json with look.tier set. Usage: node make-matrix-cuts.mjs   (writes tools/qa/accept/matrix-cuts/*.json)
import fs from 'node:fs'; import path from 'node:path';
const here = path.dirname(new URL(import.meta.url).pathname.replace(/^\/([A-Za-z]:)/, '$1'));
const root = path.resolve(here, '..', '..', '..');
const out = path.join(here, 'matrix-cuts'); fs.mkdirSync(out, { recursive: true });
const base = JSON.parse(fs.readFileSync(path.join(here, 'accept-cut.json'), 'utf8'));
const modesDir = path.join(root, 'Game', 'Content', 'Cartridges', 'modes');
for (const f of fs.readdirSync(modesDir).filter(n => /^hockey\.mode\..*\.cartridge\.json$/.test(n))) {
  const slug = f.replace(/^hockey\.mode\./, '').replace(/\.cartridge\.json$/, '');
  const c = JSON.parse(fs.readFileSync(path.join(modesDir, f), 'utf8'));
  c.id = 'a108-mode-' + slug; c.seed = 1;
  c.game.rules.mode = 'first_to'; c.game.rules.score_to_win = 9; c.game.rules.penalties = false;
  delete c.game.rules.timer_frames;
  fs.writeFileSync(path.join(out, 'mode-' + slug + '.json'), JSON.stringify(c, null, 1));
}
for (const t of ['l1_day', 'l2_night', 'l3_neon', 'p3_clay', 'p4_broadcast']) {
  const c = JSON.parse(JSON.stringify(base)); c.id = 'a108-tier-' + t.replace(/_/g, '-'); c.game.look.tier = t;
  fs.writeFileSync(path.join(out, 'tier-' + t + '.json'), JSON.stringify(c, null, 1));
}
console.log(fs.readdirSync(out).join('\n'));
