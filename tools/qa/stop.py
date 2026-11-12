import unreal, builtins, gc
# Lane A10: tear the drive harness down before StopPIE (CAMERA-120M.md Errors 1: live PIE references crash the editor at StopPIE).
msg = 'nothing registered'
S = getattr(builtins, 'A10', None)
if S is not None:
    S['done'] = True
    h = S.get('handle')
    if h is not None:
        unreal.unregister_slate_post_tick_callback(h)
    n = S.get('n')
    S.clear()
    del builtins.A10
    msg = 'unregistered after %s frames' % n
gc.collect()
msg
