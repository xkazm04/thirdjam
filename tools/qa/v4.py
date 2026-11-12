import unreal, builtins, json, math, os, re, time
# Lane A45 (C-0001 v4): a bot-only match (jam.ScriptHuman 1 puts the scripted human in the human seat) on humanoid_b1 with pro bots that check (R-0031 point 2).
# Nothing is injected: the driver listens to UJamEventBus (check / knockdown / board / poke / lift / foul), runs the match at global time dilation __DILATION__
# so one Python tick sees about every sim frame, and captures the PIE viewport every frame inside two windows: the HIT window (an opponent within 2.5 m of the human,
# and the 24 frames after a knockdown of the human) and the PIN window (a skater within 0.5 m of the boards with an opponent within 1.3 m, inside 18 m of the human).
# A pin span is the sim's rule read back from the presentation (10 frames on the boards with an opponent within 1.2 m); the JAMVIS pin lines in the editor log confirm it.
# Tokens: OUT, MAX_FRAMES, DILATION, MAX_SHOTS. Cosmetic read only. Cleanup before StopPIE: tools/qa/stop45.py.

OUT = r'__OUT__'
MAX_FRAMES = int('__MAX_FRAMES__')
DILATION = float('__DILATION__')
MAX_SHOTS = int('__MAX_SHOTS__')
HALF_L, HALF_W, R = 45.0, 22.5, 0.4   # hockey.v4.qa: 90 x 45, skater radius 0.4
os.makedirs(OUT, exist_ok=True)

S = {'n': 0, 'done': False, 'err': None, 'events': [], 'taken': [], 'landed': [], 'boards': [], 'stick_hits': [], 'pins': [], 'pres': None, 'bus': None,
     'bus_bound': False, 'last': None, 'sim_frame_last': -1, 'first_frame': -1, 'shots': 0, 'hit_frames': [], 'pin_frames': [], 'wall0': time.time(),
     'dilated': False, 'phase': '', 'onboard': {}, 'open_pins': {}}
builtins.A45 = S

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

def take_shot(frame, tag):
    name = 'a45-%s-f%06d' % (tag, frame)
    unreal.AutomationLibrary.take_high_res_screenshot(1280, 720, name)
    S['shots'] += 1
    return name

def board_dist(x, y):
    # distance from the skater centre to the nearest straight board (corners ignored: a pin in a corner still reads as near both)
    return min(HALF_L - abs(x), HALF_W - abs(y)) - R

def flush():
    status = {k: S[k] for k in ('n', 'done', 'err', 'taken', 'landed', 'boards', 'stick_hits', 'pins', 'shots', 'first_frame', 'phase')}
    status['events'] = S['events'][-400:]
    status['hit_frames'] = S['hit_frames'][-200:]
    status['pin_frames'] = S['pin_frames'][-200:]
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
        hi = st['hi']
        hx, hy, hyaw, my_team = st['skaters'][hi]
        # events this frame
        for e in S['events']:
            if e.get('seen'):
                continue
            e['seen'] = True
            if e['name'] == 'knockdown' and e['kind'] == 'check':
                chk = next((c for c in S['events'] if c['name'] == 'check' and c['f'] == e['f']), None)
                rec = {'f': e['f'], 'victim': e['who'], 'strength': e['strength'], 'kind': chk['kind'] if chk else '?', 'checker': chk['who'] if chk else -1}
                (S['taken'] if e['who'] == hi else S['landed']).append(rec)
                if e['who'] == hi:
                    S['hit_frame'] = st['frame']
            elif e['name'] == 'board' and e['kind'] == 'skater':
                S['boards'].append(e)
            elif e['name'] in ('poke', 'lift') and e['strength'] >= 0.5:
                S['stick_hits'].append(e)
        # windows
        near_opp = min((math.hypot(sx - hx, sy - hy) for i, (sx, sy, syaw, team) in enumerate(st['skaters']) if i != hi and team != my_team), default=99.0)
        hit_f = S.get('hit_frame', -10000)
        live = st['phase'] == 'PLAY' and st['countdown'] <= 0   # no captures through the faceoff countdown (the centres stand a metre apart at the dot)
        human_carries = math.hypot(st['puck'][0] - hx, st['puck'][1] - hy) < 1.6
        in_hit = live and ((near_opp < 2.5 and human_carries) or (st['frame'] - hit_f) < 24)
        in_pin = False
        for i, (sx, sy, syaw, team) in enumerate(st['skaters']):
            on = board_dist(sx, sy) < 0.5
            opp = min((math.hypot(ox - sx, oy - sy) for j, (ox, oy, oyaw, ot) in enumerate(st['skaters']) if ot != team), default=99.0)
            S['onboard'][i] = S['onboard'].get(i, 0) + 1 if on else 0
            pinned = S['onboard'][i] >= 10 and opp <= 1.2
            if pinned and i not in S['open_pins']:
                S['open_pins'][i] = st['frame']
            if not pinned and i in S['open_pins']:
                S['pins'].append({'skater': i, 'start': S['open_pins'].pop(i), 'end': st['frame'], 'dist_to_human': round(math.hypot(sx - hx, sy - hy), 1)})
            if on and opp <= 1.3 and math.hypot(sx - hx, sy - hy) < 18.0 and live:
                in_pin = True
        # Every frame for the 24 frames after a knockdown of the human or a pin start (the strips read hit-3..hit+18 every 3rd); every 3rd frame otherwise, because each
        # high-res capture stalls the editor tick and the sim with it.
        pin_recent = any(st['frame'] - f0 < 24 for f0 in S['open_pins'].values())
        every = 1 if ((st['frame'] - hit_f) < 24 or pin_recent) else 3
        if S['shots'] < MAX_SHOTS and st['frame'] % every == 0:
            if in_hit:
                S['hit_frames'].append(take_shot(st['frame'], 'win'))
            elif in_pin:
                S['pin_frames'].append(take_shot(st['frame'], 'pin'))
        S['last'] = st
        if S['n'] % 60 == 0:
            flush()
        if S['n'] >= MAX_FRAMES or st['phase'] in ('WIN', 'LOSE'):
            S['done'] = True
            flush()
    except Exception as ex:
        S['err'] = '%s: %s' % (type(ex).__name__, ex)
        S['done'] = True
        flush()

S['handle'] = unreal.register_slate_post_tick_callback(tick)
'registered'
