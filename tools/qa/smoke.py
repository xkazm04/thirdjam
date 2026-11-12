import unreal
out = {}
w = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
out['world'] = w.get_path_name()
gm = unreal.GameplayStatics.get_game_mode(w)
out['gm'] = gm.get_class().get_name()
pres = [o for o in unreal.ObjectIterator(unreal.JamPresentation) if o.get_outer().get_path_name() == w.get_path_name()]
out['pres'] = len(pres)
f = pres[0].get_frame()
out['frame'] = int(f.get_editor_property('sim_frame'))
sk = f.get_editor_property('skaters')
out['skaters'] = [[round(s.get_editor_property('pos_m').x, 1), round(s.get_editor_property('pos_m').y, 1), round(s.get_editor_property('yaw_deg')), s.get_editor_property('team_id'), s.get_editor_property('human')] for s in sk]
out['puck'] = [round(f.get_editor_property('puck_m').x, 2), round(f.get_editor_property('puck_m').y, 2)]
out['phase'] = str(f.get_editor_property('phase'))
out['score'] = [f.get_editor_property('score_home'), f.get_editor_property('score_away')]
pc = unreal.GameplayStatics.get_player_controller(w, 0)
cm = unreal.GameplayStatics.get_player_camera_manager(w, 0)
out['cam_yaw'] = round(cm.get_camera_rotation().yaw, 1)
out['cam_loc'] = [round(v) for v in (cm.get_camera_location().x, cm.get_camera_location().y, cm.get_camera_location().z)]
try:
    k = unreal.Key()
    k.set_editor_property('key_name', 'Gamepad_LeftY')
    out['key'] = str(k)
    unreal.JamPlayerInput.debug_inject_key(pc, k, 1.0, True)
    out['inject'] = 'ok'
except Exception as e:
    out['key_err'] = str(e)[:160]
hs = gm.get_human_skater()
out['human_comp'] = [c.get_class().get_name() for c in hs.get_components_by_class(unreal.ActorComponent)]
out['cart'] = str(gm.get_cartridge().get_editor_property('source_path'))
str(out)
