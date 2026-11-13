// Lane A74: eyes findings of row 3 (puck / carrier not visible), 3b (far net) and 6b (net-distance markers) per still, marker off (before) vs on (after).
// The before stills are the same package run with `-ExecCmds="jam.hud.PuckMarker 0"` (A74_TAG=-off, so the names carry "-off"); the eyes JSON is a74-<preset>-NN.json.
//   node a74score.mjs <worktree>
import { readFileSync, readdirSync } from 'node:fs';
import { join, basename } from 'node:path';
const wt = process.argv[2];
const dir = join(wt, 'docs', 'experiments', 'eyes');
const rows = [];
for (const f of readdirSync(dir).filter((x) => /^a74-(bug-hunt|hud|readability)-(off-)?[0-9]+.json$/.test(x))) {
  const j = JSON.parse(readFileSync(join(dir, f), 'utf8'));
  for (const r of j.results ?? []) { if (!r.ok || !r.answer?.stills) continue;
    r.answer.stills.forEach((a, i) => { const still = basename(r.stills[i]?.path ?? '?');
      const add = (severity, text) => rows.push({ still, preset: r.preset, severity, text });
      if (r.preset === 'bug-hunt') for (const x of a.findings ?? []) add(x.severity, `${x.what} (${x.where})`);
      else if (r.preset === 'readability') { for (const [k, v] of Object.entries(a.elements ?? {})) if (v.score <= 1) add(4, `${k} scored 1: ${v.evidence}`); }
      else if (r.preset === 'hud') { for (const c of a.collisions ?? []) add(3, `collision: ${c}`); }
    }); } }
const imgs = readdirSync(join(wt, 'docs', 'experiments', 'img', 'a74')).filter((f) => f.endsWith('.jpg'));
const def = {
  '3 puck / carrier not visible': /^(puck|carrier) scored 1|(puck|carrier).{0,40}(not (visible|in frame|in view|shown)|out of frame|off.?screen|missing|cannot be seen)/i,
  '3b far net': /^far_net scored 1|far net.{0,30}(not|out of)/i,
  '6b markers on boards / net': /(marker|indicator|diamond|\d+ ?m\b).{0,80}(net|crossbar|boards?|post|figure|player)|(net|crossbar|boards?).{0,40}(marker|indicator)/i,
};
const off = imgs.filter((f) => f.includes('-off-')), on = imgs.filter((f) => !f.includes('-off-'));
const key = (f) => f.replace('-off-', '-');
const pairs = on.filter((f) => off.some((o) => key(o) === f));
const hits = (still, re) => rows.filter((r) => r.still === still && r.severity >= 3 && re.test(r.text));
console.log(`stills: on ${on.length}, off ${off.length}, paired ${pairs.length}`);
for (const [k, re] of Object.entries(def)) {
  const b = pairs.filter((f) => hits(f.replace('.jpg', '.jpg').replace(/^(.*?)-(card|faceoff|farnet|pause|call)\.jpg$/, '$1-off-$2.jpg'), re).length);
  const a = pairs.filter((f) => hits(f, re).length);
  console.log(`${k}: before(off) ${b.length} | after(on) ${a.length}`);
  console.log('   before:', b.join(' ')); console.log('   after: ', a.join(' '));
}
console.log('\nrows of severity >= 3 on the marker-on stills:');
for (const r of rows.filter((r) => !r.still.includes('-off-') && r.severity >= 3)) console.log(`${r.still} [${r.preset} ${r.severity}] ${String(r.text).slice(0, 140)}`);
