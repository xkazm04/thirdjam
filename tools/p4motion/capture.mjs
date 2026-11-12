#!/usr/bin/env node
// tools/p4motion/capture.mjs: the proof capture of lane A29. For every clip: 2 seconds of the Manny playing it, 8 frames scrubbed by sequence time
// (never wall clock), one SceneCapture2D, composed into a strip PNG under devlog/shots/ with a caption .txt next to it.
//   node tools/p4motion/capture.mjs --clips AS_P4_skate_fwd,AS_P4_knocked_down_back [--url http://127.0.0.1:8016/mcp] [--seconds 2] [--frames 8] [--yaw 35] [--label a29]
// Steps: 1 make sure the proof level /Game/P4/Anim/Proof/L_P4_Proof exists and is open (a floor, a sun, a sky light); 2 StartPIE in Simulate;
// 3 run ue_capture.py; 4 StopPIE; 5 strips through tools/meshy/strip.py (PIL). Exit 0 ok, 1 a clip failed, 2 could not run.
import { spawnSync } from 'node:child_process';
import { mkdirSync, writeFileSync, readFileSync, readdirSync, copyFileSync, rmSync } from 'node:fs';
import { resolve, dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';
import { tmpdir } from 'node:os';

const here = dirname(fileURLToPath(import.meta.url));
const repo = resolve(here, '..', '..');
const o = {};
for (let i = 2; i < process.argv.length; i += 2) o[process.argv[i].replace(/^--/, '')] = process.argv[i + 1];
if (!o.clips) { console.error('usage: capture.mjs --clips a,b [--url] [--seconds 2] [--frames 8] [--yaw 35] [--label a29]'); process.exit(2); }
const url = o.url || process.env.UE_MCP_URL || 'http://127.0.0.1:8016/mcp';
const scratch = process.env.P4M_SCRATCH || resolve(tmpdir(), 'p4motion');
mkdirSync(scratch, { recursive: true });
const clips = o.clips.split(',').map((c) => (c.startsWith('/') ? c : `/Game/P4/Anim/${c}`));
const label = o.label || 'a29';
const now = new Date(), p2 = (n) => String(n).padStart(2, '0');
const stamp = `${now.getFullYear()}${p2(now.getMonth() + 1)}${p2(now.getDate())}-${p2(now.getHours())}${p2(now.getMinutes())}${p2(now.getSeconds())}`; // local time, as devlog/shots names it

function mcp(tool, payload, timeout = 600) {
  const f = resolve(scratch, `call-${Date.now()}.json`);
  writeFileSync(f, JSON.stringify(payload));
  const r = spawnSync(process.execPath, [resolve(repo, 'tools/mcp/mcp.mjs'), 'call', tool, '@' + f, '--url', url, '--timeout', String(timeout)], { encoding: 'utf8', maxBuffer: 1 << 26 });
  const text = (r.stdout || '') + (r.stderr || '');
  try {
    const lines = text.trimEnd().split('\n');
    const verdict = JSON.parse(lines.pop());
    const body = JSON.parse(lines.join('\n'));
    return { ok: verdict.verdict === 'pass', verdict, body, text: body.content && body.content[0] && body.content[0].text };
  } catch (e) { return { ok: false, error: text.slice(-800) }; }
}
function py(content, timeout = 600) {
  const r = mcp('editor_toolset.toolsets.python_scripting.PythonScriptingTools.execute_python', { script: { language: 'Python', content } }, timeout);
  if (!r.ok) return { ok: false, error: r.error || (r.verdict && r.verdict.error) || r.text };
  try { const inner = JSON.parse(r.text); return { ok: true, value: inner.returnValue }; } catch (e) { return { ok: true, value: r.text }; }
}
function pyFile(script, args, timeout = 600) {
  const argsFile = resolve(scratch, `args-${Date.now()}.json`).replace(/\\/g, '/');
  writeFileSync(argsFile, JSON.stringify(args));
  const s = script.replace(/\\/g, '/');
  const wrapper = `import os, io, contextlib\nos.environ['P4M_ARGS'] = ${JSON.stringify(argsFile)}\nbuf = io.StringIO()\nwith contextlib.redirect_stdout(buf):\n    exec(compile(open(${JSON.stringify(s)}).read(), ${JSON.stringify(s)}, 'exec'), {})\n[l for l in buf.getvalue().splitlines() if l.startswith('P4M_RESULT')][-1][11:]`;
  const r = py(wrapper, timeout);
  if (!r.ok) return r;
  try { return { ok: true, value: JSON.parse(r.value) }; } catch (e) { return { ok: false, error: 'unparsable: ' + String(r.value).slice(0, 400) }; }
}

// 1. proof level
const LEVEL = '/Game/P4/Anim/Proof/L_P4_Proof';
const levelPy = `
import unreal, json
lib = unreal.EditorAssetLibrary
les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
made = False
if not lib.does_asset_exist('${LEVEL}'):
    les.new_level('${LEVEL}')
    made = True
    floor = eas.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(0, 0, 0))
    floor.set_actor_label('P4_Floor')
    floor.static_mesh_component.set_static_mesh(unreal.load_asset('/Engine/BasicShapes/Plane'))
    floor.set_actor_scale3d(unreal.Vector(40, 40, 1))
    floor.static_mesh_component.set_material(0, unreal.load_asset('/Engine/EngineMaterials/WorldGridMaterial'))
    sun = eas.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 500), unreal.Rotator(-50, 30, 0))
    sun.set_actor_label('P4_Sun')
    sun.light_component.set_intensity(6.0)
    sky = eas.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 400))
    sky.set_actor_label('P4_Sky')
    sky.light_component.set_intensity(1.5)
    les.save_current_level()
else:
    cur = les.get_current_level()
    if cur is None or not str(cur.get_outer().get_path_name()).startswith('${LEVEL}'):
        les.load_level('${LEVEL}')
# the two persistent capture actors (Python cannot spawn into the simulate world)
labels = {a.get_actor_label(): a for a in eas.get_all_level_actors()}
added = []
if 'P4_Figure' not in labels:
    fig = eas.spawn_actor_from_class(unreal.SkeletalMeshActor, unreal.Vector(0, 0, 0))
    fig.set_actor_label('P4_Figure')
    fig.skeletal_mesh_component.set_skeletal_mesh_asset(unreal.load_asset('${o.mesh || '/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple'}'))
    added.append('P4_Figure')
if 'P4_Cam' not in labels:
    camA = eas.spawn_actor_from_class(unreal.SceneCapture2D, unreal.Vector(-400, -250, 150))
    camA.set_actor_label('P4_Cam')
    camA.capture_component2d.set_editor_property('capture_every_frame', False)
    camA.capture_component2d.set_editor_property('capture_on_movement', False)
    added.append('P4_Cam')
for a in eas.get_all_level_actors():
    if a.get_actor_label() == 'P4_Sky' and a.light_component.get_editor_property('real_time_capture'):
        a.light_component.set_editor_property('real_time_capture', False)   # no SkyAtmosphere in the proof level: real-time capture prints a red warning across the frame
        added.append('sky capture off')
    if a.get_actor_label() == 'P4_Floor' and a.static_mesh_component.get_material(0).get_name() != 'M_P4_ProofFloor':
        # an unlit mid-grey floor so the strips read (pass 3): made once with the material editing library
        mp = '/Game/P4/Anim/Proof/M_P4_ProofFloor'
        if not lib.does_asset_exist(mp):
            at = unreal.AssetToolsHelpers.get_asset_tools()
            m = at.create_asset('M_P4_ProofFloor', '/Game/P4/Anim/Proof', unreal.Material, unreal.MaterialFactoryNew())
            m.set_editor_property('shading_model', unreal.MaterialShadingModel.MSM_UNLIT)
            mel = unreal.MaterialEditingLibrary
            c = mel.create_material_expression(m, unreal.MaterialExpressionConstant3Vector, -300, 0)
            c.set_editor_property('constant', unreal.LinearColor(0.35, 0.36, 0.38, 1.0))
            mel.connect_material_property(c, '', unreal.MaterialProperty.MP_EMISSIVE_COLOR)
            mel.recompile_material(m)
            lib.save_asset(mp, only_if_is_dirty=False)
        a.static_mesh_component.set_material(0, lib.load_asset(mp))
        added.append('floor material')
if added:
    les.save_current_level()   # never leave the level dirty: a force-stopped editor then shows a modal "Restore Packages" dialog on the next start, which blocks every MCP tool
json.dumps({'made': made, 'added': added, 'level': str(les.get_current_level().get_outer().get_path_name())})
`;
const lv = py(levelPy, 300);
console.log(`${lv.ok ? 'PASS' : 'FAIL'} level ${lv.ok ? lv.value : lv.error}`);
if (!lv.ok) { process.exitCode = 2; }
else {
  // 2. simulate
  const pie = mcp('EditorToolset.EditorAppToolset.StartPIE', { options: { bSimulate: true } }, 120);
  console.log(`${pie.ok ? 'PASS' : 'FAIL'} simulate ${pie.ok ? 'started' : pie.error || JSON.stringify(pie.verdict)}`);
  spawnSync(process.execPath, ['-e', 'setTimeout(()=>{}, 2500)']);
  // 3. capture: the editor viewport of the simulate world (a SceneCapture2D driven from Python came back black; the viewport capture is proven).
  // The figure in the simulate world is found by label, the clip is set in SINGLE_NODE mode at play rate 0, and every frame is a scrub of the
  // sequence position (k * seconds / N, wrapped on the clip length) followed by one viewport capture.
  const outDir = resolve(scratch, 'frames').replace(/\\/g, '/');
  const N = Number(o.frames || 8), SEC = Number(o.seconds || 2);
  const dist = Number(o.dist || 420), yaw = Number(o.yaw || 35), camH = Number(o.height || 150);
  const rad = (yaw * Math.PI) / 180;
  const camPos = { x: -dist * Math.cos(rad), y: -dist * Math.sin(rad), z: camH };
  const pitch = -(Math.atan2(camH - 90, dist) * 180) / Math.PI;
  mcp('EditorToolset.EditorAppToolset.SetCameraTransform', { translation: camPos, rotation: { pitch, yaw, roll: 0 } }, 30);
  const cap = { ok: true, value: { clips: {} } };
  // Measured 17:20-17:34 on UE 5.9: (1) an execute_python that contains `await asyncio.sleep` leaves every later viewport capture black; (2) `set_animation` +
  // `play` + `set_play_rate(0)` + `set_position` spread over separate calls also blackens; (3) `override_animation_data(seq, looping, playing=False, t, 0)` in its
  // own call per frame renders. So every frame is one short synchronous call with override_animation_data, and the MCP round trip is the editor tick.
  for (const path of clips) {
    const name = path.split('/').pop();
    const d = join(outDir, name); rmSync(d, { recursive: true, force: true }); mkdirSync(d, { recursive: true });
    const len = py(`import unreal, json
seq = unreal.load_asset(${JSON.stringify(path)})
json.dumps({'length_s': round(unreal.AnimationLibrary.get_sequence_length(seq), 4) if seq else None})`, 60);
    if (!len.ok || JSON.parse(len.value).length_s === null) { cap.value.clips[name] = { error: len.ok ? 'missing clip' : len.error }; continue; }
    const rec = { files: [], length_s: JSON.parse(len.value).length_s };
    for (let k = 0; k < N; k++) {
      const t = ((k * SEC) / N) % Math.max(rec.length_s, 0.001);
      const sc = py(`import unreal
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
comp = [a for a in unreal.GameplayStatics.get_all_actors_of_class(world, unreal.SkeletalMeshActor) if a.get_actor_label().startswith('P4_Figure')][0].skeletal_mesh_component
comp.set_animation_mode(unreal.AnimationMode.ANIMATION_SINGLE_NODE)
comp.set_animation(unreal.load_asset(${JSON.stringify(path)}))
comp.set_position(${t.toFixed(4)}, False)
comp.set_play_rate(0.0)
comp.play(True)
'ok'`, 60);
      spawnSync(process.execPath, ['-e', 'setTimeout(()=>{}, 150)']);
      if (!sc.ok) { rec.error = 'scrub: ' + sc.error; break; }
      const fd = join(d, 'k' + k); mkdirSync(fd, { recursive: true });
      const f = resolve(scratch, `cap-${Date.now()}.json`);
      writeFileSync(f, JSON.stringify({ mode: 'Lit', options: { width: Number(o.width || 480), height: Number(o.height || 540) } }));
      const r = spawnSync(process.execPath, [resolve(repo, 'tools/mcp/mcp.mjs'), 'call', 'SceneCaptureToolset.SceneCaptureToolset.CaptureActiveEditorView', '@' + f, '--url', url, '--timeout', '60', '--save-images', fd], { encoding: 'utf8', maxBuffer: 1 << 26 });
      const got = readdirSync(fd).filter((x) => x.endsWith('.png'));
      if (!got.length) { rec.error = `no image for frame ${k}: ${(r.stdout || '').trim().split('\n').pop()}`; break; }
      const dst = join(d, `${name}_${String(k).padStart(2, '0')}.png`);
      copyFileSync(join(fd, got[0]), dst); rmSync(fd, { recursive: true, force: true });
      rec.files.push(dst);
    }
    cap.value.clips[name] = rec;
  }
  // 4. stop
  const stop = mcp('EditorToolset.EditorAppToolset.StopPIE', {}, 120);
  console.log(`${stop.ok ? 'PASS' : 'FAIL'} simulate stopped`);
  if (!cap.ok) { console.log(`FAIL capture ${cap.error}`); process.exitCode = 1; }
  else {
    // 5. strips
    const shots = resolve(repo, 'devlog/shots');
    mkdirSync(shots, { recursive: true });
    const results = {};
    for (const [name, rec] of Object.entries(cap.value.clips)) {
      if (rec.error) { console.log(`FAIL ${name}: ${rec.error}`); results[name] = { error: rec.error }; process.exitCode = 1; continue; }
      const d = join(outDir, name);
      // strip.py expects f*.png
      const tmp = join(scratch, 'strip-' + name); rmSync(tmp, { recursive: true, force: true }); mkdirSync(tmp, { recursive: true });
      for (const f of readdirSync(d).filter((x) => x.endsWith('.png')).sort()) copyFileSync(join(d, f), join(tmp, 'f' + f));
      const short = name.replace(/^AS_P4_/, '').replace(/_/g, '-');
      const out = join(shots, `${stamp}-A-${label}-${short}.png`);
      const cap2 = `${name}: ${rec.length_s} s, ${Number(o.frames || 8)} frames over ${Number(o.seconds || 2)} s (sequence time), UE5 Manny, three-quarter front, proof level`;
      const s = spawnSync('python', ['-I', resolve(repo, 'tools/meshy/strip.py'), tmp, out, String(Number(o.frames || 8)), '320', cap2], { encoding: 'utf8' });
      writeFileSync(out.replace(/\.png$/, '.txt'), cap2 + '\n');
      console.log(`${s.status === 0 ? 'PASS' : 'FAIL'} strip ${out} ${(s.stdout || s.stderr || '').trim().split('\n').pop()}`);
      results[name] = { strip: out, frames: rec.files.length, length_s: rec.length_s };
      if (s.status !== 0) process.exitCode = 1;
    }
    console.log(JSON.stringify({ tool: 'thirdjam.p4motion-capture/1', verdict: process.exitCode ? 'fail' : 'pass', clips: results }));
  }
}
