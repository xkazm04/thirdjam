import unreal, json
# Lane A69: the temporary under-ice floor (a persistent static mesh wider than 50 m, top within -25..0 cm): __MODE__ = info | on | off (cast shadow).
w = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
out = []
for a in unreal.GameplayStatics.get_all_actors_of_class(w, unreal.StaticMeshActor):
    o, e = a.get_actor_bounds(False)
    if e.x < 2500 or e.y < 1000 or o.z + e.z > 1 or o.z + e.z < -30: continue
    c = a.static_mesh_component
    if '__MODE__' == 'on': c.set_cast_shadow(True)
    if '__MODE__' == 'off': c.set_cast_shadow(False)
    out.append({'name': a.get_name(), 'ext': [e.x, e.y], 'top': o.z + e.z, 'cast': c.get_editor_property('cast_shadow')})
json.dumps(out)
