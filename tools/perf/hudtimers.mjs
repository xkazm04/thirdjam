// Lane A106: the HUD's and the brain's game-thread timers per frame from an Insights timer export (insights-export.ps1 ... ExportTimerStatistics).
// Usage: node tools/perf/hudtimers.mjs <gt.csv> [...]
import fs from 'node:fs';
for (const f of process.argv.slice(2)) {
  const L = fs.readFileSync(f, 'utf8').trim().split(/\r?\n/).slice(1).map(l => l.split(','));
  const fr = +(L.find(r => r[0] === 'FEngineLoop::Tick') || [0, 1])[1];
  console.log(`${f}: ${fr} frames`);
  L.filter(r => /^JamHud_|^JamHudWidget|^JamBrainThink|^HokejGameMode|^ExecuteUbergraph_BP_SkaterVisual|^Total Slate Tick Time|^Slate::DrawWindows/.test(r[0]))
    .map(r => [r[0].slice(0, 40), +r[3] / fr * 1000, +r[8] / fr * 1000, +r[1] / fr]).sort((a, b) => b[1] - a[1])
    .forEach(([n, i, e, c]) => console.log(`  ${n.padEnd(40)} incl ${i.toFixed(4)}  excl ${e.toFixed(4)}  calls/f ${c.toFixed(1)}`));
}
