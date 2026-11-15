# Lane A223 (lane A219 wrote it): the sim's hashes of two film runs (the game logs "JAMSIM frame=<n> hash=<h>" every 600 sim frames), compared over the frames both reached,
# and the strips both took. A presentation-only change must give the same hashes and the same strips.
#   python hashes.py <before run dir> <after run dir> [...more pairs]
import sys, re, os


def lines(d):
    out = {}
    with open(os.path.join(d, 'game.log'), encoding='utf-8', errors='replace') as f:
        for l in f:
            m = re.search(r'JAMSIM frame=(\d+) hash=([0-9a-f]+)', l)
            if m: out[int(m.group(1))] = m.group(2)
    return out


def strips(d):
    s = set()
    for n in os.listdir(os.path.join(d, 'hits')):
        m = re.match(r'^(a196-[a-z]+-\d+)-f\d+\.png$', n)
        if m: s.add(m.group(1))
    return s


bad = 0
a = sys.argv[1:]
for i in range(0, len(a) - 1, 2):
    B, A = lines(a[i]), lines(a[i + 1])
    common = sorted(set(B) & set(A))
    diff = [f for f in common if B[f] != A[f]]
    sb, sa = strips(a[i]), strips(a[i + 1])
    both = sorted(sb & sa)
    print('%s  %s vs %s: %d hash lines in common (to f%d), %d differ; strips in both %d (before only %d, after only %d)' % (
        'EQUAL' if common and not diff else 'DIFFER', os.path.basename(a[i]), os.path.basename(a[i + 1]), len(common), common[-1] if common else 0, len(diff), len(both), len(sb - sa), len(sa - sb)))
    if diff or not common: bad += 1
sys.exit(1 if bad else 0)
