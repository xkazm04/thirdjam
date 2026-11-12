import unreal, builtins, gc, json
# Lane A33: tear react33.py down (before StopPIE), restore time dilation, report the state.
S = getattr(builtins, 'A33R', None)
msg = 'nothing registered'
if S is not None:
    S['done'] = True
    h = S.get('handle')
    if h is not None:
        unreal.unregister_slate_post_tick_callback(h)
    try:
        w = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
        if w is not None:
            unreal.GameplayStatics.set_global_time_dilation(w, 1.0)
            unreal.SystemLibrary.execute_console_command(w, 'jam.HitHoldFrames 0')
    except Exception:
        pass
    msg = json.dumps({'n': S.get('n'), 'err': S.get('err'), 'case': S.get('case'), 'shots': len(S.get('shots', [])), 'yaw': S.get('yaw'), 'dist': S.get('target_dist_m'), 'sim_frame': S.get('sim_frame')})
    S.clear()
    del builtins.A33R
gc.collect()
msg
