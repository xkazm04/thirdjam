# Lane AU10 report: ability and movement sounds, and seventy-three commentator lines (device B), 2026-11-13 21:01

- **Both parts are complete.** Part 1: 16 effect takes kept, 15 rejected candidates, 3 previews, in `C:/Users/kazda/jam/audio-lab/verbs/`. Part 2: 73 commentator WAVs (the 60 rough lines, the 12 ability lines, one replacement), in `C:/Users/kazda/jam/audio-lab/commentary-rough/wav/`. `READY.txt` is in each folder.
- **Nobody has listened to any of this, and I cannot.** Every statement is a measurement. Whether a line is said the right way, or a sound is the right sound, is for an ear.
- Service: 110 requests, all succeeded: 30 sound effects (298 credits) and 80 speech requests (73 lines and 7 retakes, 1,289 credits). 3,073 credits used today over the three lanes; 127,720 left. No Unreal, no git write; the only file written in the repository is this report.
- Five things to know first:
  1. **The new lines are long for live play.** 34 of the 72 live lines run over 2.5 s (the picker's gate), 15 over 3.0 s; the median is 2.46 s against 1.7 s in the bank. The voice is not slower (2.81 words a second against 2.65): the texts are longer (7 words at the median against 4). Decision 1 below.
  2. **The texts are spoken as written, with the bank's delivery rule:** the closing mark is sent as "!" ("Hell of a hit." is sent as "Hell of a hit!"), as for all 340 existing lines. No word was changed. The texts I flag are listed in section B4; none was altered.
  3. **`lose.blowout.01` is re-recorded as "A night to forget. Back to practice."** (section B5). It is a new take beside the old one, not over it.
  4. **Effect levels are file levels.** Everything peaks at -3 dBTP (the stumble at -9) as asked, but at that peak the hard stop is 8 LU louder than a body check and the shield 8 LU quieter. For the operator's mix rule (effects under music, commentator and crowd) the table in A3 gives each file's loudness against `Hit_a`, so the import lane can trim them to one level.
  5. **Three sounds fall short of their description by the numbers:** the hip check's thump comes 0.18 s after the file starts (it must fire on the press, not on contact); no generated boards fall shows a clear slide and thud (one passes weakly, and an assembled alternate is supplied); for the shield I tested for a thud and something after it, not for a hum or a creak as such.

# Part A: sound effects

## A1. What was generated

- Tool and settings as lanes AU4 and AU5: `C:/Users/kazda/jam/eleven-tools/eleven.mjs sfx`, model `eleven_text_to_sound_v2`, `prompt_influence` 0.5, output `mp3_44100_192`.
- **30 requests, 30 succeeded, none failed, none retried**: 24 first (three per sound), then 6 after measuring (two hip checks, two boards falls, one shield, one shotgun). 298 credits by the response header. Ledger: `C:/Users/kazda/jam/audio-lab/verbs/ledger.jsonl`; raw mp3 files in `verbs/raw/`.
- The key was read inside `eleven.mjs` and sent only in the request header; never printed, logged or copied. The prompts are generic sound descriptions. The shotgun prompts never ask for a gun: they describe a slap shot and say "no gunshot".
- Read first: `docs/experiments/ABILITIES.md`, `CONTACT.md` (bump, stumble, down) and `MOVEMENT.md` (the reverse brake).

Every prompt is this prefix plus the text in the list: "Close, dry sound effect recorded in an indoor ice rink, isolated, no reverb tail, no crowd, no voices, no music, no whistle."

- Shotgun_1 (1 s asked; Ability_Shotgun_b): An extremely hard ice hockey slap shot: one heavy loud crack of the stick blade smashing the puck off the ice, then a deep fast whoosh of the puck flying away. One hit only. Not a firearm, no gunshot, no explosion.
- Shotgun_2 (1 s asked; rejected): A hockey stick whipping through and cracking a rubber puck with huge force, a sharp woody crack with a low thump under it, followed by a deep airy whoosh trailing off. One hit only, no gunshot.
- Shotgun_3 (1 s asked; rejected): The hardest slap shot possible on ice: the stick shaft flexes and snaps into the puck with a booming crack, and the puck tears away through the air with a low rushing whoosh. One hit only, no firearm sound.
- HipCheck_1 (0.8 s asked; Ability_HipCheck_b): A hockey player lunging sideways into a hip check: a short quick whoosh of a body moving, straight into one heavy padded thump of hip pads hitting another player. Very short.
- HipCheck_2 (0.8 s asked; rejected): A short fast swish of padded hockey gear charging forward, ending at once in a heavy dull body thud of two padded players colliding. One impact only, very short.
- HipCheck_3 (0.8 s asked; rejected): A quick rush of air and fabric as a padded athlete throws his hip, then one deep thick padded thump on impact. One impact only, very short.
- Shield_1 (0.8 s asked; Ability_Shield_a): A padded hockey player bracing hard for a hit: one low padded thud as he plants himself, with a taut leathery creak of straps and gloves tightening. Short, solid, natural, no electronic sound.
- Shield_2 (0.8 s asked; Ability_Shield_b): Thick leather and padding pulled tight with a short stretching creak, on top of a low dull thump, like a goalie setting his pads and bracing. Short and solid, no synthesizer, no hum of electricity.
- Shield_3 (0.8 s asked; rejected): A solid low thud of padded gear being slapped into place, followed by a brief rising taut creak of leather under tension. Short, physical, natural, no science fiction sound.
- SlowPuck_1 (1 s asked; Ability_SlowPuck_a): A rubber hockey puck suddenly dragging across rough, snowy ice: a short gritty scrape that falls in pitch and slows down to a stop. No impact, no skating.
- SlowPuck_2 (1 s asked; rejected): A hard rubber disc sliding fast over ice and then grinding into rough frosty ice, a descending coarse scraping sound that slows and dies away in under a second. No impact.
- SlowPuck_3 (1 s asked; Ability_SlowPuck_b): A puck skidding onto scratched, snow-covered ice and losing speed quickly: a short rasping gritty drag that drops lower and fades out. No impact, no skating.
- Deke_1 (0.5 s asked; Deke_a): A hockey stick blade flicked quickly sideways across the ice with a little snow, one light fast scrape and tick. Very short, light.
- Deke_2 (0.5 s asked; Deke_b): One quick light swish of a hockey stick blade cutting across ice and brushing a little loose snow. Very short, soft, a single motion.
- Deke_3 (0.5 s asked; rejected): A fast toe-drag of a hockey stick on ice: a tiny crisp scrape with a light tap of the blade. Very short and light.
- Stumble_1 (0.8 s asked; Stumble_b): Ice skates scrabbling on ice as a hockey player nearly falls and catches his balance: two or three quick short blade scrapes with a rustle of padded gear. No impact, no fall.
- Stumble_2 (0.8 s asked; Stumble_a): A skater losing his footing for a moment on ice, quick frantic choppy skate blade scratches and a soft rustle of pads, then steady again. No impact, no thud.
- Stumble_3 (0.8 s asked; rejected): Three quick scratchy steel skate blade scuffs on ice in a row, uneven, with a light swish of a nylon jersey, as a player staggers. No impact.
- Brake_1 (0.9 s asked; rejected): A classic ice hockey stop: two skate blades turned sideways shaving the ice with one sharp loud spraying scrape and a shower of snow, ending abruptly. One stop only.
- Brake_2 (0.9 s asked; Brake_b): A fast skater slamming on a hockey stop, a loud crisp shhhk of steel blades carving sideways through ice and throwing a spray of ice shavings. One stop only, short.
- Brake_3 (0.9 s asked; Brake_a): A hard sudden stop on ice skates: a sharp bright grinding scrape of both blades biting sideways into the ice, with the hiss of sprayed snow settling. One stop only.
- BoardsFall_1 (1.2 s asked; rejected): A padded hockey player slammed into the wooden rink boards and then sliding down them onto the ice: a deep boom of the boards, a short rattle of the glass, then a dull dragging slide of pads down the wall and a soft thud on the ice.
- BoardsFall_2 (1.2 s asked; rejected): A body in hockey gear hits the arena boards with a heavy hollow thump, the glass above rattles briefly, and the player slumps down the boards with a muffled scrape and lands on the ice with a dull thud. Within one second.
- BoardsFall_3 (1.2 s asked; rejected): A heavy boom of a padded body against a wooden rink wall, a quick shudder of plexiglass, then padding scraping downward and a low thump as the body reaches the ice. Within one second.
- HipCheck_4 (0.8 s asked; Ability_HipCheck_a): A fast whoosh of air for a quarter of a second, like a heavy padded body swinging past the microphone, ending in one deep heavy padded thump. Whoosh, then thump, half a second in total. One impact only.
- HipCheck_5 (0.8 s asked; rejected): Swoosh then thud: a quick loud fabric swoosh of a hockey player throwing his body sideways, immediately followed by one heavy low padded impact. Very short, one impact only.
- BoardsFall_4 (1.2 s asked; rejected): Two sounds in a row within one second: first a loud deep boom of a padded body hitting wooden rink boards with a short glass rattle, then, half a second later, a softer dull thud of the body dropping onto the ice. Boom, rattle, slide, thud.
- BoardsFall_5 (1.2 s asked; BoardsFall_a): A hockey player crashes into the boards and crumples to the ice: a big hollow wooden bang with rattling plexiglass at the start, a short scrape of pads sliding down the boards, and a final soft heavy thump on the ice at the end.
- Shield_4 (0.8 s asked; rejected): One low solid padded thud, like a fist hitting a thick leather pad, with a short tight creak of stretched leather right after it. Half a second, natural, no electronic sound.
- Shotgun_4 (1 s asked; Ability_Shotgun_a): A huge hockey slap shot heard up close: one very loud sharp crack of stick on puck with a heavy low thump, and then a long deep whoosh of air as the puck rockets away, fading over half a second. One hit only, no gunshot.

## A2. Processing

Scripts: `gen.mjs`, `proc.mjs`, `select.mjs` in `C:/Users/kazda/jam/audio-lab/verbs/scripts/` (copies of the scratchpad's `au10/`); they use lane AU4's `lib.mjs`.

- Mono fold, 48 kHz, 16-bit, one linear gain per file, no limiter or equaliser; no file reaches full scale.
- **Start:** shotgun and boards fall use AU4's impact rule (1.5 ms before the transient). The others start a few ms before the first 10 ms frame that comes within a set distance of the loudest one: 12 dB (hard stop), 15 dB (deke), 18 dB (stumble), 20 dB (shield, slow puck), 30 dB (hip check, so the whoosh stays; at most 0.3 s before the peak).
- **End:** 30 ms after the last frame within 45 dB of the loudest, held inside the length asked; fade-out 40 to 120 ms.
- **`BoardsFall_b`** is assembled from four kept takes of lanes AU4 and AU5 (no new generation): `BoardsBody_a` at 0 s, `GlassRattle_a` from 0.03 s at -6 dB, the first 0.4 s of `Slide_a` from 0.15 s at +3 dB, `Land_a` at 0.55 s at -6 dB; then set to -3 dBTP.
- Loudness: EBU R128 integrated with one second of silence before and after, as in AU4.

## A3. The files

`C:/Users/kazda/jam/audio-lab/verbs/`; mono 48 kHz 16-bit WAV. Lead-in is the time to the first sample within 12 dB of the peak. The last column is the loudness against the body check `Hit_a` (-22 LUFS at -3 dBTP): lower a file by that much to make it as loud as a hit.

| file | pick | from generation | length s | LUFS | true peak dBTP | lead-in ms | peak at ms | 40 dB down after ms | centroid Hz | LU against `Hit_a` |
|---|---|---|---|---|---|---|---|---|---|---|
| `Ability_Shotgun_a.wav` | **default** | Shotgun_4 | 0.8 | -21.7 | -3 | 0.9 | 31.7 | 540 | 800 | +0.3 |
| `Ability_Shotgun_b.wav` | alternate | Shotgun_1 | 0.8 | -19.6 | -3 | 0.4 | 11.4 | 430 | 1754 | +2.4 |
| `Ability_HipCheck_a.wav` | **default** | HipCheck_4 | 0.59 | -18.5 | -3 | 174.5 | 300 | 230 | 369 | +3.5 |
| `Ability_HipCheck_b.wav` | alternate | HipCheck_1 | 0.4 | -15.7 | -3 | 34.1 | 66.1 | 145 | 330 | +6.3 |
| `Ability_Shield_a.wav` | **default** | Shield_1 | 0.615 | -29.7 | -3 | 16 | 16.6 | 465 | 1688 | -7.7 |
| `Ability_Shield_b.wav` | alternate | Shield_2 | 0.51 | -25 | -3 | 11.6 | 136.1 | 325 | 1287 | -3.0 |
| `Ability_SlowPuck_a.wav` | **default** | SlowPuck_1 | 0.5 | -16.2 | -3 | 32.1 | 274.1 | 105 | 1092 | +5.8 |
| `Ability_SlowPuck_b.wav` | alternate | SlowPuck_3 | 0.735 | -18.5 | -3 | 4.8 | 113.9 | 550 | 1512 | +3.5 |
| `Deke_a.wav` | **default** | Deke_1 | 0.31 | -19.3 | -3 | 15.9 | 64.4 | 210 | 2677 | +2.7 |
| `Deke_b.wav` | alternate | Deke_2 | 0.35 | -19.2 | -3 | 3.5 | 79.6 | 205 | 2311 | +2.8 |
| `Stumble_a.wav` | **default** | Stumble_2 | 0.6 | -23.3 | -9 | 6.8 | 428.9 | 115 | 1536 | -1.3 |
| `Stumble_b.wav` | alternate | Stumble_1 | 0.415 | -22.8 | -9 | 84.2 | 192.4 | 135 | 2884 | -0.8 |
| `Brake_a.wav` | **default** | Brake_3 | 0.7 | -14.1 | -3 | 1.8 | 130.5 | 520 | 2180 | +7.9 |
| `Brake_b.wav` | alternate | Brake_2 | 0.57 | -16 | -3 | 4.6 | 301.9 | 180 | 4517 | +6.0 |
| `BoardsFall_a.wav` | **default** | BoardsFall_5 | 0.7 | -22.7 | -3 | 1.3 | 10.2 | 550 | 579 | -0.7 |
| `BoardsFall_b.wav` | alternate | assembled | 0.865 | -19.8 | -3 | 2.2 | 18.2 | 710 | 1224 | +2.2 |

Sounds the game has already, measured the same way:

| existing sound | length s | LUFS | true peak dBTP | 40 dB down after ms | centroid Hz |
|---|---|---|---|---|---|
| `ready/SFX_SlapShot_01.wav`: the shot the game plays now, in MS_shot | 0.428 | -21.6 | -1.5 | 275 | 1950 |
| `ready/SFX_SkateCarve_02.wav`: the skate carve, in MS_skate_loop | 0.789 | -24 | -1.5 | 630 | 2263 |
| `tackle/Hit_a.wav`: lane AU4, the body check | 0.3 | -22 | -3 | 180 | 286 |
| `tackle/BoardsBody_a.wav`: lane AU4, skater into the boards | 0.6 | -20.7 | -3 | 385 | 176 |
| `tackle/Bump_a.wav`: lane AU4, the standing bump | 0.45 | -19.4 | -3 | 275 | 931 |

## A4. How each default was chosen

The rules and thresholds are mine and rest on these 30 takes only. "dB of the take" means the share of the take's energy in that band.

**Ability: shotgun** (asked for: a much harder slap shot: a heavy crack of the stick and a deep whoosh of the puck leaving, 0.5 to 0.8 s; no firearm sound). Pass: a crack at once (first sample within 12 dB of the peak inside 5 ms); weight (energy under 400 Hz at least -6 dB of the take); a tail (40 dB down no sooner than 0.4 s). Order: a tail that gets deeper (spectral centroid of the last third under that of the first third) first.
- `Ability_Shotgun_a`: default: crack at 0.9 ms, the most weight of the takes that have a crack (under 400 Hz -2.5 dB of the take), and the only tail that gets deeper (centroid 840 Hz at the start, 120 Hz at the end), 0.54 s to -40 dB; the raw was out of phase between left and right (correlation -0.10), so the mono fold lost 6 dB of level and may have changed its colour.
- `Ability_Shotgun_b`: crack at 0.4 ms, broadband (centroid 1.75 kHz), 0.43 s to -40 dB; its tail gets brighter, not deeper (1.7 to 3.6 kHz): more hiss than whoosh; the raw decodes to +1.0 dBFS, so the service may have limited it.
- Rejected: `Shotgun_2` (no crack: a sub-bass thump (centroid 112 Hz) with three hits in its first 0.1 s); `Shotgun_3` (a bright crack with no weight (under 400 Hz -6.6 dB) and no tail: 40 dB down after 0.27 s, 0.35 s long).

**Ability: hip check** (asked for: a short charging whoosh into a heavy padded thump, 0.4 to 0.6 s). Pass: a lead-in before the thump (sound for at least 30 ms before the first sample within 12 dB of the peak); a low thump (energy under 400 Hz at least -3 dB of the take). Order: the longer lead-in first. The file keeps at most 0.3 s before the peak.
- `Ability_HipCheck_a`: default: the only take with a real lead-in: 0.17 s of rising noise 17 to 29 dB under the thump, then the thump from 0.18 s (peak at 0.30 s), 40 dB down 0.23 s later; 0.59 s. Fire it when the ability is pressed: the thump comes 0.18 s after the file starts.
- `Ability_HipCheck_b`: a 34 ms rise into the thump (peak at 66 ms), then gone: 40 dB down after 0.15 s; the tightest of the five, for a sound that must sit on the contact frame.
- Rejected: `HipCheck_2` (raw nearly silent (peak -41.5 dBFS)); `HipCheck_3` (a 23 ms rise only (the 0.26 s before it in the raw is 44 dB down, inaudible), sub-heavy (centroid 204 Hz)); `HipCheck_5` (no lead-in at all: a deep thump at 0 ms (centroid 195 Hz), 0.42 s to -40 dB; worth a listen as a plain heavy hit).

**Ability: shield** (asked for: a brief solid brace: a low padded thud with a short rising hum or a taut leathery creak, 0.4 to 0.7 s; no science-fiction force field). Pass: the thud comes first (loudest 20 ms frame in the first 50 ms); something follows it (at least one more burst); not sub-bass only (centroid over 200 Hz). Order: more low end in the thud first.
- `Ability_Shield_a`: default: a thud at 15 ms with low end (under 120 Hz -3 dB of the take), then three smaller bursts at 0.22, 0.30 and 0.36 s, 17 dB under it, which may be the creak; 0.62 s; the raw was quiet (peak -8.3 dBFS, 5 dB of gain); very peaky: at the same peak it is 8 LU quieter than the shotgun, so it needs about 6 dB more gain where it is played.
- `Ability_Shield_b`: no single thud: seven knocks in 0.3 s, the loudest 0.14 s in, mid-heavy (0.4 to 1.5 kHz -2 dB, under 120 Hz -13 dB); 0.51 s; fails the first test, kept as the only other usable take.
- Rejected: `Shield_3` (the main hit comes 0.37 s into the file, after a quiet low lead-in: late for a press); `Shield_4` (sub-bass rumble only (centroid 56 Hz), raw peak -14 dBFS).

**Ability: slow puck** (asked for: the puck suddenly dragging on rough ice: a short descending gritty scrape, 0.5 to 0.8 s). Pass: gritty (at least three separate bursts in the level). Order: a scrape that descends in pitch first (spectral centroid of the last third under that of the first), then one that only descends in level.
- `Ability_SlowPuck_a`: default: the only take that falls in pitch (centroid 1240 Hz in the first third, 783 Hz in the last); three bursts; it swells for 0.27 s and then stops inside 0.1 s; the sound itself is 0.43 s, the file 0.5 s.
- `Ability_SlowPuck_b`: the grittiest (five bursts), loud at once and falling in level over 0.55 s; its pitch does not fall (the tail gets brighter); 0.74 s.
- Rejected: `SlowPuck_2` (smooth and even: one unbroken scrape at full level for 0.55 s, no fall in pitch (2180 to 2060 Hz), still going at the 0.8 s cut).

**Deke** (asked for: a quick flick of the blade on ice with a little snow, light, 0.2 to 0.35 s). Pass: light (nothing under 120 Hz: at most -30 dB of the take); inside 0.35 s, or within 20 ms of it. Order: the quickest (time from the start to within 3 dB of the loudest level).
- `Deke_a`: default: the quickest (full level after 30 ms) and the shortest (0.31 s, ends on its own); centroid 2.7 kHz; the raw was very wide stereo (correlation 0.08), folded to mono.
- `Deke_b`: full level after 60 ms, 0.35 s (the sound ran 0.365 s; the last 15 ms are under the fade); more body at 0.4 to 1.5 kHz (-2 dB).
- Rejected: `Deke_3` (the slowest: loudest 0.13 s in, cut at 0.35 s; raw out of phase between left and right (correlation -0.29)).

**Stumble** (asked for: skates scrabbling as a player nearly falls: two or three quick blade scrapes and a pad rustle, no impact, 0.4 to 0.6 s, quieter than a hit). Pass: no impact (under 120 Hz at most -30 dB of the take); two or three separate bursts. Order: more bursts first. Level: -9 dBTP, 6 dB under the impacts.
- `Stumble_a`: default: three bursts, building to the loudest at 0.4 s and stopping inside 0.1 s; the most body (120 to 400 Hz -11 dB, which may be the pad rustle); the sound ran 0.625 s and is cut at 0.6 s (46 dB down there).
- `Stumble_b`: two bursts in one swell, the brightest (centroid 2.9 kHz); 0.42 s and ends on its own.
- Rejected: `Stumble_3` (an even scrape that only grows (loudest at 0.42 s), two bursts 6 dB apart; cut at 0.6 s).

**Hard stop** (asked for: the classic hockey stop: a sharp, loud spray-scrape of two skate blades shaving ice, 0.4 to 0.7 s). Pass: bright (centroid over 2 kHz); usable in mono (left and right of the raw in phase). Order: the sharpest start first (time from the start to within 3 dB of the loudest level).
- `Brake_a`: default: the sharpest start (within 4 dB of full level from the first frame, full level at 70 ms), holds for 0.4 s, then 0.25 s of decay (the spray settling); 0.7 s, ends on its own.
- `Brake_b`: swells for 0.2 s into its loudest point at 0.29 s and then drops 20 dB inside 0.1 s: a skid that ends in the stop; the brightest (centroid 4.5 kHz); 0.57 s.
- Rejected: `Brake_1` (raw out of phase between left and right (correlation -0.16), so the mono fold cancels part of it; an even scrape, cut at 0.7 s).

**Driven into the boards and down** (asked for: a padded body hitting the boards and sliding down them to the ice: boom, a short rattle, then a dull slide and thud, 0.7 to 1.0 s). Pass: two low events (a second peak under 300 Hz at least 0.25 s after the boom and within 20 dB of it). One of five generations passes. The alternate is assembled from four takes of the earlier lanes, so its timeline is exact but its sounds are heard elsewhere in the game too.
- `BoardsFall_a`: default: the only generation with a second low event: 0.29 s after the boom and 13 dB under it; content up to 5 kHz in the first 0.3 s (1.5 to 5 kHz -10 dB of the take), which is where a rattle would be; 40 dB down after 0.55 s; 0.7 s; whether the second event is a body reaching the ice cannot be told from the numbers.
- `BoardsFall_b`: assembled, no new generation: BoardsBody_a at 0 s, GlassRattle_a from 0.03 s (-6 dB), the first 0.4 s of Slide_a from 0.15 s (+3 dB), Land_a at 0.55 s (-6 dB); 0.87 s.
- Rejected: `BoardsFall_1` (a single short knock (40 dB down after 0.17 s; the sound is 0.26 s long), nothing after it); `BoardsFall_2` (a single boom, 0.43 s; a second low event only 36 dB under it); `BoardsFall_3` (a single sub-bass boom (centroid 60 Hz), no rattle, nothing over 400 Hz above -20 dB); `BoardsFall_4` (a deep boom with a long tail (0.55 s to -40 dB) but the second low event is 27 dB under it).

## A5. Previews

Stereo, each with 0.20 s of silence first; the mono sounds centred at -3 dB per channel; no gain changes.

| file | timeline (time zero is 0.20 s into the file) | length s | LUFS | true peak dBTP |
|---|---|---|---|---|
| `preview_abilities.wav` | Ability_Shotgun_a at 0.0 s; Ability_HipCheck_a at 1.0 s; Ability_Shield_a at 2.0 s; Ability_SlowPuck_a at 3.0 s | 3.8 | -18.9 | -6 |
| `preview_stumble_brake_deke.wav` | Stumble_a at 0.0 s; Brake_a at 1.0 s; Deke_a at 2.0 s | 2.61 | -16.7 | -6 |
| `preview_boardsfall.wav` | BoardsFall_a at 0.0 s; BoardsFall_b at 1.5 s | 2.665 | -20.5 | -6 |

`preview_boardsfall.wav` is an extra: the generated default and the assembled alternate one after the other.

## A6. Page and manifest

`C:/Users/kazda/jam/audio-lab/verbs/pick.html`, in the style of the tackle page: the previews, five existing sounds for comparison, then a section per sound with a player per take, its numbers, a loudness curve and the reason; the default pre-selected; "Copy picks". All 39 audio paths exist and the inline script passes `node --check`. **Not opened in a browser.** `manifest.json` holds every number, prompt and credit.

## A7. Ledger rows (effects)

Proposed names `SFX_<file name>`. `shipped` is `y` for the defaults; set it to what the MetaSounds reference in the end.

| path | kind | size_mb | origin | tool | licence | url | date | device | shipped | note |
|---|---|---|---|---|---|---|---|---|---|---|
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Ability_Shotgun_a.uasset | audio | 0.1 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU10 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/verbs/ledger.jsonl (prompts; status/AU10-report.md) | 2026-11-13 | B | y | shotgun ability shot, crack and deep tail; mono 48 kHz 0.8 s, -21.7 LUFS, -3 dBTP; from verbs/Ability_Shotgun_a.wav (76844 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Ability_Shotgun_b.uasset | audio | 0.1 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU10 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/verbs/ledger.jsonl (prompts; status/AU10-report.md) | 2026-11-13 | B | n | shotgun ability shot, broadband crack; mono 48 kHz 0.8 s, -19.6 LUFS, -3 dBTP; from verbs/Ability_Shotgun_b.wav (76844 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Ability_HipCheck_a.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU10 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/verbs/ledger.jsonl (prompts; status/AU10-report.md) | 2026-11-13 | B | y | hip check ability, whoosh into thump (thump 0.18 s in); mono 48 kHz 0.59 s, -18.5 LUFS, -3 dBTP; from verbs/Ability_HipCheck_a.wav (56684 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Ability_HipCheck_b.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU10 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/verbs/ledger.jsonl (prompts; status/AU10-report.md) | 2026-11-13 | B | n | hip check ability, tight thump; mono 48 kHz 0.4 s, -15.7 LUFS, -3 dBTP; from verbs/Ability_HipCheck_b.wav (38444 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Ability_Shield_a.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU10 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/verbs/ledger.jsonl (prompts; status/AU10-report.md) | 2026-11-13 | B | y | shield ability, thud and creak; mono 48 kHz 0.615 s, -29.7 LUFS, -3 dBTP; from verbs/Ability_Shield_a.wav (59084 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Ability_Shield_b.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU10 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/verbs/ledger.jsonl (prompts; status/AU10-report.md) | 2026-11-13 | B | n | shield ability, run of knocks; mono 48 kHz 0.51 s, -25 LUFS, -3 dBTP; from verbs/Ability_Shield_b.wav (49004 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Ability_SlowPuck_a.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU10 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/verbs/ledger.jsonl (prompts; status/AU10-report.md) | 2026-11-13 | B | y | slow puck ability, scrape falling in pitch; mono 48 kHz 0.5 s, -16.2 LUFS, -3 dBTP; from verbs/Ability_SlowPuck_a.wav (48044 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Ability_SlowPuck_b.uasset | audio | 0.1 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU10 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/verbs/ledger.jsonl (prompts; status/AU10-report.md) | 2026-11-13 | B | n | slow puck ability, gritty scrape falling in level; mono 48 kHz 0.735 s, -18.5 LUFS, -3 dBTP; from verbs/Ability_SlowPuck_b.wav (70604 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Deke_a.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU10 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/verbs/ledger.jsonl (prompts; status/AU10-report.md) | 2026-11-13 | B | y | deke, quick blade flick; mono 48 kHz 0.31 s, -19.3 LUFS, -3 dBTP; from verbs/Deke_a.wav (29804 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Deke_b.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU10 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/verbs/ledger.jsonl (prompts; status/AU10-report.md) | 2026-11-13 | B | n | deke, blade flick with more body; mono 48 kHz 0.35 s, -19.2 LUFS, -3 dBTP; from verbs/Deke_b.wav (33644 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Stumble_a.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU10 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/verbs/ledger.jsonl (prompts; status/AU10-report.md) | 2026-11-13 | B | y | stumble, three skate scrapes; mono 48 kHz 0.6 s, -23.3 LUFS, -9 dBTP; from verbs/Stumble_a.wav (57644 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Stumble_b.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU10 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/verbs/ledger.jsonl (prompts; status/AU10-report.md) | 2026-11-13 | B | n | stumble, one bright swell; mono 48 kHz 0.415 s, -22.8 LUFS, -9 dBTP; from verbs/Stumble_b.wav (39884 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Brake_a.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU10 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/verbs/ledger.jsonl (prompts; status/AU10-report.md) | 2026-11-13 | B | y | hard stop, sharp start; mono 48 kHz 0.7 s, -14.1 LUFS, -3 dBTP; from verbs/Brake_a.wav (67244 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Brake_b.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU10 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/verbs/ledger.jsonl (prompts; status/AU10-report.md) | 2026-11-13 | B | n | hard stop, skid into the stop; mono 48 kHz 0.57 s, -16 LUFS, -3 dBTP; from verbs/Brake_b.wav (54764 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_BoardsFall_a.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU10 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/verbs/ledger.jsonl (prompts; status/AU10-report.md) | 2026-11-13 | B | y | into the boards and down, one generation; mono 48 kHz 0.7 s, -22.7 LUFS, -3 dBTP; from verbs/BoardsFall_a.wav (67244 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_BoardsFall_b.uasset | audio | 0.1 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU10 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU10-report.md) | 2026-11-13 | B | n | into the boards and down, assembled from BoardsBody_a, GlassRattle_a, Slide_a, Land_a; mono 48 kHz 0.865 s, -19.8 LUFS, -3 dBTP; from verbs/BoardsFall_b.wav (83084 bytes WAV; set the uasset bytes after import) |

# Part B: the commentator's lines

## B1. How they were made

The pipeline was found complete: lane VOX2's scripts in `C:/Users/kazda/jam/audio-lab/vox2/` (`lib.mjs`, `gen.mjs`, `post.mjs`) and the voice in `audio-lab/commentators/saved.json`. I used them unchanged:

- **Voice:** C1, the designed voice the operator accepted (`status/B-audio-picks.md`), the same voice id as all 340 bank lines.
- **Settings:** `eleven_multilingual_v2`, stability 0.18, style 0.85, speed 1.15, text in normal capitals with the closing mark sent as "!" (VOX2's "rule c"). Output `mp3_44100_128`, as the bank.
- **Processing:** the bank's steps: mono 48 kHz, silence trimmed at 0.001 of full scale (-60 dBFS), 2 ms and 10 ms fades, gain to -18 LUFS unless the true peak would pass -1.5 dBTP, 16-bit.
- **Texts:** read from `Game/Content/Commentary/rough.commentary.json` (60) and `abilities.commentary.json` (12) at run time, sent exactly as written apart from the closing mark. Nothing else is in a request: no game name, no id.
- **Requests:** 73 first takes, 0 failed; 7 lines failed my checks and got one second take each (B3): **80 requests, 1,289 credits** (2,928 characters, 0.44 credits a character, the bank's rate). Ledger: `C:/Users/kazda/jam/audio-lab/commentary-rough/ledger.jsonl`. Every take is kept in `commentary-rough/mp3/` (`_t1`, `_t2`).
- **Output:** `commentary-rough/wav/VO_Commentator_<id with dots as underscores>.wav`. The WAVs are in a `wav/` subfolder because the bank's manifest format says `"wav": "wav/..."`. They are 48 kHz like the lab bank; lane AUD converted the bank to 24 kHz at import, and these need the same step.

## B2. The yardstick: the existing bank

Measured from `vox2/manifest.json` (340 lines) and the WAVs:
- Speed: median 2.65 words a second (5th to 95th percentile 1.64 to 4.05; extremes 1.04 and 5.34). Length: median 1.7 s, 90th percentile 2.42 s; 4 words at the median.
- Length against text: 0.458 s plus 0.0549 s per character, with a spread (standard deviation) of 0.392 s.
- A sample of 57 raw takes: raw loudness -18.7 LUFS at the median (-20.9 to -16.6); at least 16 ms of silence after the last word in every one.

Five bank lines as the reference (they are on the listening page too):

| bank line | text | s | words | words / s | LUFS | dBTP |
|---|---|---|---|---|---|---|
| check.big.01 | Oh, that is a big hit! | 1.33 | 6 | 4.51 | -18.9 | -1.5 |
| knockdown.any.01 | Down goes the carrier. | 2.1 | 4 | 1.9 | -18.6 | -1.5 |
| knockdown.human.01 | Back up, quickly now. | 1.63 | 4 | 2.45 | -18 | -1.5 |
| goal.for.01 | What a finish! | 1 | 3 | 3 | -19 | -1.5 |
| lose.blowout.01 | A night to forget. Back to rehearsals. | 2.31 | 7 | 3.03 | -18.7 | -1.5 |

These five sit at -18.0 to -19.0 LUFS: a line with a sharp peak is held by the -1.5 dBTP ceiling before it reaches -18. The new lines behave the same way (-18.0 to -20.0 LUFS; 26 of 73 held by the ceiling, the lowest -20.0).

## B3. Result

| | new lines | bank |
|---|---|---|
| lines | 73 | 340 |
| words, median | 7 | 4 |
| length, median | 2.46 s | 1.7 s |
| words a second, median (range) | 2.81 (1.68 to 4.33) | 2.65 (1.04 to 5.34) |
| loudness | -18.0 to -20.0 LUFS | -18 LUFS target, same ceiling |
| true peak, highest | -1.5 dBTP | -1.5 dBTP |
| live lines over 2.5 s | 34 of 72 | 1 of 287 |
| total | 181.1 s, 16.6 MB of WAV | 611.8 s |

**Checks per take** (a failed check earned one retake): clipped (four or more samples in a row at the ceiling); cut off (the last word ends within 15 ms of the end of the file and is still loud); raw loudness more than 6 LU from the bank's median; more than 4 LU under target; length more than 2.5 standard deviations from what the bank's fit expects for the text; speed outside the bank's range; under 0.5 s; more than 150 ms of leading silence.

**Seven first takes failed and were retaken once; all seven second takes pass and are the ones kept:**
- `check.solid.02`: first take 3.64 s (length 3.64 s against 2.32 s expected for its 34 characters (+3.4 sd)); second take 1.92 s, kept.
- `check.cross.01`: first take 4.06 s (length 4.06 s against 2.98 s expected for its 46 characters (+2.8 sd)); second take 3.36 s, kept.
- `knockdown.human.08`: first take 3.92 s (length 3.92 s against 2.65 s expected for its 40 characters (+3.2 sd)); second take 2.46 s, kept.
- `knockdown.human.09`: first take 4 s (length 4.00 s against 2.98 s expected for its 46 characters (+2.6 sd)); second take 3.15 s, kept.
- `knockdown.human.12`: first take 3.9 s (cut off (ends 0 ms before the file end at -21 dB); length 3.90 s against 2.76 s expected for its 42 characters (+2.9 sd)); second take 2.56 s, kept.
- `goal.after_hit.03`: first take 4.38 s (length 4.38 s against 2.43 s expected for its 36 characters (+5.0 sd)); second take 3.3 s, kept.
- `ability.slow_puck.03`: first take 2.5 s (cut off (ends 10 ms before the file end at -28 dB)); second take 2.85 s, kept.

No kept take is clipped, cut off, or outside the bank's speed range. Leading silence is 6 ms at most.

By situation:

| situation | lines | median s | longest s | over 2.5 s |
|---|---|---|---|---|
| check.light | 6 | 2.24 | 2.74 | 2 |
| check.solid | 7 | 1.92 | 2.34 | 0 |
| knockdown.big | 7 | 2.58 | 3.3 | 4 |
| check.cross | 10 | 2.8 | 3.36 | 6 |
| knockdown.human | 10 | 2.5 | 3.15 | 4 |
| knockdown.massacre | 10 | 2.88 | 3.9 | 6 |
| goal.after_hit | 10 | 2.45 | 3.3 | 4 |
| ability.shotgun | 3 | 3.32 | 3.4 | 3 |
| ability.hip_check | 3 | 2.33 | 2.61 | 1 |
| ability.shield | 3 | 2.33 | 2.87 | 1 |
| ability.slow_puck | 3 | 3.23 | 3.49 | 3 |
| lose.blowout | 1 | 2.75 | 2.75 | 1 |

All 73 lines (`*` = a live line over 2.5 s):

| id | text as written | s | words | words / s | LUFS | dBTP | takes |
|---|---|---|---|---|---|---|---|
| check.light.01 | A tap on the shoulder. Rude, but a tap. | 2.24 | 9 | 4.01 | -19.2 | -1.5 | 1 |
| check.light.02 | Just a nudge. The real one is coming. | 2.74 * | 8 | 2.92 | -18 | -2.1 | 1 |
| check.light.03 | Polite little bump. | 1.54 | 3 | 1.95 | -18 | -3.5 | 1 |
| check.light.04 | He felt that about as much as a breeze. | 2.17 | 9 | 4.16 | -18 | -2 | 1 |
| check.light.05 | Shoulder to shoulder. Warming up. | 2.59 * | 5 | 1.93 | -18 | -2 | 1 |
| check.light.06 | That one was a hello. | 1.48 | 5 | 3.38 | -18 | -2.9 | 1 |
| check.solid.01 | That one rattled the glass. | 1.48 | 5 | 3.38 | -18 | -2.8 | 1 |
| check.solid.02 | Now he knows where the boards are. | 1.92 | 7 | 3.64 | -18 | -2.6 | 2, take 2 kept |
| check.solid.03 | Square hit. Clean and loud. | 2.17 | 5 | 2.3 | -18 | -2.6 | 1 |
| check.solid.04 | Lays the body and walks away. | 2.05 | 6 | 2.93 | -18 | -1.6 | 1 |
| check.solid.05 | Hell of a hit. | 1.12 | 4 | 3.58 | -18.1 | -1.5 | 1 |
| check.solid.06 | Shoulders, hips, and a lot of gravity. | 1.79 | 7 | 3.91 | -18 | -2.7 | 1 |
| check.solid.07 | That is how you carry the check. | 2.34 | 7 | 3 | -18.7 | -1.5 | 1 |
| knockdown.big.01 | Flattened! Somebody check the ice for dents. | 3.3 * | 7 | 2.12 | -19 | -1.5 | 1 |
| knockdown.big.02 | Sent flying, and the puck stayed behind. | 2.58 * | 7 | 2.71 | -18.3 | -1.5 | 1 |
| knockdown.big.03 | Folded in half and set down gently. Not gently. | 2.93 * | 9 | 3.07 | -18.9 | -1.5 | 1 |
| knockdown.big.04 | Planted him like a flag. | 1.55 | 5 | 3.23 | -18 | -3.2 | 1 |
| knockdown.big.05 | He is wearing the boards now. | 2.21 | 6 | 2.72 | -18 | -2.6 | 1 |
| knockdown.big.06 | Gravity wins again. | 1.6 | 3 | 1.88 | -18 | -2.6 | 1 |
| knockdown.big.07 | The ice has a new best friend. | 2.8 * | 7 | 2.5 | -18.1 | -1.5 | 1 |
| check.cross.01 | Stick across the chest! That is a cross-check. | 3.36 * | 8 | 2.38 | -19.2 | -1.5 | 2, take 2 kept |
| check.cross.02 | Cross-check, and he sits down hard. | 2.56 * | 6 | 2.34 | -19.1 | -1.5 | 1 |
| check.cross.03 | Bowled over with the shaft. | 1.49 | 5 | 3.36 | -18 | -3.8 | 1 |
| check.cross.04 | Cross-check! Nobody saw that coming. He did not. | 3.33 * | 8 | 2.4 | -18 | -1.5 | 1 |
| check.cross.05 | Wood across the middle. Down he goes. | 3.29 * | 7 | 2.13 | -18 | -2.4 | 1 |
| check.cross.06 | The cross-check does the talking. | 1.61 | 5 | 3.11 | -18 | -2 | 1 |
| check.cross.07 | Sent sprawling by a cross-check. | 1.85 | 5 | 2.71 | -18 | -2.9 | 1 |
| check.cross.08 | Damn, that cross-check had a lot of mustard. | 2.06 | 8 | 3.88 | -18 | -2.1 | 1 |
| check.cross.09 | He ran into the stick and the ice caught him. | 2.93 * | 10 | 3.41 | -18 | -2.5 | 1 |
| check.cross.10 | Cross-check! The glass felt that too. | 2.8 * | 6 | 2.14 | -18 | -2.9 | 1 |
| knockdown.human.04 | You are down. Count the lights, then get up. | 2.36 | 9 | 3.81 | -18 | -1.9 | 1 |
| knockdown.human.05 | The ice caught you. Shake it off. | 1.99 | 7 | 3.52 | -18 | -3 | 1 |
| knockdown.human.06 | Good hit. Now go give one back. | 1.62 | 7 | 4.33 | -18 | -2 | 1 |
| knockdown.human.07 | Up you get. They are not done. | 2.46 | 7 | 2.84 | -18.9 | -1.5 | 1 |
| knockdown.human.08 | That one had your name on it. Go answer. | 2.46 | 9 | 3.65 | -18 | -2.4 | 2, take 2 kept |
| knockdown.human.09 | Flat on the ice. It happens to the best of us. | 3.15 * | 11 | 3.49 | -18.7 | -1.5 | 2, take 2 kept |
| knockdown.human.10 | Sit a second. Then go find him. | 2.52 * | 7 | 2.78 | -18.1 | -1.5 | 1 |
| knockdown.human.11 | That one is going on the wall. Now answer it. | 2.74 * | 10 | 3.65 | -18 | -2.9 | 1 |
| knockdown.human.12 | Folded. Back up, there is a puck to chase. | 2.56 * | 9 | 3.51 | -18.7 | -1.5 | 2, take 2 kept |
| knockdown.human.13 | Rolled right over you. Time to return the favour. | 2.5 | 9 | 3.6 | -18 | -2.9 | 1 |
| knockdown.massacre.01 | Three down in a flash. This is a massacre shift. | 3.9 * | 10 | 2.57 | -18 | -2.3 | 1 |
| knockdown.massacre.02 | They are bowling out there. Strike after strike. | 2.88 * | 8 | 2.78 | -18 | -2.1 | 1 |
| knockdown.massacre.03 | Another one down. The ice is getting crowded. | 2.29 | 8 | 3.49 | -18 | -2.1 | 1 |
| knockdown.massacre.04 | Nobody is standing. Well, nearly nobody. | 3.5 * | 6 | 1.71 | -18 | -2.4 | 1 |
| knockdown.massacre.05 | Hit after hit after hit. What a shift. | 1.92 | 8 | 4.18 | -18.6 | -1.5 | 1 |
| knockdown.massacre.06 | They are going through the other team like a lawn mower. | 2.81 * | 11 | 3.92 | -18 | -1.9 | 1 |
| knockdown.massacre.07 | Down, down, and down again. | 2.06 | 5 | 2.43 | -18 | -3.6 | 1 |
| knockdown.massacre.08 | This is not hockey, this is a wrecking yard. | 3.21 * | 9 | 2.81 | -18.2 | -1.5 | 1 |
| knockdown.massacre.09 | Pile them up! Somebody sweep the ice. | 3.22 * | 7 | 2.17 | -20 | -1.5 | 1 |
| knockdown.massacre.10 | The shift of the night, and the night is young. | 2.38 | 10 | 4.2 | -19.5 | -1.5 | 1 |
| goal.after_hit.01 | Hit him, take the puck, score. Simple. | 3.09 * | 7 | 2.27 | -18.7 | -1.5 | 1 |
| goal.after_hit.02 | The check made the goal. | 2.42 | 5 | 2.07 | -18 | -2.3 | 1 |
| goal.after_hit.03 | Flatten, grab, fire. That is a goal. | 3.3 * | 7 | 2.12 | -18 | -3.1 | 2, take 2 kept |
| goal.after_hit.04 | He went down and the net went in. | 2.25 | 8 | 3.55 | -18 | -3.2 | 1 |
| goal.after_hit.05 | The hit paid off in full. | 1.64 | 6 | 3.66 | -18 | -2.3 | 1 |
| goal.after_hit.06 | Lays the body, lights the lamp. | 2.69 * | 6 | 2.23 | -18 | -1.6 | 1 |
| goal.after_hit.07 | Rough work, pretty finish. | 1.39 | 4 | 2.89 | -18.1 | -1.5 | 1 |
| goal.after_hit.08 | One hit, one goal. Efficient. | 2.98 * | 5 | 1.68 | -19 | -1.5 | 1 |
| goal.after_hit.09 | The ice shook and the net followed. | 2.14 | 7 | 3.27 | -18 | -2.4 | 1 |
| goal.after_hit.10 | That goal started with a shoulder. | 2.45 | 6 | 2.45 | -18 | -1.6 | 1 |
| ability.shotgun.01 | Shotgun! That puck went through everybody. | 3.4 * | 6 | 1.76 | -19.8 | -1.5 | 1 |
| ability.shotgun.02 | Bowling night. The whole lane is down. | 2.76 * | 7 | 2.53 | -18 | -1.6 | 1 |
| ability.shotgun.03 | One blast and nobody is left standing. | 3.32 * | 7 | 2.11 | -19.6 | -1.5 | 1 |
| ability.hip_check.01 | Hip check! He sent him three metres. | 2.61 * | 7 | 2.68 | -18.4 | -1.5 | 1 |
| ability.hip_check.02 | Out of nowhere. That hip lands like a truck. | 2.33 | 9 | 3.86 | -18.1 | -1.5 | 1 |
| ability.hip_check.03 | From a standstill. What a hip! | 2.15 | 6 | 2.79 | -18.8 | -1.5 | 1 |
| ability.shield.01 | Shield is up. Bounce off him all you like. | 2.33 | 9 | 3.87 | -18 | -3 | 1 |
| ability.shield.02 | You cannot knock this one over right now. | 2.27 | 8 | 3.52 | -18 | -3.3 | 1 |
| ability.shield.03 | Armour on. Hit him and you just bounce. | 2.87 * | 8 | 2.79 | -18 | -2.9 | 1 |
| ability.slow_puck.01 | Slow puck. Watch it crawl, then wake up. | 3.49 * | 8 | 2.29 | -18 | -1.9 | 1 |
| ability.slow_puck.02 | A trick play. The puck is taking its time. | 3.23 * | 9 | 2.79 | -18.7 | -1.5 | 1 |
| ability.slow_puck.03 | Look at that puck dawdle. Then it goes. | 2.85 * | 8 | 2.81 | -18 | -1.8 | 2, take 2 kept |
| lose.blowout.01 | A night to forget. Back to practice. | 2.75 | 7 | 2.55 | -18 | -1.5 | 1 |

The other device's files carry a placeholder `seconds` per line; the measured lengths differ from them by -0.8 to +1.7 s (median +0.2 s). The measured values are in `commentary-rough/manifest.json`.

## B4. Texts I flag (all recorded as written)

No typo found. British spellings ("metres", "favour", "Armour") are spoken the same either way. None of these was changed:

1. **Closing mark.** Every line was sent with "!" in place of its last "." (the bank's rule, B1). If "exactly as written" was meant to include the full stop, the delivery would differ from the 340 bank lines; say so and I re-record.
2. **Cross-check lines describe a fall on a stick lift.** `check.cross.02`, `.03`, `.05`, `.07`, `.09` (and `.04` "He did not") say the skater goes down; the picker speaks them on a clean lift, which knocks nobody down (the other device's own open point in COMMENTARY.md section 9).
3. **Rating, borderline by VOICE.md's own list:**
   - `knockdown.human.04` "You are down. Count the lights, then get up.": reads as dazed; the guide allows a wince and no more.
   - `knockdown.big.03` "Folded in half and set down gently. Not gently.": "folded" is on the allowed list; "in half" pushes it toward a body image.
   - `ability.shotgun.03` "One blast and nobody is left standing.": a weapon image; the guide says the shotgun is an ability, "not a gun".
   - `knockdown.massacre.08` "This is not hockey, this is a wrecking yard." and `.06` "like a lawn mower": machines against people; the guide wants the brag to be about the hits.
   - `knockdown.human.10` "Sit a second. Then go find him.": payback aimed at a person; mild.
4. **Mild words:** `check.solid.05` "Hell of a hit." and `check.cross.08` "Damn, ..." (2 of 72, inside the guide's cap).
5. **Gendered pronouns** in 14 lines (check.light.04, check.solid.02, knockdown.big.04, knockdown.big.05, check.cross.02, check.cross.04, check.cross.05, check.cross.09, knockdown.human.10, goal.after_hit.01, goal.after_hit.04, ability.hip_check.01, ability.shield.01, ability.shield.03). The bank's register (briefs/commentary/README.md, rule 7) keeps skaters ungendered; VOICE.md's own examples use "he", so I take it as decided.
6. **A number:** `ability.hip_check.01` "He sent him three metres." It matches the rule today (a 3 m push; 2.89 m measured in the other device's test), and goes stale if that constant changes.
7. **"Massacre shift"** is spoken in `knockdown.massacre.01`; the glossary allows it.

## B5. The replacement for `lose.blowout.01`

Old text: "A night to forget. Back to rehearsals." (the film word is "rehearsals"). Three hockey replacements in the same register (a level, dry stoppage line, not gleeful):

1. **"A night to forget. Back to practice."** Recorded: the smallest change, the same rhythm, plain rink talk.
2. "A night to forget. Bag skate in the morning." (hockey slang for a punishing practice; funnier, but less widely known).
3. "A night to forget. Back to the drawing board." (safe, but not a hockey phrase).

Recorded as `commentary-rough/wav/VO_Commentator_lose_blowout_01.wav`: 2.75 s, 2.55 words a second, -18 LUFS, -1.5 dBTP (the old take: 2.31 s, -18.7 LUFS). The old WAV in `vox2/wav/` is untouched. The import lane must also change the text in `Audio/VO/commentary.json`, and `briefs/commentary/gen-lines.mjs` (line 164) still holds the old text.

## B6. Page, manifest, preview

- `commentary-rough/listen.html`: the preview, the five bank lines for comparison, then every line grouped by situation with its player, its text and its numbers; a tick box per line and a button that copies the ids to record again. All 79 audio paths exist; the script passes `node --check`. **Not opened in a browser.**
- `commentary-rough/manifest.json`: the bank's format (`voice`, `delivery`, `format`, `totals`, and per line `id`, `speaker`, `event`, `kind`, `team`, `priority`, `cooldown_s`, `text`, `wav`, `seconds`), plus one top-level `supplement` note. `measure.json` holds the per-take checks.
- `commentary-rough/preview_rough.wav`: check.light.01, check.solid.01, knockdown.big.01, check.cross.01, knockdown.human.04, knockdown.massacre.01, goal.after_hit.01, ability.shotgun.01, ability.hip_check.01, ability.shield.01; half a second between lines; 32.57 s, -18.8 LUFS, -1.5 dBTP.

## B7. Ledger rows (commentary), in lane AUD's format

The tool column says 24 kHz, as AUD's rows do, because that is what the import makes of them. `VO_Commentator_lose_blowout_01` replaces an existing asset; the other 72 are new.

| path | kind | size_mb | origin | tool | licence | url | date | device | shipped | note |
|---|---|---|---|---|---|---|---|---|---|---|
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_check_light_01.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "A tap on the shoulder. Rude, but a tap." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_check_light_02.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Just a nudge. The real one is coming." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_check_light_03.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Polite little bump." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_check_light_04.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "He felt that about as much as a breeze." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_check_light_05.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Shoulder to shoulder. Warming up." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_check_light_06.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "That one was a hello." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_check_solid_01.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "That one rattled the glass." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_check_solid_02.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Now he knows where the boards are." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_check_solid_03.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Square hit. Clean and loud." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_check_solid_04.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Lays the body and walks away." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_check_solid_05.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Hell of a hit." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_check_solid_06.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Shoulders, hips, and a lot of gravity." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_check_solid_07.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "That is how you carry the check." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_knockdown_big_01.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Flattened! Somebody check the ice for dents." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_knockdown_big_02.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Sent flying, and the puck stayed behind." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_knockdown_big_03.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Folded in half and set down gently. Not gently." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_knockdown_big_04.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Planted him like a flag." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_knockdown_big_05.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "He is wearing the boards now." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_knockdown_big_06.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Gravity wins again." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_knockdown_big_07.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "The ice has a new best friend." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_check_cross_01.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Stick across the chest! That is a cross-check." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_check_cross_02.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Cross-check, and he sits down hard." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_check_cross_03.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Bowled over with the shaft." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_check_cross_04.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Cross-check! Nobody saw that coming. He did not." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_check_cross_05.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Wood across the middle. Down he goes." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_check_cross_06.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "The cross-check does the talking." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_check_cross_07.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Sent sprawling by a cross-check." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_check_cross_08.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Damn, that cross-check had a lot of mustard." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_check_cross_09.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "He ran into the stick and the ice caught him." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_check_cross_10.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Cross-check! The glass felt that too." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_knockdown_human_04.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "You are down. Count the lights, then get up." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_knockdown_human_05.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "The ice caught you. Shake it off." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_knockdown_human_06.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Good hit. Now go give one back." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_knockdown_human_07.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Up you get. They are not done." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_knockdown_human_08.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "That one had your name on it. Go answer." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_knockdown_human_09.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Flat on the ice. It happens to the best of us." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_knockdown_human_10.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Sit a second. Then go find him." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_knockdown_human_11.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "That one is going on the wall. Now answer it." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_knockdown_human_12.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Folded. Back up, there is a puck to chase." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_knockdown_human_13.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Rolled right over you. Time to return the favour." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_knockdown_massacre_01.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Three down in a flash. This is a massacre shift." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_knockdown_massacre_02.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "They are bowling out there. Strike after strike." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_knockdown_massacre_03.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Another one down. The ice is getting crowded." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_knockdown_massacre_04.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Nobody is standing. Well, nearly nobody." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_knockdown_massacre_05.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Hit after hit after hit. What a shift." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_knockdown_massacre_06.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "They are going through the other team like a lawn mower." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_knockdown_massacre_07.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Down, down, and down again." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_knockdown_massacre_08.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "This is not hockey, this is a wrecking yard." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_knockdown_massacre_09.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Pile them up! Somebody sweep the ice." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_knockdown_massacre_10.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "The shift of the night, and the night is young." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_goal_after_hit_01.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Hit him, take the puck, score. Simple." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_goal_after_hit_02.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "The check made the goal." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_goal_after_hit_03.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Flatten, grab, fire. That is a goal." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_goal_after_hit_04.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "He went down and the net went in." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_goal_after_hit_05.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "The hit paid off in full." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_goal_after_hit_06.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Lays the body, lights the lamp." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_goal_after_hit_07.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Rough work, pretty finish." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_goal_after_hit_08.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "One hit, one goal. Efficient." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_goal_after_hit_09.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "The ice shook and the net followed." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_goal_after_hit_10.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "That goal started with a shoulder." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_ability_shotgun_01.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Shotgun! That puck went through everybody." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_ability_shotgun_02.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Bowling night. The whole lane is down." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_ability_shotgun_03.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "One blast and nobody is left standing." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_ability_hip_check_01.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Hip check! He sent him three metres." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_ability_hip_check_02.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Out of nowhere. That hip lands like a truck." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_ability_hip_check_03.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "From a standstill. What a hip!" (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_ability_shield_01.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Shield is up. Bounce off him all you like." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_ability_shield_02.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "You cannot knock this one over right now." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_ability_shield_03.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Armour on. Hit him and you just bounce." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_ability_slow_puck_01.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Slow puck. Watch it crawl, then wake up." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_ability_slow_puck_02.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "A trick play. The puck is taking its time." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_ability_slow_puck_03.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "Look at that puck dawdle. Then it goes." (set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_lose_blowout_01.uasset | audio | 0.0 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/commentary-rough/manifest.json (text); commentary-rough/ledger.jsonl | 2026-11-13 | B | y | commentator "A night to forget. Back to practice." (replaces the take "A night to forget. Back to rehearsals.") (set the uasset bytes after import) |

# What I could not verify

- **Anything audible.** For the lines: pronunciation ("dawdle", "sprawling", "mustard"), stress, whether "!" on dry lines ("Rude, but a tap.") sounds wrong, whether the wince lines sound kind and not mocking, whether any take has a breath, a stray syllable or an accent drift. For the effects: that each is the thing it is named after, and above all that the shotgun shot is not a gunshot.
- **Words in the middle of a line.** My checks catch a cut-off end, not a swallowed or garbled word inside.
- **The voice against the bank by ear.** Same voice id and settings; the measured speed and loudness match; timbre is not measured.
- **Mono colour.** `Ability_Shotgun_a` came from a raw take with left and right out of phase; the mono fold may sound thinner than the stereo raw.
- **The pages in a browser; anything in the engine.** No import, no MetaSound, no cook; the picker's behaviour with 2.5 to 3.9 s lines is not tried.
- **The pick rules** are calibrated on these takes only.

# What fell short

- **Line length** (item 1 at the top): 34 live lines over 2.5 s, the longest 3.9 s (`knockdown.massacre.01`, 10 words).
- **Hip check:** one take of five has a whoosh, and its thump is 0.18 s in.
- **Boards fall:** five generations, one with a second low event (0.29 s after the boom, 13 dB under); none shows a slide. Hence the assembled alternate.
- **Shield:** a hum or a creak was not tested for as such (only a thud and bursts after it); the default is a thud with small bursts after it, and it is very peaky (-29.7 LUFS at -3 dBTP). One take was sub-bass only.
- **Shotgun:** the "deep whoosh" is a tail that gets darker, not a measured whoosh.
- **Slow puck:** the take that falls in pitch is short (the sound is 0.43 s; the file is 0.5 s to meet the brief).
- **Three raw takes had left and right out of phase** (`Shotgun_4`, `Brake_1`, `Deke_3`); one is the shotgun default.

# Decisions for the lead

1. **The long lines.** Three ways: (a) accept: the hit-call rule already lets a hit line cut a playing line, but a 3 s line holds the 2.5 s gate longer; (b) ask the other device for shorter alternates of the 15 lines over 3 s; (c) let me spend about 600 credits on two more takes of the 34 long lines and keep the shortest (lane VOX2 brought 24 of 26 under 2.5 s that way, but those texts were shorter).
2. **The closing "!"** (B4 point 1): keep the bank's rule, or re-record as written with full stops.
3. **The replacement text** for `lose.blowout.01`: "Back to practice." is recorded; choose another and it is one request.
4. **The flagged texts** (B4 point 3): they are the other device's copy; pass the list on or let them stand.
5. **Effect levels:** import at file level and trim in the MetaSounds (A3's last column), or have me re-level every effect to one loudness.
6. **Hip check timing:** fire `Ability_HipCheck_a` on the press (the thump then lands 0.18 s later, about when the check lands), or use `_b` on contact.
7. **Boards fall:** the generated `_a` or the assembled `_b`.

# The next three evolutions for the sound track, ranked

1. **Prove the operator's mix rule in a recorded match.** Record the output of a three-minute bot match with each bus alone (effects, commentator, crowd, music) and with all. The number: the effects bus at least 6 LU under the commentator bus and under the crowd-plus-music sum over the match, and no hit window in which the effects' momentary loudness passes the commentator line playing over it. Today nobody has that number; the operator's ear set the rule.
2. **Make the booth call the hits in time.** The number: the share of knockdowns in a three-minute pro match that get a spoken line (3 of 6 in the other device's run `pro2`: two plain calls and one massacre call), raised to 5 of 6, with live lines over 2.5 s brought from 34 of 72 to under 10 by shorter texts or retakes. A hit line that ends after the next hit is a late call.
3. **Let the crowd answer the aggression.** Wire `CrowdRoar` to big knockdowns and drive the bed's `Excite` from the knockdown count. The number: the crowd bus rises at least 4 dB within one second of a knockdown of Strength 0.8 or more and is back within 1 dB of the bed five seconds later, in a recorded match; and in 20 knockdowns no reaction take plays twice in a row.
