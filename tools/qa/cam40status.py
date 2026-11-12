import builtins, json
S = getattr(builtins, 'A40', None)
json.dumps({k: S[k] for k in ('n', 'done', 'err', 'shots', 'pending', 'last_frame', 'outside', 'min_margin')}) if S else 'no harness'
