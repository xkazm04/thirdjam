# V1-T2: stage light and atmosphere - light-shaft cones, depth haze, painted backdrop flats, lens and film grain
# (plan: thirdjam/briefs/B7-visual-upgrade-plan.md, direction D1, section 3)
#
# STATUS: written 2026-11-12 by lane V1, NOT RUN. Read it before you run it; fix what the first run reports.
#
# WHAT IT BUILDS (only under /JamAssets/_Scratch/V1T2/):
#   - L_V1T2: a DUPLICATE of /JamAssets/Rigs/LR_L2_Night opened as the editor world (the real rig is only read; minimal rig if the copy fails).
#   - Throwaway layout V1_*: ice (real MI_Ice_Night, stencil 2), boards + trim, nets, beacons, figures, pucks, PlayerStart, CameraActor.
#   - M_V1Shaft_<VER> / MI_V1Shaft_<VER>: unlit, additive, two-sided; colour x intensity x facing^p x height fade x depth fade.
#     One basic Cone per rig spot (V1_Shaft_n), sized from that spot's height and outer cone angle, no shadow, no custom depth.
#   - M_V1PPHaze_<VER> / MI_V1PPHaze_<VER>: post material (Before DOF): sc * (1 - h * Absorb) + HazeColour * h, h from scene depth.
#     Added as a third blendable to the COPY's PPV_L2_Night only; HazeOn = 0 switches it off.
#   - M_V1Backdrop_<VER> / MI_V1Backdrop_<VER>: unlit opaque procedural painted night sky (gradient, mountain ridge, sparse stars), no texture.
#     Two flats (V1_Flat_XN / V1_Flat_XP), 80 x 24 m, just inside the end walls at |x| = 81.6 m, each with a steel frame of 4 cubes
#     (MI_Dress_Steel, stencil 1): a theatre flat on a sound stage. Cases hide or show them.
#   - Lens and film: the COPY's PPV film_grain_intensity, scene_fringe_intensity, lens_flare_intensity / threshold (case e, f).
#
# HOW TO RUN: exactly as v1_t1_ice_floor.py (ACTION build -> per CASE: case, then per SHOT: shot + PIE + measure + capture -> cleanup).
#   PIE world for CaptureGameViewport: /JamAssets/_Scratch/V1T2/UEDPIE_0_L_V1T2.L_V1T2 (check with ListWorlds). Shots: near (camera INSIDE
#   a cone), side (camera OUTSIDE the cones), mid, far. GetFrameTimings x 60 and TriggerGPUProfile on 'mid' for every case.
#
# WHAT TO MEASURE AND THE PASS LINES (device B numbers; the gate number comes from A's 4090 after the merge):
#   - shafts (b, b2): cones visible in 'side' and 'mid'; no pop between 'near' (inside) and 'side' (outside); far orange figure crop
#     UAVG/VAVG shift < 10 levels against a_baseline (the volumetric fog of E3 failed exactly this); profiler 'Translucency' <= 0.3 ms.
#     Also look: are the cones stepped by the toon ramp or edged by the outline? If yes, translucency renders BEFORE the post passes
#     (OUT['shaft_translucency_pass'] reports the material setting) - write that down, it is open question 3 of the plan.
#   - haze (c, c2): its PostProcessMaterial pass <= 0.05 ms; far-end crop YAVG +10..+30 over a_baseline; the beacon stays the brightest
#     thing at the far end; the near figure unchanged.
#   - backdrop (d, d2): flat crop YAVG 25..70 in 'near'; beacon >= 100 levels above the flat behind it; reads as "painted flat", not as sky.
#   - lens/film (e): <= 0.1 ms; taste only (the operator decides at DP2).
#   - all (f): <= +0.5 ms over a_baseline in the profiler sum of the passes above.
#   Time box: 45 minutes. If the cone is upside down in the first still, set CONE_APEX_UP = False, ACTION = 'rebuild_shafts', and go on.
#
# WHAT TO DELETE AFTERWARDS: /JamAssets/_Scratch/V1T2 (ACTION = 'cleanup'). Nothing else is created. Keepers -> devlog via devlog/shot.mjs.
#
# KNOWN UNCERTAINTIES (not run): duplicate_asset on a map; the basic Cone's pivot and apex direction (bounds are read, the flip is a constant);
# PPV property names film_grain_intensity / scene_fringe_intensity / lens_flare_* (failures land in 'pp_failed'); Material property names
# translucency_pass / blendable_priority (guarded); whether the haze colour needs scaling for pre-exposure (tune HazeColour by eye).

import math
import os
import shutil

import unreal

# ------------------------------------------------------------------ parameters
ACTION = 'build'            # 'build' | 'case' | 'shot' | 'report' | 'rebuild_shafts' | 'cleanup'
CASE = 'a_baseline'
SHOT = 'mid'
VER = 'a'

SCRATCH = '/JamAssets/_Scratch/V1T2'
LEVEL = SCRATCH + '/L_V1T2'
RIG = '/JamAssets/Rigs/LR_L2_Night'
LOOK = '/JamAssets/Look'
ARENA = '/JamAssets/Arena'
RETURN_MAP = '/Game/Maps/Rink'
RIG_PREFIX = 'L2_Night'

RINK_HALF_LENGTH_M = 60.0
RINK_HALF_WIDTH_M = 30.0
BOARD_HEIGHT_M = 1.2

CONE_APEX_UP = True          # assumption about /Engine/BasicShapes/Cone; flip if the first still shows it upside down
SHAFT_COLOUR = (1.0, 0.93, 0.80, 1.0)
FLAT_X_M = 81.6              # room inner faces at |x| = 82 m (B1); B2's cases stand at |x| ~ 80 m, in front of the flat
FLAT_HALF_WIDTH_M = 40.0
FLAT_HEIGHT_M = 24.0

CAM_BACK_M, CAM_UP_M, CAM_PITCH, CAM_FOV = 6.0, 3.5, -17.65, 70.0
FIGURES_BLUE = [(-45.0, 0.0), (-38.0, 16.0), (-3.0, 0.0), (40.0, 0.0)]
FIGURES_ORANGE = [(-20.0, 6.0), (10.0, -8.0), (35.0, 12.0), (48.0, -4.0)]
PUCKS = {'V1_Puck_Lit': (-35.0, 0.6), 'V1_Puck_Dark': (-20.0, 17.0)}
SHOTS = {
    'near': (-45.0, 0.0, 0.0, None),     # camera inside the cone of spot 1
    'side': (-38.0, 16.0, 0.0, None),    # camera outside every cone, looking along the dark band
    'mid': (-3.0, 0.0, 0.0, None),
    'far': (40.0, 0.0, 0.0, None),       # far net, beacon, flat behind it
}

DEFAULTS = {
    'shafts': False, 'ShaftIntensity': 0.6,
    'HazeOn': 0.0, 'HazeMax': 0.5,
    'backdrop': False, 'Brightness': 5.0,
    'lens': False,
}
CASES = {
    'a_baseline': {},
    'b_shafts': {'shafts': True},
    'b2_shafts_strong': {'shafts': True, 'ShaftIntensity': 1.2},
    'c_haze': {'HazeOn': 1.0},
    'c2_haze_strong': {'HazeOn': 1.0, 'HazeMax': 0.9},
    'd_backdrop': {'backdrop': True},
    'd2_backdrop_bright': {'backdrop': True, 'Brightness': 10.0},
    'e_lens': {'lens': True},
    'f_all': {'shafts': True, 'HazeOn': 1.0, 'backdrop': True, 'lens': True},
}
LENS_ON = {'film_grain_intensity': 0.3, 'scene_fringe_intensity': 0.5, 'lens_flare_intensity': 0.4, 'lens_flare_threshold': 6.0}

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
    unreal.log('V1T2: %s' % msg)


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


def finish(m, label):
    MEL.layout_material_expressions(m)
    MEL.recompile_material(m)
    EAL.save_loaded_asset(m)
    errs = [str(e.get_name()) for e in MEL.get_material_error_expressions(m)]
    if errs:
        OUT['errors'].append('%s errors: %s' % (label, errs))


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


def load(folder, name):
    a = unreal.load_asset('%s/%s.%s' % (folder, name, name))
    if a is None:
        OUT['errors'].append('missing asset %s/%s' % (folder, name))
    return a


def look(name):
    return load(LOOK, name)


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
        settings.set_editor_property(key, value)
        settings.set_editor_property('override_' + key, True)
    except Exception as e:
        OUT['pp_failed'].append('%s: %s' % (key, e))


def pp_clear(settings, key):
    try:
        settings.set_editor_property('override_' + key, False)
    except Exception as e:
        OUT['pp_failed'].append('override_%s: %s' % (key, e))


# ------------------------------------------------------------------ level, minimal rig, layout (same as T1)
def open_test_level():
    dirty = list(unreal.EditorLoadingAndSavingUtils.get_dirty_map_packages()) + list(unreal.EditorLoadingAndSavingUtils.get_dirty_content_packages())
    if dirty:
        raise RuntimeError('dirty packages, save or discard them first: %s' % [p.get_name() for p in dirty])
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
    if not fallback and not LES.load_level(LEVEL):
        raise RuntimeError('load_level failed for %s' % LEVEL)
    if not in_test_world():
        raise RuntimeError('the editor world is not the scratch copy: %s' % world().get_path_name())
    OUT['fallback_rig'] = fallback
    if fallback:
        build_minimal_rig()


def build_minimal_rig():
    sky = EAS.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 2000))
    sky.set_actor_label('Sky_' + RIG_PREFIX)
    sc = sky.get_editor_property('light_component')
    sc.set_mobility(unreal.ComponentMobility.MOVABLE)
    sc.set_editor_property('source_type', unreal.SkyLightSourceType.SLS_SPECIFIED_CUBEMAP)
    sc.set_editor_property('cubemap', unreal.load_asset('/Engine/MapTemplates/Sky/DaylightAmbientCubemap'))
    sc.set_intensity(2.0)
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
    spawn_mesh('V1_Ice', 'Plane', (0, 0, 0), (hl * 2, hw * 2, 1), look('MI_Ice_Night'), stencil=2)
    board = look('MI_Toon_Board')
    for lbl, loc, sc3 in (('V1_Board_YN', (0, -hw - 0.1, BOARD_HEIGHT_M / 2), (hl * 2, 0.2, BOARD_HEIGHT_M)),
                          ('V1_Board_YP', (0, hw + 0.1, BOARD_HEIGHT_M / 2), (hl * 2, 0.2, BOARD_HEIGHT_M)),
                          ('V1_Board_XN', (-hl - 0.1, 0, BOARD_HEIGHT_M / 2), (0.2, hw * 2, BOARD_HEIGHT_M)),
                          ('V1_Board_XP', (hl + 0.1, 0, BOARD_HEIGHT_M / 2), (0.2, hw * 2, BOARD_HEIGHT_M))):
        spawn_mesh(lbl, 'Cube', loc, sc3, board, stencil=1, shadow=True)
    for side in (-1, 1):
        mat = look('MI_Trim_Blue') if side < 0 else look('MI_Trim_Orange')
        cx = side * hl / 2
        spawn_mesh('V1_Trim_YN_%d' % side, 'Cube', (cx, -hw - 0.1, BOARD_HEIGHT_M + 0.05), (hl, 0.1, 0.1), mat)
        spawn_mesh('V1_Trim_YP_%d' % side, 'Cube', (cx, hw + 0.1, BOARD_HEIGHT_M + 0.05), (hl, 0.1, 0.1), mat)
        spawn_mesh('V1_Net_%d' % side, 'Cube', (side * 53.5, 0, 0.6), (1.1, 1.83, 1.2), look('MI_Toon_NetFrame'), stencil=1, shadow=True)
        spawn_mesh('V1_Beacon_%d' % side, 'Cylinder', (side * 55.5, 0, 4.0), (0.12, 0.12, 8.0), look('MI_Trim_Beacon'))
    for i, (x, y) in enumerate(FIGURES_BLUE + FIGURES_ORANGE):
        mat = look('MI_Toon_Blue') if i < len(FIGURES_BLUE) else look('MI_Toon_Orange')
        spawn_mesh('V1_Fig_%d_Body' % i, 'Cylinder', (x, y, 0.6), (0.6, 0.6, 1.2), mat, stencil=1, shadow=True)
        spawn_mesh('V1_Fig_%d_Head' % i, 'Sphere', (x, y, 1.42), (0.35, 0.35, 0.35), look('MI_Toon_Helmet'), stencil=1, shadow=True)
    for lbl, (x, y) in PUCKS.items():
        spawn_mesh(lbl, 'Cylinder', (x, y, 0.0125), (0.12, 0.12, 0.025), look('MI_Toon_Puck'), stencil=3, shadow=True)
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


# ------------------------------------------------------------------ the three materials
SHAFT = '''float facing = pow(saturate(abs(dot(normalize(N), normalize(V)))), FacingPow);
float z = P.z;
float h = saturate(z / max(FadeIn, 1.0)) * saturate((TopZ - z) / 300.0) * lerp(0.6, 1.0, saturate(z / max(TopZ, 1.0)));
return ShaftColour * Intensity * facing * h * DF;'''

HAZE = '''float3 sc = SC.rgb;
if (HazeOn < 0.5) return sc;
float d = SceneTextureLookup(GetDefaultSceneTextureUV(Parameters, 1), 1, false).r;
float h = saturate(1.0 - exp(-max(d - HazeStart, 0.0) * HazeDensity)) * HazeMax;
return sc * (1.0 - h * HazeAbsorb) + HazeColour * h;'''

BACKDROP = '''float u = P.y / 100.0;
float z = P.z / 100.0;
float3 sky = lerp(SkyLow, SkyHigh, saturate(z / FlatHeight));
sky *= 1.0 + 0.06 * sin(u * 0.9 + z * 2.3) * sin(u * 0.37 - z * 1.1);
float ridge = 7.0 + 2.2 * sin(u * 0.11 + 1.3) + 1.4 * sin(u * 0.27 + 0.4) + 0.6 * sin(u * 0.71 + 2.0);
float m = step(z, ridge);
float3 c = lerp(sky, RidgeColour, m);
float2 g = floor(float2(u, z) * 1.5);
float hsh = frac(sin(dot(g, float2(12.9898, 78.233))) * 43758.5453);
float2 f = frac(float2(u, z) * 1.5) - 0.5;
float star = step(0.985, hsh) * (1.0 - smoothstep(0.05, 0.12, length(f))) * (1.0 - m);
c += StarColour * star;
return c * Brightness;'''


def build_materials():
    res = {}
    # light shaft
    path = '%s/M_V1Shaft_%s' % (SCRATCH, VER)
    m = new_material(path)
    if m is None:
        res['shaft'] = unreal.load_asset(path)
    else:
        m.set_editor_property('blend_mode', unreal.BlendMode.BLEND_ADDITIVE)
        m.set_editor_property('shading_model', unreal.MaterialShadingModel.MSM_UNLIT)
        m.set_editor_property('two_sided', True)
        n = node(m, unreal.MaterialExpressionVertexNormalWS)
        v = node(m, unreal.MaterialExpressionCameraVectorWS)
        wp = node(m, unreal.MaterialExpressionWorldPosition)
        df = node(m, unreal.MaterialExpressionDepthFade)
        try:
            df.set_editor_property('fade_distance_default', 300.0)
            df.set_editor_property('opacity_default', 1.0)
        except Exception as e:
            OUT['errors'].append('DepthFade defaults: %s' % e)
        P = {'ShaftColour': vparam(m, 'ShaftColour', SHAFT_COLOUR)}
        for k, val in (('Intensity', 0.6), ('FacingPow', 1.5), ('FadeIn', 400.0), ('TopZ', 2500.0)):
            P[k] = sparam(m, k, val)
        c = custom(m, SHAFT, ['N', 'V', 'P', 'DF', 'ShaftColour', 'Intensity', 'FacingPow', 'FadeIn', 'TopZ'], desc='Shaft')
        wire(n, c, 'N'); wire(v, c, 'V'); wire(wp, c, 'P'); wire(df, c, 'DF')
        for k in P:
            wire(P[k], c, k)
        toprop(c, MP.MP_EMISSIVE_COLOR)
        finish(m, 'M_V1Shaft')
        res['shaft'] = m
    try:
        OUT['shaft_translucency_pass'] = str(res['shaft'].get_editor_property('translucency_pass'))
    except Exception as e:
        OUT['shaft_translucency_pass'] = 'unknown: %s' % e
    # depth haze (post)
    path = '%s/M_V1PPHaze_%s' % (SCRATCH, VER)
    m = new_material(path)
    if m is None:
        res['haze'] = unreal.load_asset(path)
    else:
        m.set_editor_property('material_domain', unreal.MaterialDomain.MD_POST_PROCESS)
        m.set_editor_property('blendable_location', unreal.BlendableLocation.BL_SCENE_COLOR_BEFORE_DOF)
        try:
            m.set_editor_property('blendable_priority', 10)   # after ramp and outline (unverified which way priority sorts)
        except Exception as e:
            log('blendable_priority not set: %s' % e)
        st = node(m, unreal.MaterialExpressionSceneTexture, scene_texture_id=unreal.SceneTextureId.PPI_POST_PROCESS_INPUT0)
        P = {'HazeColour': vparam(m, 'HazeColour', (0.22, 0.30, 0.55, 1.0))}
        for k, val in (('HazeOn', 0.0), ('HazeStart', 1500.0), ('HazeDensity', 0.00004), ('HazeMax', 0.5), ('HazeAbsorb', 0.3)):
            P[k] = sparam(m, k, val)
        c = custom(m, HAZE, ['SC', 'HazeColour', 'HazeOn', 'HazeStart', 'HazeDensity', 'HazeMax', 'HazeAbsorb'], desc='Haze')
        wire(st, c, 'SC', 'Color')
        for k in P:
            wire(P[k], c, k)
        toprop(c, MP.MP_EMISSIVE_COLOR)
        finish(m, 'M_V1PPHaze')
        res['haze'] = m
    # painted backdrop flat
    path = '%s/M_V1Backdrop_%s' % (SCRATCH, VER)
    m = new_material(path)
    if m is None:
        res['backdrop'] = unreal.load_asset(path)
    else:
        m.set_editor_property('shading_model', unreal.MaterialShadingModel.MSM_UNLIT)
        m.set_editor_property('two_sided', True)
        wp = node(m, unreal.MaterialExpressionWorldPosition)
        P = {}
        for k, val in (('SkyLow', (0.10, 0.14, 0.32, 1)), ('SkyHigh', (0.025, 0.035, 0.09, 1)), ('RidgeColour', (0.012, 0.016, 0.04, 1)),
                       ('StarColour', (0.9, 0.85, 0.7, 1))):
            P[k] = vparam(m, k, val)
        for k, val in (('Brightness', 5.0), ('FlatHeight', FLAT_HEIGHT_M)):
            P[k] = sparam(m, k, val)
        c = custom(m, BACKDROP, ['P', 'SkyLow', 'SkyHigh', 'RidgeColour', 'StarColour', 'Brightness', 'FlatHeight'], desc='Backdrop')
        wire(wp, c, 'P')
        for k in P:
            wire(P[k], c, k)
        toprop(c, MP.MP_EMISSIVE_COLOR)
        finish(m, 'M_V1Backdrop')
        res['backdrop'] = m
    res['mi_shaft'] = make_mi('MI_V1Shaft_%s' % VER, res['shaft'])
    res['mi_haze'] = make_mi('MI_V1PPHaze_%s' % VER, res['haze'])
    res['mi_backdrop'] = make_mi('MI_V1Backdrop_%s' % VER, res['backdrop'])
    return res


def build_shafts(mi_shaft):
    for a in find_prefix('V1_Shaft_'):
        EAS.destroy_actor(a)
    cone = shape('Cone')
    b = cone.get_bounds()
    oz, ez = b.origin.z, b.box_extent.z
    rx = max(b.box_extent.x, b.box_extent.y)
    OUT['cone_bounds'] = {'origin_z': oz, 'extent_z': ez, 'radius': rx}
    spots = sorted(find_prefix('Spot_' + RIG_PREFIX + '_'), key=lambda a: a.get_actor_label())
    for i, s in enumerate(spots, 1):
        loc = s.get_actor_location()
        lc = s.get_editor_property('spot_light_component')
        outer = lc.get_editor_property('outer_cone_angle')
        h = loc.z                                  # spot height above the ice (ice top at z = 0)
        r = math.tan(math.radians(outer)) * h
        sx = r / rx
        sz = h / (2.0 * ez)
        if CONE_APEX_UP:
            z = -(oz - ez) * sz                    # mesh bottom on the ice
            rr = rot()
        else:
            z = (oz + ez) * sz                     # mesh flipped: its top becomes the bottom
            rr = rot(roll=180.0)
        a = EAS.spawn_actor_from_object(cone, unreal.Vector(loc.x, loc.y, z), rr)
        a.set_actor_label('V1_Shaft_%d' % i)
        a.set_folder_path('V1')
        a.set_actor_scale3d(unreal.Vector(sx, sx, sz))
        c = a.get_editor_property('static_mesh_component')
        c.set_material(0, mi_shaft)
        c.set_cast_shadow(False)
        c.set_collision_profile_name('NoCollision')
        log('shaft %d: height %.0f cm, radius %.0f cm, outer %.1f deg' % (i, h, r, outer))
    MEL.set_material_instance_scalar_parameter_value(mi_shaft, 'TopZ', max([s.get_actor_location().z for s in spots] or [2500.0]))
    MEL.update_material_instance(mi_shaft)


def build_flats(mi_backdrop):
    steel = load(ARENA, 'MI_Dress_Steel')
    for side, tag in ((-1, 'XN'), (1, 'XP')):
        x = side * FLAT_X_M
        spawn_mesh('V1_Flat_%s' % tag, 'Plane', (x, 0, FLAT_HEIGHT_M / 2), (FLAT_HEIGHT_M, FLAT_HALF_WIDTH_M * 2, 1), mi_backdrop,
                   r=rot(pitch=90.0))
        fx = x - side * 0.3                        # frame just in front of the flat
        for lbl, loc, sc3 in (('L', (fx, -FLAT_HALF_WIDTH_M, FLAT_HEIGHT_M / 2), (0.3, 0.3, FLAT_HEIGHT_M + 0.6)),
                              ('R', (fx, FLAT_HALF_WIDTH_M, FLAT_HEIGHT_M / 2), (0.3, 0.3, FLAT_HEIGHT_M + 0.6)),
                              ('T', (fx, 0, FLAT_HEIGHT_M + 0.15), (0.3, FLAT_HALF_WIDTH_M * 2, 0.3)),
                              ('B', (fx, 0, 0.15), (0.3, FLAT_HALF_WIDTH_M * 2, 0.3))):
            spawn_mesh('V1_FlatFrame_%s_%s' % (tag, lbl), 'Cube', loc, sc3, steel, stencil=1)


def add_haze_blendable(mi_haze):
    ppv = find('PPV_' + RIG_PREFIX)
    s = ppv.get_editor_property('settings')
    wb = s.get_editor_property('weighted_blendables')
    arr = list(wb.get_editor_property('array'))
    if not any(b.get_editor_property('object') == mi_haze for b in arr):
        arr.append(unreal.WeightedBlendable(weight=1.0, object=mi_haze))
    s.set_editor_property('weighted_blendables', unreal.WeightedBlendables(array=arr))
    ppv.set_editor_property('settings', s)
    OUT['blendables'] = [b.get_editor_property('object').get_name() for b in arr if b.get_editor_property('object')]


def apply_case(name):
    c = dict(DEFAULTS)
    c.update(CASES[name])
    mi_shaft = unreal.load_asset('%s/MI_V1Shaft_%s' % (SCRATCH, VER))
    mi_haze = unreal.load_asset('%s/MI_V1PPHaze_%s' % (SCRATCH, VER))
    mi_bd = unreal.load_asset('%s/MI_V1Backdrop_%s' % (SCRATCH, VER))
    MEL.set_material_instance_scalar_parameter_value(mi_shaft, 'Intensity', c['ShaftIntensity'])
    MEL.set_material_instance_scalar_parameter_value(mi_haze, 'HazeOn', c['HazeOn'])
    MEL.set_material_instance_scalar_parameter_value(mi_haze, 'HazeMax', c['HazeMax'])
    MEL.set_material_instance_scalar_parameter_value(mi_bd, 'Brightness', c['Brightness'])
    for mi in (mi_shaft, mi_haze, mi_bd):
        MEL.update_material_instance(mi)
        EAL.save_loaded_asset(mi)
    show(find_prefix('V1_Shaft_'), c['shafts'])
    show(find_prefix('V1_Flat'), c['backdrop'])
    ppv = find('PPV_' + RIG_PREFIX)
    s = ppv.get_editor_property('settings')
    for k, v in LENS_ON.items():
        if c['lens']:
            pp_set(s, k, v)
        else:
            pp_clear(s, k)
    ppv.set_editor_property('settings', s)
    OUT['case'] = name
    OUT['case_values'] = c


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
            build_layout()
            build_shafts(res['mi_shaft'])
            build_flats(res['mi_backdrop'])
            add_haze_blendable(res['mi_haze'])
        pset = unreal.get_default_object(unreal.LevelEditorPlaySettings)
        pset.set_editor_property('new_window_width', 1920)
        pset.set_editor_property('new_window_height', 1080)
        LES.editor_set_viewport_realtime(False)
        apply_case('a_baseline')
        set_shot('mid')
        OUT['v1_actors'] = len(find_prefix('V1_'))
        return
    if not in_test_world():
        raise RuntimeError('run ACTION=build first; the editor world is %s' % world().get_path_name())
    if ACTION == 'case':
        apply_case(CASE)
    elif ACTION == 'shot':
        set_shot(SHOT)
    elif ACTION == 'rebuild_shafts':
        build_shafts(unreal.load_asset('%s/MI_V1Shaft_%s' % (SCRATCH, VER)))
        apply_case(CASE)
    elif ACTION == 'report':
        OUT['world'] = world().get_path_name()
        OUT['v1_actors'] = len(find_prefix('V1_'))
        OUT['shafts'] = [a.get_actor_label() for a in find_prefix('V1_Shaft_')]


try:
    main()
except Exception as e:
    OUT['errors'].append('%s: %s' % (type(e).__name__, e))
OUT
