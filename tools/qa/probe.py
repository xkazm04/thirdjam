import unreal
# Lane A10 probe: what Python can reach in the running PIE match. Returns one line of text.
w = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
out = {}
out['world'] = w.get_path_name() if w else None
if w:
    gm = unreal.GameplayStatics.get_game_mode(w)
    out['gm'] = gm.get_class().get_name() if gm else None
    pres = [o for o in unreal.ObjectIterator(unreal.JamPresentation) if o.get_outer() == w]
    out['pres'] = len(pres)
    if pres and pres[0].has_frame():
        f = pres[0].get_frame()
        out['frame'] = int(f.get_editor_property('sim_frame'))
        out['puck'] = [round(f.get_editor_property('puck_m').x, 2), round(f.get_editor_property('puck_m').y, 2)]
        out['score'] = [f.get_editor_property('score_home'), f.get_editor_property('score_away')]
        out['phase'] = str(f.get_editor_property('phase'))
        out['n_skaters'] = len(f.get_editor_property('skaters'))
        out['human'] = f.get_editor_property('human_index')
    if gm:
        try:
            out['scripted'] = gm.get_editor_property('scripted_human')
        except Exception as e:
            out['scripted_err'] = str(e)[:80]
        try:
            sim = gm.get_sim()
            sk = sim.get_editor_property('skaters')
            out['sim_skaters'] = [[s.get_editor_property('index'), str(s.get_editor_property('role')), round(s.get_editor_property('pos').x, 1), round(s.get_editor_property('pos').y, 1), round(s.get_editor_property('vel').size(), 2)] for s in sk]
            out['sim_puck'] = [round(sim.get_editor_property('puck').get_editor_property('pos').x, 2), round(sim.get_editor_property('puck').get_editor_property('vel').size(), 2)]
            cfg = sim.get_editor_property('config')
            out['cfg'] = [cfg.get_editor_property('rink').get_editor_property('length_m'), cfg.get_editor_property('skater').get_editor_property('speed'), cfg.get_editor_property('ai').get_editor_property('speed_scale')]
        except Exception as e:
            out['sim_err'] = str(e)[:120]
        try:
            hs = gm.get_human_skater()
            out['human_skater'] = hs.get_name()
            out['human_components'] = [c.get_class().get_name() for c in hs.get_components_by_class(unreal.ActorComponent)]
        except Exception as e:
            out['hs_err'] = str(e)[:80]
    pc = unreal.GameplayStatics.get_player_controller(w, 0)
    out['pc'] = pc.get_name() if pc else None
    out['inject'] = hasattr(unreal, 'JamPlayerInput') and hasattr(unreal.JamPlayerInput, 'debug_inject_key')
    try:
        k = unreal.Key()
        k.set_editor_property('key_name', 'Gamepad_LeftX')
        out['key_ok'] = str(k.get_editor_property('key_name'))
    except Exception as e:
        out['key_err'] = str(e)[:80]
    eng = unreal.find_object(None, '/Engine/Transient.UnrealEdEngine_0')
    out['fixed'] = [eng.get_editor_property('bUseFixedFrameRate'), eng.get_editor_property('FixedFrameRate')]
    try:
        out['cart'] = [gm.get_cartridge().get_editor_property('id'), gm.get_cartridge().get_editor_property('source_path')]
    except Exception as e:
        out['cart_err'] = str(e)[:80]
str(out)
