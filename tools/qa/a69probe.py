import unreal, json
w = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
a = unreal.GameplayStatics.get_all_actors_of_class(w, unreal.RinkActor)[0]
rs = a.get_editor_property('zone_rings')
r = rs[0]
o = {}
for k in ('scale','loc'):
    pass
o['scale']=str(r.get_world_scale()); o['loc']=str(r.get_world_location()); o['vis']=r.is_visible(); o['sec']=r.get_num_sections()
for n in ('component_has_tag','is_registered','is_render_state_created'):
    pass
try:
    o['bounds']=str(unreal.SystemLibrary.get_component_bounds(r))
except Exception as e: o['bounds']=repr(e)[:100]
try: o['mobility']=str(r.get_editor_property('mobility'))
except Exception as e: o['mob']=repr(e)[:80]
try: o['mat0']=str(r.get_material(0).get_name())
except Exception as e: o['mat']=repr(e)[:80]
json.dumps(o)
