---
name: look-painted-cel
description: Build the P2 painted-cel look at tier T2 (night arena, L2 "indoor, spotlights on") in the jam project - toon ramp and outline as two post-process materials over Lumen, the ice, trim and toon master materials with TeamColour/Emissive/Wet/Scuff, the LR_L2_Night rig (4 shadowed spots, low cool sky, bloom, fixed exposure, cool grade), the third-person camera and the puck/far-net readability kit. Use on device B from H1 once contracts/look-tiers.md exists.
track: look, device B (H1 onward; the L2 rig is due at H48, but the materials and both post passes are T1 work)
evidence: docs/experiments/LOOK-P2-NIGHT.md (built and measured once on a throwaway, UE 5.9.0-60351404, RTX 4090, 2026-11-12)
---

# Recipe: P2 painted cel, night arena (LR_L2_Night)

Everything below was executed by an agent through MCP (`execute_python` plus a handful of toolset calls) on a throwaway project of engine basic
shapes, with no human in the loop. The scripts are given in full; run them in this order, then verify with section 9. Numbers (light values,
exposure, thresholds) are the ones that produced the screenshots in `docs/experiments/LOOK-P2-NIGHT.md`; tune them, do not rename them.

## 0. Before you start (5 minutes)

- `contracts/look-tiers.md` names the rig assets (`LR_L1_Day`, `LR_L2_Night`, `LR_L3_Neon`), the gameplay-driven material parameters
  (`TeamColour`, `Emissive`, `Wet`, `Scuff`), the fog and post volume names and the frame budget. **It did not exist when this recipe was written**;
  the names here come from `docs/TRACKS.md` section 3. If the contract says otherwise, the contract wins: rename, keep the graphs.
- Asset prefixes and folders: `contracts/assets-naming.md` (`M_`, `MI_`, `LR_`). This recipe writes to `/Game/Look/`; move to the plugin path the
  contract names (`JamAssets`) by changing one constant (`ROOT`).
- **Project settings are device A's** (`Game/Config/**`). Check them first; if any is missing, file `handoff/B-to-A-NNNN.md` and wait. Without the
  first two, Lumen and virtual shadow maps are **silently off** and the look is different (measured, see the experiment):

  ```ini
  ; Game/Config/DefaultEngine.ini
  [/Script/WindowsTargetPlatform.WindowsTargetSettings]
  DefaultGraphicsRHI=DefaultGraphicsRHI_DX12
  -D3D12TargetedShaderFormats=PCD3D_SM5
  +D3D12TargetedShaderFormats=PCD3D_SM6        ; a config-less blank project runs the editor at SM5: no Lumen, no VSM

  [/Script/Engine.RendererSettings]
  r.GenerateMeshDistanceFields=True            ; software Lumen needs mesh distance fields (absent in a blank project)
  r.DynamicGlobalIlluminationMethod=1          ; Lumen
  r.ReflectionMethod=1
  r.Shadow.Virtual.Enable=1
  r.CustomDepth=3                              ; "Enabled with Stencil": both post passes read CustomStencil
  r.DefaultFeature.AutoExposure=False
  ```

  Check from the editor: `unreal.SystemLibrary.get_console_variable_int_value('r.CustomDepth')` is 3, and the editor log line
  `LogRHI: Using Highest Feature Level of D3D12: SM6`. A GPU profile (`PerfToolset.GpuProfilerToolset.TriggerGPUProfile`) must show
  `LumenSceneLighting` and `VirtualShadowMapMarkPages`; if it shows `ScreenSpace AO` and no `Lumen*` node, Lumen is off.

## 1. The stencil convention (the whole look hangs on it)

| CustomStencil | Who | Toon ramp | Outline |
|---:|---|---|---|
| 0 | emissive trim, beacon, puck halo, the dark room (floor, walls, ceiling) | no | no |
| 1 | figures (body, head, stick, blade), boards, net frame and mesh | 3-step ramp (`FigureStopsPerStep`) | yes |
| 2 | ice | banded ramp (`IceStopsPerStep`): painted spot pools | no (grazing ice only gives false edges) |
| 3 | puck | no (a near-black body has no lighting to step) | yes |

Every mesh component that carries stencil 1-3: `render_custom_depth = True`, `custom_depth_stencil_value = N`. A component without custom depth
is stencil 0. The toy skater (`skater-toy-figure.md`) must set this on its own components; put it in `handoff/` if A builds the figure.

## 2. Why the ramp is a post pass (Lumen in 5.9)

A surface material cannot read the light that reaches it, so a "ramp on N dot L" inside the material only works with one hand-fed light
direction (an MPC vector) and loses the spots, their shadows and Lumen. What works with Lumen: let the deferred renderer light normally, then in a
post-process material divide the lit scene colour by the GBuffer diffuse colour to get the received light, quantise its luminance in log2 steps,
and multiply back. Shadows, spot falloff, Lumen bounce and the sky all get stepped together. Cost measured: 0.02-0.03 ms at 1080p.

UE 5.9 also ships a native **Substrate Toon BSDF** (`MaterialExpressionSubstrateToonBSDF`, `ToonProfile` asset with `diffuse_ramp`,
`diffuse_ramp_include_shadow`). It needs `r.Substrate=1` (off in a blank project; the switch cost a 97 s editor restart for global shaders).
It was tried on the throwaway and works with Lumen (profile and material made by Python; the ramp curve is written with
`settings.export_text()` / `import_text()` on the `ToonProfile` asset, then `MaterialExpressionSubstrateToonProfileParameter` -> the BSDF's
`ToonProfile` pin, BSDF -> `MP_FRONT_MATERIAL`). But under Substrate the post ramp's diffuse lookup gets noisy on the ice. Use it only if the
project is already on Substrate, and then put the figures on stencil 3 so they are not stepped twice.

## 3. Materials (one `execute_python` call, about 25-55 s)

Call: `node tools/mcp/mcp.mjs call editor_toolset.toolsets.python_scripting.PythonScriptingTools.execute_python @args.json --url <editor> --timeout 600`
with `{"script":{"content":"<python>","language":"Python"}}`. Write the args file with Node or PowerShell (`JSON.stringify`), not by hand.

Helper (prepend to every script below):

```python
import unreal
MEL = unreal.MaterialEditingLibrary
EAL = unreal.EditorAssetLibrary
AT = unreal.AssetToolsHelpers.get_asset_tools()
MP = unreal.MaterialProperty
ROOT = '/Game/Look'

def new_material(path):
    if EAL.does_asset_exist(path):
        EAL.delete_asset(path)
    pkg, name = path.rsplit('/', 1)
    return AT.create_asset(name, pkg, unreal.Material, unreal.MaterialFactoryNew())

def node(m, cls, x=0, y=0, **props):
    e = MEL.create_material_expression(m, cls, x, y)
    for k, v in props.items():
        e.set_editor_property(k, v)
    return e

def sparam(m, name, val, x=0, y=0, group='Look'):
    return node(m, unreal.MaterialExpressionScalarParameter, x, y, parameter_name=name, default_value=val, group=group)

def vparam(m, name, rgba, x=0, y=0, group='Look'):
    return node(m, unreal.MaterialExpressionVectorParameter, x, y, parameter_name=name, default_value=unreal.LinearColor(*rgba), group=group)

def const(m, v, x=0, y=0):
    return node(m, unreal.MaterialExpressionConstant, x, y, r=v)

def wire(a, b, inp, out=''):
    if not MEL.connect_material_expressions(a, out, b, inp):
        raise RuntimeError('connect failed %s.%s -> %s.%s' % (a.get_name(), out, b.get_name(), inp))

def toprop(a, prop, out=''):
    if not MEL.connect_material_property(a, out, prop):
        raise RuntimeError('connect prop failed %s' % prop)

def custom(m, code, inputs, otype=unreal.CustomMaterialOutputType.CMOT_FLOAT3, x=0, y=0, desc='Custom'):
    c = node(m, unreal.MaterialExpressionCustom, x, y)
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
```

### 3.1 Graphs as text

```
M_Toon (Surface, Default Lit, Opaque)        figures, boards, nets, sticks, puck
  TeamColour (vec, default 0.02,0.12,0.65) ---------------------------> Base Color
  Roughness  (scalar 0.6) ------------------------------------------> Roughness
  0.25 -------------------------------------------------------------> Specular
  Fresnel(ExponentIn = RimPower 3.0) = F
  Custom "Rim+Emissive": F*RimStrength*RimColour + Emissive*TeamColour -> Emissive Color
      RimStrength 0.35 (0.6 on figures), RimColour 0.55,0.7,1.0, Emissive 0 (2-3 on figures = flat fill so the shadow side keeps its jersey colour)

M_Trim (Surface, Default Lit)                emissive strips, puck halo/rim, far-net beacon
  BaseTint (vec 0.02,0.02,0.025) -> Base Color;  TeamColour * Emissive (20) -> Emissive Color

M_Ice (Surface, Default Lit)
  N1 = Noise(world pos, scale 0.004, levels 4, turbulence, out 0..1);  N2 = Noise(scale 0.05, levels 2, out 0..1)
  ScuffMask = saturate((N1*0.65 + N2*0.35 - 0.42) * 3) * Scuff          (Scuff 0.7 at night, 0.2 by day)
  Base Color = lerp(IceColour 0.72,0.80,0.86 , ScuffColour 0.50,0.56,0.62 , ScuffMask), then painted lines from world X
               (red centre |x|<30 cm, blue lines at |x|=17.5 m, centre circle r 9 m, goal lines |x|=53 m; LineRed, LineBlue)
  Roughness  = lerp(lerp(0.28, 0.75, ScuffMask), 0.05, Wet)             (Wet 0 at T1/T2; T3 drives it to 1)
  Specular 0.5

PP_ToonRamp (Post Process, Blendable Location = Scene Color Before DOF)
  SceneTexture:PostProcessInput0 -> Custom (code below) -> Emissive Color
  params: FigureStopsPerStep 1.6, IceStopsPerStep 1.0, StepSoftness 0.08, ShadowFloor 0.004, RampOn 1

PP_Outline (Post Process, Before DOF, so TSR anti-aliases the lines)
  SceneTexture:PostProcessInput0 -> Custom (code below) -> Emissive Color
  params: OutlineWidth 1.5 px, OutlineColour 0.01,0.012,0.03 (multiplies the scene colour), DepthThreshold 0.08,
          NormalThreshold 0.35, FadeStart 1500, FadeEnd 4000, FadeMin 0.3, OutlineOn 1
```

### 3.2 The script

```python
out = {}
EAL.make_directory(ROOT)

m = new_material(ROOT + '/M_Toon')
team = vparam(m, 'TeamColour', (0.02, 0.12, 0.65, 1)); rough = sparam(m, 'Roughness', 0.6); spec = const(m, 0.25)
toprop(team, MP.MP_BASE_COLOR); toprop(rough, MP.MP_ROUGHNESS); toprop(spec, MP.MP_SPECULAR)
fres = node(m, unreal.MaterialExpressionFresnel); rimp = sparam(m, 'RimPower', 3.0); wire(rimp, fres, 'ExponentIn')
rims = sparam(m, 'RimStrength', 0.35); rimc = vparam(m, 'RimColour', (0.55, 0.7, 1.0, 1)); em = sparam(m, 'Emissive', 0.0)
c = custom(m, 'return F * RimStrength * RimColour + Emissive * TeamColour;', ['F', 'RimStrength', 'RimColour', 'Emissive', 'TeamColour'], desc='Rim+Emissive')
wire(fres, c, 'F'); wire(rims, c, 'RimStrength'); wire(rimc, c, 'RimColour'); wire(em, c, 'Emissive'); wire(team, c, 'TeamColour')
toprop(c, MP.MP_EMISSIVE_COLOR)
out['M_Toon'] = finish(m)

m = new_material(ROOT + '/M_Trim')
team = vparam(m, 'TeamColour', (0.05, 0.25, 1.0, 1)); em = sparam(m, 'Emissive', 20.0); base = vparam(m, 'BaseTint', (0.02, 0.02, 0.025, 1))
mul = node(m, unreal.MaterialExpressionMultiply); wire(team, mul, 'A'); wire(em, mul, 'B')
toprop(base, MP.MP_BASE_COLOR); toprop(mul, MP.MP_EMISSIVE_COLOR)
out['M_Trim'] = finish(m)

m = new_material(ROOT + '/M_Ice')
wp = node(m, unreal.MaterialExpressionWorldPosition)
n1 = node(m, unreal.MaterialExpressionNoise, scale=0.004, levels=4, output_min=0.0, output_max=1.0, turbulence=True, quality=1)
n2 = node(m, unreal.MaterialExpressionNoise, scale=0.05, levels=2, output_min=0.0, output_max=1.0, turbulence=False, quality=1)
scuff = sparam(m, 'Scuff', 0.7); wet = sparam(m, 'Wet', 0.0)
icec = vparam(m, 'IceColour', (0.72, 0.80, 0.86, 1)); scc = vparam(m, 'ScuffColour', (0.50, 0.56, 0.62, 1))
lr = vparam(m, 'LineRed', (0.45, 0.04, 0.04, 1)); lb = vparam(m, 'LineBlue', (0.04, 0.10, 0.40, 1))
mask = custom(m, 'return saturate((N1 * 0.65 + N2 * 0.35 - 0.42) * 3.0) * Scuff;', ['N1', 'N2', 'Scuff'], otype=unreal.CustomMaterialOutputType.CMOT_FLOAT1, desc='ScuffMask')
wire(n1, mask, 'N1'); wire(n2, mask, 'N2'); wire(scuff, mask, 'Scuff')
col = custom(m, '''float3 c = lerp(IceColour, ScuffColour, M);
float ax = abs(P.x);
float red = step(ax, 30.0) + step(abs(ax - 5300.0), 6.0);
float blue = step(abs(ax - 1750.0), 30.0) + step(abs(length(P.xy) - 900.0), 8.0);
c = lerp(c, LineRed, saturate(red));
c = lerp(c, LineBlue, saturate(blue));
return c;''', ['P', 'M', 'IceColour', 'ScuffColour', 'LineRed', 'LineBlue'], desc='IceColour+Lines')
wire(wp, col, 'P'); wire(mask, col, 'M'); wire(icec, col, 'IceColour'); wire(scc, col, 'ScuffColour'); wire(lr, col, 'LineRed'); wire(lb, col, 'LineBlue')
rgh = custom(m, 'return lerp(lerp(0.28, 0.75, M), 0.05, Wet);', ['M', 'Wet'], otype=unreal.CustomMaterialOutputType.CMOT_FLOAT1, desc='Roughness')
wire(mask, rgh, 'M'); wire(wet, rgh, 'Wet')
toprop(col, MP.MP_BASE_COLOR); toprop(rgh, MP.MP_ROUGHNESS); toprop(const(m, 0.5), MP.MP_SPECULAR)
out['M_Ice'] = finish(m)

m = new_material(ROOT + '/PP_ToonRamp')
m.set_editor_property('material_domain', unreal.MaterialDomain.MD_POST_PROCESS)
m.set_editor_property('blendable_location', unreal.BlendableLocation.BL_SCENE_COLOR_BEFORE_DOF)
st = node(m, unreal.MaterialExpressionSceneTexture, scene_texture_id=unreal.SceneTextureId.PPI_POST_PROCESS_INPUT0)
pf = sparam(m, 'FigureStopsPerStep', 1.6, group='Ramp'); pi = sparam(m, 'IceStopsPerStep', 1.0, group='Ramp')
sf = sparam(m, 'StepSoftness', 0.08, group='Ramp'); fl = sparam(m, 'ShadowFloor', 0.004, group='Ramp'); on = sparam(m, 'RampOn', 1.0, group='Ramp')
RAMP = '''float3 sc = SC.rgb;
if (RampOn < 0.5) return sc;
float stencil = SceneTextureLookup(GetDefaultSceneTextureUV(Parameters, 25), 25, false).r;
if (stencil < 0.5 || stencil > 2.5) return sc;
float3 dc = SceneTextureLookup(GetDefaultSceneTextureUV(Parameters, 2), 2, false).rgb;
float3 light = sc / max(dc, 0.03);
float L = max(dot(light, float3(0.2126, 0.7152, 0.0722)), ShadowFloor);
float3 hue = light / L;
float sps = stencil < 1.5 ? FigureStopsPerStep : IceStopsPerStep;
float b = log2(L) / sps;
float f = frac(b);
float stepped = floor(b) + smoothstep(1.0 - StepSoftness, 1.0, f);
float q = exp2((stepped + 0.5) * sps);
return dc * hue * q;'''
c = custom(m, RAMP, ['SC', 'FigureStopsPerStep', 'IceStopsPerStep', 'StepSoftness', 'ShadowFloor', 'RampOn'], desc='ToonRamp')
wire(st, c, 'SC', 'Color'); wire(pf, c, 'FigureStopsPerStep'); wire(pi, c, 'IceStopsPerStep'); wire(sf, c, 'StepSoftness'); wire(fl, c, 'ShadowFloor'); wire(on, c, 'RampOn')
toprop(c, MP.MP_EMISSIVE_COLOR)
out['PP_ToonRamp'] = finish(m)

m = new_material(ROOT + '/PP_Outline')
m.set_editor_property('material_domain', unreal.MaterialDomain.MD_POST_PROCESS)
m.set_editor_property('blendable_location', unreal.BlendableLocation.BL_SCENE_COLOR_BEFORE_DOF)
st = node(m, unreal.MaterialExpressionSceneTexture, scene_texture_id=unreal.SceneTextureId.PPI_POST_PROCESS_INPUT0)
P = {}
for n, v in (('OutlineWidth', 1.5), ('DepthThreshold', 0.08), ('NormalThreshold', 0.35), ('FadeStart', 1500.0), ('FadeEnd', 4000.0), ('FadeMin', 0.3), ('OutlineOn', 1.0)):
    P[n] = sparam(m, n, v, group='Outline')
P['OutlineColour'] = vparam(m, 'OutlineColour', (0.01, 0.012, 0.03, 1), group='Outline')
OUTLINE = '''float3 sc = SC.rgb;
if (OutlineOn < 0.5) return sc;
float2 uv = GetDefaultSceneTextureUV(Parameters, 1);
float2 px = View.BufferSizeAndInvSize.zw * OutlineWidth;
float dc = SceneTextureLookup(uv, 1, false).r;
if (dc > 1.0e6) return sc;
float stn = SceneTextureLookup(GetDefaultSceneTextureUV(Parameters, 25), 25, false).r;
if (!(abs(stn - 1.0) < 0.5 || abs(stn - 3.0) < 0.5)) return sc;
float dl = SceneTextureLookup(uv + float2(-px.x, 0), 1, false).r;
float dr = SceneTextureLookup(uv + float2( px.x, 0), 1, false).r;
float du = SceneTextureLookup(uv + float2(0, -px.y), 1, false).r;
float dd = SceneTextureLookup(uv + float2(0,  px.y), 1, false).r;
float sx = dc / dl + dc / dr - 2.0;      // second derivative of 1/depth: 0 on any plane, so grazing ice gives no lines
float sy = dc / du + dc / dd - 2.0;
float ed = max(0.0, -sx) + max(0.0, -sy); // negative = this pixel is the near side: ink goes on the object, not on the background
float3 nc = SceneTextureLookup(uv, 8, false).rgb;
float3 nl = SceneTextureLookup(uv + float2(-px.x, 0), 8, false).rgb;
float3 nr = SceneTextureLookup(uv + float2( px.x, 0), 8, false).rgb;
float3 nu = SceneTextureLookup(uv + float2(0, -px.y), 8, false).rgb;
float3 nd = SceneTextureLookup(uv + float2(0,  px.y), 8, false).rgb;
float en = 4.0 - dot(nc, nl) - dot(nc, nr) - dot(nc, nu) - dot(nc, nd);
float e = max(saturate((ed - DepthThreshold) / DepthThreshold), saturate((en - NormalThreshold) / NormalThreshold));
float fade = lerp(1.0, FadeMin, saturate((dc - FadeStart) / max(FadeEnd - FadeStart, 1.0)));
return lerp(sc, OutlineColour * sc, saturate(e * fade));'''
names = ['SC', 'OutlineWidth', 'OutlineColour', 'DepthThreshold', 'NormalThreshold', 'FadeStart', 'FadeEnd', 'FadeMin', 'OutlineOn']
c = custom(m, OUTLINE, names, desc='Outline')
wire(st, c, 'SC', 'Color')
for n in names[1:]:
    wire(P[n], c, n)
toprop(c, MP.MP_EMISSIVE_COLOR)
out['PP_Outline'] = finish(m)
out   # every list must be empty
```

## 4. Material instances (same call style, a few seconds)

```python
def mi(name, parent, vec=None, scal=None):
    path = ROOT + '/' + name
    if EAL.does_asset_exist(path):
        EAL.delete_asset(path)
    m = AT.create_asset(name, ROOT, unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
    m.set_editor_property('parent', unreal.load_asset(parent))
    for k, v in (vec or {}).items():
        MEL.set_material_instance_vector_parameter_value(m, k, unreal.LinearColor(*v))
    for k, v in (scal or {}).items():
        MEL.set_material_instance_scalar_parameter_value(m, k, v)
    MEL.update_material_instance(m)
    EAL.save_loaded_asset(m)
    return m
```

| Instance | Parent | TeamColour | Scalars |
|---|---|---|---|
| `MI_Toon_Blue` | M_Toon | 0.02, 0.10, 0.55 | Emissive 2.5, RimStrength 0.6 |
| `MI_Toon_Orange` | M_Toon | 0.95, 0.30, 0.03 | Emissive 2.0, RimStrength 0.6 |
| `MI_Toon_Board` | M_Toon | 0.80, 0.80, 0.82 | Emissive 1.5, RimStrength 0.6 |
| `MI_Toon_Helmet` | M_Toon | 0.90, 0.88, 0.80 | Emissive 1.5, RimStrength 0.6 |
| `MI_Toon_Stick` | M_Toon | 0.55, 0.40, 0.20 | Emissive 2.0, RimStrength 0.6 |
| `MI_Toon_NetFrame` | M_Toon | 0.60, 0.03, 0.03 | Emissive 3.0 |
| `MI_Toon_NetMesh` | M_Toon | 0.85, 0.85, 0.85 | Emissive 1.5 (darken it: white net on white boards was the weakest contrast at 104 m) |
| `MI_Toon_Puck` | M_Toon | 0.012 grey | RimStrength 0, Roughness 0.4 |
| `MI_Toon_Dark` (room) | M_Toon | 0.015, 0.016, 0.02 | RimStrength 0 |
| `MI_Trim_Blue` / `MI_Trim_Orange` | M_Trim | 0.05,0.30,1.0 / 1.0,0.35,0.05 | Emissive 30 |
| `MI_Trim_PuckRim` | M_Trim | 1.0, 0.92, 0.55 | Emissive 25 (use an accent colour, not white, see 7) |
| `MI_Trim_Beacon` | M_Trim | 1.0, 0.85, 0.6 | Emissive 40 |
| `MI_Ice_Night` | M_Ice | - | Scuff 0.7, Wet 0 |
| `MI_PP_ToonRamp`, `MI_PP_Outline` | the PP materials | - | defaults |

The Emissive values assume the exposure in section 5 (EV100 4.2). If the exposure changes by N stops, scale every Emissive by 2^N.

## 5. LR_L2_Night: the rig (values that produced the stills)

Put these actors in whatever container the contract names for `LR_L2_Night` (sub-level, level instance or data asset); the throwaway kept them
in an outliner folder of that name, the container itself was not tested.

| Actor | Settings |
|---|---|
| 4 x `SpotLight` | at x = -45, -15, 15, 45 m, y 0, z 25 m, pitch -90; `intensity_units CANDELAS`, `intensity 20000`, `attenuation_radius 4000`, inner 22 deg, outer 30 deg, colour (255,244,230), `cast_shadows True`, `source_radius 30`. Each pool is about 29 m across. |
| `SkyLight` | `source_type SLS_SPECIFIED_CUBEMAP`, cubemap `/Engine/MapTemplates/Sky/DaylightAmbientCubemap`, `intensity 2.0`, colour (150,185,255). With a ceiling, Lumen occludes most of it: the ice between pools is near black; raise it to 3-4 if the puck must be seen there. |
| `PostProcessVolume` | `unbound True`; set each value **and** its `override_<name>` flag: `auto_exposure_method AEM_HISTOGRAM`, `auto_exposure_min_brightness = auto_exposure_max_brightness = 4.2` (fixed EV100), `auto_exposure_bias 0`, `bloom_intensity 0.9`, `bloom_threshold 1.0`, `vignette_intensity 0.3`, `color_gain_shadows (0.85, 0.93, 1.15, 1)`, `color_saturation (1.1, 1.1, 1.1, 1)`; `weighted_blendables = [MI_PP_ToonRamp, MI_PP_Outline]` (weight 1 each, in that order). No fog. |
| Room | a dark floor plane under the rink, four dark walls 10-50 m out, a ceiling at 28 m (all `MI_Toon_Dark`, stencil 0, no shadow). "No crowd". |
| Trim | a 10 cm strip on the top of every board segment, `MI_Trim_Blue` on the x<0 half, `MI_Trim_Orange` on x>0; a flat strip under each net in the defending team's colour. Stencil 0, no shadow. |

```python
s = ppv.settings
for k, v in [('auto_exposure_method', unreal.AutoExposureMethod.AEM_HISTOGRAM), ('auto_exposure_min_brightness', 4.2), ('auto_exposure_max_brightness', 4.2),
             ('auto_exposure_bias', 0.0), ('bloom_intensity', 0.9), ('bloom_threshold', 1.0), ('vignette_intensity', 0.3),
             ('color_gain_shadows', unreal.Vector4(0.85, 0.93, 1.15, 1.0)), ('color_saturation', unreal.Vector4(1.1, 1.1, 1.1, 1.0))]:
    s.set_editor_property(k, v)
    s.set_editor_property('override_' + k, True)
s.set_editor_property('weighted_blendables', unreal.WeightedBlendables(array=[
    unreal.WeightedBlendable(weight=1.0, object=unreal.load_asset(ROOT + '/MI_PP_ToonRamp')),
    unreal.WeightedBlendable(weight=1.0, object=unreal.load_asset(ROOT + '/MI_PP_Outline'))]))
ppv.set_editor_property('settings', s)   # the struct is a copy: write it back
```

Day (T1) from the same assets (not tested here): swap the rig (sun + brighter sky, exposure about EV 10, bloom 0.2), Trim Emissive to 0-3,
Scuff 0.2, and rescale every `Emissive` fill by the exposure change (section 4). The two post passes and the stencil stay.

## 6. Third-person camera

Spring arm about 6 m back and 2.5 m above the skater pivot, pitch about -8 deg, FOV 70; on the throwaway a `CameraActor` at (-51, -3, 3) m with
`auto_activate_for_player = PLAYER0` stood in for the spring arm (gameplay owns the real one). From there the far net (x = 53 m) is 104 m away.

## 7. Readability kit (measured, see the experiment's verdict)

- **Puck**: stencil 3 (outlined, not ramped). On lit ice it is found at 10 m (about 12 x 6 px) but at 40 m it is a 4 x 1 px dash; on the dark ice
  between pools it is invisible at 24 m. A side rim (thin emissive cylinder band d 13 cm) is sub-pixel past 20 m: **not enough**. A flat glowing disc
  under the puck (cylinder d 60 cm, 4 mm thick, `M_Trim`, stencil 0, no shadow) makes it obvious on dark ice; on lit ice a white disc vanishes:
  give the halo a saturated accent colour that is neither team colour nor trim (yellow-green or magenta), Emissive 10-25.
- **Far net**: the red-framed box reads at 104 m only against the boards; an emissive post behind it (cylinder d 12 cm, 8 m tall, `MI_Trim_Beacon`)
  is the single most visible thing at the far end. Keep it.
- **Teams**: blue and orange separate at every distance in the stills, including under the cool grade; keep the jersey colours away from the
  trim hues (the trim is lighter and more saturated than the jerseys).

## 8. Order of work for the builder

1. Section 0 checks (or handoff). 2. Section 3 script, `out` lists empty. 3. Section 4 instances. 4. Stencil on every mesh you own (section 1).
5. Section 5 rig. 6. Section 7 kit. 7. Section 9 verification, screenshots into the dev log.

## 9. Verify (and the pitfalls that cost time)

- Screenshots: `StartPIE {"options":{"playMode":"PlayMode_InEditorFloating","warmupSeconds":5}}` with the PIE window set to 1920x1080
  (`unreal.get_default_object(unreal.LevelEditorPlaySettings).set_editor_property('NewWindowWidth', 1920)` / `NewWindowHeight 1080`), then
  `SceneCaptureToolset.SceneCaptureToolset.CaptureGameViewport {"world":{"refPath":"/Game/<map path>/UEDPIE_0_<Map>.<Map>"},"maxResolution":1920}`
  (world path from `ListWorlds`), then `StopPIE`. `CaptureEditorViewport {}` also shows the post materials.
- **`SceneCaptureToolset...Capture` and `CaptureActiveEditorView` do not apply post-process blendables** (bloom and exposure yes, the toon ramp and
  outline no; proven with a material that returned pure red). Never judge the look from them.
- GPU time: `PerfToolset.StatsToolset.GetFrameTimings {}` -> `gpuMs` (what `stat unit` shows). In PIE it includes the editor's own viewport
  and Slate: call `unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).editor_set_viewport_realtime(False)` first, take 60 samples and use the
  median, and expect other editors on the same GPU to add noise. The clean number is a standalone run:
  `UnrealEditor.exe <uproject> /Game/<map> -game -windowed -ResX=1920 -ResY=1080 -csvCaptureFrames=900` and the `GPUTime` column of the CSV, which
  lands in `%LOCALAPPDATA%\UnrealEngine\5.9\Saved\Profiling\CSV\` (not the project); the process does not exit by itself, stop it once the log
  says `CSV finalize time`. Per-pass cost: `TriggerGPUProfile {"minInclusiveMs":0}`; the post passes show as `PostProcessMaterial 1920x1082 Material=PP_*`.
- A PIE player spawns a `DefaultPawn` whose sphere is visible at the world origin: place a `PlayerStart` out of view or use the game's pawn.
- `LevelEditorSubsystem.set_level_viewport_camera_info` needs a viewport config key; use `EditorAppToolset.SetCameraTransform` or `pilot_level_actor(camera)`.
- `recompile_material` does not raise on errors, and whether `MEL.get_material_error_expressions(m)` catches an HLSL error inside a Custom node
  was not tested (these graphs compiled clean). If a post material seems to do nothing, first make sure you are looking through a capture path
  that applies it (above), then swap the Custom code for `return float3(1,0,0) + SC.rgb * 0;` and look in `CaptureEditorViewport`.
- Keep the SceneTexture:PostProcessInput0 node wired into the Custom node: it is what declares the scene-texture use to the material compiler.
  `SceneTextureLookup` without it was not tried.
- Thin things and the outline: the inner-edge rule keeps a stick from getting a halo, but past about 25 m a 1-3 px stick turns fully ink-black.
  The distance fade (`FadeStart 1500`, `FadeEnd 4000`, `FadeMin 0.3`) brings back its colour; keep thin emissive strips at stencil 0 or they go dark.
- Closing the editor with `editor.ps1 -Stop` waited 60 s on a save prompt and force-stopped; save, then `EditorAppToolset.ExitEditor {}` exits cleanly.
