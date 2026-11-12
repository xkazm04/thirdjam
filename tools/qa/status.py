import builtins, json
S = getattr(builtins, 'A10', None)
json.dumps({k: S[k] for k in ('n', 'phase', 'phase_name', 'done', 'err', 'flushed', 'first_touch', 'goal_frames', 'shots', 'sim_frame_last', 'teleports')}) if S else 'no harness'
