# AR13: the three master changes that need a renderer. Run ONCE in a GPU editor of the tree that holds AR13's assets, as one execute_python call
# (or: File > Execute Python Script). It edits /JamAssets/Look/M_ClayCrowd and /JamAssets/Arena/M_ArenaWood, compiles them and saves the two packages.
#
#   1. M_ClayCrowd: the cheering pose blends target A (UV2..UV4) with target B (UV5..UV7) by per-instance data 5 (0 or 1).
#   2. M_ClayCrowd: the wave and the bounce. Each fan rises over 8 of the Blueprint's 28 frames, started by its delay (data 4, 0 at the scored net
#      to 1 at the far corner):  amount = smoothstep(saturate(E * (1 + WaveK) - WaveK * delay)) * weight * PoseK, E the end's scalar in MPC_Crowd.
#      The bounce: BounceCm (15) * amount * |sin(2 pi (2.2 t + phase))| straight up, the phase from the fan's coat value (data 1).
#   3. M_ArenaWood: base colour (and the wash that is made from it) multiplied by the vertex colour, decoded like the crowd's (power 2.2).
#
# Per-instance data on the fans after AR13: 0 weight, 1 coat value, 2 accent value, 3 end (+1 / -1), 4 delay, 5 target (0 A, 1 B).
# The script is safe to run twice: each part looks for its own nodes first.
import unreal, json

PATH_C = '/JamAssets/Look/M_ClayCrowd'; PATH_W = '/JamAssets/Arena/M_ArenaWood'; COMPILE = True; SAVE = True
# @@PARAMS@@
MEL = unreal.MaterialEditingLibrary; MP = unreal.MaterialProperty; out = {}
E = unreal  # short names for the expression classes
PI = E.MaterialExpressionPerInstanceCustomData

def exprs(m): return list(MEL.get_material_expressions(m))
def node(m, cls, x, y, **p):
    e = MEL.create_material_expression(m, cls, x, y)
    for k, v in p.items(): e.set_editor_property(k, v)
    return e
def wire(a, b, inp, o=''):
    if not MEL.connect_material_expressions(a, o, b, inp): raise RuntimeError('cannot wire %s.%s -> %s.%s' % (a.get_name(), o, b.get_name(), inp))
def reach(m, root):
    seen = []; todo = [root] if root else []
    while todo:
        e = todo.pop()
        if e is None or e in seen: continue
        seen.append(e); todo += [x for x in MEL.get_inputs_for_material_expression(m, e) if x]
    return seen
def chain(m, e, d=0):
    r = e.get_class().get_name().replace('MaterialExpression', '')
    if isinstance(e, PI): r += str(e.get_editor_property('data_index'))
    if isinstance(e, E.MaterialExpressionTextureCoordinate): r += str(e.get_editor_property('coordinate_index'))
    if isinstance(e, (E.MaterialExpressionScalarParameter, E.MaterialExpressionCollectionParameter)): r += ':' + str(e.get_editor_property('parameter_name'))
    if d > 9: return r
    kids = [chain(m, x, d + 1) for x in MEL.get_inputs_for_material_expression(m, e) if x]
    return r + ('(' + ','.join(kids) + ')' if kids else '')

def crowd(m):
    if any(isinstance(e, PI) and e.get_editor_property('data_index') == 5 for e in exprs(m)): return 'already done'
    old_root = MEL.get_material_property_input_node(m, MP.MP_WORLD_POSITION_OFFSET); old = reach(m, old_root)
    keep = []
    for prop in (MP.MP_BASE_COLOR, MP.MP_EMISSIVE_COLOR, MP.MP_ROUGHNESS, MP.MP_SPECULAR, MP.MP_METALLIC, MP.MP_NORMAL, MP.MP_OPACITY, MP.MP_OPACITY_MASK):
        keep += reach(m, MEL.get_material_property_input_node(m, prop))
    mpc = None
    for e in old:
        if isinstance(e, E.MaterialExpressionCollectionParameter): mpc = e.get_editor_property('collection')
    if mpc is None: mpc = unreal.load_asset('/JamAssets/Arena/MPC_Crowd')
    X = -3600; Y = 3000; G = 'Crowd cheer (AR13)'
    def off(first, y):
        us = []
        for i in range(3):
            tc = node(m, E.MaterialExpressionTextureCoordinate, X, y + 110 * i, coordinate_index=first + i)
            cm = node(m, E.MaterialExpressionComponentMask, X + 220, y + 110 * i, r=True, g=False, b=False, a=False); wire(tc, cm, ''); us.append(cm)
        a1 = node(m, E.MaterialExpressionAppendVector, X + 440, y); wire(us[0], a1, 'A'); wire(us[1], a1, 'B')
        a2 = node(m, E.MaterialExpressionAppendVector, X + 620, y + 60); wire(a1, a2, 'A'); wire(us[2], a2, 'B'); return a2
    off_a = off(2, Y); off_b = off(5, Y + 380)
    tb = node(m, PI, X + 620, Y + 760, data_index=5, const_default_value=0.0)
    lerp = node(m, E.MaterialExpressionLinearInterpolate, X + 840, Y + 200); wire(off_a, lerp, 'A'); wire(off_b, lerp, 'B'); wire(tb, lerp, 'Alpha')
    tr = node(m, E.MaterialExpressionTransform, X + 1060, Y + 200, transform_source_type=unreal.MaterialVectorCoordTransformSource.TRANSFORMSOURCE_LOCAL, transform_type=unreal.MaterialVectorCoordTransform.TRANSFORM_WORLD); wire(lerp, tr, '')
    # the amount
    w = node(m, PI, X, Y + 1000, data_index=0, const_default_value=0.0); side = node(m, PI, X, Y + 1100, data_index=3, const_default_value=1.0); dly = node(m, PI, X, Y + 1200, data_index=4, const_default_value=0.0)
    cp = node(m, E.MaterialExpressionCollectionParameter, X, Y + 1300, collection=mpc, parameter_name='CrowdCheerPlusX'); cn = node(m, E.MaterialExpressionCollectionParameter, X, Y + 1450, collection=mpc, parameter_name='CrowdCheerMinusX')
    zero = node(m, E.MaterialExpressionConstant, X, Y + 1600, r=0.0); one = node(m, E.MaterialExpressionConstant, X, Y + 1660, r=1.0)
    wk = node(m, E.MaterialExpressionScalarParameter, X, Y + 1740, parameter_name='WaveK', default_value=2.5, group=G)
    iff = node(m, E.MaterialExpressionIf, X + 260, Y + 1150); wire(side, iff, 'A'); wire(zero, iff, 'B'); wire(cp, iff, 'A > B'); wire(cn, iff, 'A == B'); wire(cn, iff, 'A < B')
    k1 = node(m, E.MaterialExpressionAdd, X + 260, Y + 1650); wire(one, k1, 'A'); wire(wk, k1, 'B')
    e1 = node(m, E.MaterialExpressionMultiply, X + 480, Y + 1250); wire(iff, e1, 'A'); wire(k1, e1, 'B')
    dk = node(m, E.MaterialExpressionMultiply, X + 480, Y + 1450); wire(dly, dk, 'A'); wire(wk, dk, 'B')
    e2 = node(m, E.MaterialExpressionSubtract, X + 700, Y + 1300); wire(e1, e2, 'A'); wire(dk, e2, 'B')
    e3 = node(m, E.MaterialExpressionSaturate, X + 880, Y + 1300); wire(e2, e3, '')
    ss = node(m, E.MaterialExpressionSmoothStep, X + 1060, Y + 1300); wire(zero, ss, 'Min'); wire(one, ss, 'Max'); wire(e3, ss, 'Value')
    pk = node(m, E.MaterialExpressionScalarParameter, X + 1060, Y + 1500, parameter_name='PoseK', default_value=1.0, group='Crowd cheer (AR5)')
    a3 = node(m, E.MaterialExpressionMultiply, X + 1280, Y + 1300); wire(ss, a3, 'A'); wire(w, a3, 'B')
    a4 = node(m, E.MaterialExpressionMultiply, X + 1460, Y + 1300); wire(a3, a4, 'A'); wire(pk, a4, 'B')
    pose = node(m, E.MaterialExpressionMultiply, X + 1680, Y + 400); wire(tr, pose, 'A'); wire(a4, pose, 'B')
    # the bounce
    coat = node(m, PI, X, Y + 1900, data_index=1, const_default_value=1.0); c713 = node(m, E.MaterialExpressionConstant, X, Y + 2000, r=7.13); c22 = node(m, E.MaterialExpressionConstant, X, Y + 2060, r=2.2)
    ph0 = node(m, E.MaterialExpressionMultiply, X + 260, Y + 1900); wire(coat, ph0, 'A'); wire(c713, ph0, 'B'); ph = node(m, E.MaterialExpressionFrac, X + 440, Y + 1900); wire(ph0, ph, '')
    tm = node(m, E.MaterialExpressionTime, X + 260, Y + 2060); tf = node(m, E.MaterialExpressionMultiply, X + 440, Y + 2060); wire(tm, tf, 'A'); wire(c22, tf, 'B')
    arg = node(m, E.MaterialExpressionAdd, X + 620, Y + 1980); wire(tf, arg, 'A'); wire(ph, arg, 'B')
    sn = node(m, E.MaterialExpressionSine, X + 800, Y + 1980, period=1.0); wire(arg, sn, '')
    ab = node(m, E.MaterialExpressionAbs, X + 980, Y + 1980); wire(sn, ab, '')
    bcm = node(m, E.MaterialExpressionScalarParameter, X + 980, Y + 2120, parameter_name='BounceCm', default_value=15.0, group=G)
    b1 = node(m, E.MaterialExpressionMultiply, X + 1160, Y + 1980); wire(ab, b1, 'A'); wire(bcm, b1, 'B')
    b2 = node(m, E.MaterialExpressionMultiply, X + 1340, Y + 1980); wire(b1, b2, 'A'); wire(a4, b2, 'B')
    z2 = node(m, E.MaterialExpressionConstant2Vector, X + 1340, Y + 2140, r=0.0, g=0.0)
    bv = node(m, E.MaterialExpressionAppendVector, X + 1520, Y + 2000); wire(z2, bv, 'A'); wire(b2, bv, 'B')
    total = node(m, E.MaterialExpressionAdd, X + 1900, Y + 800); wire(pose, total, 'A'); wire(bv, total, 'B')
    if not MEL.connect_material_property(total, '', MP.MP_WORLD_POSITION_OFFSET): raise RuntimeError('cannot connect the world position offset')
    gone = 0
    for e in old:
        if e not in keep: MEL.delete_material_expression(m, e); gone += 1
    return 'built; %d old nodes removed' % gone

def wood(m):
    if any(isinstance(e, E.MaterialExpressionVertexColor) for e in exprs(m)): return 'already done'
    bc = MEL.get_material_property_input_node(m, MP.MP_BASE_COLOR)
    if bc is None: raise RuntimeError('M_ArenaWood has no base colour input')
    x = bc.get_editor_property('material_expression_editor_x'); y = bc.get_editor_property('material_expression_editor_y')
    users = []
    for e in exprs(m):
        if isinstance(e, E.MaterialExpressionCustom):
            names = [str(i.get_editor_property('input_name')) for i in e.get_editor_property('inputs')]
            for nm, src in zip(names, MEL.get_inputs_for_material_expression(m, e)):
                if src == bc: users.append((e, nm))
    vc = node(m, E.MaterialExpressionVertexColor, x + 60, y - 260)
    pw = node(m, E.MaterialExpressionPower, x + 240, y - 260, const_exponent=2.2); wire(vc, pw, 'Base')
    mul = node(m, E.MaterialExpressionMultiply, x + 420, y - 120); wire(bc, mul, 'A'); wire(pw, mul, 'B')
    if not MEL.connect_material_property(mul, '', MP.MP_BASE_COLOR): raise RuntimeError('cannot connect the base colour')
    for e, nm in users: wire(mul, e, nm)
    return 'built; base colour and %d custom input(s) now read base x vertex colour: %s' % (len(users), ', '.join('%s.%s' % (str(e.get_editor_property('description')), nm) for e, nm in users))

pk = []
for path, fn in ((PATH_C, crowd), (PATH_W, wood)):
    m = unreal.load_asset(path); r = fn(m); out[path] = r
    if COMPILE: MEL.recompile_material(m)
    try:
        st = MEL.get_statistics(m); out[path + ' instructions (vertex, pixel)'] = [st.num_vertex_shader_instructions, st.num_pixel_shader_instructions]
    except Exception as e: out[path + ' statistics'] = str(e)[:80]
    m.modify(); pk.append(m.get_outermost())
mc = unreal.load_asset(PATH_C); out['crowd world position offset'] = chain(mc, MEL.get_material_property_input_node(mc, MP.MP_WORLD_POSITION_OFFSET))
mw = unreal.load_asset(PATH_W); out['wood base colour'] = chain(mw, MEL.get_material_property_input_node(mw, MP.MP_BASE_COLOR), 7)
if SAVE: out['saved'] = unreal.EditorLoadingAndSavingUtils.save_packages(pk, True)
json.dumps(out)
