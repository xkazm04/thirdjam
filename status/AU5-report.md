# Lane AU5 report: rougher crowd, cross-check, glass rattle (device B), 2026-11-13 13:39

- All seven items are delivered: 16 takes kept, 12 rejected candidates and 3 previews, beside the AU4 files in `C:/Users/kazda/jam/audio-lab/tackle/`. `pick.html`, `manifest.json` and `READY.txt` are updated; the AU4 files are untouched.
- **Nobody has listened to any of this, and I cannot.** Every statement is a measurement. "Rough", "aggressive" and "rowdy" cannot be measured: I can say a take is loud at once, bright, wordless by the tonality test, or rhythmic, not that it sounds brutal.
- Service: 28 requests, all succeeded, none retried, 1,068 credits (1,486 with AU4; 129,307 left). No Unreal, no git write, no file in the repository except this report.
- Five things to know before importing:
  1. **Chants: the first three had no rhythm at all.** With the crowd prefix the service returned plain crowd noise. Three rhythm-led prompts without the prefix gave two takes with a measurable pulse: `CrowdChant_a` (one pulse every 0.32 s, 188 per minute) and `CrowdChant_b` (every 0.26 s, 231 per minute). `CrowdChant_a` is claps and voices with near-silence between pulses, so it may sound like a small group rather than a crowd.
  2. **The rough bed pulses.** All three beds are about 7 dB louder in their second half; looped, that swell returns every 9 s (`BedRough_a` swings 8.1 dB over 1 s windows, against 5.6 dB for `Bed_b`). No pounding shows in the numbers either.
  3. **Levels I chose:** roar -18 LUFS (2 LU over the "ooh", 2 LU under a goal cheer); chant -26 LUFS (a level at which the default's claps stay under -1.5 dBTP without a limiter; they peak at -2.1). Goal, groan and bed follow the existing takes (-16, -20, -30).
  4. **The glass rattle does what the boards lacked, by the numbers.** With `GlassRattle_a` from 0.03 s at -6 dB, content above 1.5 kHz stays 14 to 29 dB under the boom for the first 0.32 s after the hit; `BoardsBody_a` alone was 45 dB down by 0.12 s.
  5. **The rough goal differs little from the existing cheers by the numbers** (centroid 1035 to 1081 Hz against 1020 to 1057 Hz). Whether it sounds rougher is for the ear.

## 1. What was generated

- Same tool and settings as AU4: `C:/Users/kazda/jam/eleven-tools/eleven.mjs sfx`, model `eleven_text_to_sound_v2`, `prompt_influence` 0.5, output `mp3_44100_192`; the beds with the service's loop flag.
- **28 requests, 28 succeeded, none failed, none retried**: 25 in the first round, then 3 more chants after the first three measured as noise. Credits by the response header: 1,068. The quota endpoint reads 1,486 used after both lanes (418 before this one).
- Ledger: the 28 new lines are appended to `C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl` (68 lines in all). Raw mp3 files: `tackle/raw/`.
- The key was read inside `eleven.mjs` and sent only in the request header; never printed, logged or copied. The prompts hold generic sound descriptions only.

Prefixes:
- Rough crowd prefix (lane AU2's crowd prefix with the crowd described as rough, rowdy and loud): "Real recording of a small, rough, rowdy indoor ice hockey rink crowd, a few hundred people, close and loud, not a stadium roar. No horn, no siren, no organ, no music, no announcer, no whistle, no intelligible words."
- Foley prefix (as AU4): "Close, dry sound effect recorded in an indoor ice rink, isolated, no reverb tail, no crowd, no voices, no music, no whistle."
- The last three chants carry no prefix: the whole prompt is in the list.

- CrowdRoar_1 (rough crowd prefix; 2 s asked; CrowdRoar_b): A sudden short aggressive roar of approval as a player is flattened by a huge body check: a fast swell of rough shouting voices, over in under two seconds, then quiet. No applause, no clapping afterwards.
- CrowdRoar_2 (rough crowd prefix; 2 s asked; CrowdRoar_a): The crowd erupts in one short savage roar at a crushing hit, harsh throaty yells bursting out at once and dying away quickly. No applause, no cheering tail.
- CrowdRoar_3 (rough crowd prefix; 2 s asked; CrowdRoar_c): A brief fierce roar from the whole crowd at once, like a fight crowd reacting to a knockdown: loud rough yells swelling fast and falling within two seconds. No clapping.
- CrowdRoar_4 (rough crowd prefix; 2 s asked; rejected): One short explosive roar of delight from a rowdy crowd at a big hit, mostly men bellowing, a fast rise and a quick fall, ending quietly. No applause.
- CrowdGroan_1 (rough crowd prefix; 2 s asked; rejected): A collective wordless disappointed groan, ohhh, falling in pitch and dying away within two seconds. No applause, no cheering.
- CrowdGroan_2 (rough crowd prefix; 2 s asked; CrowdGroan_b): The whole crowd lets out one long deflated ohhh of frustration as a chance is missed, sliding downward and fading out. No clapping.
- CrowdGroan_3 (rough crowd prefix; 2 s asked; CrowdGroan_a): A rough collective groan of disappointment and annoyance, awww, starting strong and sinking away within two seconds, then quiet. No applause.
- CrowdChant_1 (rough crowd prefix; 6 s asked; rejected): A rowdy rhythmic crowd pattern at a steady tempo: stomp, stomp, clap, repeated over and over, feet pounding the bleachers and hands clapping together in time, no singing.
- CrowdChant_2 (rough crowd prefix; 6 s asked; rejected): A rowdy crowd chanting a rhythmic wordless hey, hey, hey in unison at a steady tempo with a clap on every beat, no melody.
- CrowdChant_3 (rough crowd prefix; 6 s asked; rejected): A rough crowd pounding on the boards and bleachers in a steady unison rhythm, thump, thump, clap, with grunted wordless shouts on the beat, constant tempo.
- BedRough_1 (rough crowd prefix; 10 s asked, loop flag on; rejected): A restless rowdy crowd between plays: a loud rough murmur, scattered wordless shouts and jeers, fists pounding on the boards now and then, constant level.
- BedRough_2 (rough crowd prefix; 10 s asked, loop flag on; BedRough_b): An impatient, noisy crowd waiting for play to resume: gruff chatter, occasional yells, a few people banging on the boards and glass, steady level, no cheering peaks.
- BedRough_3 (rough crowd prefix; 10 s asked, loop flag on; BedRough_a): Rowdy spectators milling and heckling, a dense rough murmur with sporadic wordless shouts and dull thumps on the boards, even level.
- GoalRough_1 (rough crowd prefix; 3.5 s asked; GoalRough_b): The crowd erupts at a goal: a sudden huge rough roar right from the first instant, loud immediately, with people pounding on the glass and boards, decaying naturally.
- GoalRough_2 (rough crowd prefix; 3.5 s asked; GoalRough_a): A sudden explosive roar of a rowdy crowd right from the first instant, loud immediately, fierce yelling and fists hammering on the plexiglass, then falling away.
- GoalRough_3 (rough crowd prefix; 3.5 s asked; rejected): A wild eruption of roaring and yelling right from the first instant, loud immediately, with heavy banging on the boards and glass mixed in, decaying naturally.
- CrossCheck_1 (foley prefix; 0.5 s asked; CrossCheck_a): A hockey stick shaft driven hard into a padded player's back: a sharp woody crack with a padded thud under it. One impact only, very short.
- CrossCheck_2 (foley prefix; 0.5 s asked; rejected): A wooden hockey stick slammed crosswise against shoulder pads: a loud dry crack of wood on hard plastic with a dull body thump. One impact only, very short.
- CrossCheck_3 (foley prefix; 0.5 s asked; CrossCheck_b): A cross-check in ice hockey: the shaft of a stick cracks against a padded back, a snappy wooden whack and a muffled thud. One impact only, very short.
- CrossCheck_4 (foley prefix; 0.5 s asked; rejected): A hard whack of a wooden stick shaft on a padded torso, a sharp crack like wood hitting a thick padded jacket, with body weight behind it. One impact only, very short.
- CrossCheck_5 (foley prefix; 0.5 s asked; CrossCheck_c): A composite hockey stick shaft smacking across a player's padded back: a bright snappy crack and a short padded thump. One impact only, very short.
- GlassRattle_1 (foley prefix; 1 s asked; rejected): An arena plexiglass panel rattling and shuddering in its metal frame after something hit the wall below it, a loose buzzing rattle that settles within a second. No impact, no thud.
- GlassRattle_2 (foley prefix; 1 s asked; GlassRattle_b): A large sheet of plexiglass shaking and clattering in aluminium supports, a quick wobbling rattle dying away. No impact, no thud.
- GlassRattle_3 (foley prefix; 1 s asked; GlassRattle_a): Rink glass panels vibrating and rattling against their metal stanchions for half a second after a jolt, a bright clattering shudder that fades. No impact, no thud.
- GlassRattle_4 (foley prefix; 1 s asked; rejected): A loose acrylic panel in a metal frame chattering rapidly and then settling, a light metallic and plastic rattle. No impact, no thud.
- CrowdChant_4 (no prefix; 6 s asked; CrowdChant_b): A crowd of a hundred people in a small arena performing a stomp, stomp, clap rhythm in tight unison: two heavy foot stomps on wooden bleachers, then one loud hand clap, repeated over and over at a steady 100 beats per minute. Only stomps and claps, no voices, no cheering, no music, no words.
- CrowdChant_5 (no prefix; 6 s asked; CrowdChant_a): A sports crowd clapping in unison in a slow steady rhythm, clap, clap, clap, about two claps per second, with a short gruff wordless hey shouted together on every clap. Tight and rhythmic, no cheering, no music, no melody, no intelligible words.
- CrowdChant_6 (no prefix; 6 s asked; rejected): Hockey fans banging their fists on the boards in unison in a steady repeating rhythm, thump, thump, thump-thump-thump, pause, again and again at the same tempo, in a small indoor rink. Only the rhythmic banging and a low crowd murmur, no cheering, no music, no words.

## 2. Processing

Scripts: `gen.mjs`, `proc.mjs`, `tempo.mjs`, `select.mjs`, `page.mjs`, `report.mjs` in `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/au5/`, also copied to `C:/Users/kazda/jam/audio-lab/tackle/scripts/au5/` beside the AU4 scripts you saved. They use AU4's `lib.mjs`.

- **Crowd one-shots** (roar, groan, chant, goal): stereo; lane AU2's trim (10 ms before the first sample over -45 dBFS, to 100 ms after the last over -55 dBFS), held inside the length asked. Fade-in 30 ms (roar), 60 ms (groan), 10 ms (chant), 2 ms (goal, as the existing cheers); fade-out 0.25 s (roar, groan) or 0.2 s. A chant also ends 100 ms after its last frame within 40 dB of the loudest, because the service left up to 1.4 s of near-silence at the end.
- **Bed:** lane AU2's loop method unchanged: 10 s generated, the last second cross-faded (equal power) into the first, 9 s kept, the seam inside the file.
- **Cross-check:** AU4's impact rule (start 1.5 ms before the transient, 1 ms fade-in, 50 ms fade-out). **Glass rattle:** start 5 ms before the first 10 ms frame within 12 dB of the loudest, 3 ms fade-in, 100 ms fade-out.
- **Level:** one linear gain per file; no limiter, equaliser or compressor; 16-bit, no dither; no file reaches full scale. Crowd takes are set by lane AU2's method (EBU R128 integrated, unpadded); the table gives the padded reading used in AU4, which differs by 0.5 LU at most.
- **Rhythm:** the positive change of the 10 ms band energy (under 300 Hz, 300 Hz to 2 kHz, over 2 kHz), autocorrelated over lags of 0.2 to 1.6 s; the strongest lag is the pulse and its autocorrelation the regularity. Takes with no rhythm (the beds, the goal cheers, the first three chants) read 0.07 to 0.18.

## 3. The files

48 kHz 16-bit WAV; the crowd takes stereo, cross-check and glass rattle mono.

| file | pick | from generation | length s | LUFS | true peak dBTP | loudest at ms | 20 dB down after ms | centroid Hz | note |
|---|---|---|---|---|---|---|---|---|---|
| `CrowdRoar_a.wav` | **default** | CrowdRoar_2 | 2 | -18 | -10.7 | 175 | 1180 | 1076 |  |
| `CrowdRoar_b.wav` | alternate | CrowdRoar_1 | 2 | -18.2 | -12.6 | 200 | 1485 | 1086 |  |
| `CrowdRoar_c.wav` | alternate | CrowdRoar_3 | 2 | -18.2 | -10.6 | 180 | 1485 | 998 |  |
| `CrowdGroan_a.wav` | **default** | CrowdGroan_3 | 2 | -20.1 | -13.7 | 225 | 1425 | 835 |  |
| `CrowdGroan_b.wav` | alternate | CrowdGroan_2 | 2 | -20.5 | -14.8 | 470 | 1295 | 905 |  |
| `CrowdChant_a.wav` | **default** | CrowdChant_5 | 5.715 | -26.1 | -2.1 | 1605 | 3895 | 1661 | pulse every 0.32 s (188 per minute), regularity 0.66 |
| `CrowdChant_b.wav` | alternate | CrowdChant_4 | 4.655 | -26.1 | -15 | 170 | 4195 | 1277 | pulse every 0.26 s (231 per minute), regularity 0.47 |
| `BedRough_a.wav` | **default** | BedRough_3 | 9 | -30.2 | -22.4 | - | - | 1081 | loop; seam step 0 dB, sample step 0.06; 1 s level swings 8.1 dB |
| `BedRough_b.wav` | alternate | BedRough_2 | 9 | -30.2 | -18.3 | - | - | 1072 | loop; seam step -1.1 dB, sample step 0.87; 1 s level swings 8.6 dB |
| `GoalRough_a.wav` | **default** | GoalRough_2 | 3.48 | -16.3 | -10.2 | 395 | 2440 | 1081 |  |
| `GoalRough_b.wav` | alternate | GoalRough_1 | 3.48 | -16.4 | -9.6 | 475 | 2575 | 1035 |  |
| `CrossCheck_a.wav` | **default** | CrossCheck_1 | 0.25 | -22.4 | -3 | 5 | 50 | 809 | lead-in 3.1 ms; 40 dB down after 130 ms |
| `CrossCheck_b.wav` | alternate | CrossCheck_3 | 0.25 | -23 | -3 | 0 | 20 | 1582 | lead-in 1.3 ms; 40 dB down after 70 ms |
| `CrossCheck_c.wav` | alternate | CrossCheck_5 | 0.33 | -25.8 | -3 | 0 | 25 | 1317 | lead-in 3 ms; 40 dB down after 40 ms |
| `GlassRattle_a.wav` | **default** | GlassRattle_3 | 0.585 | -22.6 | -3 | 45 | 260 | 4285 | 5 bursts; 40 dB down after 480 ms |
| `GlassRattle_b.wav` | alternate | GlassRattle_2 | 0.795 | -20.5 | -3 | 85 | 300 | 690 | 3 bursts; 40 dB down after 525 ms |

Existing takes, measured the same way:

| existing sample | length s | LUFS | true peak dBTP | centroid Hz | 1 s level swing dB |
|---|---|---|---|---|---|
| `crowd/GoalFor_d.wav` (goal cheer) | 4 | -16.4 | -8 | 1040 | 21.7 |
| `crowd/GoalFor_c.wav` (goal cheer) | 4 | -16.1 | -6.4 | 1020 | 12.2 |
| `crowd/GoalAgainst_a.wav` (groan on a conceded goal) | 3 | -20.1 | -11.3 | 1182 | 12.3 |
| `crowd/Save_b.wav` (save reaction) | 2 | -20.1 | -7.3 | 1443 | 7.6 |
| `crowd/Bed_b.wav` (the bed in the game) | 9 | -30.1 | -12.3 | 1953 | 5.6 |
| `crowd/Bed_a.wav` (alternate bed) | 9 | -30.2 | -11.7 | 960 | 4.1 |
| `tackle/CrowdHit_a.wav` (the AU4 "ooh") | 1.79 | -20.3 | -11.7 | 737 | 3.9 |
| `tackle/Hit_a.wav` (the AU4 body check) | 0.3 | -22 | -3 | 286 | - |

## 4. How each default was chosen

The rules and thresholds are mine and rest on these 28 takes only. "dB of the take" means the share of the take's energy in that band.

**Crowd roar on a knockdown** (asked for: short aggressive roar of approval, a fast swell, 1.5 to 2.0 s, wordless, no applause tail). Pass: lasts 1.5 s or more; loud at once (the first 150 ms at or over the level of the whole take); falls (20 dB down before the 2.0 s mark). Order: no tonality flag first (lane AU2 test), then the take that has fallen furthest by the end. Level: -18 LUFS, 2 LU over the "ooh" and 2 LU under the goal cheers; that level is my choice.
- `CrowdRoar_a`: default: at full level inside 0.1 s, 20 dB down at 1.36 s and gone by 1.9 s on its own; no tonality flag; brighter than the "ooh" (centroid 1076 Hz against 737 Hz, 1.5 to 5 kHz at -9 dB against -19 dB); almost nothing over 5 kHz (-29 dB), so no applause by the numbers.
- `CrowdRoar_b`: holds full level for 0.9 s and is 20 dB down only at 1.7 s; still 23 dB under its loudest at the 2.0 s cut, closed with a 0.25 s fade; very wide stereo (L/R correlation 0.12).
- `CrowdRoar_c`: like b in shape, but flagged tone-ish (prominence 18.3 dB at 809 Hz): a voice or whoop may stand out.
- Rejected: `CrowdRoar_4` (too short: stops dead 1.0 s in (24 dB drop inside 40 ms), silence after).

**Crowd groan on a save** (asked for: collective wordless disappointed groan, "ohhh", falling, 1.5 to 2.0 s). Pass: no tonal flag; the level falls through the take. Order: no tone-ish flag first, then the largest fall in pitch, read as the spectral centroid of the last 0.7 s against the first 0.7 s. Level: -20 LUFS, as Save_* and GoalAgainst_*.
- `CrowdGroan_a`: default: the largest fall (centroid 870 Hz at the start, 606 Hz at the end), the darkest (835 Hz overall, the most energy at 120 to 400 Hz: -12 dB); loudest at 0.2 s, 10 dB down at 1.0 s; fills the 2.0 s asked and is closed with a 0.25 s fade (22 dB down at the cut).
- `CrowdGroan_b`: falls from 968 to 749 Hz; holds its level for 0.9 s, 10 dB down at 1.2 s; 20 dB down at the cut.
- Rejected: `CrowdGroan_1` (passes, but third: flagged tone-ish (ratio 0.124, prominence 18 dB at 797 Hz) and the smallest fall (993 to 777 Hz)).

**Chant** (asked for: rowdy rhythmic wordless chant, stomp-stomp-clap or "hey, hey, hey", 4 to 6 s, steady tempo). Pass: a rhythm that can be measured (autocorrelation of the onsets at least 0.3; takes with no rhythm, such as a bed or a cheer, read 0.07 to 0.18). Order: that regularity, higher first. Level: -26 LUFS, set so that the default (short loud claps) stays under -1.5 dBTP without a limiter (it peaks at -2.1 dBTP).
- `CrowdChant_a`: default: the most regular (0.66): one pulse every 0.32 s, 188 per minute, with a louder accent about every 1.5 s; claps and voices (centroid 1.7 kHz), no stomps; between pulses the level drops 35 dB or more, so it may sound like a small group in a dry room; raw nearly mono (correlation 1.0).
- `CrowdChant_b`: regular (0.47): one pulse every 0.26 s, 231 per minute; stomps and claps (energy under 120 Hz -5 dB, over 1.5 kHz -5 dB); the pattern stops after 4.5 s and the file ends there (4.66 s).
- Rejected: `CrowdChant_1` (no measurable rhythm (0.14, in the range of a plain cheer); a crowd noise that decays over 5 s); `CrowdChant_2` (no measurable rhythm (0.11)); `CrowdChant_3` (no measurable rhythm (0.10); fades in over 0.3 s); `CrowdChant_6` (weak rhythm (0.27) and flagged tone-ish (287 Hz): irregular low thumps, centroid 155 Hz).

**A rougher bed** (asked for: restless rowdy crowd between plays, murmur, shouts, pounding on the boards, 8 to 10 s, loopable). Loop made as lane AU2 did (the last second cross-faded into the first; 10 s generated, 9 s kept). Pass: no tonal flag; seam level step under 2 dB. Order: the smallest swing of the 1 s level through the loop (a swing returns every 9 s when looped; Bed_a swings 4.1 dB, Bed_b 5.6 dB). Level: -30 LUFS, as Bed_*.
- `BedRough_a`: default: the smallest swing (8.1 dB) and the cleanest seam (0.0 dB step, sample step 0.06); still about 7 dB louder in its second half than in its first; a dense murmur with no sharp peaks (true peak 8 dB over its loudness, against 18 dB for Bed_b), so no pounding shows in the numbers.
- `BedRough_b`: swing 8.6 dB, seam step -1.1 dB; the same quiet first half and louder second half; true peak 12 dB over its loudness.
- Rejected: `BedRough_1` (the largest swing (11.5 dB: a swell from 6 to 8 s) and narrow stereo (L/R correlation 0.81 against 0.25 for Bed_b)).

**A rougher goal reaction** (asked for: small arena erupting, a roar with pounding on the glass, 2.5 to 3.5 s, no horn). Lane AU2 rule for goal cheers (no tonal flag, then no tone-ish flag, then a fast onset), with one step added before the onset: separate bursts in the level, which is where pounding would show. Level: -16 LUFS, as GoalFor_*.
- `GoalRough_a`: default: no flag; loud at once (lead-in 2.3 ms, first 150 ms 0.5 dB over the take); four separate bursts in the decay from 2 s on, which may be the pounding; 3.48 s.
- `GoalRough_b`: no flag; lead-in 1.3 ms; a smooth roar that decays evenly (two bursts), the most energy at 120 to 400 Hz (-17 dB).
- Rejected: `GoalRough_3` (flagged tone-ish (prominence 18.6 dB at 650 Hz); one unbroken roar, no bursts).

**Cross-check** (asked for: stick shaft into a padded back, a sharp woody crack with a padded thud under it, 0.25 to 0.4 s; more crack and less bass than the body check). Pass: raw level needs at most 20 dB of gain; less bass than the body check (at least half of the energy from 400 Hz up; Hit_a has 15 % there); a thud under it (under 400 Hz at least -10 dB of the take); first sample within 12 dB of the peak inside 5 ms. Order: length of the body (time to -40 dB), longer first.
- `CrossCheck_a`: default: a mid knock (centroid 809 Hz, 0.4 to 1.5 kHz -2 dB) over a thud (under 400 Hz -5 dB), the longest body (130 ms to -40 dB); raw was very wide stereo (L/R correlation 0.11), folded to mono.
- `CrossCheck_b`: the brightest crack (centroid 1.6 kHz, energy spread evenly from 20 Hz to over 5 kHz), very short: 20 dB down after 20 ms.
- `CrossCheck_c`: a click with a thump (centroid 1.3 kHz), the shortest: 40 dB down after 40 ms; a faint second sound about 0.25 s later, 42 dB under.
- Rejected: `CrossCheck_2` (bass-led like a body check (from 400 Hz up only -7 dB of the take, centroid 448 Hz); worth a listen as a heavier variant); `CrossCheck_4` (raw nearly silent (peak -39 dBFS)).

**Glass rattle alone** (asked for: plexiglass panel rattling in its metal frame, no impact boom, 0.5 to 0.8 s). Pass: no boom (under 120 Hz at most -12 dB of the take); a rattle (at least three separate bursts); settles on its own inside 0.8 s. Order: more bursts first.
- `GlassRattle_a`: default: five bursts that die away over 0.5 s, energy from 400 Hz to over 5 kHz (centroid 4.3 kHz), nothing under 120 Hz (-36 dB); 0.585 s, ends on its own.
- `GlassRattle_b`: three slower bursts in the first 0.25 s, then a decay; a duller wobble (centroid 690 Hz, 0.4 to 1.5 kHz -2 dB, nothing over 5 kHz), under 120 Hz -15 dB.
- Rejected: `GlassRattle_1` (almost only above 5 kHz (centroid 9.9 kHz): two separate buzzing bursts with a 0.3 s gap, still going at the 0.8 s cut); `GlassRattle_4` (a fast bright chatter (ten bursts, centroid 7.7 kHz) whose loudest burst comes 0.1 s in; does not settle inside 0.8 s).

## 5. Levels for the import lane

- **Roar -18 LUFS** (true peaks -10.6 to -12.6 dBTP), **groan -20**, **rough goal -16**, **rough bed -30**: each matches the existing take it sits beside (`GoalFor_*` -16, `Save_*` and `GoalAgainst_*` -20, `Bed_*` -30), except the roar, which has no counterpart and is placed between the "ooh" and the goal.
- **Chant -26 LUFS.** `CrowdChant_a` peaks at -2.1 dBTP at that level; `CrowdChant_b` at -15 dBTP. A chant fired over the bed (-30) is then 4 LU over it; raise `CrowdChant_b` freely, `CrowdChant_a` has 2 dB of headroom.
- **Cross-check and glass rattle** peak at -3.0 dBTP like the AU4 impacts. `CrossCheck_a` is -22.4 LUFS against -22 for `Hit_a`: within 0.4 LU at the same peak. For the boards, the rattle was mixed 6 dB under `BoardsBody_a` in the preview.
- **Tone against the existing crowd:** roar, goal and bed centroids are 998 to 1086 Hz, like the goal cheers (1020 to 1057 Hz). The rough beds are darker and denser than `Bed_b` (centroid 1953 Hz, true peak 18 dB over its loudness; `BedRough_a` 8 dB over).

## 6. Previews

Stereo, 48 kHz 16-bit, each with 0.20 s of silence first (checked: first sound at 0.200 s). Mono sounds centred at -3 dB per channel; the landing 3 dB down, the crowd 6 dB down, the rattle 6 dB down: my gains, not the game's mix. In `preview_crosscheck.wav` I put the lighter AU4 "ooh" under the hit; that pairing is my choice.

| file | timeline (time zero is 0.20 s into the file) | length s | LUFS | true peak dBTP |
|---|---|---|---|---|
| `preview_knockdown_rough.wav` | HitBig_a at 0.00 s; CrowdRoar_a at 0.05 s (-6 dB); Land_a at 0.17 s (-3 dB); Slide_a at 0.20 s; GetUp_a at 1.05 s | 2.35 | -18.5 | -3.4 |
| `preview_boards_rattle.wav` | BoardsBody_a at 0.00 s; GlassRattle_a at 0.03 s (-6 dB) | 0.915 | -20 | -6 |
| `preview_crosscheck.wav` | CrossCheck_a at 0.00 s; CrowdHit_a at 0.05 s (-6 dB) | 2.14 | -24.1 | -6 |

## 7. Page, manifest, READY

- `C:/Users/kazda/jam/audio-lab/tackle/pick.html`: the three new previews first, then the four from AU4; the comparison block now also has `GoalFor_d`, `GoalAgainst_a` and `Bed_b`; then the seven new groups, then the eight AU4 groups unchanged. Beds play looped. A chant shows its pulse and regularity. Picks saved in the browser from the AU4 page are kept and the new groups start on their defaults.
- All 82 audio paths exist and the inline script passes `node --check`. **The page was not opened in a browser.**
- `manifest.json` holds both rounds (each take marked AU4 or AU5). `READY.txt` has an AU5 section appended.

## 8. Asset ledger rows for the import lane

Proposed names follow the existing ones. `shipped` is `y` for the defaults; set it to what the MetaSounds reference in the end. Sizes are guesses from the WAV size.

| path | kind | size_mb | origin | tool | licence | url | date | device | shipped | note |
|---|---|---|---|---|---|---|---|---|---|---|
| Game/Plugins/JamAssets/Content/Audio/Crowd/CRD_Roar_a.uasset | audio | 0.2 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU5 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU5-report.md) | 2026-11-13 | B | y | crowd roar on a knockdown; stereo 48 kHz 2 s, -18 LUFS, -10.7 dBTP; from tackle/CrowdRoar_a.wav (384044 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/Crowd/CRD_Roar_b.uasset | audio | 0.2 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU5 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU5-report.md) | 2026-11-13 | B | n | crowd roar on a knockdown, longer hold; stereo 48 kHz 2 s, -18.2 LUFS, -12.6 dBTP; from tackle/CrowdRoar_b.wav (384044 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/Crowd/CRD_Roar_c.uasset | audio | 0.2 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU5 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU5-report.md) | 2026-11-13 | B | n | crowd roar on a knockdown, flagged tone-ish; stereo 48 kHz 2 s, -18.2 LUFS, -10.6 dBTP; from tackle/CrowdRoar_c.wav (384044 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/Crowd/CRD_Groan_a.uasset | audio | 0.2 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU5 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU5-report.md) | 2026-11-13 | B | y | crowd groan on a save; stereo 48 kHz 2 s, -20.1 LUFS, -13.7 dBTP; from tackle/CrowdGroan_a.wav (384044 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/Crowd/CRD_Groan_b.uasset | audio | 0.2 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU5 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU5-report.md) | 2026-11-13 | B | n | crowd groan on a save, alternate; stereo 48 kHz 2 s, -20.5 LUFS, -14.8 dBTP; from tackle/CrowdGroan_b.wav (384044 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/Crowd/CRD_Chant_a.uasset | audio | 0.7 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU5 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU5-report.md) | 2026-11-13 | B | y | rhythmic clap and shout chant; stereo 48 kHz 5.715 s, -26.1 LUFS, -2.1 dBTP; from tackle/CrowdChant_a.wav (1097324 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/Crowd/CRD_Chant_b.uasset | audio | 0.6 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU5 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU5-report.md) | 2026-11-13 | B | n | stomp and clap chant; stereo 48 kHz 4.655 s, -26.1 LUFS, -15 dBTP; from tackle/CrowdChant_b.wav (893804 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/Crowd/CRD_BedRough_a.uasset | audio | 1.1 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU5 proc.mjs/select.mjs (ffmpeg, crossfaded loop) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU5-report.md) | 2026-11-13 | B | y | rougher crowd bed loop; stereo 48 kHz 9 s, -30.2 LUFS, -22.4 dBTP, looping; from tackle/BedRough_a.wav (1728044 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/Crowd/CRD_BedRough_b.uasset | audio | 1.1 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU5 proc.mjs/select.mjs (ffmpeg, crossfaded loop) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU5-report.md) | 2026-11-13 | B | n | rougher crowd bed loop, alternate; stereo 48 kHz 9 s, -30.2 LUFS, -18.3 dBTP, looping; from tackle/BedRough_b.wav (1728044 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/Crowd/CRD_GoalRough_a.uasset | audio | 0.4 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU5 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU5-report.md) | 2026-11-13 | B | y | rougher goal reaction; stereo 48 kHz 3.48 s, -16.3 LUFS, -10.2 dBTP; from tackle/GoalRough_a.wav (668204 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/Crowd/CRD_GoalRough_b.uasset | audio | 0.4 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU5 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU5-report.md) | 2026-11-13 | B | n | rougher goal reaction, alternate; stereo 48 kHz 3.48 s, -16.4 LUFS, -9.6 dBTP; from tackle/GoalRough_b.wav (668204 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_CrossCheck_a.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU5 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU5-report.md) | 2026-11-13 | B | y | cross-check, knock over a thud; mono 48 kHz 0.25 s, -22.4 LUFS, -3 dBTP; from tackle/CrossCheck_a.wav (24044 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_CrossCheck_b.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU5 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU5-report.md) | 2026-11-13 | B | n | cross-check, bright crack; mono 48 kHz 0.25 s, -23 LUFS, -3 dBTP; from tackle/CrossCheck_b.wav (24044 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_CrossCheck_c.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU5 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU5-report.md) | 2026-11-13 | B | n | cross-check, short click; mono 48 kHz 0.33 s, -25.8 LUFS, -3 dBTP; from tackle/CrossCheck_c.wav (31724 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_GlassRattle_a.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU5 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU5-report.md) | 2026-11-13 | B | y | glass rattle layer for the boards; mono 48 kHz 0.585 s, -22.6 LUFS, -3 dBTP; from tackle/GlassRattle_a.wav (56204 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_GlassRattle_b.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU5 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU5-report.md) | 2026-11-13 | B | n | glass rattle layer, duller wobble; mono 48 kHz 0.795 s, -20.5 LUFS, -3 dBTP; from tackle/GlassRattle_b.wav (76364 bytes WAV; set the uasset bytes after import) |

Licence wording as in the AU2, AU3 and AU4 rows; lane AU1's caveats stand (output may not be unique; it must be reviewed before distribution, and the operator's listen on the page is that review; the organisers' rule on AI-generated audio is still to be confirmed).

## 9. What I could not verify

- **Anything audible**, and above all the tone: that a roar is aggressive and not just a cheer, that a groan is disappointed, that the bed is rowdy, that nothing is cruel or out of place for a teen rating.
- **Words.** The prompts ask for wordless sound and the tonality test finds no sustained tone (one roar and one rejected groan are flagged tone-ish), but a shouted word would pass that test. The chant "hey" is a shouted syllable by design.
- **Horn, siren, whistle.** No kept take has a tonal flag, which is the test that caught the old horn; a short whistle would pass it.
- **What the chant's pulse is made of.** The numbers give a pulse and its regularity, not whether it is stomp-stomp-clap. `CrowdChant_b` has both low stomps and bright claps; `CrowdChant_a` has no low stomps.
- **Whether the rhythm of a chant resembles a known song.** Stomp-stomp-clap is a common stadium pattern; listen for that before shipping.
- **The loop seams by ear**, the page in a browser, and anything in the engine.

## 10. What fell short

- **Chants:** 4 of 6 unusable (three with no rhythm, one weak and tone-ish). Both kept chants end mid-pattern: the files are cut where the take ends, not on a bar.
- **Bed:** the 8 to 9 dB swing inside the loop (second half louder) in both kept takes, against 4 to 6 dB in the existing beds; no measurable pounding. If the pulse is audible, the fix is a new generation or a level-riding pass, which I did not do (one gain per file).
- **Roar:** two of the three kept roars (`b`, `c`) do not finish inside 2.0 s and are faded out at the cut, 23 dB under their loudest; only the default ends on its own. `CrowdRoar_4` stops dead at 1.0 s.
- **Groan:** both kept groans fill the full 2.0 s and are faded at the cut (20 to 22 dB down).
- **Cross-check:** all three kept takes are very short (40 dB down after 40 to 130 ms; the files are 0.25 to 0.33 s with the quiet tail). One of five came back nearly silent.
- **Goal:** "pounding on the glass" shows only as four bursts in the decay of the default; I cannot tell them from yells.
- The budget of 28 requests is used up.

## 11. For the lead to decide

1. **Roar and chant levels** (-18 and -26 LUFS): keep, or name the levels you want.
2. **The bed's 9 s pulse:** accept, or spend two or three generations on a steadier one (asking for "constant level" was not enough).
3. **Which chant:** `CrowdChant_a` is the more regular but may sound thin; `CrowdChant_b` has stomps and claps and runs 4.7 s.
4. **The cross-check's weight:** `rejected/CrossCheck_2.wav` is the heavy one (bass-led, like a body check); the kept three are crack-led as the brief asked.
5. **Glass rattle in the game:** it is a separate layer; the MetaSound that plays the boards hit needs it added about 0.03 s after the boom at about -6 dB, as in `preview_boards_rattle.wav`.
