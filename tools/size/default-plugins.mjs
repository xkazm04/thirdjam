#!/usr/bin/env node
// Lists the engine plugins that are enabled by default and would reach a Win64 Game (Shipping) target:
// runtime-type modules allowed on Win64 and the Game target, and whether the plugin carries Content.
// Usage: node tools/size/default-plugins.mjs [--engine <UE root>] [--project <.uproject>] [--json]
// Prints one line per plugin: name, runtime module count, content (y/n), state in the .uproject (on/off/default), path.
import fs from 'node:fs';
import path from 'node:path';

const args = process.argv.slice(2);
const opt = (k, d) => { const i = args.indexOf(k); return i >= 0 ? args[i + 1] : d; };
const engine = opt('--engine', process.env.UE_ROOT || 'C:/Program Files/Epic Games/UE_5.9');
const project = opt('--project', path.join(path.dirname(new URL(import.meta.url).pathname.replace(/^\/([A-Za-z]:)/, '$1')), '../../Game/ThirdJam.uproject'));
const asJson = args.includes('--json');

const RUNTIME = new Set(['Runtime', 'RuntimeNoCommandlet', 'RuntimeAndProgram', 'ClientOnly', 'ClientOnlyNoCommandlet', 'CookedOnly', 'ServerOnly']);
const proj = JSON.parse(fs.readFileSync(project, 'utf8'));
const projState = new Map((proj.Plugins || []).map((p) => [p.Name, p.Enabled ? 'on' : 'off']));

function* walk(dir) {
  let ents = [];
  try { ents = fs.readdirSync(dir, { withFileTypes: true }); } catch { return; }
  for (const e of ents) {
    const p = path.join(dir, e.name);
    if (e.isDirectory()) { if (!['Source', 'Content', 'Binaries', 'Intermediate', 'Resources', 'Shaders'].includes(e.name)) yield* walk(p); }
    else if (e.name.endsWith('.uplugin')) yield p;
  }
}
const rows = [];
for (const f of walk(path.join(engine, 'Engine/Plugins'))) {
  let d; try { d = JSON.parse(fs.readFileSync(f, 'utf8').replace(/^\uFEFF/, '')); } catch { continue; }
  const name = path.basename(f, '.uplugin');
  const state = projState.get(name) || 'default';
  const enabled = state === 'on' || (state === 'default' && d.EnabledByDefault === true);
  if (!enabled) continue;
  const mods = (d.Modules || []).filter((m) => RUNTIME.has(m.Type)
    && (!m.PlatformAllowList && !m.WhitelistPlatforms || (m.PlatformAllowList || m.WhitelistPlatforms).includes('Win64'))
    && (!m.TargetAllowList || m.TargetAllowList.includes('Game'))
    && !(m.TargetDenyList || []).includes('Game'));
  const hasContent = fs.existsSync(path.join(path.dirname(f), 'Content'));
  const editorOnly = mods.length === 0;
  rows.push({ name, runtime_modules: mods.length, content: hasContent || d.CanContainContent === true, editor_only: editorOnly, state, path: path.relative(engine, f).split(path.sep).join('/') });
}
rows.sort((a, b) => (a.editor_only - b.editor_only) || a.name.localeCompare(b.name));
if (asJson) { console.log(JSON.stringify(rows, null, 1)); process.exit(0); }
for (const r of rows) console.log(`${r.editor_only ? 'editor ' : 'RUNTIME'} ${r.name.padEnd(36)} mods=${String(r.runtime_modules).padStart(2)} content=${r.content ? 'y' : 'n'} ${r.state.padEnd(7)} ${r.path}`);
console.log(`INFO ${rows.filter((r) => !r.editor_only).length} runtime plugin(s), ${rows.filter((r) => r.editor_only).length} editor-only, enabled by default or by the project`);
