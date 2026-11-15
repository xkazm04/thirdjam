# lane A217 (the Director's ruling 07:25): re-pin the constants the LADDER SET 3 profile moved, from an automation log's
# "Expected '...' to be "A", but it was "B". [file(line)]" / "to be N, but it was M. [file(line)]" lines. Replaces A by B on that line only (hex: with or
# without 0x / ull) and puts one comment line above it. Prints what it did; lines it cannot place are listed for a hand edit.
import re, sys
log = open(sys.argv[1], encoding='utf-8', errors='replace').read()
NOTE = '// re-pinned to LADDER SET 3, 15 Nov (the Director\'s ruling 07:25): the level profiles\' numbers changed, no rule did'
pat = re.compile(r"Expected '(.*?)' to be \"?([^\"\]]*?)\"?, but it was \"?([^\"\]]*?)\"?\. \[(.*?)\((\d+)\)\]")
edits = {}
for m in pat.finditer(log):
    what, a, b, path, line = m.group(1), m.group(2).strip(), m.group(3).strip(), m.group(4), int(m.group(5))
    edits.setdefault(path, []).append((line, a, b, what))
for path, es in edits.items():
    L = open(path, encoding='utf-8', newline='').read().split('\n')
    for line, a, b, what in sorted(es, key=lambda e: -e[0]):
        ha = re.match(r'^([0-9a-f]{16})', a); hb = re.match(r'^([0-9a-f]{16})', b)
        if ha and hb: a, b = ha.group(1), hb.group(1)
        # the constant may sit on the asserting line or in a table a few lines above it
        hit = None
        for k in range(line - 1, max(-1, line - 40), -1):
            if re.search(r'(?<![0-9a-fA-F])' + re.escape(a) + r'(?![0-9a-fA-F])', L[k]): hit = k; break
        if hit is None: print(f'HAND {path}:{line}: {what}: {a} -> {b}'); continue
        L[hit] = re.sub(r'(?<![0-9a-fA-F])' + re.escape(a) + r'(?![0-9a-fA-F])', b, L[hit], count=1)
        if NOTE not in L[hit - 1]:
            ind = re.match(r'^\s*', L[hit]).group(0)
            CR = chr(13); L.insert(hit, ind + NOTE + (CR if L[hit].endswith(CR) else "")); hit += 1
        print(f'PIN {path}:{hit + 1}: {a} -> {b}')
    open(path, 'w', encoding='utf-8', newline='').write('\n'.join(L))
