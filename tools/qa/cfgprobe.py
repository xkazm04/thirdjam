import unreal
w = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
gm = unreal.GameplayStatics.get_game_mode(w)
g = gm.get_cartridge().get_editor_property('game')
sh = g.get_editor_property('shot'); cam = g.get_editor_property('camera'); sk = g.get_editor_property('skater'); ai = g.get_editor_property('ai')
hs = gm.get_human_skater()
cams = hs.get_components_by_class(unreal.JamFollowCamera)
cc = cams[0].get_editor_property('config') if cams else None
str({'aim_cone': [sh.get_editor_property('has_aim_cone'), sh.get_editor_property('aim_cone_deg')], 'shot': [sh.get_editor_property('speed'), sh.get_editor_property('reach_m'), sh.get_editor_property('cooldown_frames')],
     'cam_cart': [cam.get_editor_property('distance_m'), cam.get_editor_property('height_m'), cam.get_editor_property('look_ahead_m'), cam.get_editor_property('lag_frames'), cam.get_editor_property('fov_deg')],
     'cam_live': [cc.get_editor_property('distance_m'), cc.get_editor_property('height_m'), cc.get_editor_property('lag_frames'), cc.get_editor_property('fov_deg'), cc.get_editor_property('off_axis_m')] if cc else None,
     'skater': [sk.get_editor_property('speed'), sk.get_editor_property('accel'), sk.get_editor_property('lateral_grip'), sk.get_editor_property('glide_drag'), sk.get_editor_property('check_enabled')],
     'ai': [ai.get_editor_property('yield_margin_m'), ai.get_editor_property('yield_radius_m'), ai.get_editor_property('hold_radius_m'), ai.get_editor_property('speed_scale'), ai.get_editor_property('red_skill')],
     'mean_step_ms': round(gm.get_mean_step_ms(), 4), 'worst_step_ms': round(gm.get_worst_step_ms(), 3)})
