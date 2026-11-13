// Lane A58: GPU passes per run, ms per frame, from the Insights timer statistics over the CSV window (insights/gpu-match.csv)
// and the frame count of the same window (csv-summary.json). Inclusive times; Lumen runs partly on async compute, so groups overlap.
// Usage: node tools/perf/gpu-passes.mjs Game/Packaged/perf-out/<run> [...]
import fs from 'node:fs'; import path from 'node:path';
const groups = [
  ['Lumen GI (DiffuseIndirectAndAO + LumenSceneLighting + LumenSceneUpdate:)', ['DiffuseIndirectAndAO', 'LumenSceneLighting%s', 'LumenSceneUpdate:']],
  ['of which screen probe gather', ['LumenScreenProbeGather']],
  ['Post (PostProcessing)', ['PostProcessing']],
  ['of which TSR', ['TemporalSuperResolution(sg.AntiAliasingQuality=%d%s) %dx%d -> %dx%d']],
  ['Translucency + volume lighting', ['Translucency', 'TranslucencyVolumeLighting']],
  ['Niagara (FXSystemPreRender)', ['FXSystemPreRender']],
  ['Distance fields', ['DistanceFields']],
  ['Direct lighting', ['DirectLighting']],
  ['Shadow depths', ['ShadowDepths']],
  ['VRS prep (CAS)', ['PrepareImageBasedVRS']],
  ['Base pass', ['BasePass']],
];
for (const dir of process.argv.slice(2)) {
  const frames = JSON.parse(fs.readFileSync(path.join(dir, 'csv-summary.json'), 'utf8')).frames;
  const rows = fs.readFileSync(path.join(dir, 'insights', 'gpu-match.csv'), 'utf8').split(/\r?\n/).slice(1).filter(Boolean).map(l => {
    const m = l.match(/^("[^"]*"|.*?),(\d+),[^,]*,([\d.]+),/); return m ? { n: m[1].replace(/"/g, ''), c: +m[2], i: +m[3] } : null; }).filter(Boolean);
  // A timer can appear on both queues and at several depths with the same name: take the largest inclusive entry per name.
  const best = {}; for (const r of rows) if (!best[r.n] || r.i > best[r.n]) best[r.n] = r.i;
  const csv = JSON.parse(fs.readFileSync(path.join(dir, 'csv-summary.json'), 'utf8'));
  console.log(`== ${path.basename(dir)} (${frames} frames, GPUTime mean ${csv.unit.GPUTime.mean} ms)`);
  for (const [label, names] of groups) {
    const ms = names.reduce((a, n) => a + (best[n] || 0), 0) / frames * 1000;
    console.log(`  ${ms.toFixed(2).padStart(6)}  ${label}`);
  }
}
