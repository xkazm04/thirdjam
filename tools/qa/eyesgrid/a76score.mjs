// Lane A76: tally of the eyes columns. node a76score.mjs <worktree> [column...]  (default before after cardmove)
import { readFileSync, readdirSync } from 'node:fs';
import { join, basename } from 'node:path';
const wt = process.argv[2], cols = process.argv.slice(3).length ? process.argv.slice(3) : ['before', 'after', 'cardmove'];
const dir = join(wt, 'docs', 'experiments', 'eyes');
const rowsOf = (col, preset) => {
  const out = [];
  for (const f of readdirSync(dir).filter((x) => new RegExp(`^a76-${preset}-${col}-[0-9]+\.json$`).test(x)).sort()) {
    const j = JSON.parse(readFileSync(join(dir, f), 'utf8'));
    for (const r of j.results ?? []) { if (!r.ok || !r.answer?.stills) continue;
      r.answer.stills.forEach((a, i) => out.push({ still: basename(r.stills[i]?.path ?? '?'), a })); }
  }
  return out;
};
const names = { card: 'card', faceoff: 'faceoff', farnet: 'farnet' };
for (const col of cols) {
  const loc = rowsOf(col, 'locate'), hud = rowsOf(col, 'hud'), rd = rowsOf(col, 'readability');
  const n = loc.length;
  const c = (p) => loc.filter(p).length;
  console.log(`\n== ${col}: ${n} stills located`);
  console.log(`puck told (score >= 3): ${c((r) => r.a.puck.score >= 3)} of ${n}; mean ${(loc.reduce((s, r) => s + r.a.puck.score, 0) / (n || 1)).toFixed(2)}`);
  console.log(`far net told (score >= 3): ${c((r) => r.a.far_net.score >= 3)} of ${n}; mean ${(loc.reduce((s, r) => s + r.a.far_net.score, 0) / (n || 1)).toFixed(2)}`);
  console.log(`badge covers the net: ${c((r) => r.a.far_net.badge_covers_net)} of ${n}: ${loc.filter((r) => r.a.far_net.badge_covers_net).map((r) => r.still).join(' ')}`);
  console.log(`overlapping marks listed: ${c((r) => r.a.overlapping_marks.length)} of ${n}: ${loc.filter((r) => r.a.overlapping_marks.length).map((r) => r.still).join(' ')}`);
  console.log(`MATCH card in the middle third: ${c((r) => r.a.match_card_in_middle_third)} of ${n}: ${loc.filter((r) => r.a.match_card_in_middle_third).map((r) => r.still).join(' ')}`);
  const coll = hud.filter((r) => (r.a.collisions ?? []).length);
  console.log(`hud collisions listed: ${coll.length} of ${hud.length}: ${coll.map((r) => r.still).join(' ')}`);
  const cross = hud.filter((r) => (r.a.collisions ?? []).some((x) => /crossbar|net/i.test(x)));
  console.log(`hud collisions naming a net / crossbar: ${cross.length}: ${cross.map((r) => r.still).join(' ')}`);
  const pk = rd.filter((r) => (r.a.elements?.puck?.score ?? 5) <= 1).length;
  console.log(`readability (6 m) puck scored 1: ${pk} of ${rd.length}`);
}
