import unreal, builtins, json, os, math
# Lane A40 proof harness: in a running PIE match (jam.ScriptHuman 1, seed from the cartridge), fires a HighResShot at exact SIM frames,
# so a legacy run and a clamped run of the same cut land their stills on the same sim state; logs the human's position, the camera's
# position and whether the camera is inside the boards, one row per sim frame, to <OUT>/rows.jsonl.
# Tokens replaced by tools/qa/py.mjs --var: OUT, FRAMES (comma list of sim frames to shoot), TAG (file prefix), RES (e.g. 2560x1440),
# SLOW (time dilation during the shoot window so no sim frame is skipped; 1 = off).
# Inject BEFORE StartPIE (the tick waits for the PIE world), so sim frame 60 can be shot. Cleanup (before StopPIE!): tools/qa/cam40stop.py.

OUT = r'__OUT__'
FRAMES = sorted(int(x) for x in '__FRAMES__'.split(',') if x.strip())
TAG = '__TAG__'
RES = '__RES__'
SLOW = float('__SLOW__')
os.makedirs(OUT, exist_ok=True)

S = {'n': 0, 'done': False, 'err': None, 'shots': [], 'pending': list(FRAMES), 'last_frame': -1, 'rows': [], 'pres': None,
     'outside': 0, 'min_margin': 99.0, 'slowed': False, 'cam': None, 'seen_world': False}
builtins.A40 = S


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


def camera(w):
    # the follow camera on the possessed skater (the camera manager's view target)
    pc = unreal.GameplayStatics.get_player_controller(w, 0)
    if pc is None:
        return None
    cm = pc.player_camera_manager
    return cm


def flush():
    with open(os.path.join(OUT, 'rows.jsonl'), 'a') as f:
        for r in S['rows']:
            f.write(json.dumps(r) + '\n')
    S['rows'] = []
    with open(os.path.join(OUT, 'status.json'), 'w') as f:
        json.dump({k: S[k] for k in ('n', 'done', 'err', 'shots', 'pending', 'last_frame', 'outside', 'min_margin')}, f)


def inside_margin(x, y, L, W, rc):
    # distance (m) from the camera to the nearest board inner face, negative when outside; corner arcs included
    hx, hy = L * 0.5, W * 0.5
    ax, ay = abs(x), abs(y)
    if rc > 0 and ax > hx - rc and ay > hy - rc:
        d = math.hypot(ax - (hx - rc), ay - (hy - rc))
        return rc - d
    return min(hx - ax, hy - ay)


def tick(dt):
    if S['done']:
        return
    try:
        w = world()
        if w is None:
            # registered before StartPIE: wait for the world; after it was seen once, its loss means PIE ended
            if S['seen_world']:
                S['err'] = 'PIE world gone before the last shot' if S['pending'] else None
                S['done'] = True
                flush()
            return
        S['seen_world'] = True
        p = presentation(w)
        if p is None or not p.has_frame():
            return
        f = p.get_frame()
        frame = int(f.get_editor_property('sim_frame'))
        if frame == S['last_frame']:
            return
        S['last_frame'] = frame
        S['n'] += 1
        sk = f.get_editor_property('skaters')
        hi = f.get_editor_property('human_index')
        hp = sk[hi].get_editor_property('pos_m')
        L = f.get_editor_property('rink_length_m')
        W = f.get_editor_property('rink_width_m')
        try:
            rc = float(f.get_editor_property('corner_radius_m'))
        except Exception:
            rc = 0.0
        cm = camera(w)
        row = {'f': frame, 'hx': round(hp.x, 3), 'hy': round(hp.y, 3), 'hyaw': round(sk[hi].get_editor_property('yaw_deg'), 2)}
        if cm is not None:
            loc = cm.get_camera_location()
            rot = cm.get_camera_rotation()
            cx, cy, cz = loc.x / 100.0, loc.y / 100.0, loc.z / 100.0
            m = inside_margin(cx, cy, L, W, rc)
            row.update({'cx': round(cx, 3), 'cy': round(cy, 3), 'cz': round(cz, 3), 'cyaw': round(rot.yaw, 2), 'cpitch': round(rot.pitch, 2), 'margin': round(m, 3)})
            if m < 0:
                S['outside'] += 1
            S['min_margin'] = min(S['min_margin'], m)
        S['rows'].append(row)
        # slow the clock a little before the first pending shot so every sim frame gets its own render tick
        if S['pending'] and SLOW < 1.0 and not S['slowed'] and frame >= S['pending'][0] - 90:
            unreal.GameplayStatics.set_global_time_dilation(w, SLOW)
            S['slowed'] = True
        if S['pending'] and frame >= S['pending'][0]:
            target = S['pending'].pop(0)
            name = '%s-f%05d' % (TAG, frame)
            unreal.SystemLibrary.execute_console_command(w, 'HighResShot %s filename=%s' % (RES, name))
            S['shots'].append({'target': target, 'frame': frame, 'name': name})
            if not S['pending']:
                S['done'] = True
                if S['slowed']:
                    unreal.GameplayStatics.set_global_time_dilation(w, 1.0)
        if S['n'] % 60 == 0 or S['done']:
            flush()
    except Exception as e:
        S['err'] = repr(e)[:300]
        S['done'] = True
        try:
            flush()
        except Exception:
            pass


S['handle'] = unreal.register_slate_post_tick_callback(tick)
'registered tag=%s frames=%s out=%s' % (TAG, FRAMES, OUT)
