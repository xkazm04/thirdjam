// usage: node poss.mjs <log...>   who has the puck (the brain's carrier of record, TRACE car=), over all play frames and over the frames at least 10 s
// after the human's last touch (since >= 600); plus the longest-stretch table of idle.mjs's JSON if given. Lane A115.
import { readFileSync } from 'node:fs';
for (const f of process.argv.slice(2)) {
  const all = { h: 0, m: 0, o: 0, l: 0 }, idle = { h: 0, m: 0, o: 0, l: 0 }, long = { h: 0, m: 0, o: 0, l: 0 };
  for (const line of readFileSync(f, 'utf8').split(/\r?\n/)) {
    const i = line.indexOf('TRACE f=');
    if (i < 0) continue;
    const t = line.slice(i);
    if (t.includes(' frozen ')) continue;
    const s = /since=(\d+) ht=(\d) car=(-?\d+) lt=(-?\d+) fo=(\d)/.exec(t);
    if (!s || s[5] === '1' || s[2] === '1') continue;
    const sk = [...t.matchAll(/ (\d)([PCSBH])\(/g)].map((x) => ({ team: +x[1], role: x[2] }));
    const hi = sk.findIndex((k) => k.role === 'H'); if (hi < 0) continue;
    const car = +s[3];
    const who = car < 0 ? 'l' : car === hi ? 'h' : sk[car].team === sk[hi].team ? 'm' : 'o';
    all[who]++;
    if (+s[1] >= 600) idle[who]++;
    if (+s[1] >= 1200) long[who]++;
  }
  const pct = (o) => { const n = o.h + o.m + o.o + o.l || 1; return `human ${(100 * o.h / n).toFixed(0)} mate ${(100 * o.m / n).toFixed(0)} opp ${(100 * o.o / n).toFixed(0)} loose ${(100 * o.l / n).toFixed(0)} (n ${n})`; };
  console.log(f.split(/[\/]/).pop(), '| all play:', pct(all), '| 10 s+ since his touch:', pct(idle), '| 20 s+:', pct(long));
}
