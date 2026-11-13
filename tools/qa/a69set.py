import unreal, json
# Lane A69: __TIER__ -> every RinkActor.set_zone_rings_for_tier (the rig stays as it is), then counts ProceduralMeshComponents on the rink actors.
w = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
acts = unreal.GameplayStatics.get_all_actors_of_class(w, unreal.RinkActor)
out = {'actors': len(acts)}
for a in acts:
    if '__TIER__' != 'count':
        a.set_zone_rings_for_tier('__TIER__')
    comps = a.get_components_by_class(unreal.ProceduralMeshComponent)
    out['pmc'] = len(comps)
    out['rings'] = a.get_zone_ring_count()
    out['shown'] = a.are_zone_rings_shown()
    out['sm'] = len(a.get_components_by_class(unreal.StaticMeshComponent))
json.dumps(out)
