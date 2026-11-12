import unreal
w = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
gm = unreal.GameplayStatics.get_game_mode(w)
ok = gm.load_cut(r'__PATH__')
str(ok)
