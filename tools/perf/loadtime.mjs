// Lane A58: boot and menu -> match load times from a run's game.log (UTC stamps) and run.json (process start, local time).
//   boot_s        process start -> "Load map complete /Game/Maps/Menu"
//   loadmap_s     the engine's own "Took X seconds to LoadMap(/Game/Maps/Rink)"
//   rig_s         "JAMMENU start match" -> "JAMLOOK rig <tier> loaded" (the tier's light rig level streamed in and lit)
//   first_drop_s  "JAMMENU start match" -> the first faceoff drop (frame 90 = 1.5 s of sim after the build, so minus 1.5 s is the load)
//   pso_misses    "Encountered a new ... PSO" lines after the menu's Enter (runtime pipeline compiles: the shader-side cost of the load)
//   sync_loads    LogStreaming SkipPackage lines (missing packages looked up synchronously) after the Enter
// Usage: node tools/perf/loadtime.mjs Game/Packaged/perf-out/<run> [...]
import fs from 'node:fs'; import path from 'node:path';
const stamp = l => { const m = l.match(/^\[(\d{4})\.(\d\d)\.(\d\d)-(\d\d)\.(\d\d)\.(\d\d):(\d{3})\]/); return m ? Date.UTC(+m[1], +m[2] - 1, +m[3], +m[4], +m[5], +m[6], +m[7]) / 1000 : null; };
const out = [];
for (const dir of process.argv.slice(2).filter(a => !a.startsWith('--'))) {
  const log = fs.readFileSync(path.join(dir, 'game.log'), 'utf8').split(/\r?\n/);
  const run = JSON.parse(fs.readFileSync(path.join(dir, 'run.json'), 'utf8'));
  const t0 = new Date(run.start).getTime() / 1000;
  const find = re => log.find(l => re.test(l));
  const menuDone = stamp(find(/Load map complete \/Game\/Maps\/Menu/) || '');
  const enterL = log.findIndex(l => /JAMMENU start match/.test(l)); const enter = stamp(log[enterL] || '');
  const lm = (find(/Took ([\d.]+) seconds to LoadMap\(\/Game\/Maps\/Rink\)/) || '').match(/Took ([\d.]+) seconds/);
  const rig = stamp(find(/JAMLOOK rig \S+ loaded:/) || '');
  const drop = stamp(find(/JAMEVENT faceoff .* kind=drop/) || '');
  const after = log.slice(enterL);
  const r = { run: path.basename(dir), tier: run.tier, boot_s: +(menuDone - t0).toFixed(2), loadmap_s: lm ? +(+lm[1]).toFixed(3) : null,
    rig_s: rig ? +(rig - enter).toFixed(3) : null, first_drop_s: drop ? +(drop - enter).toFixed(2) : null,
    pso_misses: after.filter(l => /Encountered a new (graphics|compute) PSO/.test(l)).length,
    missing_packages: [...new Set(after.filter(l => /SkipPackage:/.test(l)).map(l => (l.match(/SkipPackage: (\S+)/) || [])[1]))] };
  out.push(r);
  console.log(`${r.run.padEnd(14)} boot ${r.boot_s}s  LoadMap(Rink) ${r.loadmap_s}s  rig lit ${r.rig_s}s  first drop ${r.first_drop_s}s (1.5 s of it is the faceoff wait)  PSO misses ${r.pso_misses}  missing ${r.missing_packages.join(' ')}`);
}
if (process.argv.includes('--json')) fs.writeFileSync('loadtime.json', JSON.stringify(out, null, 1));
