#!/usr/bin/env node
// Lane A110: one markdown row per profile summary (thirdjam.soaklock.summary/1) in the given .jsonl files, with the guest's abilities (A110's counter).
// Usage: node tools/soak/lockstep/modes-table.mjs Game/Saved/SoakLockstep/a110/*.jsonl
import { readFileSync } from 'node:fs';
const rows = [];
for (const f of process.argv.slice(2)) {
  for (const line of readFileSync(f, 'utf8').split(/\r?\n/)) {
    if (!line.startsWith('{')) continue;
    let r; try { r = JSON.parse(line); } catch { continue; }
    if (r.schema !== 'thirdjam.soaklock.summary/1') continue;
    const [prof, cut] = r.profile.split('@');
    rows.push({ cut: (cut || 'hockey.regular').replace('modes/hockey.mode.', ''), prof, r });
  }
}
rows.sort((a, b) => a.cut.localeCompare(b.cut) || a.prof.localeCompare(b.prof));
console.log('| mode | profile | matches | desync | silent drift | lost | missed | edge presses lost | speed | F applied | abilities used (guest) |');
console.log('|---|---|---|---|---|---|---|---|---|---|---|');
const tot = { m: 0, d: 0, s: 0, l: 0, x: 0, e: 0, g: 0 };
for (const { cut, prof, r } of rows) {
  const lost = (r.edges || '').split(/\s+(?=[a-z])/).reduce((n, t) => n + (+(t.match(/ (\d+)\//) || [0, 0])[1]), 0);
  console.log(`| ${cut} | ${prof} | ${r.matches} | ${r.desyncs} | ${r.silent_drifts} | ${r.lost} | ${r.missed_checkpoints} | ${lost} | ${r.speed} | ${r.verbs_applied?.ability ?? '-'} | ${r.abilities_used} (${r.guest_abilities_used ?? '-'}) |`);
  tot.m += r.matches; tot.d += r.desyncs; tot.s += r.silent_drifts; tot.l += r.lost; tot.x += r.missed_checkpoints; tot.e += lost; tot.g += r.guest_abilities_used || 0;
}
console.log(`\n${JSON.stringify({ tool: 'thirdjam.soaklock.modes/1', profiles: rows.length, ...tot })}`);
