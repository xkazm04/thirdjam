import unreal, builtins, json
# Lane A47: the driver took its still; unpause and let the harness run to the next target (full speed again until 30 frames before it).
S = getattr(builtins, 'A47', None)
msg = 'no harness'
if S is not None:
    w = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
    if w is not None:
        unreal.GameplayStatics.set_global_time_dilation(w, 1.0)
        unreal.GameplayStatics.set_game_paused(w, False)
    S['paused_at'] = None
    if not S['pending']:
        S['done'] = True
    msg = json.dumps({'pending': S['pending'], 'done': S['done']})
msg
