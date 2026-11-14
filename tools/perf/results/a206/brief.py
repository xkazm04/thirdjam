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
