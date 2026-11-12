import unreal, builtins, json, os
# Lane A47 proof harness: in a running PIE match, PAUSES the game at exact SIM frames (one at a time) so the driver (tools/qa/a47run.mjs) can take a
# CaptureGameViewport still with the UMG over it (HighResShot has no UI; a bark bubble, the banner and the commentary caption are UMG), then resumes
# through tools/qa/a47resume.py. Tokens replaced by tools/qa/py.mjs --var: OUT, FRAMES (comma list of sim frames). Inject BEFORE StartPIE (the tick waits
# for the PIE world). Cleanup (before StopPIE!): tools/qa/a47stop.py.

OUT = r'__OUT__'
FRAMES = sorted(int(x) for x in '__FRAMES__'.split(',') if x.strip())
os.makedirs(OUT, exist_ok=True)

S = {'n': 0, 'done': False, 'err': None, 'pending': list(FRAMES), 'last_frame': -1, 'paused_at': None, 'shots': [], 'seen_world': False, 'pres': None}
builtins.A47 = S


def world():
    return unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()


def presentation(w):
    if S['pres'] is None:
        wp = w.get_path_name()
        for o in unreal.ObjectIterator(unreal.JamPresentation):
            if o.get_outer().get_path_name() == wp:
                S['pres'] = o
                break
    return S['pres']


def flush():
    with open(os.path.join(OUT, 'shoot-status.json'), 'w') as f:
        json.dump({k: S[k] for k in ('n', 'done', 'err', 'pending', 'last_frame', 'paused_at', 'shots')}, f)


def tick(dt):
    if S['done']:
        return
    try:
        w = world()
        if w is None:
            if S['seen_world']:
                S['err'] = 'PIE world gone before the last shot' if S['pending'] else None
                S['done'] = True
                flush()
            return
        S['seen_world'] = True
        if S['paused_at'] is not None:
            return   # waiting for the driver's resume
        p = presentation(w)
        if p is None or not p.has_frame():
            return
        f = p.get_frame()
        frame = int(f.get_editor_property('sim_frame'))
        if frame == S['last_frame']:
            return
        S['last_frame'] = frame
        S['n'] += 1
        # slow the clock a little before a target so the pause lands on the frame itself, not a few frames past it
        if S['pending'] and frame >= S['pending'][0] - 30:
            unreal.GameplayStatics.set_global_time_dilation(w, 0.25)
        if S['pending'] and frame >= S['pending'][0]:
            target = S['pending'].pop(0)
            unreal.GameplayStatics.set_game_paused(w, True)
            S['paused_at'] = frame
            S['shots'].append({'target': target, 'frame': frame})
            flush()
            return
        if S['n'] % 60 == 0:
            flush()
    except Exception as e:
        S['err'] = repr(e)[:300]
        S['done'] = True
        try:
            flush()
        except Exception:
            pass


S['handle'] = unreal.register_slate_post_tick_callback(tick)
'registered frames=%s out=%s' % (FRAMES, OUT)
