# Lane AU4 report: the sounds of a tackle (device B), 2026-11-13 13:13

- All seven items are done, the get-up included: 21 takes kept, 20 rejected candidates, 4 previews and a listening page, in `C:/Users/kazda/jam/audio-lab/tackle/`. `READY.txt` is written.
- **Nobody has listened to any of this, and I cannot.** Every statement below is a measurement. The picks are defaults by numbers; the operator's ear decides.
- Service: 40 requests, all succeeded, none retried, 418 credits. No Unreal, no git write (one read-only `git status`), nothing written under `Game/`, `tools/`, `docs/` or `contracts/`.
- Four things to know before importing:
  1. **Boards: no take has a clear glass rattle.** All four are a deep boom with a short knock on top; above 1.5 kHz the default is 45 dB under the boom by 0.12 s after the hit. Options are in section 11.
  2. **The bump default is layered from two generations.** No single generation held both the thud and the skate scrape (six tries), so `Bump_a` is the `Bump_1` thud plus the `Bump_4` scrape.
  3. **The new hit is shorter than the sample the game plays now, so it is quieter by loudness.** `Hit_a` is 4.7 LU under `SFX_BodyCheck_02` as files (0.3 s against 1.4 s), although its peak is only 1.5 dB lower. `HitBig_a` is within 0.7 LU of it.
  4. **"10 to 12 dB quieter" for the slide and the get-up is taken as peak level** (11 dB under an impact's peak). By loudness the slide is then only 3.3 LU under the hit, because it lasts 0.8 s. Section 5 has the other readings.

## 1. What was generated

- Tool: `C:/Users/kazda/jam/eleven-tools/eleven.mjs sfx` (lane AU1's client, unchanged). It posts to the ElevenLabs sound-generation endpoint with model `eleven_text_to_sound_v2`, `prompt_influence` 0.5 and output `mp3_44100_192`. The earlier lanes took 128 kbit/s; I asked for 192 to keep the transients cleaner.
- **40 requests, 40 succeeded, none failed, none retried.** 28 in a first round, 10 after measuring it (hits, big hits, bumps, one slide), 2 last (one big hit, one bump). Credits by the response header: 418; the quota endpoint agrees (0 used before, 418 after, of 130,793).
- One generation per request. Ledger with every prompt: `C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl`. The unprocessed mp3 files are in `tackle/raw/`.
- The key was read from `.env` inside `eleven.mjs` and sent only in the request header. It was never printed, logged or copied; the ledger holds no key.
- The prompts hold generic sound descriptions only: no game, team, operator, path or other project detail.

Every prompt is one of two prefixes plus the text in the list.

- Foley prefix: "Close, dry sound effect recorded in an indoor ice rink, isolated, no reverb tail, no crowd, no voices, no music, no whistle."
- Crowd prefix (verbatim from lane AU2, so the tone matches the existing crowd takes; used for the four `CrowdHit` generations): "Real recording of a small to medium indoor ice hockey rink crowd, a few hundred people, close and lively, not a stadium roar. No horn, no siren, no organ, no music, no announcer, no whistle, no intelligible words."

Each line gives the generation, the seconds asked for, and where it went. `HitBig_7` repeats the `HitBig_5` prompt on purpose (a second, independent generation).

- Hit_1 (0.6 s asked; Hit_b): Two padded ice hockey players collide at full speed: one heavy low body thump with a short clack of plastic shoulder pads and a hockey stick. One impact only, very short.
- Hit_2 (0.6 s asked; rejected): A hard ice hockey body check in open ice: a deep dull thud of two padded bodies with a brief rattle of plastic armour. One impact only, very short.
- Hit_3 (0.6 s asked; rejected): Shoulder pads slamming into a padded chest at speed: a heavy low thump, a sharp short plastic clack and a hockey stick knocking once. One impact only, very short.
- Hit_4 (0.6 s asked; rejected): A heavy tackle between two athletes in hard plastic protective gear: a punchy low thud and a short crack of plastic on plastic. One impact only, very short.
- HitBig_1 (1 s asked; HitBig_b): A huge open-ice hockey hit: two padded players collide very hard, a massive deep body thump with strong low end, then a longer rattle of plastic armour and a stick clattering. One impact only.
- HitBig_2 (1 s asked; Hit_a): A bone-rattling ice hockey body check at top speed: a big booming low thud of padded bodies, then plastic pads, helmet and stick rattling for half a second. One impact only.
- HitBig_3 (1 s asked; rejected): Two heavy athletes in hard plastic protective gear crash into each other at full sprint: a very heavy deep impact with a lot of bass, followed by a loose rattle of equipment. One impact only.
- Land_1 (0.6 s asked; Land_a): A padded hockey player falls and lands flat on hard ice: a dull slap-thud of a padded body and a helmet hitting ice, with a little plastic clatter. One impact only, very short.
- Land_2 (0.6 s asked; Land_b): A body in thick sports padding dropping onto a hard ice surface: a flat dull thud and slap, a helmet knocking the ice once, a small plastic clatter. One impact only, very short.
- Land_3 (0.6 s asked; rejected): An ice hockey player knocked off his feet hits the ice on his back: a heavy muffled slap-thud of pads on ice and a short plastic rattle. One impact only, very short.
- Land_4 (0.6 s asked; Land_c): A heavy padded body flopping onto hard ice: a dull low thud with a wet slap of fabric on ice and a light click of plastic gear. One impact only, very short.
- Slide_1 (1.2 s asked; Slide_a): A padded body sliding across ice after a fall: a soft hiss and scrape of fabric and plastic on ice, steady at first and then fading out as it slows to a stop. No impact, no skating steps.
- Slide_2 (1.2 s asked; Slide_b): A hockey player in padded gear sliding on his back across a smooth ice rink: a soft continuous hissing scrape of nylon fabric and plastic on ice that gets quieter and stops. No impact, no skating steps.
- Slide_3 (1.2 s asked; rejected): Fabric and plastic sports equipment dragged quickly over smooth ice, a gentle airy hiss with a light gritty scrape, slowing down and fading away within one second. No impact, no skating steps.
- BoardsBody_1 (1 s asked; BoardsBody_c): An ice hockey player crashing body-first into the wooden arena boards: a deep hollow wooden boom, then the rattle of the glass panel above shaking in its frame. One impact only.
- BoardsBody_2 (1 s asked; rejected): A padded body slammed hard into the wooden boards of an ice rink: a big low boomy thud of the board wall, followed by the plexiglass panel rattling and settling. One impact only.
- BoardsBody_3 (1 s asked; BoardsBody_b): A heavy body hitting a hollow wooden rink wall with a glass pane on top: a deep resonant wooden boom with a loose rattling shudder of the glass and its metal supports. One impact only.
- BoardsBody_4 (1 s asked; BoardsBody_a): A hockey player driven into the boards: a heavy bass-rich boom of the wooden wall flexing, then a short clattering rattle of the glass above. One impact only, not a puck, not a sharp crack.
- Bump_1 (0.6 s asked; Bump_a, Bump_b): A light shoulder-to-shoulder bump between two padded hockey players: a soft muffled thud of padding, then a short skate blade scrape on ice as one of them staggers. Gentle, very short.
- Bump_2 (0.6 s asked; Bump_c): Two ice hockey players jostling and bumping shoulders: a soft padded thump and a quick scrape of a skate blade on ice. Light, gentle, very short.
- Bump_3 (0.6 s asked; rejected): A soft body bump between two athletes in padded gear on ice: a muffled low thud of foam padding and fabric, followed by a brief gritty skate scrape as one catches his balance. Very short.
- CrowdHit_1 (2 s asked; CrowdHit_b): A short sudden collective wordless ooh of surprise and approval at a big body check, swelling up quickly and falling away within one and a half seconds, then quiet. No applause, no cheering afterwards.
- CrowdHit_2 (2 s asked; CrowdHit_c): The crowd reacts to a hard hit with one short collective wordless ooh, rising fast and dying down again within one and a half seconds. No clapping, no cheering afterwards.
- CrowdHit_3 (2 s asked; rejected): A sharp collective wordless ooh and wince from the crowd as a player is knocked down, a single short swell that rises and falls within one and a half seconds, then settles to quiet. No applause, no cheering.
- CrowdHit_4 (2 s asked; CrowdHit_a): One brief impressed wordless oooh from the whole crowd at once, a quick rise and fall lasting about one second, ending quietly. No applause, no cheering, no clapping.
- GetUp_1 (0.8 s asked; GetUp_a): A hockey player pushing himself back up to his feet on ice: two or three light skate blade scrapes and scuffs on ice, soft. No impact, very short.
- GetUp_2 (0.8 s asked; rejected): Ice skates scraping and digging into the ice as a fallen skater gets back up, light short gritty blade scrapes with a soft rustle of padded gear. No impact, very short.
- GetUp_3 (0.8 s asked; GetUp_b): A skater scrambling to stand up on an ice rink: quick light steel blade scrapes on ice, quiet and short. No impact.
- Hit_5 (0.6 s asked; rejected): Two ice hockey players in full gear smash together shoulder to shoulder at speed: a loud punchy smack of hard plastic shoulder pads with a solid body thud underneath and one knock of a hockey stick. One impact only, very short.
- Hit_6 (0.6 s asked; rejected): A hard open-ice hockey body check: a loud crunching whack of plastic armour and padding as two players collide, solid and punchy, with a short equipment clack. One impact only, very short.
- Hit_7 (0.6 s asked; Hit_c): Hard plastic shoulder pads and helmets crunching together in a heavy collision between two skaters, a loud thick smack with body weight behind it. One impact only, very short.
- Hit_8 (0.6 s asked; rejected): A hockey player is body checked hard in open ice: a loud meaty thud of bodies and a sharp crack of plastic pads and sticks clashing at the same instant. One impact only, very short.
- HitBig_4 (1 s asked; rejected): A huge open-ice hockey hit, much harder than a normal check: a very loud crunching smash of plastic shoulder pads and bodies with a heavy thud, then sticks, helmets and gear rattling and clattering for half a second. One impact only.
- HitBig_5 (1 s asked; rejected): A devastating full-speed collision between two ice hockey players: a loud thick booming smack of pads and bodies, followed by a loose clatter of a dropped stick and rattling plastic equipment. One impact only.
- HitBig_6 (1 s asked; rejected): The hardest body check of the game: an enormous crunch of hard plastic armour and a heavy body thud with weight, then a rattling shudder of gear and a stick skittering on ice. One impact only.
- Bump_4 (0.6 s asked; Bump_a, rejected): Two ice hockey players bump shoulders lightly while skating: a soft padded thud first, and right after it a short scratchy skate blade scrape on ice as one of them stumbles. Quiet, light, very short.
- Bump_5 (0.6 s asked; rejected): A gentle nudge between two skaters in padded jerseys: a small soft thump of foam pads, then a quick skate edge scratching the ice. Light and short.
- Slide_4 (1.2 s asked; rejected): A hockey player who has fallen slides across the ice on his padded gear: a hissing scrape that starts strong right away and steadily dies away to nothing as he slows down and stops. No impact, no skating steps.
- HitBig_7 (1 s asked; HitBig_a): A devastating full-speed collision between two ice hockey players: a loud thick booming smack of pads and bodies, followed by a loose clatter of a dropped stick and rattling plastic equipment. One impact only.
- Bump_6 (1 s asked; rejected): A light shoulder bump between two padded ice hockey players, then one of them staggers: first a soft padded thud, immediately followed by a half-second gritty skate blade scrape on the ice. Quiet and light.

## 2. Processing

Scripts: `gen.mjs`, `lib.mjs`, `proc.mjs`, `select.mjs`, `page.mjs` in `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/au4/`. They are adapted from lane AU2's `crowd/gen.mjs`, `proc.mjs` and `page.mjs`: the decoder, the loudness reader and the tonality test are taken over unchanged.

- **Decode** to 48 kHz float with ffmpeg. Foley is folded to mono ((L+R)/2); the crowd stays stereo like the existing crowd takes.
- **Start, impacts:** walk back from the peak until the 4 ms level has stayed 12 dB under its maximum for 3 ms, start 1.5 ms before that point, 1 ms fade-in. Where the raw file already begins on the transient, its first sample is kept. Measured lead-in (first sample within 12 dB of the peak) on the kept impacts: 0.3 to 3.6 ms.
- **Start, others:** the slide starts 30 ms before it first comes within 5 dB of its loudest level (30 ms fade-in), so the swell the service put in front is cut. The get-up starts 10 ms before it first comes within 18 dB (8 ms fade-in). The crowd uses lane AU2's rule (10 ms before the first sample over -45 dBFS) with a 60 ms fade-in.
- **End:** 30 ms after the last 20 ms frame within 45 dB of the loudest one, held inside the length the brief gives for the item. Fade-out 50 ms for impacts, 100 ms for the slide, 60 ms for the get-up, 250 ms for the crowd.
- **Level:** one linear gain per file. No limiter, no equaliser, no compressor. 16-bit, no dither. No file reaches full scale (checked sample by sample).
- **`Bump_a`:** the trimmed `Bump_1` thud at 0 ms plus the whole `Bump_4` scrape from 30 ms, with the scrape's peak 8 dB under the thud's; 0.45 s, 50 ms fade-out. Checked in bands: under 400 Hz it peaks at 20 ms; above 1 kHz there is a second swell from 120 to 260 ms.
- **Loudness** is EBU R128 integrated (ffmpeg `ebur128`) with one second of silence before and after the file, because most takes are shorter than the 400 ms measuring block. Mono files are measured as one channel. The crowd files are also measured unpadded, as lane AU2 did.

## 3. The files

48 kHz 16-bit WAV in `C:/Users/kazda/jam/audio-lab/tackle/`; mono, except `CrowdHit_*` (stereo). Lead-in is the time to the first sample within 12 dB of the peak.

| file | pick | from generation | length s | LUFS | true peak dBTP | lead-in ms | 20 dB down after ms | 40 dB down after ms | centroid Hz |
|---|---|---|---|---|---|---|---|---|---|
| `Hit_a.wav` | **default** | HitBig_2 | 0.3 | -22 | -3 | 1.7 | 105 | 180 | 286 |
| `Hit_b.wav` | alternate | Hit_1 | 0.32 | -23.9 | -3 | 0.3 | 95 | 230 | 143 |
| `Hit_c.wav` | alternate | Hit_7 | 0.365 | -26 | -3 | 0.4 | 35 | 145 | 575 |
| `HitBig_a.wav` | **default** | HitBig_7 | 0.635 | -16.6 | -3 | 2.4 | 270 | 520 | 1027 |
| `HitBig_b.wav` | alternate | HitBig_1 | 0.575 | -19.4 | -3 | 0.5 | 80 | 400 | 71 |
| `Land_a.wav` | **default** | Land_1 | 0.315 | -21.6 | -3 | 0.5 | 100 | 210 | 333 |
| `Land_b.wav` | alternate | Land_2 | 0.3 | -21 | -3 | 0.3 | 60 | 200 | 666 |
| `Land_c.wav` | alternate | Land_4 | 0.3 | -19.7 | -3 | 0.5 | 40 | 95 | 242 |
| `Slide_a.wav` | **default** | Slide_1 | 0.835 | -25.3 | -14 | - | 480 | 565 | 2280 |
| `Slide_b.wav` | alternate | Slide_2 | 1 | -25.3 | -12.3 | - | 545 | 620 | 5445 |
| `BoardsBody_a.wav` | **default** | BoardsBody_4 | 0.6 | -20.7 | -3 | 2.2 | 130 | 385 | 176 |
| `BoardsBody_b.wav` | alternate | BoardsBody_3 | 0.645 | -21.2 | -3 | 0.3 | 175 | 465 | 143 |
| `BoardsBody_c.wav` | alternate | BoardsBody_1 | 0.695 | -22.9 | -3 | 3.6 | 205 | 560 | 124 |
| `Bump_a.wav` | **default** | Bump_1 + Bump_4 | 0.45 | -19.4 | -3 | 3.1 | 245 | 275 | 931 |
| `Bump_b.wav` | alternate | Bump_1 | 0.3 | -19.3 | -3 | 3.1 | 40 | 155 | 129 |
| `Bump_c.wav` | alternate | Bump_2 | 0.3 | -20.4 | -3 | 0.9 | 55 | 120 | 650 |
| `CrowdHit_a.wav` | **default** | CrowdHit_4 | 1.79 | -20.3 | -11.7 | - | 875 | 1135 | 737 |
| `CrowdHit_b.wav` | alternate | CrowdHit_1 | 1.515 | -20.2 | -9.7 | - | 895 | 1170 | 976 |
| `CrowdHit_c.wav` | alternate | CrowdHit_2 | 1.8 | -20.1 | -11 | - | 1110 | 1340 | 915 |
| `GetUp_a.wav` | **default** | GetUp_1 | 0.575 | -28 | -14 | - | 190 | 275 | 1993 |
| `GetUp_b.wav` | alternate | GetUp_3 | 0.515 | -28 | -15.1 | - | 130 | 270 | 2987 |

The existing samples, measured the same way:

| existing sample | length s | LUFS | true peak dBTP | 20 dB down after ms | 40 dB down after ms | centroid Hz | note |
|---|---|---|---|---|---|---|---|
| `ready/SFX_BodyCheck_02.wav` | 1.397 | -17.3 | -1.5 | 330 | 715 | 457 | the body check as imported (mono), in `MS_check` |
| `SFX_BodyCheck_02.mp3` | 1.48 | -12.5 | 0.3 | - | - | - | the same take as the service returned it (stereo lab file) |
| `ready/SFX_PuckBoards_01.wav` | 0.32 | -24.4 | -3 | 85 | 215 | 452 | puck on boards as imported, in `MS_board` |
| `crowd/Save_b.wav` | 2 | -20.1 | -7.3 | 1360 | 1580 | 1443 | the save reaction in `MS_save`; -20 LUFS by lane AU2's unpadded method |
| `crowd/Save_a.wav` | 2 | -19.9 | -12.8 | 840 | 1250 | 543 | alternate save reaction |
| `crowd/Save_c.wav` | 2 | -20.3 | -9.7 | 1455 | 1550 | 794 | alternate save reaction |

## 4. How each default was chosen

The rules and thresholds are mine and rest on these 40 takes only. "dB of the take" means the share of the take's energy in that band.

**Body check impact** (asked for: heavy low thump with a short clack, 0.3 to 0.5 s). Pass: raw level needs at most 20 dB of gain; thump present (energy under 400 Hz at least -6 dB of the take); clack present (energy over 1.5 kHz at least -30 dB); first sample within 12 dB of the peak inside 5 ms. Order: loudness at equal peak (more energy behind the same peak).
- `Hit_a`: default: thump and clack both present (under 400 Hz -0.7 dB, over 1.5 kHz -14 dB of the take), the loudest of the three at equal peak; one clean event.
- `Hit_b`: deeper (centroid 143 Hz), clack weaker (over 1.5 kHz -24 dB); raw was wide stereo (L/R correlation 0.38), folded to mono.
- `Hit_c`: thump with the brightest clack, but very short: 20 dB down after 35 ms.
- Rejected: `Hit_2` (no clack: nothing over 1.5 kHz above -40 dB of the take (a bare sub thump)); `Hit_3` (no clack (over 1.5 kHz -32 dB of the take): sub-bass only, centroid 40 Hz; raw peak -21.7 dBFS); `Hit_4` (raw nearly silent (peak -38 dBFS) and sub-bass only); `Hit_5` (raw peak -25.7 dBFS (over 20 dB of gain needed); two events 96 ms apart); `Hit_6` (raw peak -27.4 dBFS (over 20 dB of gain needed); a noise tail 25 dB under the hit for 0.5 s); `Hit_8` (crack only: energy under 400 Hz is 11 dB under the take, no thump).

**Big hit** (asked for: the same, harder, more low end, longer rattle, up to 0.7 s). Pass: more energy under 400 Hz than Hit_a at equal peak; a longer decay (time to -40 dB) than Hit_a; a rattle (content from 400 Hz up, later than 60 ms after the peak, at least -25 dB of the take). Order: that rattle share, larger first.
- `HitBig_a`: default: 6.8 dB more energy under 400 Hz than Hit_a at equal peak, a broadband rattle in the tail (-8 dB of the take), 0.52 s to -40 dB; the raw decodes to +0.8 dBFS, so the service may have limited it; a soft 30 ms lead-in in the raw was trimmed off.
- `HitBig_b`: deepest (centroid 71 Hz, 6.9 dB more energy under 400 Hz than Hit_a, 0.4 s to -40 dB); the rattle is weak (-22 dB of the take), so little of it may be left on small speakers.
- Rejected: `HitBig_3` (no rattle (-51 dB of the take): a sub-bass rumble only, centroid 51 Hz; raw peak -19.3 dBFS); `HitBig_4` (less energy under 400 Hz than Hit_a at equal peak (a mid crunch, centroid 1.5 kHz), two events, only 0.36 s); `HitBig_5` (strong rattle, but 6 dB less energy under 400 Hz than the normal Hit_a at equal peak, so not harder by the rule; worth a listen as a rattle-led alternative); `HitBig_6` (less energy under 400 Hz than Hit_a; hiss-dominated (centroid 7.6 kHz), L/R correlation 0.08, raw peak -15 dBFS).

**Body lands on the ice** (asked for: dull slap-thud with a little plastic clatter, 0.3 to 0.5 s). Pass: dull (at least half of the energy under 400 Hz), a little clatter (over 1.5 kHz between -25 and -8 dB of the take). All four pass. Order: length of the body (time to -40 dB), longer first; a tie goes to the take with more clatter after the impact.
- `Land_a`: default: balanced slap-thud with clatter at -15 dB, the longest body (0.21 s to -40 dB); raw was quiet (peak -20 dBFS, 17 dB of gain).
- `Land_b`: brighter slap (centroid 666 Hz), strongest clatter (-11 dB), 0.2 s to -40 dB.
- `Land_c`: dullest (centroid 242 Hz) and shortest: 40 dB down after 95 ms.
- Rejected: `Land_3` (passes, but fourth: ties for the shortest body (95 ms to -40 dB) and has no clatter after the impact (over 1.5 kHz later than 60 ms: -58 dB of the take, against -39 dB for Land_c)).

**Slide on ice** (asked for: soft hiss and scrape, 0.6 to 1.0 s, fading out). Pass: fades out (loudest 20 ms frame in the first 60 % of the take, then a decay). Order: share of scrape body (400 Hz to 5 kHz), larger first, pure hiss last. The start is trimmed to where the take first comes within 5 dB of its loudest level.
- `Slide_a`: default: scrape body with hiss on top (centroid 2.3 kHz), fades over its last 0.45 s, 0.84 s without a cut.
- `Slide_b`: hissier (centroid 5.4 kHz), level holds for 0.6 s and then fades; cut at 1.0 s.
- Rejected: `Slide_3` (does not fade: loudest at 0.65 s of 1.0 s, then stops abruptly); `Slide_4` (passes, but third: almost pure hiss (centroid 7.4 kHz, over 5 kHz -1.7 dB of the take), the least scrape body).

**Skater into the boards** (asked for: deep wooden boom with the rattle of the glass, 0.5 to 0.8 s). Pass: deep (centroid under 250 Hz; the puck-on-boards crack in the game sits at 452 Hz) and some upper content, where a rattle would be (from 400 Hz up, at least -25 dB of the take). Order: that upper content, more first.
- `BoardsBody_a`: default: boom (centroid 176 Hz) with the most upper content (400 Hz to 1.5 kHz -14.5 dB, over 1.5 kHz -23 dB), 0.39 s to -40 dB; a soft lead-in noise in the raw was trimmed off.
- `BoardsBody_b`: boom (143 Hz) with content at 400 Hz to 1.5 kHz (-15 dB) but almost nothing over 1.5 kHz (-34 dB), 0.47 s to -40 dB.
- `BoardsBody_c`: deepest of the three kept (124 Hz), longest (0.56 s to -40 dB), least content above 400 Hz (-20 dB); raw was wide stereo (correlation 0.53).
- Rejected: `BoardsBody_2` (sub boom only (centroid 77 Hz): everything above 400 Hz together is 32 dB under the take).

**Hit taken standing (bump)** (asked for: soft thud and a skate scrape, 0.3 to 0.5 s). No single generation holds both a thud and a scrape (six tries: thuds without a scrape, or scrapes without a thud). Default: the thud layered with the scrape-only take. Then the single thuds that are not sub-bass only (centroid over 100 Hz), lower first.
- `Bump_a`: default: layered from two generations: the Bump_1 thud at 0 ms plus the Bump_4 scrape from 30 ms, its peak 8 dB under the thud.
- `Bump_b`: the thud alone: soft and low (centroid 129 Hz), 40 dB down after 155 ms, no scrape.
- `Bump_c`: a brighter thud (centroid 650 Hz), 40 dB down after 120 ms; its 0.2 s rising lead-in in the raw was trimmed off.
- Rejected: `Bump_3` (sub-bass thud only (centroid 83 Hz), raw peak -10 dBFS); `Bump_4` (scrape only, no thud (used as the scrape layer of Bump_a)); `Bump_5` (low thud (centroid 77 Hz) with only a faint movement after it (-33 dB), no scrape); `Bump_6` (no low thud at all (under 400 Hz -24 dB of the take): a mid knock, then a scrape; the trim keeps only the scrape).

**Crowd reacts to a big hit** (asked for: short wordless ooh, rising and falling within 1.2 to 1.8 s, no cheering after). Pass: no tonal flag (lane AU2 test) and it falls: 20 dB down within 1.2 s of its loudest frame. Order: takes that end on their own inside 1.8 s first, then the least content over 1.5 kHz (clapping and cheering live there).
- `CrowdHit_a`: default: comes up over the first 0.15 s, is loudest at 0.5 s and has fallen away by 1.8 s on its own; least content over 1.5 kHz (-19 dB), so the least clapping by the numbers; darker than the others (centroid 737 Hz).
- `CrowdHit_b`: loud from the first instant, 20 dB down 1.1 s in, over at 1.5 s on its own; more content over 1.5 kHz (-13 dB).
- `CrowdHit_c`: slower fall (20 dB down 1.5 s in); the raw ran 1.93 s and is cut at 1.8 s with a 0.25 s fade (32 dB down at the cut).
- Rejected: `CrowdHit_3` (does not fall in time: 20 dB down only 1.42 s after its loudest frame (raw 2.05 s), and the most content over 1.5 kHz (-9.5 dB of the take, applause-like)).

**Get-up** (asked for: skates scraping as a player gets up, light, 0.4 to 0.6 s). Pass: no hard stop. Order: longest stretch of activity (20 ms frames within 20 dB of the loudest).
- `GetUp_a`: default: soft scuffs for 0.25 s, then one stronger scrape; active for 0.42 s of 0.58 s.
- `GetUp_b`: one brighter scrape (centroid 3 kHz) that swells and decays smoothly; active for 0.30 s.
- Rejected: `GetUp_2` (one scrape that stops dead (22 dB drop inside 20 ms), active for 0.29 s only).

## 5. Levels for the import lane

- **Impacts** (`Hit`, `HitBig`, `Land`, `BoardsBody`, `Bump`): every file peaks at -3.0 dBTP. Loudness differs inside a group at that equal peak, so variants will not sound equally loud without a trim:
  - `Hit` a / b / c: -22 / -23.9 / -26 LUFS. `HitBig` a / b: -16.6 / -19.4.
  - `Land` a / b / c: -21.6 / -21 / -19.7. `BoardsBody` a / b / c: -20.7 / -21.2 / -22.9. `Bump` a / b / c: -19.4 / -19.3 / -20.4.
- **Against the body check in the game:** `ready/SFX_BodyCheck_02.wav` is -17.3 LUFS at -1.5 dBTP over 1.4 s. `Hit_a` is -22 LUFS at -3 dBTP over 0.3 s. Swapped one for one in `MS_check`, the check would be about 4.7 dB quieter by loudness. `HitBig_a` (-16.6 LUFS) matches the old sample within 0.7 LU.
- **Slide and get-up:** `Slide_a` and `GetUp_a` peak at -14.0 dBTP, 11 dB under an impact's peak. The alternate of each group matches its default in loudness (`Slide_*` -25.3 LUFS, `GetUp_*` -28 LUFS). The same gap read three ways against `Hit_a`:

| | by true peak | by the loudest 100 ms (RMS) | by loudness (LUFS) |
|---|---|---|---|
| `Slide_a` under `Hit_a` | 11.0 dB | 8.3 dB | 3.3 LU |
| `GetUp_a` under `Hit_a` | 11.0 dB | 9.0 dB | 6.0 LU |

  If the brief meant 10 to 12 LU by loudness, lower the slide by about 8 dB and the get-up by about 5 dB in the MetaSound.
- **Crowd:** `CrowdHit_*` are at -20.0 LUFS by lane AU2's unpadded method, the level of `Save_*` (padded: -20.3, -20.2, -20.1, against -20.1 for `Save_b`). Their peaks are lower (-11.7 dBTP against -7.3).
- **Crowd tone:** centroids are 737 to 976 Hz, inside the range of the three save takes (543 to 1443 Hz). Above 5 kHz the new takes are duller than every save take: -30 to -34 dB of the take, against -10 dB for `Save_b` (which has applause) and -23 dB for `Save_a` and `Save_c`. No take has a tonal or tone-ish flag by lane AU2's test. Stereo width (L/R correlation 0.41 to 0.66) is inside the range of the save takes (0.32 to 0.83).

## 6. Previews

Stereo, 48 kHz 16-bit. Each file starts with **0.20 s of silence**, so a player that swallows the first milliseconds does not eat the hit; the timeline is exact after that (checked: the first sound sits at 0.200 s in every file). Mono sounds are centred at -3 dB per channel, as the engine plays a 2D mono sound. The landing is 3 dB down and the crowd 6 dB down: those two gains are my choice, not the game's mix.

| file | timeline (time zero is 0.20 s into the file) | length s | LUFS | true peak dBTP |
|---|---|---|---|---|
| `preview_knockdown.wav` | Hit_a at 0.00 s; CrowdHit_a at 0.05 s (-6 dB); Land_a at 0.17 s (-3 dB); Slide_a at 0.20 s; GetUp_a at 1.05 s | 2.14 | -21.9 | -6 |
| `preview_bighit.wav` | HitBig_a at 0.00 s; CrowdHit_a at 0.05 s (-6 dB); Land_a at 0.17 s (-3 dB); Slide_a at 0.20 s; GetUp_a at 1.05 s | 2.14 | -19 | -5.3 |
| `preview_boards.wav` | BoardsBody_a at 0.00 s; CrowdHit_b at 0.05 s (-6 dB) | 1.865 | -21.9 | -6 |
| `preview_bump.wav` | Bump_a at 0.00 s | 0.75 | -19.4 | -6 |

## 7. Listening page

`C:/Users/kazda/jam/audio-lab/tackle/pick.html`: open it locally; the WAVs sit beside it.
- The four previews first, then the three samples the game plays now for comparison (`../ready/`, `../crowd/`).
- Then one section per item: a player per take, its numbers, a small loudness curve, why it was picked or rejected. The measured default is pre-selected; the rejected candidates can be played and picked too.
- "Copy picks" puts the choice on the clipboard; the choice is kept in the browser's local storage.
- All 48 audio paths exist and the inline script passes `node --check`. **The page was not opened in a browser.**
- `manifest.json` beside it holds every number, level target, gain, prompt and credit per take.

## 8. Asset ledger rows for the import lane

Proposed asset names follow the existing ones (`SFX_*` in `Audio/SFX`, `CRD_*` in `Audio/Crowd`). `shipped` is `y` for the defaults and `n` for the alternates: set it to what the MetaSounds reference in the end. The sizes are guesses from the WAV size; replace them with the uasset bytes.

| path | kind | size_mb | origin | tool | licence | url | date | device | shipped | note |
|---|---|---|---|---|---|---|---|---|---|---|
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Hit_a.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | y | body check impact, thump and clack; mono 48 kHz 0.3 s, -22 LUFS, -3 dBTP; from tackle/Hit_a.wav (28844 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Hit_b.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | n | body check impact, deeper; mono 48 kHz 0.32 s, -23.9 LUFS, -3 dBTP; from tackle/Hit_b.wav (30764 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Hit_c.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | n | body check impact, short and bright; mono 48 kHz 0.365 s, -26 LUFS, -3 dBTP; from tackle/Hit_c.wav (35084 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_HitBig_a.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | y | big hit, low end and rattle; mono 48 kHz 0.635 s, -16.6 LUFS, -3 dBTP; from tackle/HitBig_a.wav (61004 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_HitBig_b.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | n | big hit, deepest; mono 48 kHz 0.575 s, -19.4 LUFS, -3 dBTP; from tackle/HitBig_b.wav (55244 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Land_a.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | y | body lands on ice, slap-thud; mono 48 kHz 0.315 s, -21.6 LUFS, -3 dBTP; from tackle/Land_a.wav (30284 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Land_b.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | n | body lands on ice, brighter slap; mono 48 kHz 0.3 s, -21 LUFS, -3 dBTP; from tackle/Land_b.wav (28844 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Land_c.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | n | body lands on ice, dull and short; mono 48 kHz 0.3 s, -19.7 LUFS, -3 dBTP; from tackle/Land_c.wav (28844 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Slide_a.uasset | audio | 0.1 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | y | padded body sliding on ice, scrape and hiss; mono 48 kHz 0.835 s, -25.3 LUFS, -14 dBTP; from tackle/Slide_a.wav (80204 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Slide_b.uasset | audio | 0.1 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | n | padded body sliding on ice, hissier; mono 48 kHz 1 s, -25.3 LUFS, -12.3 dBTP; from tackle/Slide_b.wav (96044 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_BoardsBody_a.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | y | skater into the boards, boom; mono 48 kHz 0.6 s, -20.7 LUFS, -3 dBTP; from tackle/BoardsBody_a.wav (57644 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_BoardsBody_b.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | n | skater into the boards, boom; mono 48 kHz 0.645 s, -21.2 LUFS, -3 dBTP; from tackle/BoardsBody_b.wav (61964 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_BoardsBody_c.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | n | skater into the boards, deepest and longest; mono 48 kHz 0.695 s, -22.9 LUFS, -3 dBTP; from tackle/BoardsBody_c.wav (66764 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Bump_a.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | y | standing bump, thud plus skate scrape, layered from two generations; mono 48 kHz 0.45 s, -19.4 LUFS, -3 dBTP; from tackle/Bump_a.wav (43244 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Bump_b.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | n | standing bump, soft thud; mono 48 kHz 0.3 s, -19.3 LUFS, -3 dBTP; from tackle/Bump_b.wav (28844 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Bump_c.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | n | standing bump, brighter thud; mono 48 kHz 0.3 s, -20.4 LUFS, -3 dBTP; from tackle/Bump_c.wav (28844 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/Crowd/CRD_Hit_a.uasset | audio | 0.2 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | y | crowd ooh at a big hit; stereo 48 kHz 1.79 s, -20.3 LUFS, -11.7 dBTP; from tackle/CrowdHit_a.wav (343724 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/Crowd/CRD_Hit_b.uasset | audio | 0.2 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | n | crowd ooh at a big hit, sudden; stereo 48 kHz 1.515 s, -20.2 LUFS, -9.7 dBTP; from tackle/CrowdHit_b.wav (290924 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/Crowd/CRD_Hit_c.uasset | audio | 0.2 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | n | crowd ooh at a big hit, slower fall; stereo 48 kHz 1.8 s, -20.1 LUFS, -11 dBTP; from tackle/CrowdHit_c.wav (345644 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_GetUp_a.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | y | get-up, skate scuffs then a scrape; mono 48 kHz 0.575 s, -28 LUFS, -14 dBTP; from tackle/GetUp_a.wav (55244 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_GetUp_b.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | n | get-up, one skate scrape; mono 48 kHz 0.515 s, -28 LUFS, -15.1 dBTP; from tackle/GetUp_b.wav (49484 bytes WAV; set the uasset bytes after import) |

Licence wording as in the AU2 and AU3 rows. Lane AU1's caveats stand: output may not be unique across users; it must be reviewed before distribution, and nobody has heard these yet, so the operator's listen on the page is that review; the organisers' rule on AI-generated audio is still to be confirmed.

## 9. What I could not verify

- **Anything audible.** That a hit sounds like a body check and not a drum; that the landing reads as ice; that the slide and the get-up are skates and fabric and not plain noise; that the layered bump sounds like one event; that the takes are dry.
- **The crowd.** That the takes are a crowd's "ooh" and wordless, with no cheer, whistle or voice that stands out. The tonality test only catches sustained narrow tones; a spoken word would pass it.
- **Limiting at the source.** Several raw files sit at or just over full scale after decoding (`HitBig_7` +0.8 dBFS, `Slide_1` +1.2, `GetUp_3` +1.4, the crowd takes 0.0). I found no flat-topped clipping, but I cannot tell whether the service limited them.
- **The source is lossy** (mp3, 192 kbit/s). Pre-echo before the transients was not examined.
- **The page in a browser**, and anything in the engine: no import, no MetaSound, no cook.
- **The pick rules.** The thresholds are calibrated on these 40 takes and on nothing else.

## 10. What failed or fell short

No request failed. The misses are in what came back:
- **Sub-bass only or nearly silent: 9 of the 40.** Seven came back as a bare rumble or thud centred at 40 to 83 Hz (`Hit_2`, `Hit_3`, `Hit_4`, `HitBig_3`, `BoardsBody_2`, `Bump_3`, `Bump_5`), mostly from prompts with "deep", "dull", "low" or "bass". Three hits peaked 26 to 38 dB under full scale (`Hit_4`, `Hit_5`, `Hit_6`).
- **Boards rattle.** In all four boards takes the content from 400 Hz up, later than 60 ms after the hit, is 26 to 48 dB under the take. By the numbers this is a boom, not a boom with a rattling glass panel. It is distinct from the puck crack (centroid 124 to 176 Hz against 452 Hz; 0.39 to 0.56 s to -40 dB against 0.22 s).
- **Bump.** Six generations gave thuds without a scrape or scrapes without a thud, hence the layered default.
- **Hit length.** `Hit_a` and `Hit_c` are 40 dB down in under 0.2 s; the files are 0.30 and 0.37 s only because the quiet tail is kept.
- **`Land_a`** needed 16.9 dB of gain (raw peak -20 dBFS). Its decay runs down to 50 dB under the peak with no noise floor showing, so I kept it.
- The budget of about 40 requests is used up, so the boards rattle was not retried.

## 11. For the lead to decide

1. **Boards rattle.** Three ways: (a) accept the boom; (b) layer a rattle in the MetaSound that plays it: the existing `SFX_BodyCheck_02` was in fact generated from a boards prompt ("slamming a player into the boards, heavy thump and plexiglass rattling") and carries 10 to 20 dB more content from 400 Hz up in its tail (0.2 to 0.3 s after the hit) than the new boards takes, and `rejected/HitBig_5.wav` has the strongest rattle tail among the usable impact takes; (c) three or four more generations asking only for a glass panel rattling in its frame.
2. **Which sample `MS_check` should play.** `SFX_BodyCheck_02` is a boards sound by its prompt. If `Hit_a` replaces it for open-ice checks, the old sample is free for the boards.
3. **Slide and get-up level:** the peak reading used here, or the loudness reading (section 5).
4. **The scripts are in the session scratchpad**, as the brief asked, and that folder is not permanent. The ledger rows name them, so copy them beside the takes if they should outlive the session.
5. **Alternates worth an ear first:** `rejected/HitBig_5.wav` (rattle-led big hit), `Bump_b` (the plain thud, if the layered bump sounds like two events), `CrowdHit_b` (starts at full level, which may sit better on the hit frame than the rising `CrowdHit_a`).
