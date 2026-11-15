import re,sys
# brief.py <rows...>: the bars per cell, one line each
for f in sys.argv[1:]:
    for L in open(f,encoding='utf-8',errors='ignore'):
        if 'INTEREST cell' not in L: continue
        cell=re.search(r'@(\w+) player (\w+)',L); wins=re.search(r'human wins (\d+)',L).group(1)
        parts=L.split(' | ')
        def side(tag):
            t=[p for p in parts if p.startswith(tag)]
            i=parts.index(t[0]); seg=parts[i:i+8]; j=' | '.join(seg)
            g=lambda r: (re.search(r,j).groups() if re.search(r,j) else ('?',))
            sh=g(r'shots/min ([\d.]+) \((?:<10 \d+, 10-20 \d+, 20-30 \d+, >30 \d+); aimed (\d+) / (\d+) / (\d+) / (\d+)')
            return dict(fast=g(r'fast ([\d.]+)')[0],idle=g(r'idle ([\d.]+)')[0],shots=sh,att=g(r'att-third s/match ([\d.]+)')[0],
              carry=g(r'carries (\d+): shot (\d+) % pass (\d+) % lost (\d+)'),tonet=g(r'to-net (\d+)')[0],mean=g(r'mean ([\d.]+) s')[0],
              poss=g(r'possession ([\d.]+)')[0],pas=g(r'passes (\d+) done (\d+)'),nf=g(r'net front in att third (\d+)')[0])
        m,o=side('MATES'),side('OPPS')
        tail=' | '.join(parts[-12:])
        g=lambda r: (re.search(r,tail).group(1) if re.search(r,tail) else '?')
        print(f"{f.split('/')[-1]:10} {cell.group(1):6} {cell.group(2):7} wins {wins:>2}/20 | M aimed {'/'.join(m['shots'][1:])} sh/min {m['shots'][0]} netfront {m['nf']} att3 {m['att']} carry {'/'.join(m['carry'])} tonet {m['tonet']} pass% {m['pas'][1] if len(m['pas'])>1 else '?'} | O aimed {'/'.join(o['shots'][1:])} sh/min {o['shots'][0]} netfront {o['nf']} att3 {o['att']} fast {o['fast']} poss {o['poss']} carry {'/'.join(o['carry'])} pass% {o['pas'][1] if len(o['pas'])>1 else '?'} | open-ahead {g(r'open mate ahead ([\d.]+)')} over10 {g(r'skaters over 10 % idle (\d+)')} looseBot {g(r'within 1.5 s ([\d.]+)')} pressed {g(r'pressed \(opp within 4 m\) ([\d.]+)')}")
# goals for / against a match (the human's side), from the match lines
import re as _re
for f in sys.argv[1:]:
    gf=ga=n=0
    for L in open(f,encoding='utf-8',errors='ignore'):
        m=_re.search(r'INTEREST match \S+ seed \d+: \d+ frames, (\d+):(\d+)',L)
        if m: gf+=int(m.group(1)); ga+=int(m.group(2)); n+=1
    if n: print(f"{f.split('/')[-1]:10} goals a match: for {gf/n:.2f} against {ga/n:.2f} ({n} matches)")
# the human's hits a match (HITS fields of the match lines)
for f in sys.argv[1:]:
    cells={}
    for L in open(f,encoding='utf-8',errors='ignore'):
        m=_re.search(r'INTEREST match \S+@(\w+) seed .*HITS kd_given (\d+) checks_landed (\d+) presses (\d+) cross_landed (\d+) knocked_down (\d+)',L)
        if not m: continue
        c=cells.setdefault(m.group(1),[0,0,0,0,0,0]); c[0]+=1
        for i in range(5): c[i+1]+=int(m.group(i+2))
    for k,c in cells.items():
        n=c[0]; print(f"{f.split('/')[-1]:10} {k:7} HITS a match: bots felled by the human {c[1]/n:.2f}, his checks landed {c[2]/n:.2f} of {c[3]/n:.2f} presses ({100*c[2]/max(1,c[3]):.0f} %), cross-checks landed {c[4]/n:.2f}, the human knocked down {c[5]/n:.2f}")
# wave 3: pass quality, defence, far shots (per side)
for f in sys.argv[1:]:
    for L in open(f,encoding='utf-8',errors='ignore'):
        if 'INTEREST cell' not in L: continue
        cell=_re.search(r'@(\w+) player',L).group(1)
        for tag,m in zip(('MATES','OPPS'),_re.finditer(r'PASSQ re-passed within 15 f (\d+) % back to the passer (\d+) % done over 6 m (\d+) % of passes \| DEF \(defending\) frames (\d+) on-carrier (\d+) % crease (\d+) % receiver (\d+) % clump (\d+) % \| far shots against (\d+) crease held (\d+) %',L)):
            g=m.groups(); print(f"{f.split('/')[-1]:10} {cell:7} {tag:5} quick-repass {g[0]} % back-pass {g[1]} % done>6m {g[2]} % | defending: on carrier {g[4]} % crease {g[5]} % receiver {g[6]} % clump {g[7]} % | far shots against {g[8]}, crease held {g[9]} %")
