import builtins, json
S = getattr(builtins, 'A47', None)
json.dumps({k: S[k] for k in ('n', 'done', 'err', 'pending', 'last_frame', 'paused_at', 'shots')}) if S else 'no harness'
