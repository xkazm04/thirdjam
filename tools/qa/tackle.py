import unreal, builtins, json, math, os, re, time
# Lane A33 (tackling that reads): drives the human skater into the opposing carrier through the real input path (UJamPlayerInput.DebugInjectKey) and
# presses check inside 1.35 m above 5.3 m/s; listens to UJamEventBus for check / knockdown / board / poke / foul; runs the match at global time
# dilation __DILATION__ so one Python tick sees every sim frame; captures the PIE viewport every __CLIP_EVERY__ sim frames while the clip is on and every
# frame inside a hit window (within 2.5 m of the target, or the 24 frames after a hit) for the 8-frame strips. Tokens: OUT, MAX_FRAMES, HITS, DILATION, CLIP_EVERY.
# Cosmetic read only: the sim is driven through input like a player. Cleanup before StopPIE: tools/qa/stop33.py.

OUT = r'__OUT__'
MAX_FRAMES = int('__MAX_FRAMES__')
HITS_WANTED = int('__HITS__')
DILATION = float('__DILATION__')
CLIP_EVERY = int('__CLIP_EVERY__')
os.makedirs(OUT, exist_ok=True)

def mk_key(name):
    k = unreal.Key()
    k.set_editor_property('key_name', name)
    return k

KEYS = {'lx': mk_key('Gamepad_LeftX'), 'ly': mk_key('Gamepad_LeftY'), 'b': mk_key('Gamepad_FaceButton_Right')}

S = {'n': 0, 'done': False, 'err': None, 'events': [], 'hits': [], 'taken': [], 'boards': [], 'pres': None, 'bus': None, 'bus_bound': False,
     'pressed_b': False, 'last': None, 'sim_frame_last': -1, 'retreat_until': -1, 'first_frame': -1, 'shots': 0, 'clip_frames': [],
     'strip_frames': [], 'wall0': time.time(), 'target': -1, 'checks_pressed': 0, 'dilated': False, 'mode': 'approach', 'phase': ''}
builtins.A33 = S

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

def on_event(name, payload):
    try:
        b = S['bus']
        f = int(b.get_sim_frame()) if b else -1
        wh = payload.get_editor_property('where')
        e = {'f': f, 'name': str(name), 'who': int(payload.get_editor_property('who')), 'kind': str(payload.get_editor_property('kind')),
             'strength': round(float(payload.get_editor_property('strength')), 3), 'x': round(wh.x / 100.0, 2), 'y': round(wh.y / 100.0, 2)}
        if e['name'] in ('check', 'knockdown', 'board', 'poke', 'foul', 'lift'):
            S['events'].append(e)
    except Exception as ex:
        S['err'] = 'event: %s' % ex

def bind_bus(w):
    if S['bus_bound']:
        return
    gi = unreal.GameplayStatics.get_game_instance(w)
    for o in unreal.ObjectIterator(unreal.JamEventBus):
        if o.get_outer() == gi:
            S['bus'] = o
            o.on_jam_event.add_callable(on_event)
            S['bus_bound'] = True
            break

def phase_name(ph):
    m = re.search(r'(PLAY|GOAL_PAUSE|WIN|LOSE|PAUSED)', str(ph))
    return m.group(1) if m else str(ph)

def read(w):
    f = presentation(w).get_frame()
    sk = f.get_editor_property('skaters')
    hi = f.get_editor_property('human_index')
    skaters = []
    for s in sk:
        p = s.get_editor_property('pos_m')
        skaters.append((p.x, p.y, s.get_editor_property('yaw_deg'), s.get_editor_property('team_id')))
    pk = f.get_editor_property('puck_m')
    return {'frame': int(f.get_editor_property('sim_frame')), 'skaters': skaters, 'hi': hi, 'puck': (pk.x, pk.y),
            'phase': phase_name(f.get_editor_property('phase')), 'countdown': int(f.get_editor_property('faceoff_countdown_frames'))}

def inject(w, dx, dy, check):
    pc = unreal.GameplayStatics.get_player_controller(w, 0)
    cm = unreal.GameplayStatics.get_player_camera_manager(w, 0)
    yaw = math.radians(cm.get_camera_rotation().yaw)
    fx, fy = math.cos(yaw), math.sin(yaw)
    rx, ry = -math.sin(yaw), math.cos(yaw)
    # world (dx, dy) -> stick (right, forward)
    ax = dx * rx + dy * ry
    ay = dx * fx + dy * fy
    m = math.hypot(ax, ay)
    if m > 1.0:
        ax, ay = ax / m, ay / m
    unreal.JamPlayerInput.debug_inject_key(pc, KEYS['lx'], float(ax), True)
    unreal.JamPlayerInput.debug_inject_key(pc, KEYS['ly'], float(ay), True)
    if check and not S['pressed_b']:
        unreal.JamPlayerInput.debug_inject_key(pc, KEYS['b'], 1.0, True)
        S['pressed_b'] = True
    elif not check and S['pressed_b']:
        unreal.JamPlayerInput.debug_inject_key(pc, KEYS['b'], 0.0, True)
        S['pressed_b'] = False

def unit(x, y):
    m = math.hypot(x, y)
    return (x / m, y / m) if m > 1e-9 else (0.0, 0.0)

def take_shot(frame, tag):
    name = 'a33-%s-f%06d' % (tag, frame)
    unreal.AutomationLibrary.take_high_res_screenshot(1280, 720, name)
    S['shots'] += 1
    return name

def flush():
    status = {k: S[k] for k in ('n', 'done', 'err', 'hits', 'taken', 'boards', 'shots', 'checks_pressed', 'first_frame', 'mode', 'phase')}
    status['events'] = S['events'][-400:]
    status['clip_frames'] = len(S['clip_frames'])
    status['strip_frames'] = S['strip_frames'][-200:]
    status['wall_s'] = round(time.time() - S['wall0'], 1)
    status['last_frame'] = S['sim_frame_last']
    with open(os.path.join(OUT, 'status.json'), 'w') as fh:
        json.dump(status, fh)

def tick(dt):
    if S['done']:
        return
    try:
        w = world()
        if w is None:
            S['err'] = 'no PIE world'
            S['done'] = True
            flush()
            return
        bind_bus(w)
        if not S['dilated']:
            unreal.GameplayStatics.set_global_time_dilation(w, DILATION)
            S['dilated'] = True
        st = read(w)
        if st['frame'] == S['sim_frame_last']:
            return
        S['sim_frame_last'] = st['frame']
        S['n'] += 1
        S['phase'] = st['phase']
        if S['first_frame'] < 0:
            S['first_frame'] = st['frame']
        last = S['last']
        hi = st['hi']
        hx, hy, hyaw, my_team = st['skaters'][hi]
        v = (0.0, 0.0)
        if last is not None and st['frame'] == last['frame'] + 1:
            lx, ly = last['skaters'][hi][0], last['skaters'][hi][1]
            v = ((hx - lx) * 60.0, (hy - ly) * 60.0)
        speed = math.hypot(*v)
        px, py = st['puck']
        # the target: the opposing carrier (within 1.6 m of the puck), else the opponent nearest the human
        target, tdist, carrier = -1, 1e9, -1
        for i, (sx, sy, syaw, team) in enumerate(st['skaters']):
            if i == hi or team == my_team:
                continue
            if math.hypot(sx - px, sy - py) < 1.6:
                carrier = i
            d = math.hypot(sx - hx, sy - hy)
            if d < tdist:
                target, tdist = i, d
        if carrier >= 0:
            target = carrier
            tdist = math.hypot(st['skaters'][carrier][0] - hx, st['skaters'][carrier][1] - hy)
        S['target'] = target
        # events this frame: a knockdown of an opponent by a check = a hit landed; of the human = taken; board hits of the human
        for e in S['events']:
            if e.get('seen'):
                continue
            e['seen'] = True
            if e['name'] == 'knockdown' and e['kind'] == 'check':
                chk = next((c for c in S['events'] if c['name'] == 'check' and c['f'] == e['f']), None)
                rec = {'f': e['f'], 'victim': e['who'], 'strength': e['strength'], 'kind': chk['kind'] if chk else '?', 'checker': chk['who'] if chk else -1,
                       'human_speed': round(speed, 2), 'dist': round(tdist, 2)}
                if e['who'] == hi:
                    S['taken'].append(rec)
                else:
                    S['hits'].append(rec)
                S['retreat_until'] = st['frame'] + 150
                S['hit_frame'] = st['frame']
            elif e['name'] == 'board' and e['kind'] == 'skater':
                S['boards'].append(e)
        # control
        tx, ty, tyaw, _ = st['skaters'][target] if target >= 0 else (hx, hy, 0, 0)
        tv = (0.0, 0.0)
        if last is not None and st['frame'] == last['frame'] + 1 and target >= 0:
            tv = ((tx - last['skaters'][target][0]) * 60.0, (ty - last['skaters'][target][1]) * 60.0)
        check = False
        if st['phase'] != 'PLAY' or st['countdown'] > 0:
            mv = (0.0, 0.0)
            S['mode'] = 'wait'
        elif st['frame'] < S['retreat_until']:
            S['mode'] = 'retreat'
            ux, uy = unit(hx - tx, hy - ty)
            mv = (ux, uy)
            if tdist > 9.0:
                mv = (0.0, 0.0)
        else:
            S['mode'] = 'approach'
            lead = min(0.4, tdist / 12.0)
            ax, ay = tx + tv[0] * lead - hx, ty + tv[1] * lead - hy
            mv = unit(ax, ay)
            if tdist < 1.35 and speed > 5.3:
                check = True
                S['checks_pressed'] += 1
        inject(w, mv[0], mv[1], check)
        # captures: the clip every CLIP_EVERY frames once the first approach is within 8 m; every frame in the hit window
        hit_f = S.get('hit_frame', -10000)
        in_window = (S['mode'] == 'approach' and tdist < 2.5) or (st['frame'] - hit_f) < 24
        if 'clip_start' not in S and tdist < 8.0 and st['phase'] == 'PLAY':
            S['clip_start'] = st['frame']
        clip_on = 'clip_start' in S and (st['frame'] - S['clip_start']) < 600
        if clip_on and (st['frame'] - S['clip_start']) % CLIP_EVERY == 0:
            S['clip_frames'].append(take_shot(st['frame'], 'clip'))
        if in_window:
            S['strip_frames'].append(take_shot(st['frame'], 'win'))
        S['last'] = st
        if S['n'] % 60 == 0:
            flush()
        if len(S['hits']) >= HITS_WANTED or S['n'] >= MAX_FRAMES or st['phase'] in ('WIN', 'LOSE'):
            S['done'] = True
            inject(w, 0, 0, False)
            flush()
    except Exception as ex:
        S['err'] = '%s: %s' % (type(ex).__name__, ex)
        S['done'] = True
        flush()

S['handle'] = unreal.register_slate_post_tick_callback(tick)
'registered'
