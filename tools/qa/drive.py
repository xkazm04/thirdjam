import unreal, builtins, json, math, os, re, time
# Lane A10 feel QA: drives the human skater in a running PIE match through the real input path
# (UJamPlayerInput.DebugInjectKey -> Enhanced Input -> HokejGameMode::ReadHumanInput), one plan per run,
# and records one row per simulation frame to <OUT>/rows.jsonl (flushed every 60 frames) plus <OUT>/status.json.
# Tokens replaced by tools/qa/py.mjs --var: OUT, PLAN, SHOT_EVERY, MAX_FRAMES.
# Plans: skate (accel / glide / reverse-stop / circle), play (faceoff, chase, carry, shoot, score, repeat until win),
#        raw (play + body checks), boost (play + go for pickups when they spawn).
# Cleanup (before StopPIE!): tools/qa/stop.py unregisters the tick and drops every PIE reference.

OUT = r'__OUT__'
PLAN = '__PLAN__'
SHOT_EVERY = int('__SHOT_EVERY__')
MAX_FRAMES = int('__MAX_FRAMES__')
os.makedirs(OUT, exist_ok=True)

def mk_key(name):
    k = unreal.Key()
    k.set_editor_property('key_name', name)
    return k

KEYS = {'lx': mk_key('Gamepad_LeftX'), 'ly': mk_key('Gamepad_LeftY'),
        'a': mk_key('Gamepad_FaceButton_Bottom'), 'b': mk_key('Gamepad_FaceButton_Right')}

S = {'plan': PLAN, 'n': 0, 'phase': 0, 'phase_name': '', 'phase_start': 0, 'done': False, 'err': None,
     'rows': [], 'flushed': 0, 'last': None, 'pres': None, 'pressed': {'a': False, 'b': False},
     'first_touch': None, 'goal_frames': [], 'shots': 0, 'had_puck_frames': 0, 'carry_since': None,
     'hardturn_done': False, 'hardturn': None, 'lost_after_hardturn': None, 'checks_pressed': 0,
     'knock_seen': 0, 'faceoffs': 0, 'last_phase_str': '', 'far_frames': 0, 'wall0': time.time(),
     'pickup_target': None, 'pickups_taken': 0, 'shot_log': [], 'sim_frame_last': -1, 'teleports': 0}
builtins.A10 = S

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

def phase_name(ph):
    # str(unreal.JamPhase.PLAY) prints like "<JamPhase.PLAY: 0>"; keep the bare name
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
    return {'frame': int(f.get_editor_property('sim_frame')), 'skaters': skaters, 'hi': hi,
            'puck': (pk.x, pk.y), 'score': (f.get_editor_property('score_home'), f.get_editor_property('score_away')),
            'phase': phase_name(f.get_editor_property('phase')), 'attack': f.get_editor_property('attack_sign'),
            'len': f.get_editor_property('rink_length_m')}

def inject(w, dx, dy, shoot, check):
    pc = unreal.GameplayStatics.get_player_controller(w, 0)
    cm = unreal.GameplayStatics.get_player_camera_manager(w, 0)
    yaw = math.radians(cm.get_camera_rotation().yaw)
    fx, fy = math.cos(yaw), math.sin(yaw)
    rx, ry = -math.sin(yaw), math.cos(yaw)
    ax = dx * rx + dy * ry
    ay = dx * fx + dy * fy
    m = math.hypot(ax, ay)
    if m > 1.0:
        ax, ay = ax / m, ay / m
    unreal.JamPlayerInput.debug_inject_key(pc, KEYS['lx'], float(ax), True)
    unreal.JamPlayerInput.debug_inject_key(pc, KEYS['ly'], float(ay), True)
    for name, want in (('a', shoot), ('b', check)):
        if want and not S['pressed'][name]:
            unreal.JamPlayerInput.debug_inject_key(pc, KEYS[name], 1.0, True)
            S['pressed'][name] = True
        elif not want and S['pressed'][name]:
            unreal.JamPlayerInput.debug_inject_key(pc, KEYS[name], 0.0, True)
            S['pressed'][name] = False
    return ax, ay

def unit(x, y):
    m = math.hypot(x, y)
    return (x / m, y / m) if m > 1e-9 else (0.0, 0.0)

# ---- plans -------------------------------------------------------------------------------------------------

SKATE_PHASES = [('accel', 150), ('glide', 300), ('reverse', 150), ('circle', 240), ('settle', 60)]

def ctrl_skate(st, v):
    name = S['phase_name']
    if name == 'accel':
        return (1.0, 0.0, False, False)
    if name == 'glide':
        return (0.0, 0.0, False, False)
    if name == 'reverse':
        return (-1.0, 0.0, False, False)
    if name == 'circle':
        # steer 90 degrees left of the current velocity: a constant-rate turn
        vx, vy = v
        if math.hypot(vx, vy) < 0.5:
            return (-1.0, 0.0, False, False)
        ux, uy = unit(vx, vy)
        return (-uy, ux, False, False)
    return (0.0, 0.0, False, False)

def ctrl_play(st, v, want_check=False, want_pickup=False):
    hx, hy, hyaw, _ = st['skaters'][st['hi']]
    px, py = st['puck']
    goal_x = st['attack'] * (st['len'] / 2.0 - 4.0)
    dpx, dpy = px - hx, py - hy
    dist = math.hypot(dpx, dpy)
    has = dist < 1.45  # shot reach is 1.5 m
    shoot = False
    if has:
        S['had_puck_frames'] += 1
        if S['carry_since'] is None:
            S['carry_since'] = st['frame']
        carried = st['frame'] - S['carry_since']
        gx, gy = goal_x - hx, 0.0 - hy
        gd = math.hypot(gx, gy)
        ux, uy = unit(gx, gy)
        fx, fy = math.cos(math.radians(hyaw)), math.sin(math.radians(hyaw))
        facing = ux * fx + uy * fy
        # one hard 90 degree turn at the first long carry: does the puck stay on the stick? (it should not)
        if not S['hardturn_done'] and carried >= 60 and gd > 30:
            S['hardturn'] = st['frame']
            S['hardturn_done'] = True
        if S['hardturn'] is not None and st['frame'] - S['hardturn'] < 40:
            return (-uy, ux, False, False)
        if gd < 22.0 and facing > 0.985:
            shoot = True
            S['shots'] += 1
            S['shot_log'].append((st['frame'], round(gd, 1)))
        return (ux, uy, shoot, False)
    S['carry_since'] = None
    # not on the puck: go to the point just behind it, seen from the goal
    gx, gy = unit(goal_x - px, 0.0 - py)
    tx, ty = px - gx * 0.7, py - gy * 0.7
    dx, dy = tx - hx, ty - hy
    d = math.hypot(dx, dy)
    mv = unit(dx, dy) if d > 0.3 else (gx, gy)
    check = False
    if want_check:
        for i, (sx, sy, syaw, team) in enumerate(st['skaters']):
            if i == st['hi'] or team == st['skaters'][st['hi']][3]:
                continue
            od = math.hypot(sx - hx, sy - hy)
            if od < 1.3 and math.hypot(*v) > 5.2:
                check = True
                S['checks_pressed'] += 1
                break
    return (mv[0], mv[1], False, check)

def take_shot(st):
    name = 'A10-%s-%d-%s-%05d' % (PLAN, S['phase'], S['phase_name'], st['frame'])
    unreal.AutomationLibrary.take_high_res_screenshot(1280, 720, name)

def flush(final=False):
    rows = S['rows']
    if rows:
        with open(os.path.join(OUT, 'rows.jsonl'), 'a') as fh:
            for r in rows:
                fh.write(json.dumps(r) + '\n')
        S['flushed'] += len(rows)
        S['rows'] = []
    status = {k: S[k] for k in ('plan', 'n', 'phase', 'phase_name', 'phase_start', 'done', 'err', 'flushed', 'first_touch',
                                'goal_frames', 'shots', 'had_puck_frames', 'hardturn', 'lost_after_hardturn', 'checks_pressed',
                                'faceoffs', 'far_frames', 'pickups_taken', 'shot_log', 'teleports')}
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
            flush(True)
            return
        st = read(w)
        if st['frame'] == S['sim_frame_last']:
            return  # no sim step this engine frame
        last = S['last']
        hx, hy, hyaw, _ = st['skaters'][st['hi']]
        v = (0.0, 0.0)
        if last is not None and st['frame'] == last['frame'] + 1:
            lx, ly = last['skaters'][last['hi']][0], last['skaters'][last['hi']][1]
            v = ((hx - lx) * 60.0, (hy - ly) * 60.0)
            if math.hypot(hx - lx, hy - ly) > 3.0:
                S['teleports'] += 1
        px, py = st['puck']
        pv = (0.0, 0.0)
        if last is not None and st['frame'] == last['frame'] + 1:
            pv = ((px - last['puck'][0]) * 60.0, (py - last['puck'][1]) * 60.0)
        dist = math.hypot(px - hx, py - hy)
        if st['phase'] == 'PLAY' and dist > 25.0:
            S['far_frames'] += 1
        if S['first_touch'] is None and dist < 1.1:
            S['first_touch'] = st['frame']
        if S['hardturn'] is not None and S['lost_after_hardturn'] is None and st['frame'] - S['hardturn'] >= 40:
            S['lost_after_hardturn'] = dist > 1.5
        if st['phase'] != S['last_phase_str']:
            if st['phase'] == 'GOAL_PAUSE':
                S['goal_frames'].append(st['frame'])
            if S['last_phase_str'] == 'GOAL_PAUSE' and st['phase'] == 'PLAY':
                S['faceoffs'] += 1
            S['last_phase_str'] = st['phase']
        # phases
        if PLAN == 'skate':
            if S['phase_name'] == '':
                S['phase_name'] = SKATE_PHASES[0][0]
                S['phase_start'] = st['frame']
            elif st['frame'] - S['phase_start'] >= SKATE_PHASES[S['phase']][1]:
                S['phase'] += 1
                if S['phase'] >= len(SKATE_PHASES):
                    S['done'] = True
                else:
                    S['phase_name'] = SKATE_PHASES[S['phase']][0]
                    S['phase_start'] = st['frame']
            ctrl = ctrl_skate(st, v) if not S['done'] else (0.0, 0.0, False, False)
        else:
            if S['phase_name'] == '':
                S['phase_name'] = 'faceoff'
                S['phase_start'] = st['frame']
            # phase labels for the record: faceoff (first 30 frames after a faceoff) -> chase -> carry -> goal -> faceoff again
            if 'faceoff_frame' not in S:
                S['faceoff_frame'] = st['frame']
            if S['last_phase_str'] == 'PLAY' and last is not None and last['phase'] == 'GOAL_PAUSE':
                S['faceoff_frame'] = st['frame']
            label = 'goal' if st['phase'] == 'GOAL_PAUSE' else ('carry' if dist < 1.1 else ('faceoff' if st['frame'] - S['faceoff_frame'] < 30 else 'chase'))
            if label != S['phase_name']:
                S['phase_name'] = label
                S['phase'] += 1
                S['phase_start'] = st['frame']
            if st['phase'] == 'PLAY':
                ctrl = ctrl_play(st, v, want_check=(PLAN == 'raw'), want_pickup=(PLAN == 'boost'))
            else:
                ctrl = (0.0, 0.0, False, False)
            if st['phase'] in ('WIN', 'LOSE'):
                S['done'] = True
        if st['frame'] - (S['rows'][0]['f'] if S['rows'] else st['frame']) > MAX_FRAMES or S['n'] >= MAX_FRAMES:
            S['done'] = True
        ax, ay = inject(w, ctrl[0], ctrl[1], ctrl[2], ctrl[3]) if not S['done'] else inject(w, 0, 0, False, False)
        if SHOT_EVERY > 0 and (st['frame'] - S['phase_start']) < 300 and (st['frame'] - S['phase_start']) % SHOT_EVERY == 0:
            take_shot(st)
        # nearest opponent
        od = 999.0
        my_team = st['skaters'][st['hi']][3]
        for i, (sx, sy, syaw, team) in enumerate(st['skaters']):
            if i != st['hi'] and team != my_team:
                od = min(od, math.hypot(sx - hx, sy - hy))
        S['rows'].append({'f': st['frame'], 'ph': S['phase_name'], 'x': round(hx, 3), 'y': round(hy, 3), 'yaw': round(hyaw, 1),
                          'vx': round(v[0], 3), 'vy': round(v[1], 3), 'px': round(px, 3), 'py': round(py, 3),
                          'pvx': round(pv[0], 2), 'pvy': round(pv[1], 2), 'd': round(dist, 2), 'od': round(od, 2),
                          'sc': list(st['score']), 'sp': st['phase'], 'cx': round(ctrl[0], 2), 'cy': round(ctrl[1], 2),
                          'ax': round(ax, 2), 'ay': round(ay, 2), 'sh': int(ctrl[2]), 'ck': int(ctrl[3])})
        S['n'] += 1
        S['last'] = st
        S['sim_frame_last'] = st['frame']
        if S['n'] % 60 == 0 or S['done']:
            flush(S['done'])
    except Exception as e:
        S['err'] = repr(e)[:300]
        S['done'] = True
        try:
            flush(True)
        except Exception:
            pass

S['handle'] = unreal.register_slate_post_tick_callback(tick)
'registered plan=%s out=%s' % (PLAN, OUT)
