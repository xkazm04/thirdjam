#!/usr/bin/env node
// Lane A37, brief item 4: one still per look tier and per zone / obstacle / pickup kind from a cut that uses it, through the editor's MCP
// server (PIE on the Rink map, `jam.ScriptHuman 1`, `jam.playcut <cut>`, a time-dilation freeze, the camera 24 m behind the blue centre
// at the game camera's own geometry: pitch -17.7 deg, FOV 70, as docs/experiments/LOOK-LAB.md measured it). The camera is a CameraActor
// placed in the editor world before PIE starts (PIE duplicates the level; GameplayStatics cannot spawn into PIE from Python), moved per still.
//
//   node tools/cuts/stills.mjs [--url http://127.0.0.1:8021/mcp] [--out docs/experiments/img/cuts] [--distance 24] [--only <id>] [--max-res 2560]
// Writes <out>/<id>-24m.png per still and <out>/stills.json (what was shot, when in the match, the capture size). Exit 0 all captured, 1 a capture
// failed, 2 the editor did not answer. Never stops the editor: tools/run/editor.ps1 -Stop does that, for the editor it started.
import { spawnSync } from 'node:child_process';
import { mkdirSync, writeFileSync, readdirSync, renameSync, existsSync } from 'node:fs';
import { join, dirname, resolve } from 'node:path';
import { fileURLToPath } from 'node:url';

const here = dirname(fileURLToPath(import.meta.url));
const repo = resolve(here, '..', '..');
function arg(name, dflt) {
  const i = process.argv.indexOf(`--${name}`);
  return i > 0 && i + 1 < process.argv.length ? process.argv[i + 1] : dflt;
}
const url = arg('url', 'http://127.0.0.1:8021/mcp');
const out = resolve(arg('out', join(repo, 'docs', 'experiments', 'img', 'cuts')));
const distance = Number(arg('distance', '24'));
const maxRes = Number(arg('max-res', '2560'));
const only = arg('only', null);
mkdirSync(out, { recursive: true });
const env = { ...process.env, MSYS_NO_PATHCONV: '1' };
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
const cutFile = (id) => join(here, 'out', 'cuts', `${id}.cartridge.json`).split('\\').join('/');

// What to shoot: the cut, what it proves, and how long the match runs before the freeze (pickups spawn every 600 frames = 10 s).
export const STILLS = [
  { id: 'matinee-cut', cut: cutFile('matinee-cut'), shows: 'look tier l1_day', wait: 4 },
  { id: 'pillar-cut', cut: cutFile('pillar-cut'), shows: 'look tier l2_night; obstacles pillar x4', wait: 4 },
  { id: 'neon-cut', cut: cutFile('neon-cut'), shows: 'look tier l3_neon; zone fast_ice circle r 6 at centre', wait: 4 },
  { id: 'carpet-cut', cut: cutFile('carpet-cut'), shows: 'zone slow_ice rect 40 x 45 at centre', wait: 4 },
  { id: 'take-7', cut: 'ladder/take-7', shows: 'zones slow_ice circle r 10 + fast_ice rects on both boards; 140 x 70 rink', wait: 4 },
  { id: 'pinball-cut', cut: cutFile('pinball-cut'), shows: 'obstacles bumper x4', wait: 4 },
  { id: 'maze-cut', cut: cutFile('maze-cut'), shows: 'obstacles box x11', wait: 4 },
  // target 'pickup': the camera sits 24 m from the first pickup on the ice (they spawn at seeded spots, rarely at centre), looking at it.
  { id: 'power-up-cut', cut: cutFile('power-up-cut'), shows: 'pickups speed + big_puck on the ice', wait: 14, target: 'pickup' },
];

function call(tool, args, extra = [], timeout = 90) {
  const argFile = join(out, '_args.json');
  writeFileSync(argFile, JSON.stringify(args));
  const r = spawnSync(process.execPath, [join(repo, 'tools', 'mcp', 'mcp.mjs'), 'call', tool, `@${argFile}`, '--url', url, '--timeout', String(timeout), ...extra], { encoding: 'utf8', env });
  const lines = (r.stdout || '').trim().split('\n');
  let verdict = {}; try { verdict = JSON.parse(lines.at(-1)); } catch { /* no status line */ }
  let body = null; try { body = JSON.parse(lines.slice(0, -1).join('\n')); } catch { /* not JSON */ }
  const text = body?.content?.find((c) => c.type === 'text')?.text ?? '';
  return { ok: r.status === 0 && verdict.verdict === 'pass', verdict, body, text };
}
const console_ = (command) => call('ConsoleToolset.ConsoleToolset.RunConsoleCommand', { command });
const python = (content) => call('editor_toolset.toolsets.python_scripting.PythonScriptingTools.execute_python', { script: { content, language: 'Python' } });

const CAM = 'TJ_A37_Still';
// Rink metres -> world: x -> X, z -> Y, height -> Z (UJamSim::ToWorld, centimetres). Blue (team 0) attacks from -x, so "behind the blue centre" is -x.
function cameraScript(freeze, target) {
  const pitch = -17.7;
  const h = distance * Math.tan((-pitch * Math.PI) / 180);
  return `
import unreal, math
w = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
pc = unreal.GameplayStatics.get_player_controller(w, 0)
cams = [a for a in unreal.GameplayStatics.get_all_actors_of_class(w, unreal.CameraActor) if a.get_actor_label().startswith('${CAM}')]
if not cams:
    raise RuntimeError('no ${CAM} camera in the PIE world')
cam = cams[0]
at = unreal.Vector(0.0, 0.0, 0.0)
${target === 'pickup' ? `picks = unreal.GameplayStatics.get_all_actors_of_class(w, unreal.JamPickupActor)
if not picks:
    raise RuntimeError('no pickup on the ice at the freeze')
at = picks[0].get_actor_location()
at = unreal.Vector(at.x, at.y, 0.0)` : ''}
cam.set_actor_location_and_rotation(unreal.Vector(at.x + ${(-distance * 100).toFixed(1)}, at.y, ${(h * 100).toFixed(1)}), unreal.Rotator(0.0, ${pitch}, 0.0), False, True)
cam.camera_component.set_field_of_view(70.0)
${freeze ? `unreal.GameplayStatics.set_global_time_dilation(w, 0.0001)
pc.set_view_target_with_blend(cam, 0.0)` : `unreal.GameplayStatics.set_global_time_dilation(w, 1.0)
pawn = unreal.GameplayStatics.get_player_pawn(w, 0)
if pawn:
    pc.set_view_target_with_blend(pawn, 0.0)`}
'frozen' if ${freeze ? 'True' : 'False'} else 'running'
`;
}

const placeCameraScript = `
import unreal
ell = unreal.EditorLevelLibrary
found = [a for a in ell.get_all_level_actors() if a.get_actor_label().startswith('${CAM}')]
if found:
    cam = found[0]
else:
    cam = ell.spawn_actor_from_class(unreal.CameraActor, unreal.Vector(-2400.0, 0.0, 766.0), unreal.Rotator(0.0, -17.7, 0.0))
    cam.set_actor_label('${CAM}')
cam.get_path_name()
`;

function capture(label) {
  const c = call('SceneCaptureToolset.SceneCaptureToolset.CaptureGameViewport', { world: { refPath: '/Game/Maps/UEDPIE_0_Rink.Rink' }, maxResolution: maxRes }, ['--save-images', out]);
  let size = null;
  try { size = c.body?.structuredContent?.returnValue; size = size ? { width: size.width, height: size.height } : null; } catch { /* none */ }
  if (c.ok) {
    const newest = readdirSync(out).filter((f) => f.startsWith('mcp-image')).sort().at(-1);
    if (newest) renameSync(join(out, newest), join(out, `${label}.png`));
  }
  return { ok: c.ok && existsSync(join(out, `${label}.png`)), size, error: c.ok ? '' : c.verdict.error || c.text };
}

(async () => {
  const t0 = Date.now();
  const report = { tool: 'thirdjam.cuts.stills/1', url, distance_m: distance, stills: [] };
  const alive = call('ConsoleToolset.LogsToolset.GetLogEntries', { pattern: 'ZZZ_never', maxEntries: 1 });
  if (!alive.ok && alive.verdict.verdict === 'unreachable') { console.log(`FAIL stills editor unreachable at ${url}`); process.exit(2); }
  const placed = python(placeCameraScript);
  console.log(`${placed.ok ? 'PASS' : 'FAIL'} camera placed in the editor world: ${placed.ok ? placed.text.slice(0, 120) : placed.verdict.error || placed.text}`);
  if (!placed.ok) process.exit(1);
  const pie = call('EditorToolset.EditorAppToolset.StartPIE', {}, [], 180);
  console.log(`${pie.ok ? 'PASS' : 'FAIL'} StartPIE ${pie.ok ? `${pie.verdict.ms} ms` : pie.verdict.error || pie.text}`);
  if (!pie.ok) process.exit(1);
  await sleep(3000);
  const sh = console_('jam.ScriptHuman 1');
  console.log(`${sh.ok ? 'PASS' : 'FAIL'} jam.ScriptHuman 1`);
  let failed = 0;
  for (const s of STILLS.filter((x) => !only || x.id.includes(only))) {
    const pc = console_(`jam.playcut ${s.cut}`);
    await sleep(s.wait * 1000);
    const fr = python(cameraScript(true, s.target));
    await sleep(1200);
    const cap = capture(`${s.id}-${distance}m`);
    const un = python(cameraScript(false, s.target));
    const row = { id: s.id, cut: s.cut, shows: s.shows, match_seconds_before_freeze: s.wait, playcut_ok: pc.ok, freeze: fr.ok ? fr.text.slice(0, 40) : fr.verdict.error || fr.text.slice(0, 200), unfreeze_ok: un.ok, capture: cap.ok, size: cap.size, file: cap.ok ? `${s.id}-${distance}m.png` : null, error: cap.error || '' };
    report.stills.push(row);
    if (!cap.ok || !fr.ok) failed += 1;
    console.log(`${cap.ok && fr.ok ? 'PASS' : 'FAIL'} still ${s.id}: ${s.shows}${cap.size ? ` (${cap.size.width} x ${cap.size.height})` : ''}${row.error ? ` ${row.error}` : ''}${fr.ok ? '' : ` freeze: ${row.freeze}`}`);
    await sleep(800);
  }
  const stop = call('EditorToolset.EditorAppToolset.StopPIE', {}, [], 120);
  console.log(`${stop.ok ? 'PASS' : 'FAIL'} StopPIE`);
  report.seconds = Math.round((Date.now() - t0) / 1000);
  writeFileSync(join(out, 'stills.json'), `${JSON.stringify(report, null, 2)}\n`);
  console.log(`${failed ? 'FAIL' : 'PASS'} stills ${report.stills.length - failed} of ${report.stills.length} captured into ${out.split('\\').join('/')} in ${report.seconds} s`);
  console.log(JSON.stringify({ tool: 'thirdjam.cuts.stills/1', verdict: failed ? 'fail' : 'pass', exit: failed ? 1 : 0, captured: report.stills.length - failed, out: out.split('\\').join('/') }));
  process.exit(failed ? 1 : 0);
})();
