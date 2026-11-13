import unreal, builtins, gc
# Lane A45: tear v4.py down before StopPIE (live PIE references crash the editor at StopPIE, CAMERA-120M.md Errors 1).
msg = 'nothing registered'
S = getattr(builtins, 'A45', None)
if S is not None:
    S['done'] = True
    h = S.get('handle')
    if h is not None:
        unreal.unregister_slate_post_tick_callback(h)
    try:
        w = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
        if w is not None:
            unreal.GameplayStatics.set_global_time_dilation(w, 1.0)
    except Exception:
        pass
    n = S.get('n')
    S.clear()
    del builtins.A45
    msg = 'unregistered after %s frames' % n
gc.collect()
msg
