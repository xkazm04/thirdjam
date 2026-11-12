import unreal, builtins, gc, json
# Lane A40: tear the cam40 harness down before StopPIE (live PIE references crash the editor at StopPIE, CAMERA-120M.md Errors 1).
msg = 'nothing registered'
S = getattr(builtins, 'A40', None)
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
    msg = json.dumps({'n': S.get('n'), 'shots': S.get('shots'), 'outside': S.get('outside'), 'min_margin': S.get('min_margin'), 'err': S.get('err')})
    S.clear()
    del builtins.A40
gc.collect()
msg
