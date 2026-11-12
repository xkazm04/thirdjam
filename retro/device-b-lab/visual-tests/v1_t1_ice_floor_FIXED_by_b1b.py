# V1-T1: ice floor, blue-line fix, skate scratches, reflection lever  (plan: thirdjam/briefs/B7-visual-upgrade-plan.md, section 3)
#
# STATUS: written 2026-11-12 by lane V1, NOT RUN. Read it before you run it; fix what the first run reports.
#
# WHAT IT BUILDS (only under /JamAssets/_Scratch/V1T1/, never anything else):
#   - L_V1T1: a DUPLICATE of /JamAssets/Rigs/LR_L2_Night (PPV, sky, 4 spots, dark room, 289 dressing actors), opened as the editor world.
#     The real rig is only read. If the duplicate fails, a minimal rig with the recipe values is built instead (reported as 'fallback_rig').
#   - Throwaway layout in that copy, labels V1_*: ice plane 120 x 60 m (stencil 2), boards + trim strips, two nets, two beacons,
#     four figures (stencil 1), two pucks (stencil 3: one on lit ice, one on the dark side band at (-20, 17) m), a PlayerStart, a CameraActor.
#   - M_V1Ice_<VER> + MI_V1Ice_<VER>: the recipe's M_Ice (with B1's RinkHalfLength/RinkHalfWidth lines) plus
#       IceAmbient (emissive floor so the dark ice is dark navy, not black), AmbientColour,
#       LineFix (1 = lines keep a constant roughness, the suspected cause of the blue-line blotches), LineRoughness,
#       Scratch / ScratchColour / ScratchSpacing / ScratchWidth / ScratchFadeEnd (procedural skate scratches, faded with distance).
#   - Two unshadowed wide "house fill" spots V1_Fill_1/2 (hidden except in case d).
#
# HOW TO RUN (device B, one editor slot, the other editor NOT in PIE and its viewport not realtime):
#   1. Set ACTION = 'build', run through execute_python (briefs/B-machine-notes.md rule 6, --timeout 600). returnValue must show errors == [].
#   2. For each CASE in CASES (start with 'a_baseline' and 'a2_v1_neutral'; a2 must look like a, it proves the scratch ice matches the real one):
#        ACTION = 'case', CASE = '<name>'  -> run
#        for each SHOT in ('near', 'side', 'line', 'far'): ACTION = 'shot', SHOT = '<name>' -> run, then
#          EditorToolset.EditorAppToolset.StartPIE {"options":{"playMode":"PlayMode_InEditorFloating","warmupSeconds":5}}
#          (near only:) PerfToolset.StatsToolset.GetFrameTimings {} x 60 -> median gpuMs
#          (near only:) PerfToolset.GpuProfilerToolset.TriggerGPUProfile {"minInclusiveMs":0} -> BasePass, LumenReflections, RenderDeferredLighting
#          SceneCaptureToolset.SceneCaptureToolset.CaptureGameViewport {"world":{"refPath":"/JamAssets/_Scratch/V1T1/UEDPIE_0_L_V1T1.L_V1T1"},"maxResolution":1920}
#              (check the world name with ListWorlds first; save with `call ... --save-images C:\Users\kazda\jam\skater-lab\v1\shots\t1`)
#          EditorToolset.EditorAppToolset.StopPIE {}
#      Shots change only the camera, so one PIE per shot is needed (editor-world edits do not reach a running PIE world).
#   3. ACTION = 'report' prints the state (current case, params, which PPV properties could not be set).
#   4. ACTION = 'cleanup' at the end, ALWAYS, also after a failure: saves the scratch copy (scratch only, to avoid a save prompt), loads
#      /Game/Maps/Rink (never saved), deletes /JamAssets/_Scratch/V1T1 through the asset library and then on disk if it is still there.
#      Afterwards Game/Plugins/JamAssets/Content/ must hold no _Scratch folder (the ledger gate counts every file).
#
# WHAT TO MEASURE AND THE PASS LINES (pixel numbers from ffmpeg on the 1920x1085 PNGs:
#   ffmpeg -i x.png -vf "crop=W:H:X:Y,signalstats,metadata=print" -f null -  -> lavfi.signalstats.YAVG / YMIN / YMAX / UAVG / VAVG;
#   pick the crops once on the baseline stills and reuse them for every case):
#   - floor ('side' shot): side-band ice crop YAVG 20..60 (baseline expected ~0..5); pool-centre crop YAVG within 5 % of baseline; the ramp's
#     bands still visible; the dark-band puck (24 m) differs from its surround by >= 40 luma levels.
#   - line ('line' shot): YMAX - YMIN inside a crop of the blue line <= 30 (baseline expected > 60).
#   - cost: BasePass delta <= 0.05 ms in the profiler (gpuMs noise is +-0.5..1 ms on B, do not read material deltas from it).
#   - lever: if 'i_refl_ssr' or 'j_refl_none' saves >= 0.5 ms (LumenReflections gone / RenderDeferredLighting down) with no visible change
#     on dry ice, propose it for the L2 PPV. These are DEVICE B numbers; the gate number is A's 4090 (handoff).
#   Time box: 45 minutes including the stills. Stop at the box and report what you have.
#
# WHAT TO DELETE AFTERWARDS: everything under /JamAssets/_Scratch/V1T1 (ACTION = 'cleanup' does it). Nothing else was created.
# The stills you keep go to devlog/shots via `node devlog/shot.mjs B v1-t1-<case>-<shot> <file> "<caption>"`; the rest stay in skater-lab.
#
# KNOWN UNCERTAINTIES (not run, so unverified): EditorAssetLibrary.duplicate_asset on a map; the PPV property names reflection_method /
# lumen_max_roughness_to_trace_reflections and the enum unreal.ReflectionMethod (failures are collected in 'pp_failed', not fatal);
# the Custom-node HLSL compiles (check 'errors' and the Output Log; recipe section 9 says Custom-node HLSL errors may not be reported).

import math
import os
import shutil

import unreal

# ------------------------------------------------------------------ parameters
ACTION = 'build'            # 'build' | 'case' | 'shot' | 'report' | 'cleanup'
CASE = 'a_baseline'         # a key of CASES
SHOT = 'near'               # a key of SHOTS
VER = 'a'                   # bump to rebuild materials under a fresh name (recipe pitfall 7: same-name recreate returns None)
# B1b fix: parameters are injected here by the runner (py.mjs KEY=VALUE), so one file serves every action
# @@PARAMS@@

SCRATCH = '/JamAssets/_Scratch/V1T1'
LEVEL = SCRATCH + '/L_V1T1'
RIG = '/JamAssets/Rigs/LR_L2_Night'
LOOK = '/JamAssets/Look'
RETURN_MAP = '/Game/Maps/Rink'      # device A's map: loaded, never saved
RIG_PREFIX = 'L2_Night'             # rig actor labels: PPV_L2_Night, Sky_L2_Night, Spot_L2_Night_n

RINK_HALF_LENGTH_M = 60.0
RINK_HALF_WIDTH_M = 30.0
BOARD_HEIGHT_M = 1.2

FILL_CANDELAS = 600.0               # case d: two unshadowed wide spots ("house lights at a few percent")
FILL_COLOUR = (150, 185, 255)

# third-person camera: 6 m back, 3.5 m up, pitch -17.65, FOV 70 (tools/skills/camera-third-person.md)
CAM_BACK_M, CAM_UP_M, CAM_PITCH, CAM_FOV = 6.0, 3.5, -17.65, 70.0
FIGURES_BLUE = [(-45.0, 0.0), (-38.0, 16.0), (-3.0, 0.0), (40.0, 0.0)]
FIGURES_ORANGE = [(-20.0, 6.0), (10.0, -8.0), (35.0, 12.0)]
PUCKS = {'V1_Puck_Lit': (-35.0, 0.6), 'V1_Puck_Dark': (-20.0, 17.0)}
SHOTS = {
    # name: (figure x, figure y, camera yaw, pitch override or None)
    'near': (-45.0, 0.0, 0.0, None),       # own end, pool + gap ahead
    'side': (-38.0, 16.0, 0.0, None),      # along the dark side band; the dark puck is 24 m ahead
    'line': (-15.0, -3.0, 0.0, -32.0),     # camera at x -21: the -x blue line (x -17.4 m) is 3.5 m ahead, about 45 deg down, lower part of the frame
    'far': (40.0, 0.0, 0.0, None),         # far end, net at 53.5 m, beacon, room behind
}

# every case sets ALL of these; unspecified keys take DEFAULTS, so cases can run in any order
DEFAULTS = {
    'ice': 'orig',                 # 'orig' = /JamAssets/Look/MI_Ice_Night, 'v1' = MI_V1Ice_<VER>
    'IceAmbient': 0.0, 'LineFix': 0.0, 'Scratch': 0.0,
    'LineBlue': (0.04, 0.10, 0.40, 1.0),
    'fill': False,
    'sky': 2.0,
    'refl': None,                  # None = project default (Lumen) | 'lumen' | 'ssr' | 'none'
    'max_rough': None,             # None = default | float, Lumen max roughness to trace reflections
}
CASES = {
    'a_baseline': {},
    'a2_v1_neutral': {'ice': 'v1'},                                         # must look like a_baseline
    'b_amb15': {'ice': 'v1', 'IceAmbient': 0.15},
    'c_amb40': {'ice': 'v1', 'IceAmbient': 0.40},
    # B1b: at fixed EV100 4.2 an emissive of 0.15-0.4 is expected to be near invisible (figures need Emissive 2-3 for a fill); add larger steps
    'c2_amb100': {'ice': 'v1', 'IceAmbient': 1.0},
    'c3_amb200': {'ice': 'v1', 'IceAmbient': 2.0},
    'c4_amb400': {'ice': 'v1', 'IceAmbient': 4.0},
    'c5_amb50_fix': {'ice': 'v1', 'IceAmbient': 0.5, 'LineFix': 1.0},
    'm_b60': {'ice': 'v1', 'IceAmbient': 0.6, 'LineFix': 1.0},
    'm_b90': {'ice': 'v1', 'IceAmbient': 0.9, 'LineFix': 1.0},
    'c6_amb60_fix': {'ice': 'v1', 'IceAmbient': 0.6, 'LineFix': 1.0},
    'f2_linefix_amb': {'ice': 'v1', 'IceAmbient': 2.0, 'LineFix': 1.0},
    'd_housefill': {'fill': True},
    'e_sky35': {'sky': 3.5},
    'f_linefix': {'ice': 'v1', 'IceAmbient': 0.25, 'LineFix': 1.0},
    'g_lineblue_light': {'ice': 'v1', 'IceAmbient': 0.25, 'LineFix': 1.0, 'LineBlue': (0.10, 0.20, 0.62, 1.0)},
    'h_scratch': {'ice': 'v1', 'IceAmbient': 0.25, 'LineFix': 1.0, 'Scratch': 1.0},
    'h2_scratch_strong': {'ice': 'v1', 'IceAmbient': 0.25, 'LineFix': 1.0, 'Scratch': 2.0},
    'i_refl_ssr': {'refl': 'ssr'},
    'j_refl_none': {'refl': 'none'},
    'k_refl_rough02': {'max_rough': 0.2},
}

# ------------------------------------------------------------------ helpers (style of tools/skills/look-painted-cel.md)
EAL = unreal.EditorAssetLibrary
MEL = unreal.MaterialEditingLibrary
AT = unreal.AssetToolsHelpers.get_asset_tools()
MP = unreal.MaterialProperty
EAS = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
LES = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
UES = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
OUT = {'action': ACTION, 'log': [], 'errors': [], 'pp_failed': []}


def log(msg):
    OUT['log'].append(str(msg))
    unreal.log('V1T1: %s' % msg)


def must_scratch(path):
    if not path.startswith(SCRATCH):
        raise RuntimeError('refusing to write outside the scratch folder: %s' % path)


def world():
    return UES.get_editor_world()


def in_test_world():
    w = world()
    return w is not None and w.get_path_name().startswith(LEVEL)


def pie_running():
    try:
        return LES.is_in_play_in_editor()
    except Exception:
        return False


def new_material(path):
    must_scratch(path)
    if EAL.does_asset_exist(path):
        return None   # caller loads it instead (bump VER to rebuild)
    pkg, name = path.rsplit('/', 1)
    return AT.create_asset(name, pkg, unreal.Material, unreal.MaterialFactoryNew())


def node(m, cls, x=0, y=0, **props):
    e = MEL.create_material_expression(m, cls, x, y)
    for k, v in props.items():
        e.set_editor_property(k, v)
    return e


def sparam(m, name, val, group='V1'):
    return node(m, unreal.MaterialExpressionScalarParameter, parameter_name=name, default_value=val, group=group)


def vparam(m, name, rgba, group='V1'):
    return node(m, unreal.MaterialExpressionVectorParameter, parameter_name=name, default_value=unreal.LinearColor(*rgba), group=group)


def const(m, v):
    return node(m, unreal.MaterialExpressionConstant, r=v)


def wire(a, b, inp, out=''):
    if not MEL.connect_material_expressions(a, out, b, inp):
        raise RuntimeError('connect failed %s.%s -> %s.%s' % (a.get_name(), out, b.get_name(), inp))


def toprop(a, prop, out=''):
    if not MEL.connect_material_property(a, out, prop):
        raise RuntimeError('connect prop failed %s' % prop)


def custom(m, code, inputs, otype=unreal.CustomMaterialOutputType.CMOT_FLOAT3, desc='Custom'):
    c = node(m, unreal.MaterialExpressionCustom)
    c.set_editor_property('code', code)
    c.set_editor_property('output_type', otype)
    c.set_editor_property('description', desc)
    ins = []
    for n in inputs:
        ci = unreal.CustomInput()
        ci.set_editor_property('input_name', n)
        ins.append(ci)
    c.set_editor_property('inputs', ins)
    return c


def finish(m):
    MEL.layout_material_expressions(m)
    MEL.recompile_material(m)
    EAL.save_loaded_asset(m)
    return [str(e.get_name()) for e in MEL.get_material_error_expressions(m)]


def make_mi(name, parent, vec=None, scal=None):
    path = SCRATCH + '/' + name
    must_scratch(path)
    if EAL.does_asset_exist(path):
        m = unreal.load_asset(path)
    else:
        m = AT.create_asset(name, SCRATCH, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
        m.set_editor_property('parent', parent)
    for k, v in (vec or {}).items():
        MEL.set_material_instance_vector_parameter_value(m, k, unreal.LinearColor(*v))
    for k, v in (scal or {}).items():
        MEL.set_material_instance_scalar_parameter_value(m, k, v)
    MEL.update_material_instance(m)
    EAL.save_loaded_asset(m)
    return m


def look(name):
    a = unreal.load_asset('%s/%s.%s' % (LOOK, name, name))
    if a is None:
        OUT['errors'].append('missing look asset %s' % name)
    return a


def shape(name):
    return unreal.load_asset('/Engine/BasicShapes/%s.%s' % (name, name))


def rot(pitch=0.0, yaw=0.0, roll=0.0):
    return unreal.Rotator(roll=roll, pitch=pitch, yaw=yaw)   # unreal.Rotator is (roll, pitch, yaw): always keywords


def spawn_mesh(label, shp, loc_m, scale, mat, stencil=0, shadow=False, r=None):
    a = EAS.spawn_actor_from_object(shape(shp), unreal.Vector(loc_m[0] * 100, loc_m[1] * 100, loc_m[2] * 100), r or rot())
    a.set_actor_label(label)
    a.set_folder_path('V1')
    a.set_actor_scale3d(unreal.Vector(*scale))
    c = a.get_editor_property('static_mesh_component')
    c.set_material(0, mat)
    c.set_cast_shadow(shadow)
    c.set_collision_profile_name('NoCollision')
    if stencil:
        c.set_render_custom_depth(True)
        c.set_custom_depth_stencil_value(stencil)
    return a


def actors():
    # B1b fix: 'pie_*' actions work on the running PIE world (one PIE for all cases and shots instead of one PIE per shot)
    if ACTION.startswith('pie_'):
        return unreal.GameplayStatics.get_all_actors_of_class(UES.get_game_world(), unreal.Actor)
    return EAS.get_all_level_actors()


def find(label):
    for a in actors():
        if a.get_actor_label() == label:
            return a
    return None


def find_prefix(prefix):
    return [a for a in actors() if a.get_actor_label().startswith(prefix)]


def pp_set(settings, key, value):
    try:
        settings.set_editor_property(key, value)
        settings.set_editor_property('override_' + key, True)
    except Exception as e:
        OUT['pp_failed'].append('%s: %s' % (key, e))


def pp_clear(settings, key):
    try:
        settings.set_editor_property('override_' + key, False)
    except Exception as e:
        OUT['pp_failed'].append('override_%s: %s' % (key, e))


# ------------------------------------------------------------------ the scratch ice material
ICE_MASK = 'return saturate((N1 * 0.65 + N2 * 0.35 - 0.42) * 3.0) * Scuff;'

ICE_LINES = '''float ax = abs(P.x);
float inside = step(abs(P.y), HW);
float red = (step(ax, 30.0) + step(abs(ax - (HL - 700.0)), 6.0)) * inside;
float blue = (step(abs(ax - HL * 0.29), 30.0) + step(abs(length(P.xy) - 900.0), 8.0)) * inside;
return float2(saturate(red), saturate(blue));'''

# skate scratches: three families of long, gently curved, dashed lines; faded out with camera distance (TSR shimmer otherwise)
ICE_SCRATCH = '''float2 p = P.xy * 0.01;
float s = 0.0;
[unroll] for (int k = 0; k < 3; k++) {
  float a = 0.35 + k * 1.1;
  float2 d = float2(cos(a), sin(a));
  float2 q = float2(dot(p, d), dot(p, float2(-d.y, d.x)));
  float wob = sin(q.x * 0.21 + k * 3.0) * 1.7 + sin(q.x * 0.053 + k) * 4.0;
  float v = (q.y + wob) / Spacing;
  float id = floor(v);
  float f = abs(frac(v) - 0.5) * Spacing;
  float h = frac(sin(id * 12.9898 + k * 78.233) * 43758.5453);
  float dash = step(0.45, frac(sin(floor(q.x / (6.0 + 10.0 * h)) * 4.1 + id) * 9631.7));
  s = max(s, (1.0 - smoothstep(Width * 0.5, Width, f)) * dash * step(0.3, h));
}
float fade = 1.0 - saturate(D / max(FadeEnd, 1.0));
return s * Scratch * fade;'''

ICE_COLOUR = '''float3 c = lerp(IceColour, ScuffColour, M);
c = lerp(c, ScratchColour, saturate(S));
c = lerp(c, LineRed, L.x);
c = lerp(c, LineBlue, L.y);
return c;'''

ICE_ROUGH = '''float r = lerp(lerp(0.28, 0.75, M), 0.05, Wet);
float lines = saturate(L.x + L.y);
return lerp(r, LineRoughness, lines * LineFix);'''

# B1b fix (VER b): the ramp divides by the diffuse colour, so an emissive that does not follow the base colour (noise N1, scuff) becomes
# 'light' that varies 1.4x across the scuff mask and the 1-stop ice ramp cuts it into hard blotches. Emissive = base colour * ambient: flat light.
ICE_EMISSIVE = 'return BaseC * AmbientColour.rgb * IceAmbient;'


def build_ice_material():
    path = '%s/M_V1Ice_%s' % (SCRATCH, VER)
    m = new_material(path)
    if m is None:
        log('material exists, reused: %s' % path)
        return unreal.load_asset(path)
    wp = node(m, unreal.MaterialExpressionWorldPosition)
    cam = node(m, unreal.MaterialExpressionCameraPositionWS)
    dist = node(m, unreal.MaterialExpressionDistance)
    wire(wp, dist, 'A'); wire(cam, dist, 'B')
    n1 = node(m, unreal.MaterialExpressionNoise, scale=0.004, levels=4, output_min=0.0, output_max=1.0, turbulence=True, quality=1)
    n2 = node(m, unreal.MaterialExpressionNoise, scale=0.05, levels=2, output_min=0.0, output_max=1.0, turbulence=False, quality=1)
    P = {}
    for n, v in (('Scuff', 0.7), ('Wet', 0.0), ('RinkHalfLength', RINK_HALF_LENGTH_M * 100), ('RinkHalfWidth', RINK_HALF_WIDTH_M * 100),
                 ('IceAmbient', 0.0), ('LineFix', 0.0), ('LineRoughness', 0.6),
                 ('Scratch', 0.0), ('ScratchSpacing', 2.5), ('ScratchWidth', 0.03), ('ScratchFadeEnd', 3500.0)):
        P[n] = sparam(m, n, v)
    for n, v in (('IceColour', (0.72, 0.80, 0.86, 1)), ('ScuffColour', (0.50, 0.56, 0.62, 1)), ('LineRed', (0.45, 0.04, 0.04, 1)),
                 ('LineBlue', (0.04, 0.10, 0.40, 1)), ('ScratchColour', (0.90, 0.94, 1.0, 1)), ('AmbientColour', (0.25, 0.35, 0.65, 1))):
        P[n] = vparam(m, n, v)
    mask = custom(m, ICE_MASK, ['N1', 'N2', 'Scuff'], otype=unreal.CustomMaterialOutputType.CMOT_FLOAT1, desc='ScuffMask')
    wire(n1, mask, 'N1'); wire(n2, mask, 'N2'); wire(P['Scuff'], mask, 'Scuff')
    lines = custom(m, ICE_LINES, ['P', 'HL', 'HW'], otype=unreal.CustomMaterialOutputType.CMOT_FLOAT2, desc='LineMasks')
    wire(wp, lines, 'P'); wire(P['RinkHalfLength'], lines, 'HL'); wire(P['RinkHalfWidth'], lines, 'HW')
    scr = custom(m, ICE_SCRATCH, ['P', 'D', 'Scratch', 'Spacing', 'Width', 'FadeEnd'], otype=unreal.CustomMaterialOutputType.CMOT_FLOAT1, desc='Scratches')
    wire(wp, scr, 'P'); wire(dist, scr, 'D'); wire(P['Scratch'], scr, 'Scratch'); wire(P['ScratchSpacing'], scr, 'Spacing')
    wire(P['ScratchWidth'], scr, 'Width'); wire(P['ScratchFadeEnd'], scr, 'FadeEnd')
    col = custom(m, ICE_COLOUR, ['M', 'S', 'L', 'IceColour', 'ScuffColour', 'ScratchColour', 'LineRed', 'LineBlue'], desc='IceColour')
    wire(mask, col, 'M'); wire(scr, col, 'S'); wire(lines, col, 'L')
    for n in ('IceColour', 'ScuffColour', 'ScratchColour', 'LineRed', 'LineBlue'):
        wire(P[n], col, n)
    rgh = custom(m, ICE_ROUGH, ['M', 'L', 'Wet', 'LineRoughness', 'LineFix'], otype=unreal.CustomMaterialOutputType.CMOT_FLOAT1, desc='Roughness')
    wire(mask, rgh, 'M'); wire(lines, rgh, 'L'); wire(P['Wet'], rgh, 'Wet'); wire(P['LineRoughness'], rgh, 'LineRoughness'); wire(P['LineFix'], rgh, 'LineFix')
    emi = custom(m, ICE_EMISSIVE, ['BaseC', 'AmbientColour', 'IceAmbient'], desc='Ambient')
    wire(col, emi, 'BaseC'); wire(P['AmbientColour'], emi, 'AmbientColour'); wire(P['IceAmbient'], emi, 'IceAmbient')
    toprop(col, MP.MP_BASE_COLOR); toprop(rgh, MP.MP_ROUGHNESS); toprop(const(m, 0.5), MP.MP_SPECULAR); toprop(emi, MP.MP_EMISSIVE_COLOR)
    errs = finish(m)
    if errs:
        OUT['errors'].append('M_V1Ice errors: %s' % errs)
    return m


# ------------------------------------------------------------------ level and layout
def open_test_level():
    dirty = list(unreal.EditorLoadingAndSavingUtils.get_dirty_map_packages()) + list(unreal.EditorLoadingAndSavingUtils.get_dirty_content_packages())
    if dirty:
        raise RuntimeError('dirty packages, save or discard them first (a save prompt would block MCP): %s' % [p.get_name() for p in dirty])
    EAL.make_directory(SCRATCH)
    fallback = False
    if not EAL.does_asset_exist(LEVEL):
        dup = None
        try:
            dup = EAL.duplicate_asset(RIG, LEVEL)
        except Exception as e:
            log('duplicate_asset raised: %s' % e)
        if dup is None:
            fallback = True
            if not LES.new_level(LEVEL):
                raise RuntimeError('new_level failed for %s' % LEVEL)
        else:
            # B1b fix: the first run CRASHED the editor here. duplicate_asset returns the new UWorld; while Python holds it, load_level
            # of the same map fails the editor's world-leak check (fatal "Old world ... not cleaned up by garbage collection",
            # referenced by FPyReferenceCollector). Save the copy, drop every Python reference, collect garbage, then load.
            EAL.save_loaded_asset(dup)
            dup = None
        pass
        __import__("gc").collect()
        unreal.SystemLibrary.collect_garbage()
    if not fallback and not LES.load_level(LEVEL):
        raise RuntimeError('load_level failed for %s' % LEVEL)
    if not in_test_world():
        raise RuntimeError('the editor world is not the scratch copy: %s' % world().get_path_name())
    OUT['fallback_rig'] = fallback
    if fallback:
        build_minimal_rig()


def build_minimal_rig():
    # recipe section 5 values; only used when the rig copy could not be made
    sky = EAS.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 2000))
    sky.set_actor_label('Sky_' + RIG_PREFIX)
    sc = sky.get_editor_property('light_component')
    sc.set_mobility(unreal.ComponentMobility.MOVABLE)
    sc.set_editor_property('source_type', unreal.SkyLightSourceType.SLS_SPECIFIED_CUBEMAP)
    sc.set_editor_property('cubemap', unreal.load_asset('/Engine/MapTemplates/Sky/DaylightAmbientCubemap'))
    sc.set_intensity(2.0)
    sc.set_light_color(unreal.LinearColor(150 / 255.0, 185 / 255.0, 1.0, 1.0))
    for i, x in enumerate((-45, -15, 15, 45), 1):
        s = EAS.spawn_actor_from_class(unreal.SpotLight, unreal.Vector(x * 100, 0, 2500), rot(pitch=-90))
        s.set_actor_label('Spot_%s_%d' % (RIG_PREFIX, i))
        lc = s.get_editor_property('spot_light_component')
        lc.set_mobility(unreal.ComponentMobility.MOVABLE)
        lc.set_editor_property('intensity_units', unreal.LightUnits.CANDELAS)
        lc.set_intensity(20000.0)
        lc.set_editor_property('attenuation_radius', 4000.0)
        lc.set_editor_property('inner_cone_angle', 22.0)
        lc.set_editor_property('outer_cone_angle', 30.0)
        lc.set_editor_property('source_radius', 30.0)
        lc.set_editor_property('light_color', unreal.Color(r=255, g=244, b=230, a=255))
        lc.set_cast_shadows(True)
    ppv = EAS.spawn_actor_from_class(unreal.PostProcessVolume, unreal.Vector(0, 0, 0))
    ppv.set_actor_label('PPV_' + RIG_PREFIX)
    ppv.set_editor_property('unbound', True)
    s = ppv.get_editor_property('settings')
    for k, v in [('auto_exposure_method', unreal.AutoExposureMethod.AEM_HISTOGRAM), ('auto_exposure_min_brightness', 4.2),
                 ('auto_exposure_max_brightness', 4.2), ('auto_exposure_bias', 0.0), ('bloom_intensity', 0.9), ('bloom_threshold', 1.0),
                 ('vignette_intensity', 0.3), ('color_gain_shadows', unreal.Vector4(0.85, 0.93, 1.15, 1.0)),
                 ('color_saturation', unreal.Vector4(1.1, 1.1, 1.1, 1.0))]:
        pp_set(s, k, v)
    s.set_editor_property('weighted_blendables', unreal.WeightedBlendables(array=[
        unreal.WeightedBlendable(weight=1.0, object=look('MI_PP_ToonRamp')),
        unreal.WeightedBlendable(weight=1.0, object=look('MI_PP_Outline'))]))
    ppv.set_editor_property('settings', s)
    dark = look('MI_Toon_Dark')
    spawn_mesh('V1_Room_Floor', 'Plane', (0, 0, -0.05), (164, 94, 1), dark)
    spawn_mesh('V1_Room_Ceiling', 'Cube', (0, 0, 28.25), (164, 94, 0.5), dark)
    for lbl, loc, sc3 in (('V1_Room_WallXN', (-82.25, 0, 14), (0.5, 94, 28)), ('V1_Room_WallXP', (82.25, 0, 14), (0.5, 94, 28)),
                          ('V1_Room_WallYN', (0, -47.25, 14), (164, 0.5, 28)), ('V1_Room_WallYP', (0, 47.25, 14), (164, 0.5, 28))):
        spawn_mesh(lbl, 'Cube', loc, sc3, dark)


def build_layout():
    hl, hw = RINK_HALF_LENGTH_M, RINK_HALF_WIDTH_M
    ice = spawn_mesh('V1_Ice', 'Plane', (0, 0, 0), (hl * 2, hw * 2, 1), look('MI_Ice_Night'), stencil=2)
    board = look('MI_Toon_Board')
    for lbl, loc, sc3 in (('V1_Board_YN', (0, -hw - 0.1, BOARD_HEIGHT_M / 2), (hl * 2, 0.2, BOARD_HEIGHT_M)),
                          ('V1_Board_YP', (0, hw + 0.1, BOARD_HEIGHT_M / 2), (hl * 2, 0.2, BOARD_HEIGHT_M)),
                          ('V1_Board_XN', (-hl - 0.1, 0, BOARD_HEIGHT_M / 2), (0.2, hw * 2, BOARD_HEIGHT_M)),
                          ('V1_Board_XP', (hl + 0.1, 0, BOARD_HEIGHT_M / 2), (0.2, hw * 2, BOARD_HEIGHT_M))):
        spawn_mesh(lbl, 'Cube', loc, sc3, board, stencil=1, shadow=True)
    # 10 cm trim strips on the board tops, blue on x < 0, orange on x > 0 (R-0002 point 5: A spawns these in the real rink)
    for side in (-1, 1):
        mat = look('MI_Trim_Blue') if side < 0 else look('MI_Trim_Orange')
        cx = side * hl / 2
        spawn_mesh('V1_Trim_YN_%d' % side, 'Cube', (cx, -hw - 0.1, BOARD_HEIGHT_M + 0.05), (hl, 0.1, 0.1), mat)
        spawn_mesh('V1_Trim_YP_%d' % side, 'Cube', (cx, hw + 0.1, BOARD_HEIGHT_M + 0.05), (hl, 0.1, 0.1), mat)
    for side in (-1, 1):
        spawn_mesh('V1_Net_%d' % side, 'Cube', (side * 53.5, 0, 0.6), (1.1, 1.83, 1.2), look('MI_Toon_NetFrame'), stencil=1, shadow=True)
        spawn_mesh('V1_Beacon_%d' % side, 'Cylinder', (side * 55.5, 0, 4.0), (0.12, 0.12, 8.0), look('MI_Trim_Beacon'))
    for i, (x, y) in enumerate(FIGURES_BLUE + FIGURES_ORANGE):
        mat = look('MI_Toon_Blue') if i < len(FIGURES_BLUE) else look('MI_Toon_Orange')
        spawn_mesh('V1_Fig_%d_Body' % i, 'Cylinder', (x, y, 0.6), (0.6, 0.6, 1.2), mat, stencil=1, shadow=True)
        spawn_mesh('V1_Fig_%d_Head' % i, 'Sphere', (x, y, 1.42), (0.35, 0.35, 0.35), look('MI_Toon_Helmet'), stencil=1, shadow=True)
        spawn_mesh('V1_Fig_%d_Stick' % i, 'Cylinder', (x + 0.6, y + 0.35, 0.5), (0.04, 0.04, 1.4), look('MI_Toon_Stick'), stencil=1, shadow=True,
                   r=rot(pitch=-55.0))
    for lbl, (x, y) in PUCKS.items():
        spawn_mesh(lbl, 'Cylinder', (x, y, 0.0125), (0.12, 0.12, 0.025), look('MI_Toon_Puck'), stencil=3, shadow=True)
    for i, x in enumerate((-30.0, 30.0), 1):
        f = EAS.spawn_actor_from_class(unreal.SpotLight, unreal.Vector(x * 100, 0, 2650), rot(pitch=-90))
        f.set_actor_label('V1_Fill_%d' % i)
        f.set_folder_path('V1')
        lc = f.get_editor_property('spot_light_component')
        lc.set_mobility(unreal.ComponentMobility.MOVABLE)
        lc.set_editor_property('intensity_units', unreal.LightUnits.CANDELAS)
        lc.set_intensity(FILL_CANDELAS)
        lc.set_editor_property('attenuation_radius', 9000.0)
        lc.set_editor_property('inner_cone_angle', 50.0)
        lc.set_editor_property('outer_cone_angle', 70.0)
        lc.set_editor_property('light_color', unreal.Color(r=FILL_COLOUR[0], g=FILL_COLOUR[1], b=FILL_COLOUR[2], a=255))
        lc.set_cast_shadows(False)
        f.set_actor_hidden_in_game(True)
        f.set_is_temporarily_hidden_in_editor(True)
    ps = EAS.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(-7800, -4400, 100))
    ps.set_actor_label('V1_PlayerStart')
    cam = EAS.spawn_actor_from_class(unreal.CameraActor, unreal.Vector(0, 0, 350))
    cam.set_actor_label('V1_Camera')
    cam.set_folder_path('V1')
    cam.set_editor_property('auto_activate_for_player', unreal.AutoReceiveInput.PLAYER0)
    cam.get_editor_property('camera_component').set_editor_property('field_of_view', CAM_FOV)
    try:
        world().get_world_settings().set_editor_property('default_game_mode', unreal.GameModeBase.static_class())
    except Exception as e:
        OUT['errors'].append('game mode override failed (A\'s game mode may run in PIE): %s' % e)
    return ice


def set_shot(name):
    fx, fy, yaw, pitch = SHOTS[name]
    cam = find('V1_Camera')
    rad = math.radians(yaw)
    loc = unreal.Vector((fx - CAM_BACK_M * math.cos(rad)) * 100, (fy - CAM_BACK_M * math.sin(rad) - 0.7) * 100, CAM_UP_M * 100)
    cam.set_actor_location(loc, False, False)
    cam.set_actor_rotation(rot(pitch=CAM_PITCH if pitch is None else pitch, yaw=yaw), False)
    log('shot %s at %s' % (name, loc))


def apply_case(name):
    c = dict(DEFAULTS)
    c.update(CASES[name])
    ice = find('V1_Ice')
    mi_v1 = unreal.load_asset('%s/MI_V1Ice_%s' % (SCRATCH, VER))
    for k in ('IceAmbient', 'LineFix', 'Scratch'):
        MEL.set_material_instance_scalar_parameter_value(mi_v1, k, c[k])
    MEL.set_material_instance_vector_parameter_value(mi_v1, 'LineBlue', unreal.LinearColor(*c['LineBlue']))
    MEL.update_material_instance(mi_v1)
    EAL.save_loaded_asset(mi_v1)
    ice.get_editor_property('static_mesh_component').set_material(0, mi_v1 if c['ice'] == 'v1' else look('MI_Ice_Night'))
    for f in find_prefix('V1_Fill_'):
        f.set_actor_hidden_in_game(not c['fill'])
        if not ACTION.startswith('pie_'):
            f.set_is_temporarily_hidden_in_editor(not c['fill'])
        else:
            f.get_editor_property('spot_light_component').set_visibility(c['fill'])
    sky = find('Sky_' + RIG_PREFIX)
    if sky:
        sky.get_editor_property('light_component').set_intensity(c['sky'])
    else:
        OUT['errors'].append('no Sky_%s in the level' % RIG_PREFIX)
    ppv = find('PPV_' + RIG_PREFIX)
    s = ppv.get_editor_property('settings')
    if c['refl'] is None:
        pp_clear(s, 'reflection_method')
    else:
        RM = getattr(unreal, 'ReflectionMethod', None)
        if RM is None:
            OUT['pp_failed'].append('unreal.ReflectionMethod not found')
        else:
            pp_set(s, 'reflection_method', {'lumen': RM.LUMEN, 'ssr': RM.SCREEN_SPACE, 'none': RM.NONE}[c['refl']])
    if c['max_rough'] is None:
        pp_clear(s, 'lumen_max_roughness_to_trace_reflections')
    else:
        pp_set(s, 'lumen_max_roughness_to_trace_reflections', c['max_rough'])
    ppv.set_editor_property('settings', s)
    OUT['case'] = name
    OUT['case_values'] = {k: (list(v) if isinstance(v, tuple) else v) for k, v in c.items()}


def cleanup():
    if pie_running():
        raise RuntimeError('stop PIE first')
    if in_test_world():
        EAL.save_directory(SCRATCH, only_if_is_dirty=True, recursive=True)   # scratch only: avoids a modal save prompt on load
    LES.load_level(RETURN_MAP)
    EAL.delete_directory(SCRATCH)
    disk = os.path.normpath(os.path.join(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_plugins_dir()),
                                         'JamAssets', 'Content', '_Scratch', SCRATCH.rsplit('/', 1)[1]))
    if not ('JamAssets' in disk and '_Scratch' in disk):
        raise RuntimeError('refusing to delete %s' % disk)
    if os.path.isdir(disk):
        shutil.rmtree(disk)
        log('deleted on disk: %s' % disk)
    parent = os.path.dirname(disk)
    if os.path.isdir(parent) and not os.listdir(parent):
        os.rmdir(parent)
        log('deleted empty %s' % parent)
    unreal.AssetRegistryHelpers.get_asset_registry().scan_paths_synchronous(['/JamAssets'], True)
    OUT['scratch_left_on_disk'] = os.path.isdir(parent)


def main():
    if ACTION == 'cleanup':
        cleanup()
        return
    if ACTION == 'pie_build_mat':
        make_mi('MI_V1Ice_%s' % VER, build_ice_material())
        return
    if ACTION == 'pie_case':
        apply_case(CASE)
        return
    if ACTION == 'pie_shot':
        set_shot(SHOT)
        return
    if pie_running():
        raise RuntimeError('stop PIE before %s' % ACTION)
    if ACTION == 'build':
        open_test_level()
        m = build_ice_material()
        make_mi('MI_V1Ice_%s' % VER, m)
        if find('V1_Ice') is None:
            build_layout()
        else:
            log('layout already present, not spawned again')
        pset = unreal.get_default_object(unreal.LevelEditorPlaySettings)
        pset.set_editor_property('new_window_width', 1920)
        pset.set_editor_property('new_window_height', 1080)
        LES.editor_set_viewport_realtime(False)
        apply_case('a_baseline')
        set_shot('near')
        OUT['v1_actors'] = len(find_prefix('V1_'))
        return
    if not in_test_world():
        raise RuntimeError('run ACTION=build first; the editor world is %s' % world().get_path_name())
    if ACTION == 'case':
        apply_case(CASE)
    elif ACTION == 'shot':
        set_shot(SHOT)
    elif ACTION == 'report':
        mi_v1 = unreal.load_asset('%s/MI_V1Ice_%s' % (SCRATCH, VER))
        OUT['mi_v1'] = {k: MEL.get_material_instance_scalar_parameter_value(mi_v1, k) for k in ('IceAmbient', 'LineFix', 'Scratch')}
        OUT['ice_material'] = find('V1_Ice').get_editor_property('static_mesh_component').get_material(0).get_path_name()
        OUT['world'] = world().get_path_name()
        OUT['v1_actors'] = len(find_prefix('V1_'))


try:
    main()
except Exception as e:
    OUT['errors'].append('%s: %s' % (type(e).__name__, e))
OUT
