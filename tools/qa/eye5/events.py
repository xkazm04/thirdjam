# Lane A226: the moments of a run worth a picture, from its game.log (goals, knockdowns, hits by the human, held cross-checks, saves, faceoffs, periods).
#   python events.py <run dir>
import sys, re, os
L = open(os.path.join(sys.argv[1], 'game.log'), encoding='utf-8', errors='replace').read().splitlines()
keep = ('JAMHUD goal_banner frame', 'JAMVIS knockdown', 'JAMHIT frame', 'JAMVIS crosshold', 'JAMVIS crosscheck', 'JAMEVENT faceoff', 'JAMEVENT period',
        'JAMGOALIE frame', 'JAMEVENT breakaway', 'JAMNET', 'JAMMENU measure end', 'JAMEVENT shot', 'JAMCART loaded', 'JAMMATCH', 'JAMSCRIPT human', 'JAMEVENT goal', 'JAMEVENT end', 'JAMHIT whiff')
last = {}
for l in L:
    l = re.sub(r'^\[[^\]]*\]\[[^\]]*\]', '', l)
    for k in keep:
        if k in l:
            if k == 'JAMGOALIE frame':
                m = re.search(r'frame=(\d+)', l); f = int(m.group(1)) if m else 0
                if f - last.get(k, -999) < 60: break
                last[k] = f
            print(l[:230]); break
