import unreal, json
# Lane A47: count the skater actors and every actor whose class name carries "SkaterVisual" in the PIE world (B-to-A-0030 fault 6: ghost figures).
w = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
out = {'world': w.get_path_name() if w else None, 'skaters': 0, 'visuals': 0, 'classes': {}}
if w is not None:
    for a in unreal.GameplayStatics.get_all_actors_of_class(w, unreal.Actor):
        cn = a.get_class().get_name()
        if cn == 'JamSkater':
            out['skaters'] += 1
        if 'SkaterVisual' in cn:
            out['visuals'] += 1
            out['classes'][cn] = out['classes'].get(cn, 0) + 1
json.dumps(out)
