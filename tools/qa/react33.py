import unreal, builtins, math, os
# Lane A33: the reaction sheet. On a frozen PIE frame (global time dilation 0.0001, so the pawn stops driving its visual) the HUMAN's figure (always on the follow
# camera) is knocked down by hand through the C-0001 interface in four directions relative to its facing (front, side_l, side_r, behind) and, for front, with
# jam.HitHoldFrames 3; the visual is stepped one UpdateMotion per Python tick (frame-based, as the pawn would) and the viewport is captured every __EVERY__ frames
# for 44 frames down and 24 up. Cosmetic only: the sim is frozen and untouched. Token: EVERY. Cleanup: stop33r.py.
EVERY = int('__EVERY__')
CASES = [('front', (-1.0, 0.0), 0), ('side_l', (0.0, 1.0), 0), ('side_r', (0.0, -1.0), 0), ('behind', (1.0, 0.0), 0), ('front-hold3', (-1.0, 0.0), 3)]
S = {'n': 0, 'done': False, 'err': None, 'case': -1, 't': 0, 'shots': [], 'phase': 'init', 'yaw': 0.0}
builtins.A33R = S

def world():
    return unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()

def rot_dir(local, yaw_deg):
    # local (along, across) in the figure's frame -> rink (x, z): the sim's yaw grows to the right, the right vector is (-sin, cos)
    y = math.radians(yaw_deg)
    fx, fy = math.cos(y), math.sin(y)
    rx, ry = -math.sin(y), math.cos(y)
    return (local[0] * fx + local[1] * rx, local[0] * fy + local[1] * ry)

def tick(dt):
    if S['done']:
        return
    try:
        w = world()
        if w is None:
            S['err'] = 'no PIE world'; S['done'] = True; return
        if S['phase'] == 'init':
            # Freeze during the opening faceoff countdown (the two centres stand at the dot, the opposing one faces the camera about 8 m away), or at once
            # when the countdown is over; the figure is the non-human skater nearest the human (a bot's figure, not the one the camera belongs to).
            pres = None
            wp = w.get_path_name()
            for o in unreal.ObjectIterator(unreal.JamPresentation):
                if o.get_outer().get_path_name() == wp:
                    pres = o
            f = pres.get_frame() if pres else None
            cd = int(f.get_editor_property('faceoff_countdown_frames')) if f else 0
            sf = int(f.get_editor_property('sim_frame')) if f else 0
            S['sim_frame'] = sf
            if not ((cd > 0 and cd < 75) or sf > 400):
                return
            unreal.GameplayStatics.set_global_time_dilation(w, 0.0001)
            human = unreal.GameplayStatics.get_player_pawn(w, 0)
            hl = human.get_actor_location()
            best, bd = None, 1e9
            for a in unreal.GameplayStatics.get_all_actors_of_class(w, unreal.JamSkater):
                if a == human:
                    continue
                d = (a.get_actor_location() - hl).length()
                if d < bd:
                    best, bd = a, d
            S['pawn'] = best if best is not None else human
            S['target_dist_m'] = round(bd / 100.0, 2)
            S['vis'] = S['pawn'].get_visual_object()
            S['yaw'] = S['pawn'].get_actor_rotation().yaw
            S['phase'] = 'settle'; S['t'] = 0
            return
        if S['phase'] == 'settle':
            S['t'] += 1
            if S['t'] >= 10:
                S['phase'] = 'next'
            return
        vis = S['vis']
        facing = unreal.Rotator(0.0, S['yaw'], 0.0)
        if S['phase'] == 'next':
            S['case'] += 1
            if S['case'] >= len(CASES):
                S['done'] = True
                unreal.SystemLibrary.execute_console_command(w, 'jam.HitHoldFrames 0')
                return
            name, local, hold = CASES[S['case']]
            unreal.SystemLibrary.execute_console_command(w, 'jam.HitHoldFrames %d' % hold)
            dx, dz = rot_dir(local, S['yaw'])
            # side_l = the checker came from the left, so the push goes to the right (+right vector)
            vis.set_knocked_down(True, dx, dz)
            S['phase'] = 'down'; S['t'] = 0
            return
        if S['phase'] in ('down', 'up'):
            # Measured 19:49 and 19:54: at dilation 0.0001 only the first viewport capture after the freeze renders (the rest come back as a flat fill), so the
            # game runs at 1.0 for the one tick that captures (the pawn steps the visual by about one frame itself on that tick) and is frozen again right after.
            if S.get('thaw'):
                unreal.GameplayStatics.set_global_time_dilation(w, 0.0001)
                S['thaw'] = False
                return
            if S['t'] % EVERY == 0:
                unreal.GameplayStatics.set_global_time_dilation(w, 1.0)
                shot = 'a33-react-%s-%s-f%03d' % (CASES[S['case']][0], S['phase'], S['t'])
                unreal.AutomationLibrary.take_high_res_screenshot(1280, 720, shot)
                S['shots'].append(shot)
                S['thaw'] = True
            else:
                vis.update_motion(0.0, 0.0, facing)
            S['t'] += 1
            if S['phase'] == 'down' and S['t'] >= 45:
                vis.set_knocked_down(False, 0.0, 0.0)
                S['phase'] = 'up'; S['t'] = 0
            elif S['phase'] == 'up' and S['t'] >= 24:
                S['phase'] = 'next'
            S['n'] += 1
    except Exception as ex:
        S['err'] = '%s: %s' % (type(ex).__name__, ex)
        S['done'] = True

S['handle'] = unreal.register_slate_post_tick_callback(tick)
'registered'
