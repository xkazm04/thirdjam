#!/usr/bin/env node
// Lane A55: per-translation-unit compile time measured one TU at a time (no parallel build around it), with MSVC /Bt+ /d1reportTime.
// Why: a UBT `-Timing` build runs 16 compiles at once next to other lanes' builds; its per-TU numbers came out 5-8x the isolated cost
// (DifficultyProfile.cpp: 11.8 s in the -Timing build, 1.6 s alone; <cmath> 4.9 s there, 0.03 s alone), so they rank contention, not cost.
// Reuses the <TU>.obj.rsp files a plain MSVC build left in Game/Intermediate; writes objects and logs under Game/Intermediate/A55TuTimes only.
// Usage: node tools/analysis/tu-times.mjs [--top 20] [--only JamCore]   Prints TU and HDR rows, then a JSON line last.
import fs from 'node:fs';
import path from 'node:path';
import { spawnSync } from 'node:child_process';
import { fileURLToPath } from 'node:url';

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..', '..');
const arg = (k, d) => { const i = process.argv.indexOf(k); return i > 0 ? process.argv[i + 1] : d; };
const TOP = Number(arg('--top', 20));
const only = arg('--only', '');
const ue = process.env.UE_ROOT || 'C:/Program Files/Epic Games/UE_5.9';
const inter = path.join(root, 'Game', 'Intermediate', 'Build', 'Win64', 'x64', 'UnrealEditor', 'Development');
const out = path.join(root, 'Game', 'Intermediate', 'A55TuTimes');
fs.mkdirSync(out, { recursive: true });
const fwd = (p) => p.split(path.sep).join('/');

// the cl.exe UBT used: from the shared response file's MSVC include path (…/MSVC/<ver>/INCLUDE -> …/MSVC/<ver>/bin/Hostx64/x64/cl.exe)
let cl = null;
const rows = [];
const hdr = new Map();
for (const mod of fs.readdirSync(inter)) {
  if (only && mod !== only) continue;
  const dir = path.join(inter, mod);
  const shared = path.join(dir, `${mod}.Shared.rsp`);
  if (!fs.existsSync(shared)) continue;
  const s = fs.readFileSync(shared, 'utf8');
  if (s.includes('--analyze') || s.includes('/analyze')) { console.log(`FAIL tu-times ${mod}.Shared.rsp is from an analyzer run: build plainly first`); process.exit(2); }
  if (!cl) { const m = s.match(/\/external:I "([^"]+\/MSVC\/[^/"]+)\/INCLUDE"/i); if (m) cl = `${m[1]}/bin/Hostx64/x64/cl.exe`; }
  for (const f of fs.readdirSync(dir).filter((x) => x.endsWith('.cpp.obj.rsp')).sort()) {
    const tu = f.replace('.obj.rsp', '');
    const lines = fs.readFileSync(path.join(dir, f), 'utf8').split(/\r?\n/).filter((l) => l && !/^\/(Fo|experimental:log|sourceDependencies)/.test(l));
    lines.push(`/Fo"${fwd(path.join(out, `${tu}.obj`))}"`, '/Bt+', '/d1reportTime');
    const rsp = path.join(out, `${tu}.rsp`);
    fs.writeFileSync(rsp, lines.join('\n'));
    const t0 = process.hrtime.bigint();
    const r = spawnSync(cl, [`@${rsp}`], { cwd: path.join(ue, 'Engine', 'Source'), encoding: 'utf8', maxBuffer: 256 * 1024 * 1024 });
    const wall = Number(process.hrtime.bigint() - t0) / 1e9;
    const txt = (r.stdout || '') + (r.stderr || '');
    const fe = Number((txt.match(/c1xx\.dll\)=([\d.]+)s/) || [0, 0])[1]);
    const be = Number((txt.match(/c2\.dll\)=([\d.]+)s/) || [0, 0])[1]);
    rows.push({ module: mod, tu, wall, fe, be, exit: r.status });
    const sec = txt.split('Include Headers:')[1];
    if (sec) {
      for (const line of sec.split(/\n\s*\n/)[0].split(/\r?\n/)) {
        const m = line.match(/^(\t+)(.+?):\s*([\d.]+)s/);
        if (!m) continue;
        const raw = fwd(m[2].trim());
        const i = raw.toLowerCase().indexOf('/game/source/'); const e = raw.toLowerCase().indexOf('/engine/');
        const k = i >= 0 ? raw.slice(i + 1) : e >= 0 ? 'Engine' + raw.slice(e + 7) : raw;
        const c = hdr.get(k) || { seconds: 0, opens: 0 }; c.seconds += Number(m[3]); c.opens++; hdr.set(k, c);
      }
    }
    fs.writeFileSync(path.join(out, `${tu}.timing.txt`), txt);
  }
}
rows.sort((a, b) => b.wall - a.wall);
const failed = rows.filter((r) => r.exit !== 0);
const byModule = {};
for (const r of rows) byModule[r.module] = Math.round(((byModule[r.module] || 0) + r.wall) * 10) / 10;
for (const r of rows.slice(0, TOP)) console.log(`TU ${r.wall.toFixed(2)}s fe ${r.fe.toFixed(2)} be ${r.be.toFixed(2)} ${r.module}/${r.tu}${r.exit ? ` (exit ${r.exit})` : ''}`);
for (const [k, v] of [...hdr.entries()].sort((a, b) => b[1].seconds - a[1].seconds).slice(0, TOP)) console.log(`HDR ${v.seconds.toFixed(2)}s ${v.opens} opens ${k}`);
for (const [m, s] of Object.entries(byModule)) console.log(`MODULE ${s}s ${m}`);
console.log(`${failed.length ? 'FAIL' : 'PASS'} tu-times ${rows.length} TUs, ${failed.length} failed, ${rows.reduce((s, r) => s + r.wall, 0).toFixed(1)}s serial`);
console.log(JSON.stringify({ schema: 'a55.tu-times/1', tus: rows.length, failed: failed.length, byModule }));
process.exit(failed.length ? 1 : 0);
