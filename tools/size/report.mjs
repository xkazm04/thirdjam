#!/usr/bin/env node
// Size report of a packaged build (lane A13, docs/experiments/SIZE.md).
//
// Usage:
//   node tools/size/report.mjs <staged folder | .zip> [--label <name>] [--save] [--history <dir>] [--against <report.json|latest|none>]
//                              [--top <n>] [--zip-mb <n>] [--json]
//
// What it prints:
//   - the size by top-level folder of the staged build (exe, paks, redist, plugins),
//   - the size by asset class (textures, meshes, materials, audio, anims, blueprints, ui, maps, shaders, engine-other, game-other),
//     read from `UnrealPak -List=<utoc> -csv=<file>` on every .utoc (stored size after pak compression, and raw size),
//   - the largest packages in the containers (default 30),
//   - the delta against the previous report in tools/size/history/ (or --against <file>).
// A .zip is unpacked into a temporary folder with Windows' tar (bsdtar reads zip), analysed, and the folder is deleted.
// --save writes tools/size/history/<stamp>-<label>.json. Exit 0 report made, 2 could not run (no input, no UnrealPak).
// Prints INFO lines and one JSON line last ({"tool":"thirdjam.size/1", ...}).
import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import { spawnSync } from 'node:child_process';
import { fileURLToPath } from 'node:url';

const HERE = path.dirname(fileURLToPath(import.meta.url));
export const HISTORY = path.join(HERE, 'history');
const MB = 1024 * 1024;
const r1 = (b) => Math.round((b / MB) * 10) / 10;

// ---------- classification ----------
// One asset class per package, from the ClassType column when UnrealPak gives it, else from the name prefix.
const CLASS_OF_TYPE = [
  [/^(Texture2D|TextureCube|Texture2DArray|VolumeTexture|TextureRenderTarget2D|TextureCubeArray|SparseVolumeTexture|LightMapTexture2D|ShadowMapTexture2D)$/, 'textures'],
  [/^(StaticMesh|SkeletalMesh|GeometryCollection|PhysicsAsset|Skeleton|DestructibleMesh)$/, 'meshes'],
  [/^(Material|MaterialInstanceConstant|MaterialFunction|MaterialParameterCollection|MaterialFunctionMaterialLayer|MaterialFunctionMaterialLayerBlend|SubsurfaceProfile|SpecularProfile)$/, 'materials'],
  [/^(SoundWave|SoundCue|SoundAttenuation|SoundClass|SoundMix|SoundConcurrency|SoundSubmix|MetaSoundSource|MetaSoundPatch|ReverbEffect|DialogueWave)$/, 'audio'],
  [/^(AnimSequence|AnimMontage|AnimBlueprint|BlendSpace|BlendSpace1D|AnimComposite|PoseAsset|IKRigDefinition|IKRetargeter)$/, 'anims'],
  [/^(NiagaraSystem|NiagaraEmitter|NiagaraScript|NiagaraParameterCollection|ParticleSystem)$/, 'vfx'],
  [/^(Font|FontFace|WidgetBlueprint|SlateBrushAsset|SlateWidgetStyleAsset)$/, 'ui'],
  [/^(World|MapBuildDataRegistry|LevelInstance)$/, 'maps'],
  [/^(Blueprint|BlueprintGeneratedClass|UserDefinedStruct|UserDefinedEnum|DataTable|CurveFloat|CurveTable|DataAsset|PrimaryDataAsset)$/, 'blueprints'],
];
const CLASS_OF_PREFIX = [
  [/^T_/, 'textures'], [/^(SM|SK|SKM|PHYS|SKEL)_/, 'meshes'], [/^(M|MI|MF|MPC)_/, 'materials'], [/^(SFX|S|SW|SC|MS|MUS|A)_/, 'audio'],
  [/^(AS|AM|ABP|BS|ANIM)_/, 'anims'], [/^(NS|NE|P)_/, 'vfx'], [/^(WBP|UI|F|FONT)_/, 'ui'], [/^(BP|DT|DA|CF)_/, 'blueprints'],
];
export function classify(row) {
  const file = String(row.filename || row.package || '');
  const ext = path.extname(file).toLowerCase();
  const chunk = String(row.chunkType || '');
  if (/ushaderbytecode|shadercode|shaderlib/i.test(file + ' ' + chunk)) return 'shaders';
  if (/^\.\.\/\.\.\/\.\.\/Engine\/Plugins\/NNE\//.test(file) || /^NNE_/.test(path.basename(file))) return 'nne-models';
  if (ext === '.umap') return 'maps';
  const t = String(row.classType || '').replace(/^\/Script\/[^.]+\./, '');
  for (const [re, c] of CLASS_OF_TYPE) if (re.test(t)) return c;
  const base = path.basename(String(row.package || file)).replace(/\.[^.]+$/, '');
  for (const [re, c] of CLASS_OF_PREFIX) if (re.test(base)) return c;
  if (/^\/?(Engine|Script)\b|\/Engine\//.test(String(row.package || file)) || /^\.\.\/\.\.\/\.\.\/Engine\//.test(file)) return 'engine-other';
  if (/^\/?(Game|JamAssets)\b|\/(Game|JamAssets)\//.test(String(row.package || file))) return 'game-other';
  if (/ContainerHeader|ScriptObjects|PackageStoreEntry/i.test(chunk)) return 'container-meta';
  return t ? `other:${t}` : 'other';
}

// Where a container entry comes from: Engine/Content/<dir>, Engine/Plugins/<plugin>, the project, a project plugin, or shader code.
export function sourceOf(filename) {
  const f = String(filename).replace(/^(\.\.\/)+/, '');
  if (!f || f.startsWith('<')) return 'shader-code';
  let m = f.match(/^Engine\/Plugins\/(?:[^/]+\/)*?([^/]+)\/Content\//); if (m) return `engine-plugin:${m[1]}`;
  m = f.match(/^Engine\/Content\/([^/]+)\//); if (m) return `engine:${m[1]}`;
  m = f.match(/^[^/]+\/Plugins\/(?:[^/]+\/)*?([^/]+)\/Content\//); if (m) return `project-plugin:${m[1]}`;
  m = f.match(/^[^/]+\/Content\/([^/]+)\//); if (m) return `game:${m[1]}`;
  if (/\.ushaderbytecode$/.test(f)) return 'shader-library';
  return 'other';
}

// ---------- CSV ----------
export function parseCsv(text) {
  const lines = text.split(/\r?\n/).filter((l) => l.trim());
  if (!lines.length) return [];
  const split = (l) => { const out = []; let cur = ''; let q = false; for (const ch of l) { if (ch === '"') q = !q; else if (ch === ',' && !q) { out.push(cur.trim()); cur = ''; } else cur += ch; } out.push(cur.trim()); return out; };
  const head = split(lines[0]).map((h) => h.trim());
  return lines.slice(1).map((l) => { const v = split(l); const o = {}; head.forEach((h, i) => { o[h] = v[i]; }); return o; });
}

// ---------- engine ----------
function engineRoot() {
  if (process.env.UE_ROOT && fs.existsSync(process.env.UE_ROOT)) return process.env.UE_ROOT;
  const dat = 'C:/ProgramData/Epic/UnrealEngineLauncher/LauncherInstalled.dat';
  try { for (const i of JSON.parse(fs.readFileSync(dat, 'utf8')).InstallationList) if (i.AppName === 'UE_5.9' && fs.existsSync(i.InstallLocation)) return i.InstallLocation; } catch { /* fall through */ }
  const d = 'C:/Program Files/Epic Games/UE_5.9';
  return fs.existsSync(d) ? d : null;
}

// Lists one IoStore container. Returns rows {package, filename, chunkType, classType, size, stored}.
export function listContainer(utoc, unrealPak, tmp) {
  const csv = path.join(tmp, path.basename(utoc) + '.csv');
  // -ListContainer is the IoStore form (UE 5.9); plain -List=<utoc> without a pak name is rejected.
  const r = spawnSync(unrealPak, [`-ListContainer=${utoc}`, `-csv=${csv}`], { encoding: 'utf8', maxBuffer: 256 * MB, windowsHide: true });
  if (!fs.existsSync(csv)) return { rows: [], error: `UnrealPak -List exit ${r.status}: ${(r.stdout || '').split(/\r?\n/).filter((l) => /error/i.test(l)).slice(0, 3).join(' | ')}` };
  const rows = parseCsv(fs.readFileSync(csv, 'utf8')).map((o) => ({
    package: o.PackageName || '', filename: o.Filename || '', chunkType: o.ChunkType || '', classType: o.ClassType || '',
    size: Number(o.Size) || 0, stored: Number(o.CompressedSize) || Number(o.Size) || 0, container: path.basename(utoc), source: sourceOf(o.Filename || ''),
  }));
  return { rows };
}

function walkFiles(dir, out = []) {
  for (const e of fs.readdirSync(dir, { withFileTypes: true })) {
    const p = path.join(dir, e.name);
    if (e.isDirectory()) walkFiles(p, out); else out.push({ path: p, size: fs.statSync(p).size });
  }
  return out;
}

// Folder buckets of a staged Windows build, by what they are.
export function folderBucket(rel) {
  const p = rel.replace(/\\/g, '/');
  if (/\/Content\/Paks\//.test(p) || /^[^/]+\/Content\/Paks\//.test(p)) return 'paks';
  if (/^Engine\/Extras\/Redist\//.test(p)) return 'redist';
  if (/\/Plugins\//.test(p)) return 'plugins';
  if (/\/Binaries\/Win64\//.test(p) || /\.(exe|dll)$/i.test(p)) return 'code';
  if (/^Engine\//.test(p)) return 'engine-loose';
  return 'other';
}

export function buildReport(staged, opts = {}) {
  const files = walkFiles(staged).map((f) => ({ rel: path.relative(staged, f.path).split(path.sep).join('/'), size: f.size, abs: f.path }));
  const total = files.reduce((a, f) => a + f.size, 0);
  const folders = {};
  for (const f of files) { const b = folderBucket(f.rel); folders[b] = (folders[b] || 0) + f.size; }
  const bigFiles = [...files].sort((a, b) => b.size - a.size).slice(0, 10).map((f) => ({ path: f.rel, mb: r1(f.size) }));

  const classes = {}; const rawClasses = {}; const sources = {}; let assets = []; const errors = [];
  const utocs = files.filter((f) => f.rel.endsWith('.utoc'));
  const ue = opts.engine || engineRoot();
  const unrealPak = ue ? path.join(ue, 'Engine/Binaries/Win64/UnrealPak.exe') : null;
  if (utocs.length && unrealPak && fs.existsSync(unrealPak)) {
    const tmp = fs.mkdtempSync(path.join(os.tmpdir(), 'tlsize-'));
    try {
      for (const u of utocs) {
        const { rows, error } = listContainer(u.abs, unrealPak, tmp);
        if (error) errors.push(`${u.rel}: ${error}`);
        assets = assets.concat(rows);
      }
    } finally { fs.rmSync(tmp, { recursive: true, force: true }); }
  } else if (utocs.length) errors.push('UnrealPak not found: no per-class breakdown (set UE_ROOT)');
  // Sum chunks per package (export bundle + bulk data + optional segments) so the top list shows packages.
  const byPkg = new Map();
  for (const a of assets) {
    const c = classify(a);
    classes[c] = (classes[c] || 0) + a.stored; rawClasses[c] = (rawClasses[c] || 0) + a.size;
    sources[a.source] = (sources[a.source] || 0) + a.stored;
    const key = a.chunkType === 'ShaderCode' ? '<ShaderCode chunks>' : (a.filename || a.package || a.chunkType).replace(/^(\.\.\/)+/, '').replace(/\.(uasset|umap|ubulk|uexp|uptnl|m\.ubulk)$/, '');
    const cur = byPkg.get(key) || { name: key, class: c, stored: 0, size: 0 };
    cur.stored += a.stored; cur.size += a.size; byPkg.set(key, cur);
  }
  const top = [...byPkg.values()].sort((a, b) => b.stored - a.stored).slice(0, opts.top || 30).map((p) => ({ name: p.name, class: p.class, mb: r1(p.stored), raw_mb: r1(p.size) }));
  const toMb = (o) => Object.fromEntries(Object.entries(o).sort((a, b) => b[1] - a[1]).map(([k, v]) => [k, r1(v)]));
  return {
    staged_mb: r1(total), folders_mb: toMb(folders), classes_mb: toMb(classes), classes_raw_mb: toMb(rawClasses),
    sources_mb: Object.fromEntries(Object.entries(toMb(sources)).filter(([, v]) => v >= 0.1)),
    container_entries: assets.length, largest_files: bigFiles, largest_assets: top, errors,
  };
}

function latestHistory(dir, exclude) {
  if (!fs.existsSync(dir)) return null;
  const f = fs.readdirSync(dir).filter((n) => n.endsWith('.json') && path.join(dir, n) !== exclude).sort();
  return f.length ? path.join(dir, f[f.length - 1]) : null;
}

export function delta(cur, prev) {
  const d = (a = {}, b = {}) => { const keys = new Set([...Object.keys(a), ...Object.keys(b)]); const o = {}; for (const k of keys) { const v = Math.round(((a[k] || 0) - (b[k] || 0)) * 10) / 10; if (v) o[k] = v; } return o; };
  const out = { staged_mb: Math.round((cur.staged_mb - prev.staged_mb) * 10) / 10, folders_mb: d(cur.folders_mb, prev.folders_mb), classes_mb: d(cur.classes_mb, prev.classes_mb) };
  if (cur.zip_mb !== undefined && prev.zip_mb !== undefined) out.zip_mb = Math.round((cur.zip_mb - prev.zip_mb) * 10) / 10;
  return out;
}

function stamp() { const d = new Date(); const p = (n) => String(n).padStart(2, '0'); return `${d.getFullYear()}${p(d.getMonth() + 1)}${p(d.getDate())}-${p(d.getHours())}${p(d.getMinutes())}${p(d.getSeconds())}`; }

function main(argv) {
  const args = argv.slice(2);
  const opt = (k, d) => { const i = args.indexOf(k); return i >= 0 && i + 1 < args.length ? args[i + 1] : d; };
  const input = args.find((a, i) => !a.startsWith('--') && (i === 0 || !['--label', '--history', '--against', '--top', '--zip-mb', '--engine'].includes(args[i - 1])));
  const label = (opt('--label', 'report') || 'report').replace(/[^A-Za-z0-9_.-]/g, '_');
  const histDir = opt('--history', HISTORY);
  const result = { tool: 'thirdjam.size/1', verdict: 'pass', exit: 0, input: input || '', label };
  const finish = (code) => { result.exit = code; result.verdict = ['pass', 'fail', 'could_not_run'][code]; console.log(JSON.stringify(result)); process.exitCode = code; };
  if (!input || !fs.existsSync(input)) { console.log(`FAIL size: no staged folder or zip at '${input || ''}'`); return finish(2); }

  let staged = path.resolve(input); let tmp = null; let zipMb;
  if (/\.zip$/i.test(input)) {
    zipMb = r1(fs.statSync(input).size);
    tmp = fs.mkdtempSync(path.join(os.tmpdir(), 'tlsize-zip-'));
    const r = spawnSync('C:/Windows/System32/tar.exe', ['-xf', path.resolve(input), '-C', tmp], { encoding: 'utf8', windowsHide: true });
    if (r.status !== 0) { fs.rmSync(tmp, { recursive: true, force: true }); console.log(`FAIL size: could not unpack ${input}: ${r.stderr}`); return finish(2); }
    const ents = fs.readdirSync(tmp); staged = ents.length === 1 && fs.statSync(path.join(tmp, ents[0])).isDirectory() ? path.join(tmp, ents[0]) : tmp;
  }
  if (opt('--zip-mb')) zipMb = Number(opt('--zip-mb'));
  let rep;
  try { rep = buildReport(staged, { top: Number(opt('--top', 30)), engine: opt('--engine') }); } finally { if (tmp) fs.rmSync(tmp, { recursive: true, force: true }); }
  if (zipMb !== undefined) rep.zip_mb = zipMb;
  Object.assign(result, rep);

  const line = (k, v, w = 16) => console.log(`INFO size: ${String(k).padEnd(w)} ${String(v.toFixed ? v.toFixed(1) : v).padStart(8)} MB`);
  console.log(`INFO size: staged ${rep.staged_mb} MB${zipMb !== undefined ? `, zip ${zipMb} MB` : ''} (${input})`);
  console.log('INFO size: -- by folder --'); for (const [k, v] of Object.entries(rep.folders_mb)) line(k, v);
  if (rep.container_entries) {
    console.log('INFO size: -- by asset class (stored in the containers after pak compression; raw in brackets) --');
    for (const [k, v] of Object.entries(rep.classes_mb)) console.log(`INFO size: ${k.padEnd(16)} ${v.toFixed(1).padStart(8)} MB  (${(rep.classes_raw_mb[k] || 0).toFixed(1)} raw)`);
    console.log('INFO size: -- by source (stored MB, 0.1 MB and up) --');
    for (const [k, v] of Object.entries(rep.sources_mb)) console.log(`INFO size: ${v.toFixed(1).padStart(8)} MB  ${k}`);
    console.log(`INFO size: -- ${rep.largest_assets.length} largest packages --`);
    for (const a of rep.largest_assets) console.log(`INFO size: ${a.mb.toFixed(1).padStart(8)} MB  ${a.class.padEnd(13)} ${a.name}`);
  }
  for (const e of rep.errors) console.log(`WARN size: ${e}`);

  const against = opt('--against', 'latest');
  const prevPath = against === 'none' ? null : against === 'latest' ? latestHistory(histDir) : against;
  if (prevPath && fs.existsSync(prevPath)) {
    const prev = JSON.parse(fs.readFileSync(prevPath, 'utf8'));
    result.delta = delta(rep, prev); result.against = path.basename(prevPath);
    const fmt = (v) => (v > 0 ? `+${v}` : `${v}`);
    console.log(`INFO size: -- delta against ${path.basename(prevPath)} --`);
    console.log(`INFO size: staged ${fmt(result.delta.staged_mb)} MB${result.delta.zip_mb !== undefined ? `, zip ${fmt(result.delta.zip_mb)} MB` : ''}`);
    for (const [k, v] of Object.entries({ ...result.delta.folders_mb })) console.log(`INFO size:   folder ${k.padEnd(14)} ${fmt(v)} MB`);
    for (const [k, v] of Object.entries({ ...result.delta.classes_mb })) console.log(`INFO size:   class  ${k.padEnd(14)} ${fmt(v)} MB`);
  }
  if (args.includes('--save')) {
    fs.mkdirSync(histDir, { recursive: true });
    const out = path.join(histDir, `${stamp()}-${label}.json`);
    const keep = { ...rep, label, input: path.resolve(input), at: new Date().toISOString() };
    fs.writeFileSync(out, JSON.stringify(keep, null, 1) + '\n');
    result.saved = out;
    console.log(`PASS size: report saved to ${out}`);
  } else console.log('PASS size: report made');
  return finish(0);
}

if (process.argv[1] && path.resolve(process.argv[1]) === fileURLToPath(import.meta.url)) main(process.argv);
