# V1-T3: the L3 tier in P2 style - wet neon with a reflection fallback ladder, against the P2-T3 "poster" variant
# (plan: thirdjam/briefs/B7-visual-upgrade-plan.md, direction D3, section 3)
#
# STATUS: written 2026-11-12 by lane V1, NOT RUN. Read it before you run it; fix what the first run reports.
#
# WHAT IT BUILDS (only under /JamAssets/_Scratch/V1T3/):
#   - L_V1T3: a DUPLICATE of /JamAssets/Rigs/LR_L2_Night opened as the editor world (the real rig and the empty LR_L3_Neon are not touched).
#   - Throwaway layout V1_* as in T1/T2, with the ice on a scratch M_V1IceNeon_<VER> (the recipe ice and B1's lines, plus
#     Wet, IceAmbient and a painted reflection term 'Streak': emissive bands near the boards in StreakNeg (x < 0) / StreakPos (x > 0)
#     colours, which the toon ramp turns into painted bands - a reflection with no reflection cost).
#   - Scratch trim instances of the real M_Trim: MI_V1Trim_Cyan / _Magenta (Emissive 60) and MI_V1Pickup (gold, Emissive 30);
#     three lying capsule pickups V1_Pickup_n (stencil 0) with a glow disc each.
#   - V1_Fog: ExponentialHeightFog, volumetric OFF (plain fog was 0.03 ms on the 4090 in E3; the volumetric spot was 0.3-0.5 ms).
#   - Cases change, in the COPY only: spot colours and intensity, PPV bloom / saturation / contrast / highlights / fringe / reflection
#     method, trim materials, ice parameters, pickups and fog visibility.
#
# CASES:
#   a_l2_ref        today's L2 look on this layout (the reference for every delta)
#   b_neon_lumen    D3b: cyan/magenta trim and tinted spots, Wet 1 (roughness 0.05), Lumen reflections (project default)
#   c_neon_ssr      as b, PPV reflection method = screen space
#   d_neon_painted  as b, Wet 0.3, reflection method = none, painted Streak 1
#   e_neon_fog      d + plain height fog
#   f_poster        D3a (STYLE-PATHS P2-T3): warm spots, saturated warm grade, bloom 1.4, dry ice, Streak 0.5 in the team trim colours, pickups
#
# HOW TO RUN: as v1_t1_ice_floor.py (build -> per case: case, then per shot: shot + PIE + measure + capture -> cleanup).
#   PIE world: /JamAssets/_Scratch/V1T3/UEDPIE_0_L_V1T3.L_V1T3. Shots: near, mid, far. GetFrameTimings x 60 and TriggerGPUProfile on 'mid'.
#
# WHAT TO MEASURE AND THE PASS LINES (device B profile = mid-range proxy, loosely; A's 4090 gives the gate number after a merge):
#   - read from the profiler per case: LumenReflections, ScreenSpaceReflections (or 'SSR'), Bloom / PostProcessing, HeightFog, and the sum
#     of the frame's Scene node. Deltas against a_l2_ref.
#   - D3b goes on only if c_neon_ssr OR d_neon_painted costs <= +1.0 ms over a_l2_ref on B, the puck halo and team colours stay separate
#     (a blue figure next to a cyan strip is the case to watch: crop both, |UAVG difference| >= 15), and the far beacon is findable.
#     b_neon_lumen is kept only if B's delta is < +2 ms AND A's 4090 later shows the L3 rig's lighting + post <= 5.5 ms (C-0002).
#   - D3a goes on if f_poster costs <= +0.6 ms over a_l2_ref and reads as "climax" next to a_l2_ref (operator, DP2).
#   Time box: 60 minutes.
#
# WHAT TO DELETE AFTERWARDS: /JamAssets/_Scratch/V1T3 (ACTION = 'cleanup'). Keepers -> devlog via devlog/shot.mjs.
#
# KNOWN UNCERTAINTIES (not run): duplicate_asset on a map; PPV names reflection_method / color_contrast / color_gain_highlights /
# scene_fringe_intensity and the enum unreal.ReflectionMethod (failures go to 'pp_failed'); fog property names (fog_inscattering_luminance
# in 5.1+, fog_inscattering_color before; both are tried); whether SSR alone looks acceptable on a 120 m plane (off-screen strips vanish).

import math
import os
import shutil

import unreal

# ------------------------------------------------------------------ parameters
ACTION = 'build'            # 'build' | 'case' | 'shot' | 'report' | 'cleanup'
CASE = 'a_l2_ref'
SHOT = 'mid'
VER = 'a'

SCRATCH = '/JamAssets/_Scratch/V1T3'
LEVEL = SCRATCH + '/L_V1T3'
RIG = '/JamAssets/Rigs/LR_L2_Night'
LOOK = '/JamAssets/Look'
RETURN_MAP = '/Game/Maps/Rink'
RIG_PREFIX = 'L2_Night'

RINK_HALF_LENGTH_M = 60.0
RINK_HALF_WIDTH_M = 30.0
BOARD_HEIGHT_M = 1.2

CAM_BACK_M, CAM_UP_M, CAM_PITCH, CAM_FOV = 6.0, 3.5, -17.65, 70.0
FIGURES_BLUE = [(-45.0, 0.0), (-3.0, 0.0), (40.0, 0.0), (-20.0, -27.5)]      # the last one stands next to a strip (blue vs cyan check)
FIGURES_ORANGE = [(-20.0, 6.0), (10.0, -8.0), (35.0, 12.0), (20.0, 27.5)]   # ... and orange vs magenta
PUCKS = {'V1_Puck_Lit': (-35.0, 0.6), 'V1_Puck_Dark': (-20.0, 17.0)}
PICKUPS = [(-25.0, -8.0), (5.0, 10.0), (28.0, -5.0)]
SHOTS = {
    'near': (-45.0, 0.0, 0.0, None),
    'mid': (-3.0, 0.0, 0.0, None),
    'far': (40.0, 0.0, 0.0, None),
}

SPOT_BASE = ((255, 244, 230), 20000.0)
SPOT_NEON = [((140, 220, 255), 16000.0), ((255, 140, 230), 16000.0)]   # alternate 1,3 / 2,4
SPOT_POSTER = ((255, 214, 160), 22000.0)
CYAN, MAGENTA = (0.10, 0.85, 1.0, 1.0), (1.0, 0.15, 0.75, 1.0)
TEAM_BLUE_TRIM, TEAM_ORANGE_TRIM = (0.05, 0.30, 1.0, 1.0), (1.0, 0.35, 0.05, 1.0)

DEFAULTS = {
    'spots': 'base', 'trim': 'team', 'pickups': False, 'fog': False,
    'Wet': 0.0, 'Streak': 0.0, 'IceAmbient': 0.0,
    'StreakNeg': TEAM_BLUE_TRIM, 'StreakPos': TEAM_ORANGE_TRIM,
    'refl': None,
    'pp': 'l2',
}
CASES = {
    'a_l2_ref': {},
    'b_neon_lumen': {'spots': 'neon', 'trim': 'neon', 'pickups': True, 'Wet': 1.0, 'IceAmbient': 0.15,
                     'StreakNeg': CYAN, 'StreakPos': MAGENTA, 'refl': 'lumen', 'pp': 'neon'},
    'c_neon_ssr': {'spots': 'neon', 'trim': 'neon', 'pickups': True, 'Wet': 1.0, 'IceAmbient': 0.15,
                   'StreakNeg': CYAN, 'StreakPos': MAGENTA, 'refl': 'ssr', 'pp': 'neon'},
    'd_neon_painted': {'spots': 'neon', 'trim': 'neon', 'pickups': True, 'Wet': 0.3, 'Streak': 1.0, 'IceAmbient': 0.15,
                       'StreakNeg': CYAN, 'StreakPos': MAGENTA, 'refl': 'none', 'pp': 'neon'},
    'e_neon_fog': {'spots': 'neon', 'trim': 'neon', 'pickups': True, 'Wet': 0.3, 'Streak': 1.0, 'IceAmbient': 0.15,
                   'StreakNeg': CYAN, 'StreakPos': MAGENTA, 'refl': 'none', 'pp': 'neon', 'fog': True},
    'f_poster': {'spots': 'poster', 'trim': 'team', 'pickups': True, 'Wet': 0.0, 'Streak': 0.5, 'IceAmbient': 0.2, 'pp': 'poster'},
}
PP = {
    'l2': {'bloom_intensity': 0.9, 'bloom_threshold': 1.0, 'color_saturation': (1.1, 1.1, 1.1, 1.0)},
    'neon': {'bloom_intensity': 1.6, 'bloom_threshold': 0.8, 'color_saturation': (1.25, 1.25, 1.25, 1.0),
             'color_contrast': (1.1, 1.1, 1.1, 1.0), 'scene_fringe_intensity': 0.4},
    'poster': {'bloom_intensity': 1.4, 'bloom_threshold': 0.9, 'color_saturation': (1.35, 1.35, 1.35, 1.0),
               'color_contrast': (1.05, 1.05, 1.05, 1.0), 'color_gain_highlights': (1.10, 1.0, 0.85, 1.0)},
}
PP_OPTIONAL = ('color_contrast', 'color_gain_highlights', 'scene_fringe_intensity')   # cleared when a case does not set them

# ------------------------------------------------------------------ helpers (same as v1_t1_ice_floor.py)
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
    unreal.log('V1T3: %s' % msg)


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
        return None
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
    return unreal.Rotator(roll=roll, pitch=pitch, yaw=yaw)


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


def find(label):
    for a in EAS.get_all_level_actors():
        if a.get_actor_label() == label:
            return a
    return None


def find_prefix(prefix):
    return [a for a in EAS.get_all_level_actors() if a.get_actor_label().startswith(prefix)]


def show(actors, visible):
    for a in actors:
        a.set_actor_hidden_in_game(not visible)
        a.set_is_temporarily_hidden_in_editor(not visible)


def pp_set(settings, key, value):
    try:
        if isinstance(value, tuple):
            value = unreal.Vector4(*value)
        settings.set_editor_property(key, value)
        settings.set_editor_property('override_' + key, True)
    except Exception as e:
        OUT['pp_failed'].append('%s: %s' % (key, e))


def pp_clear(settings, key):
    try:
        settings.set_editor_property('override_' + key, False)
    except Exception as e:
        OUT['pp_failed'].append('override_%s: %s' % (key, e))


# ------------------------------------------------------------------ level (same as T1; minimal-rig fallback omitted: T3 needs the real rig)
def open_test_level():
    dirty = list(unreal.EditorLoadingAndSavingUtils.get_dirty_map_packages()) + list(unreal.EditorLoadingAndSavingUtils.get_dirty_content_packages())
    if dirty:
        raise RuntimeError('dirty packages, save or discard them first: %s' % [p.get_name() for p in dirty])
    EAL.make_directory(SCRATCH)
    if not EAL.does_asset_exist(LEVEL):
        if EAL.duplicate_asset(RIG, LEVEL) is None:
            raise RuntimeError('duplicate_asset failed; run T1 or T2 first to see whether their minimal-rig fallback works, then port it here')
    if not LES.load_level(LEVEL):
        raise RuntimeError('load_level failed for %s' % LEVEL)
    if not in_test_world():
        raise RuntimeError('the editor world is not the scratch copy: %s' % world().get_path_name())


# ------------------------------------------------------------------ neon ice: recipe ice + Wet + IceAmbient + painted Streak
ICE_MASK = 'return saturate((N1 * 0.65 + N2 * 0.35 - 0.42) * 3.0) * Scuff;'
ICE_COLOUR = '''float3 c = lerp(IceColour, ScuffColour, M);
float ax = abs(P.x);
float inside = step(abs(P.y), HW);
float red = (step(ax, 30.0) + step(abs(ax - (HL - 700.0)), 6.0)) * inside;
float blue = (step(abs(ax - HL * 0.29), 30.0) + step(abs(length(P.xy) - 900.0), 8.0)) * inside;
c = lerp(c, LineRed, saturate(red));
c = lerp(c, LineBlue, saturate(blue));
return c;'''
ICE_ROUGH = 'return lerp(lerp(0.28, 0.75, M), 0.05, Wet);'
ICE_EMISSIVE = '''float inside = step(abs(P.y), HW);
float edge = saturate((abs(P.y) - (HW - Reach)) / max(Reach, 1.0));
float wav = 0.55 + 0.45 * sin(P.x * 0.004 + N1 * 6.0);
float3 col = P.x < 0.0 ? StreakNeg : StreakPos;
float3 streak = col * Streak * edge * edge * wav * inside;
return streak + AmbientColour * IceAmbient * (0.75 + 0.5 * N1);'''


def build_materials():
    res = {}
    path = '%s/M_V1IceNeon_%s' % (SCRATCH, VER)
    m = new_material(path)
    if m is None:
        res['ice'] = unreal.load_asset(path)
    else:
        wp = node(m, unreal.MaterialExpressionWorldPosition)
        n1 = node(m, unreal.MaterialExpressionNoise, scale=0.004, levels=4, output_min=0.0, output_max=1.0, turbulence=True, quality=1)
        n2 = node(m, unreal.MaterialExpressionNoise, scale=0.05, levels=2, output_min=0.0, output_max=1.0, turbulence=False, quality=1)
        P = {}
        for n, v in (('Scuff', 0.7), ('Wet', 0.0), ('RinkHalfLength', RINK_HALF_LENGTH_M * 100), ('RinkHalfWidth', RINK_HALF_WIDTH_M * 100),
                     ('Streak', 0.0), ('StreakReach', 900.0), ('IceAmbient', 0.0)):
            P[n] = sparam(m, n, v)
        for n, v in (('IceColour', (0.72, 0.80, 0.86, 1)), ('ScuffColour', (0.50, 0.56, 0.62, 1)), ('LineRed', (0.45, 0.04, 0.04, 1)),
                     ('LineBlue', (0.04, 0.10, 0.40, 1)), ('StreakNeg', TEAM_BLUE_TRIM), ('StreakPos', TEAM_ORANGE_TRIM),
                     ('AmbientColour', (0.25, 0.35, 0.65, 1))):
            P[n] = vparam(m, n, v)
        mask = custom(m, ICE_MASK, ['N1', 'N2', 'Scuff'], otype=unreal.CustomMaterialOutputType.CMOT_FLOAT1, desc='ScuffMask')
        wire(n1, mask, 'N1'); wire(n2, mask, 'N2'); wire(P['Scuff'], mask, 'Scuff')
        col = custom(m, ICE_COLOUR, ['P', 'M', 'HL', 'HW', 'IceColour', 'ScuffColour', 'LineRed', 'LineBlue'], desc='IceColour')
        wire(wp, col, 'P'); wire(mask, col, 'M'); wire(P['RinkHalfLength'], col, 'HL'); wire(P['RinkHalfWidth'], col, 'HW')
        for n in ('IceColour', 'ScuffColour', 'LineRed', 'LineBlue'):
            wire(P[n], col, n)
        rgh = custom(m, ICE_ROUGH, ['M', 'Wet'], otype=unreal.CustomMaterialOutputType.CMOT_FLOAT1, desc='Roughness')
        wire(mask, rgh, 'M'); wire(P['Wet'], rgh, 'Wet')
        emi = custom(m, ICE_EMISSIVE, ['P', 'N1', 'HW', 'Reach', 'Streak', 'StreakNeg', 'StreakPos', 'AmbientColour', 'IceAmbient'], desc='Streak+Ambient')
        wire(wp, emi, 'P'); wire(n1, emi, 'N1'); wire(P['RinkHalfWidth'], emi, 'HW'); wire(P['StreakReach'], emi, 'Reach')
        for n in ('Streak', 'StreakNeg', 'StreakPos', 'AmbientColour', 'IceAmbient'):
            wire(P[n], emi, n)
        toprop(col, MP.MP_BASE_COLOR); toprop(rgh, MP.MP_ROUGHNESS); toprop(const(m, 0.5), MP.MP_SPECULAR); toprop(emi, MP.MP_EMISSIVE_COLOR)
        MEL.layout_material_expressions(m)
        MEL.recompile_material(m)
        EAL.save_loaded_asset(m)
        errs = [str(e.get_name()) for e in MEL.get_material_error_expressions(m)]
        if errs:
            OUT['errors'].append('M_V1IceNeon errors: %s' % errs)
        res['ice'] = m
    res['mi_ice'] = make_mi('MI_V1IceNeon_%s' % VER, res['ice'])
    trim = look('M_Trim')
    res['cyan'] = make_mi('MI_V1Trim_Cyan', trim, vec={'TeamColour': CYAN}, scal={'Emissive': 60.0})
    res['magenta'] = make_mi('MI_V1Trim_Magenta', trim, vec={'TeamColour': MAGENTA}, scal={'Emissive': 60.0})
    res['pickup'] = make_mi('MI_V1Pickup', trim, vec={'TeamColour': (1.0, 0.75, 0.2, 1.0)}, scal={'Emissive': 30.0})
    res['pickup_glow'] = make_mi('MI_V1PickupGlow', trim, vec={'TeamColour': (1.0, 0.75, 0.2, 1.0)}, scal={'Emissive': 8.0})
    return res


def build_layout(res):
    hl, hw = RINK_HALF_LENGTH_M, RINK_HALF_WIDTH_M
    spawn_mesh('V1_Ice', 'Plane', (0, 0, 0), (hl * 2, hw * 2, 1), res['mi_ice'], stencil=2)
    board = look('MI_Toon_Board')
    for lbl, loc, sc3 in (('V1_Board_YN', (0, -hw - 0.1, BOARD_HEIGHT_M / 2), (hl * 2, 0.2, BOARD_HEIGHT_M)),
                          ('V1_Board_YP', (0, hw + 0.1, BOARD_HEIGHT_M / 2), (hl * 2, 0.2, BOARD_HEIGHT_M)),
                          ('V1_Board_XN', (-hl - 0.1, 0, BOARD_HEIGHT_M / 2), (0.2, hw * 2, BOARD_HEIGHT_M)),
                          ('V1_Board_XP', (hl + 0.1, 0, BOARD_HEIGHT_M / 2), (0.2, hw * 2, BOARD_HEIGHT_M))):
        spawn_mesh(lbl, 'Cube', loc, sc3, board, stencil=1, shadow=True)
    for side in (-1, 1):
        cx = side * hl / 2
        # trim labels carry the side: V1_Trim_N_* is x < 0, V1_Trim_P_* is x > 0
        tag = 'N' if side < 0 else 'P'
        mat = look('MI_Trim_Blue') if side < 0 else look('MI_Trim_Orange')
        spawn_mesh('V1_Trim_%s_YN' % tag, 'Cube', (cx, -hw - 0.1, BOARD_HEIGHT_M + 0.05), (hl, 0.1, 0.1), mat)
        spawn_mesh('V1_Trim_%s_YP' % tag, 'Cube', (cx, hw + 0.1, BOARD_HEIGHT_M + 0.05), (hl, 0.1, 0.1), mat)
        spawn_mesh('V1_Net_%d' % side, 'Cube', (side * 53.5, 0, 0.6), (1.1, 1.83, 1.2), look('MI_Toon_NetFrame'), stencil=1, shadow=True)
        spawn_mesh('V1_Beacon_%d' % side, 'Cylinder', (side * 55.5, 0, 4.0), (0.12, 0.12, 8.0), look('MI_Trim_Beacon'))
    for i, (x, y) in enumerate(FIGURES_BLUE + FIGURES_ORANGE):
        mat = look('MI_Toon_Blue') if i < len(FIGURES_BLUE) else look('MI_Toon_Orange')
        spawn_mesh('V1_Fig_%d_Body' % i, 'Cylinder', (x, y, 0.6), (0.6, 0.6, 1.2), mat, stencil=1, shadow=True)
        spawn_mesh('V1_Fig_%d_Head' % i, 'Sphere', (x, y, 1.42), (0.35, 0.35, 0.35), look('MI_Toon_Helmet'), stencil=1, shadow=True)
    for lbl, (x, y) in PUCKS.items():
        spawn_mesh(lbl, 'Cylinder', (x, y, 0.0125), (0.12, 0.12, 0.025), look('MI_Toon_Puck'), stencil=3, shadow=True)
    for i, (x, y) in enumerate(PICKUPS, 1):
        spawn_mesh('V1_Pickup_%d' % i, 'Sphere', (x, y, 0.3), (0.5, 0.5, 1.2), res['pickup'], r=rot(pitch=90.0, yaw=30.0 * i))
        spawn_mesh('V1_Pickup_%d_Glow' % i, 'Cylinder', (x, y, 0.004), (1.8, 1.8, 0.008), res['pickup_glow'])
    fog = EAS.spawn_actor_from_class(unreal.ExponentialHeightFog, unreal.Vector(0, 0, 0))
    fog.set_actor_label('V1_Fog')
    fog.set_folder_path('V1')
    fc = fog.get_editor_property('component')
    for k, v in (('fog_density', 0.0), ('fog_height_falloff', 0.35), ('enable_volumetric_fog', False)):
        try:
            fc.set_editor_property(k, v)
        except Exception as e:
            OUT['errors'].append('fog %s: %s' % (k, e))
    for k in ('fog_inscattering_luminance', 'fog_inscattering_color'):
        try:
            fc.set_editor_property(k, unreal.LinearColor(0.20, 0.10, 0.35, 1.0))
            OUT['fog_colour_property'] = k
            break
        except Exception:
            pass
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
        OUT['errors'].append('game mode override failed: %s' % e)


def set_shot(name):
    fx, fy, yaw, pitch = SHOTS[name]
    cam = find('V1_Camera')
    rad = math.radians(yaw)
    loc = unreal.Vector((fx - CAM_BACK_M * math.cos(rad)) * 100, (fy - CAM_BACK_M * math.sin(rad) - 0.7) * 100, CAM_UP_M * 100)
    cam.set_actor_location(loc, False, False)
    cam.set_actor_rotation(rot(pitch=CAM_PITCH if pitch is None else pitch, yaw=yaw), False)


def set_spot(actor, colour, candelas):
    lc = actor.get_editor_property('spot_light_component')
    lc.set_editor_property('light_color', unreal.Color(r=colour[0], g=colour[1], b=colour[2], a=255))
    lc.set_intensity(candelas)


def apply_case(name):
    c = dict(DEFAULTS)
    c.update(CASES[name])
    spots = sorted(find_prefix('Spot_' + RIG_PREFIX + '_'), key=lambda a: a.get_actor_label())
    for i, s in enumerate(spots):
        if c['spots'] == 'neon':
            set_spot(s, *SPOT_NEON[i % 2])
        elif c['spots'] == 'poster':
            set_spot(s, *SPOT_POSTER)
        else:
            set_spot(s, *SPOT_BASE)
    neon = c['trim'] == 'neon'
    cyan = unreal.load_asset(SCRATCH + '/MI_V1Trim_Cyan')
    magenta = unreal.load_asset(SCRATCH + '/MI_V1Trim_Magenta')
    for a in find_prefix('V1_Trim_N_'):
        a.get_editor_property('static_mesh_component').set_material(0, cyan if neon else look('MI_Trim_Blue'))
    for a in find_prefix('V1_Trim_P_'):
        a.get_editor_property('static_mesh_component').set_material(0, magenta if neon else look('MI_Trim_Orange'))
    show(find_prefix('V1_Pickup_'), c['pickups'])
    fog = find('V1_Fog')
    fog.get_editor_property('component').set_editor_property('fog_density', 0.02 if c['fog'] else 0.0)
    show([fog], c['fog'])
    mi = unreal.load_asset('%s/MI_V1IceNeon_%s' % (SCRATCH, VER))
    for k in ('Wet', 'Streak', 'IceAmbient'):
        MEL.set_material_instance_scalar_parameter_value(mi, k, c[k])
    for k in ('StreakNeg', 'StreakPos'):
        MEL.set_material_instance_vector_parameter_value(mi, k, unreal.LinearColor(*c[k]))
    MEL.update_material_instance(mi)
    EAL.save_loaded_asset(mi)
    ppv = find('PPV_' + RIG_PREFIX)
    s = ppv.get_editor_property('settings')
    values = PP[c['pp']]
    for k, v in values.items():
        pp_set(s, k, v)
    for k in PP_OPTIONAL:
        if k not in values:
            pp_clear(s, k)
    if c['refl'] is None:
        pp_clear(s, 'reflection_method')
    else:
        RM = getattr(unreal, 'ReflectionMethod', None)
        if RM is None:
            OUT['pp_failed'].append('unreal.ReflectionMethod not found')
        else:
            pp_set(s, 'reflection_method', {'lumen': RM.LUMEN, 'ssr': RM.SCREEN_SPACE, 'none': RM.NONE}[c['refl']])
    ppv.set_editor_property('settings', s)
    OUT['case'] = name
    OUT['case_values'] = {k: (list(v) if isinstance(v, tuple) else v) for k, v in c.items()}


def cleanup():
    if pie_running():
        raise RuntimeError('stop PIE first')
    if in_test_world():
        EAL.save_directory(SCRATCH, only_if_is_dirty=True, recursive=True)
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
    unreal.AssetRegistryHelpers.get_asset_registry().scan_paths_synchronous(['/JamAssets'], True)
    OUT['scratch_left_on_disk'] = os.path.isdir(parent)


def main():
    if ACTION == 'cleanup':
        cleanup()
        return
    if pie_running():
        raise RuntimeError('stop PIE before %s' % ACTION)
    if ACTION == 'build':
        open_test_level()
        res = build_materials()
        if find('V1_Ice') is None:
            build_layout(res)
        pset = unreal.get_default_object(unreal.LevelEditorPlaySettings)
        pset.set_editor_property('new_window_width', 1920)
        pset.set_editor_property('new_window_height', 1080)
        LES.editor_set_viewport_realtime(False)
        apply_case('a_l2_ref')
        set_shot('mid')
        OUT['v1_actors'] = len(find_prefix('V1_'))
        return
    if not in_test_world():
        raise RuntimeError('run ACTION=build first; the editor world is %s' % world().get_path_name())
    if ACTION == 'case':
        apply_case(CASE)
    elif ACTION == 'shot':
        set_shot(SHOT)
    elif ACTION == 'report':
        OUT['world'] = world().get_path_name()
        OUT['v1_actors'] = len(find_prefix('V1_'))


try:
    main()
except Exception as e:
    OUT['errors'].append('%s: %s' % (type(e).__name__, e))
OUT
