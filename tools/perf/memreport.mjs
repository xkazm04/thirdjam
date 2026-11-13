// Lane A58: read one or more memreport -full files (in order) and print
//   - per report: process physical used/peak, UObject count and resource size, RHI total, the ten largest RHI resources
//   - the top 20 assets by size (textures from ListTextures InMem KB, meshes / sounds / Niagara from obj list ResExcKB), last report
//   - textures over budget (uncompressed or BGRA8 over 1 MB, UI textures without mips over 1 MB, any texture over 4 MB)
//   - the deltas between consecutive reports (the leak view across jam.livecut rebuilds): objects per class that grew
// Usage: node tools/perf/memreport.mjs a.memreport b.memreport ... [--json out.json]
import fs from 'node:fs';
const args = process.argv.slice(2);
const jsonOut = args.includes('--json') ? args[args.indexOf('--json') + 1] : null;
const files = args.filter((a, i) => !a.startsWith('--') && args[i - 1] !== '--json');

function sections(txt) {
  const out = {}; const re = /MemReport: Begin command "([^"]+)"\r?\n([\s\S]*?)(?=MemReport: (?:End|Begin) command)/g; let m;
  while ((m = re.exec(txt))) if (!(m[1] in out)) out[m[1]] = m[2];
  return out;
}
function parse(file) {
  const txt = fs.readFileSync(file, 'utf8'); const s = sections(txt); const r = { file: file.split(/[\\/]/).pop() };
  const mem = s['Mem FromReport'] || '';
  let m = mem.match(/Process Physical Memory: ([\d.]+) MB used, ([\d.]+) MB peak/); if (m) { r.physUsedMB = +m[1]; r.physPeakMB = +m[2]; }
  m = mem.match(/Process Virtual Memory: ([\d.]+) MB used/); if (m) r.virtUsedMB = +m[1];
  const ol = s['obj list -resourcesizesort'] || '';
  m = ol.match(/(\d+) Objects \(Total: ([\d.]+)M .*?Res: ([\d.]+)M/); if (m) { r.objects = +m[1]; r.objTotalMB = +m[2]; r.objResMB = +m[3]; }
  r.classes = {};
  for (const l of ol.split(/\r?\n/)) { const c = l.trim().split(/\s+/); if (c.length === 8 && /^\d+$/.test(c[1])) r.classes[c[0]] = { count: +c[1], numKB: +c[2], resKB: +c[4] }; }
  const rd = s['rhi.DumpMemory'] || ''; m = rd.match(/([\d.]+)MB total/); if (m) r.rhiTotalMB = +m[1];
  r.rhiLines = [...rd.matchAll(/^\s*([\d.]+)MB\s+-\s+([^-]+?) - STAT_/gm)].map(x => ({ mb: +x[1], name: x[2].trim() })).filter(x => x.mb > 0).sort((a, b) => b.mb - a.mb);
  const rr = s['rhi.DumpResourceMemory'] || '';
  r.rhiTop = [...rr.matchAll(/Name: (.+?) - Type: (\w+) - Size: ([\d.]+) MB/g)].map(x => ({ name: x[1], type: x[2], mb: +(+x[3]).toFixed(2) })).slice(0, 25);
  const tex = (s['ListTextures'] || '').split(/\r?\n/).filter(l => /^\d+x\d+ \(/.test(l)).map(l => {
    // "<cooked WxH> (<KB> KB, <bias>), <inmem WxH> (<KB> KB), <format>, <group>, <name>, <streaming>, <unknownref>, <vt>, <usage>, <mips>, <uncompressed>"
    const m = l.match(/^\d+x\d+ \(\d+ KB, [^)]*\), (\d+)x(\d+) \((\d+) KB\), (\S+), (\S+), (\S+), (\S+), (\S+), (\S+), (\S+), (\d+), (\S+)/);
    if (!m) return null;
    return { asset: m[6], kind: 'Texture', kb: +m[3], dims: `${m[1]}x${m[2]}`, format: m[4], group: m[5], streaming: m[7], usage: +m[10], mips: +m[11], uncompressed: m[12] === 'YES' }; }).filter(Boolean);
  r.textures = tex;
  const objs = [];
  for (const cls of ['SkeletalMesh', 'StaticMesh', 'SoundWave', 'NiagaraSystem', 'AnimSequence']) {
    const sec = s[`obj list class=${cls} -resourcesizesort`] || s[`obj list class=${cls} -alphasort`] || '';
    for (const l of sec.split(/\r?\n/)) { const c = l.trim().split(/\s+/); if (c.length === 8 && c[0] === cls && c[1].startsWith('/')) objs.push({ asset: c[1].split('.')[0], kind: cls, kb: +c[4] + +c[2] }); }
  }
  r.assets = [...tex.map(t => ({ asset: t.asset.split('.')[0], kind: 'Texture', kb: t.kb, note: `${t.dims} ${t.format} ${t.group}` })), ...objs].sort((a, b) => b.kb - a.kb);
  r.overBudget = tex.filter(t => (t.uncompressed && t.kb > 1024) || (t.format === 'PF_B8G8R8A8' && t.kb > 1024) || t.kb > 4096 || (t.group === 'TEXTUREGROUP_UI' && t.kb > 1024));
  r.actors = ((s['ListSpawnedActors'] || '').match(/Total: (\d+)/) || [])[1];
  return r;
}
const reps = files.map(parse);
for (const r of reps) {
  console.log(`== ${r.file}: phys ${r.physUsedMB} MB (peak ${r.physPeakMB}), objects ${r.objects} (${r.objTotalMB} MB, res ${r.objResMB} MB), RHI ${r.rhiTotalMB} MB`);
}
const last = reps[reps.length - 1];
if (last) {
  console.log('\nRHI memory by kind (last):'); for (const x of last.rhiLines.slice(0, 10)) console.log(`  ${String(x.mb).padStart(9)} MB  ${x.name}`);
  console.log('\nLargest RHI resources (last):'); for (const x of last.rhiTop.slice(0, 15)) console.log(`  ${String(x.mb).padStart(9)} MB  ${x.name}`);
  console.log('\nTop 20 assets (last):'); for (const a of last.assets.slice(0, 20)) console.log(`  ${String(Math.round(a.kb)).padStart(7)} KB  ${a.kind.padEnd(13)} ${a.asset} ${a.note || ''}`);
  console.log('\nTextures over budget (last):'); for (const t of last.overBudget) console.log(`  ${t.kb} KB ${t.dims} ${t.format} ${t.group} mips=${t.mips} ${t.asset}`);
}
for (let i = 1; i < reps.length; i++) {
  const a = reps[i - 1], b = reps[i];
  const grew = Object.entries(b.classes).map(([k, v]) => ({ k, d: v.count - (a.classes[k]?.count || 0), kb: v.numKB + v.resKB - ((a.classes[k]?.numKB || 0) + (a.classes[k]?.resKB || 0)) })).filter(x => x.d !== 0).sort((x, y) => y.kb - x.kb);
  console.log(`\ndelta ${a.file} -> ${b.file}: phys ${(b.physUsedMB - a.physUsedMB).toFixed(1)} MB, objects ${b.objects - a.objects}, RHI ${(b.rhiTotalMB - a.rhiTotalMB).toFixed(1)} MB`);
  for (const x of grew.slice(0, 8)) console.log(`  ${x.k}: ${x.d > 0 ? '+' : ''}${x.d} objects, ${x.kb >= 0 ? '+' : ''}${x.kb.toFixed(0)} KB`);
}
if (jsonOut) fs.writeFileSync(jsonOut, JSON.stringify(reps.map(r => ({ ...r, textures: undefined })), null, 1));
