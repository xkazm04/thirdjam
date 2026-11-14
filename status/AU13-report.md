# Lane AU13 report: the sounds of the dive, the dash and the toy weapons (device B), 2026-11-14 02:44

- **All fifteen sounds are delivered**, each with a default and an alternate: 30 kept takes, 31 rejected candidates, 3 previews and a listening page, in `C:/Users/kazda/jam/audio-lab/sets/`. `READY.txt` is written.
- **Nobody has listened to any of this, and I cannot.** Every statement is a measurement. Whether the cannon sounds like a toy and not a gun, and whether the tumble is free of anything like a cry, is for the operator's ear at 10:00.
- **Credits: 437 of the 450 allowed**, by the response headers: 53 requests, all succeeded, none retried (45 first, 8 in a second round where the numbers said the first had failed). 127,128 credits are left on the account. No Unreal, no GPU, no git write; this report is the only file written in the repository.
- Four things to know before importing:
  1. **7 of the 15 defaults are assembled, not single generations** (`Slide_Tackle_a`, `Dash_Loop_a`, `Giant_Sweep_a`, `Giant_Poke_a`, `Mallet_Bonk_a`, `Cannon_Fire_a`, `Cannon_Kick_a`): six from two generations each, the loop from one stride placed twice. No single generation held the sequence asked for (a slide and then a clatter; a whoosh and then a clack; a thump with a knock; a shot with a low thoomp; a loop). Each alternate is a single generation (except the second loop, also built from strides), so the operator can choose by ear.
  2. **No generated stride cycle loops.** The two loops are built from the single stride: a push every 0.2 s in a file of exactly 0.400 s, the take's own cycle.
  3. **Timing is given per file** (section 3): the time of the main transient, and in section 6 the take frame each sound should fire on. Three need an offset or a hold: the mallet swing (fire 4 frames late), the wipeout (fire at frame 13), the skid (0.93 s against a 0.33 s take: loop its body while the take holds).
  4. **What falls short by the numbers** is listed in section 5; the worst are the mallet swing (bright, not deep), the mallet bonk (a pitched woody knock) and the cannon (no rattle, a borrowed thump).

## 1. What was generated

- Tool and settings as lane AU10: `C:/Users/kazda/jam/eleven-tools/eleven.mjs sfx`, model `eleven_text_to_sound_v2`, `prompt_influence` 0.5, output `mp3_44100_192`; the three stride cycles with the service's loop flag. The runner stops before the ledger would pass 450 credits.
- **53 requests, 53 succeeded, 437 credits.** First round: three candidates for each of the 15 sounds (378 credits). Second round (59 credits): two clatters and one swish as layers, two more mallet swings, two more mallet bonks, one more cannon shot.
- Ledger: `C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl`; raw mp3 files in `sets/raw/`. The key was read inside `eleven.mjs` and sent only in the request header; never printed, logged or copied.
- The prompts are generic sound descriptions: nothing of the project, the operator or any name. The cannon prompts describe a spring toy and say "no gunshot, no explosion"; the tumble prompts say "no voice".
- Read first: `status/SET-dive-report.md`, `status/SET-dash-report.md` and `status/SET-weapons-report.md` (it existed by then; its frame numbers are used here), all at 60 frames a second.

Every prompt is this prefix plus the text in the list: "Close, dry sound effect recorded in an indoor ice rink, isolated, no reverb tail, no crowd, no voices, no music, no whistle."

- DiveBlock_1 (0.9 s asked; Dive_Block_a): A padded hockey player throws himself flat onto the ice to block a shot: a short rustle of gear, a heavy flat slap-thud of the padded body landing, then a short scraping slide of pads on ice. One landing only.
- DiveBlock_2 (0.9 s asked; rejected): A body in thick hockey padding dives sideways and lands full length on hard ice with a dull flat thud, then slides a short way with a hiss of fabric and plastic on ice. Under one second.
- DiveBlock_3 (0.9 s asked; Dive_Block_b): A sprawling save on ice: padded legs and a padded body dropping flat with one muffled heavy thump and sliding for a third of a second, scraping. One landing only, no skating.
- DiveHit_1 (0.5 s asked; Dive_Block_Hit_a): A hard rubber hockey puck striking the padding of a player lying on the ice: one dull hard thud. One impact only, very short.
- DiveHit_2 (0.5 s asked; rejected): A fast puck hitting thick leg pads: a single flat dull smack with a little weight. One impact only, very short.
- DiveHit_3 (0.5 s asked; Dive_Block_Hit_b): A rubber puck thudding into a padded body at speed, a short dead thump with no ring. One impact only, very short.
- SlideTackle_1 (0.9 s asked; layer of Slide_Tackle_a, Slide_Tackle_b): A hockey player sliding feet first on the ice: a fast hiss of a hip and leg pad sliding with a stick sweeping along the ice, ending in a clatter of a stick against skate blades. Under one second.
- SlideTackle_2 (0.9 s asked; rejected): A fast sliding scrape of padded gear on ice with a wooden stick dragged in an arc, then a sharp clatter of stick and steel skates knocking together.
- SlideTackle_3 (0.9 s asked; rejected): A low slide on ice: a quick rasp of fabric and plastic for a fifth of a second, then wood and skate blades clattering together twice. No body impact.
- DashStart_1 (0.5 s asked; Dash_Start_b): Two quick hard digs of ice skate blades into the ice as a skater explodes forward: two sharp crunchy bites of steel on ice, very fast, a quarter of a second.
- DashStart_2 (0.5 s asked; rejected): A skater's explosive start: two rapid crunching digs of steel blades into hard ice, one right after the other. Very short.
- DashStart_3 (0.5 s asked; Dash_Start_a): Two fast gritty skate blade bites into ice in quick succession, each a short sharp crunch with a little ice spray. Very short, nothing else.
- DashCycle_1 (1 s asked, loop flag on; rejected): Very fast short ice skating strides, a sprinter on skates: quick even crunching pushes of steel blades on ice, left, right, left, right, about five pushes a second, steady rhythm, seamless loop.
- DashCycle_2 (1 s asked, loop flag on; rejected): Rapid choppy skating strides on ice at a steady quick tempo, short sharp blade scrapes alternating evenly, five per second, constant level.
- DashCycle_3 (1 s asked, loop flag on; rejected): A speed skater's rapid short strides: fast even rasping blade pushes on ice, tight steady rhythm, no glide between them.
- DashStride_1 (0.5 s asked; layer of Dash_Loop_b, Dash_Stride_b): One single fast ice skating stride: a short sharp rasping push of a steel blade on ice. One stride only, very short.
- DashStride_2 (0.5 s asked; rejected): A single quick skate push on ice, a short crisp scrape with a tiny crunch at the start. One only, very short.
- DashStride_3 (0.5 s asked; layer of Dash_Loop_a, layer of Dash_Loop_b, Dash_Stride_a): One short hard blade push on ice by a sprinting skater: a brief gritty scrape. Very short, one stride.
- DashSkid_1 (1.1 s asked; rejected): A long hard two-skate skid on ice from full speed: a deep throaty grinding scrape of both blades sideways, held steady for half a second, then a long hissing tail of sprayed ice settling. One skid only.
- DashSkid_2 (1.1 s asked; Dash_Skid_b): A very long hockey stop from a sprint: a loud low growling scrape of steel blades shaving ice, sustained, then fading into a spray of snow raining down.
- DashSkid_3 (1.1 s asked; Dash_Skid_a): Both skates thrown sideways into a long sliding skid: a heavy rough roaring scrape on ice that holds, lower and longer than a normal stop, ending with a soft hiss of ice spray.
- Wipeout_1 (1 s asked; Dash_Wipeout_a): A comic tumble on ice: a padded skater belly-flops with a flat slap, bounces once with a smaller thud, and slides to a stop with a scrape. Light and funny, no voice.
- Wipeout_2 (1 s asked; rejected): A padded body slapping flat onto ice, a quick second bump as it bounces, then a short hissing slide of pads. Slapstick, soft, within one second, no voice.
- Wipeout_3 (1 s asked; Dash_Wipeout_b): A pratfall on an ice rink: slap, bump, slide: a flat slap of pads on ice, a lighter bounce, and a short scrape as the body skids. No voice, no cry.
- BoostTake_1 (0.5 s asked; Boost_Take_a): A quick bright tick of a steel skate blade on ice together with one sharp clap of a padded hockey glove. Short, crisp, natural, no chime, no synthesizer.
- BoostTake_2 (0.5 s asked; Boost_Take_b): One crisp clap of two padded gloves with a tiny bright metallic tick of a blade at the same instant. Very short, natural sound only.
- BoostTake_3 (0.5 s asked; rejected): A short snappy glove slap and a light bright tick of steel on ice, like a player tapping in celebration. Very short, no bell, no jingle.
- GiantSweep_1 (0.8 s asked; Mallet_Swing_a): A very long wooden stick swept low and fast across ice: a broad whoosh for a third of a second, then a woody clack as it hits a rubber puck. One sweep only.
- GiantSweep_2 (0.8 s asked; rejected, layer of Cannon_Fire_a, layer of Cannon_Kick_a): A long heavy wooden pole swinging in a wide low arc with a deep airy whoosh and ending in a hollow wooden knock against a hard rubber puck.
- GiantSweep_3 (0.8 s asked; layer of Giant_Sweep_a, Giant_Sweep_b): An oversized hockey stick sweeping over the ice: a long swish with a light scrape, then one solid wooden clack on a puck. One sweep.
- GiantPoke_1 (0.5 s asked; rejected): A long wooden stick jabbed forward quickly: a short tight whoosh and a small wooden tick as its tip taps a puck. Very short.
- GiantPoke_2 (0.5 s asked; Giant_Poke_b): A quick jab with a long wooden pole: a brief swish of air, then one light dry wooden tick. Very short.
- GiantPoke_3 (0.5 s asked; layer of Giant_Poke_a, rejected): A fast poke with a long hockey stick along the ice: a short swish and a little woody tap on rubber. Very short.
- MalletSwing_1 (0.5 s asked; rejected): A big heavy soft object swung hard through the air: one deep low whoosh that rises and swells over a third of a second and then stops. No impact.
- MalletSwing_2 (0.5 s asked; layer of Giant_Poke_a, Mallet_Swing_b): A large padded carnival mallet swung overhead: a deep, slow, heavy whoosh of air building up. No impact, no hit.
- MalletSwing_3 (0.5 s asked; rejected): A heavy sandbag swung through the air in one swing, a low rising whoosh. No impact.
- MalletBonk_1 (0.6 s asked; rejected): A big felt-wrapped wooden mallet landing on thick padding: a deep soft round thump with a hollow wooden knock under it. Comic and round, one hit, no squeak, no squeaky toy.
- MalletBonk_2 (0.6 s asked; layer of Mallet_Bonk_a, rejected): A giant soft carnival mallet bonking a padded surface: a low pillowy thud with a woody tock inside it. One hit only, round and funny, no squeak.
- MalletBonk_3 (0.6 s asked; rejected): A heavy felt-covered hammer head dropping onto a thick cushion over wood: a deep muffled bump and a dull wooden knock. One hit only.
- CannonFire_1 (0.8 s asked; rejected): A spring-loaded toy launcher made of a stovepipe firing a rubber disc: a hollow metallic thoomp, a spring's twang, and a short tin rattle. Clearly a toy. No gunshot, no explosion, no firearm.
- CannonFire_2 (0.8 s asked; layer of Cannon_Fire_a, Cannon_Fire_b): A big coil spring snapping inside a hollow metal drainpipe and popping a puck out: a round hollow thump, a boingy metallic twang, a brief rattle of the pipe. A toy, no gunshot.
- CannonFire_3 (0.8 s asked; rejected): A homemade spring catapult in a tin tube going off: a dull hollow pop from the pipe, the spring ringing for a moment, loose metal rattling. No gunpowder, no bang, no explosion.
- CannonKick_1 (1 s asked; rejected): A spring-loaded stovepipe toy launcher firing with too much kick: a heavy hollow metallic thoomp and spring twang, then a clatter of the pipe and of padded gear as the person holding it sits down hard on ice. No gunshot, no explosion.
- CannonKick_2 (1 s asked; layer of Cannon_Kick_a, Cannon_Kick_b): A big spring toy cannon made of tin pipe going off with a deep hollow thump and a twang, followed by a tumble of metal pipe and skates clattering on ice. A toy, no firearm sound.
- CannonKick_3 (1 s asked; rejected): A hollow heavy thoomp from a spring-loaded metal tube, the spring rattling, then a bump and clatter as its carrier falls on his seat on the ice. No gunshot, no explosion.
- Clatter_1 (0.5 s asked; rejected, layer of Giant_Sweep_a): A wooden hockey stick clattering against steel ice skate blades: three quick sharp knocks of wood on metal, very short. Nothing else.
- Clatter_2 (0.5 s asked; layer of Slide_Tackle_a, rejected): A hockey stick and a pair of skates knocking together on ice: a short rattle of two or three woody and metallic clacks. Very short.
- Swish_1 (0.5 s asked; rejected): A thin long wooden stick thrust quickly through the air: one short tight swish. No impact, very short.
- MalletSwing_4 (0.8 s asked; rejected): A long deep whoosh of a very large heavy object swinging through the air, starting quiet and growing louder and lower over a third of a second, then cut off. Only air, no impact, no tone.
- MalletSwing_5 (0.8 s asked; rejected): A big heavy club swung in a wide arc past the microphone: a low airy whoosh that swells steadily and passes. Only wind noise, no impact, no hum.
- MalletBonk_4 (0.6 s asked; rejected): A wooden mallet wrapped in thick felt hitting a padded wooden box: one hollow woody bonk, mid-pitched and round, with a soft thump. Loud and clear. One hit only, no squeak.
- MalletBonk_5 (0.6 s asked; layer of Mallet_Bonk_a, Mallet_Bonk_b): A comic bonk made with real objects: a felt-covered wooden hammer knocking a padded hollow barrel once, a round woody knock with body. Loud. One hit only, no squeak, no synthesizer.
- CannonFire_4 (0.8 s asked; rejected): A toy spring launcher made from a wide metal stovepipe: one loud hollow low thoomp like a palm slapping the open end of a big pipe, then the spring twangs and the tin rattles briefly. No gunshot, no explosion.

## 2. Processing

Scripts: `gen.mjs`, `proc.mjs`, `picks.mjs`, `select.mjs`, `report.mjs` in `C:/Users/kazda/jam/audio-lab/sets/scripts/` (copies of the scratchpad's `au13/`); they use lane AU4's `lib.mjs`.

- Mono fold, 48 kHz, 16-bit, one linear gain per file, no limiter or equaliser; no file reaches full scale.
- **Peaks as AU10:** -3 dBTP, and -9 dBTP for the stride and the stride loops (movement sounds, as the stumble).
- **Start, by kind:** impacts (block hit, bonk, cannon, boost take) by AU4's rule (1.5 ms before the transient). Sounds with a lead-in (dive block, slide tackle, sweep, poke) keep at most the wanted time before the main transient (0.15, 0.17, 0.20 and 0.133 s). The others start a few ms before the first frame within 12 to 25 dB of the loudest.
- **End:** 30 ms after the last frame within 45 dB of the loudest, held inside the length asked; fade-out 40 to 150 ms.
- **Assembled takes:** each layer is a cut of a candidate's trimmed take, set to the same peak, lowered by the stated amount and placed at the stated time; then one gain for the file. The layers are named in section 4 and in `manifest.json`.
- **Loops:** a generated cycle is cut to a whole number of its own push periods with the last 50 ms cross-faded into what precedes its start (lane AU2's method). An assembled loop is two strides in exactly 0.400 s, with whatever runs past the end folded back onto the start.
- **Loudness:** EBU R128 integrated with one second of silence before and after (a loop: four passes end to end, no silence). `Hit_a` measures -22 LUFS at -3 dBTP by the same method.
- **Main transient:** the start, if the file begins within 6 dB of its loudest; else the last sharp rise (8 dB inside 20 ms) in the 120 ms before the loudest frame; else the loudest frame itself (a swell).

## 3. The table

`C:/Users/kazda/jam/audio-lab/sets/`; mono 48 kHz 16-bit WAV. "Gain to `Hit_a` level" is the linear factor that brings the file to the body check's loudness (lane AU11 multiplied each level by it).

| file | pick | made from | length s | peak dBTP | LUFS | LU against `Hit_a` | gain to `Hit_a` level | main transient ms | other events ms | loudest at ms | 40 dB down after ms |
|---|---|---|---|---|---|---|---|---|---|---|---|
| `Dive_Block_a.wav` | **default** | DiveBlock_1 | 0.615 | -3 | -22 | 0.0 | x 1 | 150 | - | 255 | 250 |
| `Dive_Block_b.wav` | alternate | DiveBlock_3 | 0.7 | -3 | -27.4 | -5.4 | x 1.862 | 3 | 60, 155, 205, 275 | 0 | 590 |
| `Dive_Block_Hit_a.wav` | **default** | DiveHit_1 | 0.25 | -3 | -28.6 | -6.6 | x 2.138 | 3 | - | 0 | 165 |
| `Dive_Block_Hit_b.wav` | alternate | DiveHit_3 | 0.295 | -3 | -18.5 | +3.5 | x 0.668 | 2 | - | 10 | 225 |
| `Slide_Tackle_a.wav` | **default** | assembled: SlideTackle_1 (the slide, 5 dB under) + Clatter_2 from 0.167 s | 0.575 | -3 | -22.3 | -0.3 | x 1.035 | 170 | - | 175 | 320 |
| `Slide_Tackle_b.wav` | alternate | SlideTackle_1 | 0.575 | -3 | -16.1 | +5.9 | x 0.507 | 3 | - | 170 | 340 |
| `Dash_Start_a.wav` | **default** | DashStart_3 | 0.3 | -3 | -21.1 | +0.9 | x 0.902 | 0 | 135 | 60 | 205 |
| `Dash_Start_b.wav` | alternate | DashStart_1 | 0.25 | -3 | -18.3 | +3.7 | x 0.653 | 4 | - | 45 | 150 |
| `Dash_Loop_a.wav` | **default** | assembled: DashStride_3 at 0 ms and again at 200 ms (1.5 dB under), 0.400 s loop | 0.4 | -9 | -22.7 | -0.7 | x 1.084 | pushes at 0 and 200 | - | - | - |
| `Dash_Loop_b.wav` | alternate | assembled: DashStride_3 at 0 ms + DashStride_1 at 200 ms, 0.400 s loop | 0.4 | -9 | -21.1 | +0.9 | x 0.902 | pushes at 0 and 200 | - | - | - |
| `Dash_Stride_a.wav` | **default** | DashStride_3 | 0.3 | -9 | -26 | -4.0 | x 1.585 | 4 | - | 100 | 170 |
| `Dash_Stride_b.wav` | alternate | DashStride_1 | 0.3 | -9 | -21.6 | +0.4 | x 0.955 | 1 | - | 20 | 135 |
| `Dash_Skid_a.wav` | **default** | DashSkid_3 | 0.925 | -3 | -12.5 | +9.5 | x 0.335 | 335 | - | 335 | 470 |
| `Dash_Skid_b.wav` | alternate | DashSkid_2 | 0.95 | -3 | -17.7 | +4.3 | x 0.61 | 255 | - | 255 | 675 |
| `Dash_Wipeout_a.wav` | **default** | Wipeout_1 | 0.735 | -3 | -21.2 | +0.8 | x 0.912 | 1 | 295, 350 | 0 | 575 |
| `Dash_Wipeout_b.wav` | alternate | Wipeout_3 | 0.7 | -3 | -24.7 | -2.7 | x 1.365 | 2 | - | 0 | 225 |
| `Boost_Take_a.wav` | **default** | BoostTake_1 | 0.3 | -3 | -16.6 | +5.4 | x 0.537 | 0 | 170 | 0 | 205 |
| `Boost_Take_b.wav` | alternate | BoostTake_2 | 0.3 | -3 | -20.3 | +1.7 | x 0.822 | 6 | - | 30 | 220 |
| `Giant_Sweep_a.wav` | **default** | assembled: GiantSweep_3 (the whoosh, 4 dB under) + Clatter_1 (one woody knock) at 0.20 s | 0.5 | -3 | -20.2 | +1.8 | x 0.813 | 190 | - | 200 | 245 |
| `Giant_Sweep_b.wav` | alternate | GiantSweep_3 | 0.5 | -3 | -16.3 | +5.7 | x 0.519 | 155 | - | 155 | 320 |
| `Giant_Poke_a.wav` | **default** | assembled: the first 0.125 s of MalletSwing_2 (8 dB under) + the first knock of GiantPoke_3, placed so that it peaks at 0.133 s | 0.288 | -3 | -19.6 | +2.4 | x 0.759 | 120 | 60 | 135 | 150 |
| `Giant_Poke_b.wav` | alternate | GiantPoke_2 | 0.3 | -3 | -21.8 | +0.2 | x 0.977 | 20 | - | 50 | 90 |
| `Mallet_Swing_a.wav` | **default** | GiantSweep_1 | 0.41 | -3 | -17.8 | +4.2 | x 0.617 | 185 | - | 185 | 155 |
| `Mallet_Swing_b.wav` | alternate | MalletSwing_2 | 0.3 | -3 | -17.6 | +4.4 | x 0.603 | 60 | - | 65 | 155 |
| `Mallet_Bonk_a.wav` | **default** | assembled: MalletBonk_2 (the deep thump) + MalletBonk_5 (the woody knock, 6 dB under), both at 0 s | 0.4 | -3 | -22.6 | -0.6 | x 1.072 | 0 | - | 0 | 355 |
| `Mallet_Bonk_b.wav` | alternate | MalletBonk_5 | 0.4 | -3 | -17.8 | +4.2 | x 0.617 | 0 | - | 0 | 275 |
| `Cannon_Fire_a.wav` | **default** | assembled: CannonFire_2 + GiantSweep_2 from its thump on (a hollow thump with a tone at 246 Hz, 8 dB under in peak), both at 0 s | 0.5 | -3 | -23.1 | -1.1 | x 1.135 | 0 | - | 75 | 345 |
| `Cannon_Fire_b.wav` | alternate | CannonFire_2 | 0.5 | -3 | -25 | -3.0 | x 1.413 | 1 | - | 0 | 440 |
| `Cannon_Kick_a.wav` | **default** | assembled: CannonKick_2 + GiantSweep_2 from its thump on (4 dB under in peak), both at 0 s | 0.8 | -3 | -18.6 | +3.4 | x 0.676 | 1 | - | 75 | 345 |
| `Cannon_Kick_b.wav` | alternate | CannonKick_2 | 0.8 | -3 | -17.4 | +4.6 | x 0.589 | 0 | 330 | 150 | 310 |

Loops (kept and rejected):

| file | length s | pushes | level step at the wrap dB | sample step at the wrap (x the mean step) | last sample dBFS |
|---|---|---|---|---|---|
| `Dash_Loop_a.wav` | 0.4 | 2, exactly 200 ms apart | 2.4 | 0.05 | -48 |
| `Dash_Loop_b.wav` | 0.4 | 2, exactly 200 ms apart | 32.8 | 0.01 | -56 |
| `rejected/DashCycle_1.wav` | 1.274 | 7, every 182 ms by autocorrelation (regularity 0.14) | -36.7 | 0 | - |
| `rejected/DashCycle_2.wav` | 1.41 | 15, every 94 ms by autocorrelation (regularity 0.13) | -26.7 | 0.46 | - |
| `rejected/DashCycle_3.wav` | 1.308 | 6, every 218 ms by autocorrelation (regularity 0.14) | 6.5 | 1.03 | - |

The level step is the last 50 ms against the first 50 ms. For `Dash_Loop_b` it is large because the loop ends in a gap between pushes, not because of a click: the sample step there is 0.01 of the file's mean step.

`Dash_Skid_a` holds within 4 dB of its loudest from 35 to 680 ms (level step between the ends 0.4 dB); `Dash_Skid_b` from 135 to 340 ms.

Sounds the game has already, measured the same way:

| existing sound | length s | LUFS | peak dBTP | centroid Hz |
|---|---|---|---|---|
| `tackle/Hit_a.wav`: lane AU4, the body check; the level reference | 0.3 | -22 | -3 | 286 |
| `tackle/Land_a.wav`: lane AU4, a body landing on the ice | 0.315 | -21.6 | -3 | 333 |
| `tackle/Slide_a.wav`: lane AU4, a body sliding | 0.835 | -25.3 | -14 | 2280 |
| `verbs/Brake_a.wav`: lane AU10, the hard stop | 0.7 | -14.1 | -3 | 2180 |
| `verbs/Ability_Shotgun_a.wav`: lane AU10 | 0.8 | -21.7 | -3 | 800 |
| `ready/SFX_SkateStrideLoop_01.wav`: the stride loop in MS_skate_loop | 3.85 | -26.3 | -10.1 | 1451 |

## 4. How each default was chosen

The rules and thresholds are mine and rest on these 53 takes only. "dB of the take" means the share of the take's energy in that band.

### The dive block and the slide tackle

**Dive_Block** (asked for: a padded body thrown flat onto the ice and sliding: the drop lands 0.15 s in, a slide of about 0.3 s after; 0.7 s). Pass: a landing with weight (energy under 400 Hz at least -10 dB of the take) and sound after it for 0.25 s or more. Order: a lead-in before the landing first (the file keeps at most 0.15 s before it).
- `Dive_Block_a`: default: a rustle for 0.15 s (18 to 35 dB under), the landing from 0.15 s swelling to its loudest at 0.26 s, with weight (under 120 Hz -4 dB of the take); then 0.2 s of quieter sound, 21 to 31 dB under; 0.62 s, ends on its own.
- `Dive_Block_b`: lands at once (no lead-in: fire it 0.15 s after the take starts), three more knocks at 0.06, 0.16 and 0.21 s, then a tail 30 to 40 dB under for 0.4 s; brighter (centroid 2.7 kHz); the raw was quiet (peak -6 dBFS).
- Rejected: `DiveBlock_2` (no landing: a bright hiss only (centroid 5.4 kHz, under 400 Hz -19 dB of the take), 0.4 s; it would serve as a slide layer).

**Dive_Block_Hit** (asked for: a puck striking pads on a lying body: one dull hard thud, 0.3 s). Pass: dull (centroid under 400 Hz) and hard (first sample within 12 dB of the peak inside 5 ms); one event. Order: the one that small speakers can carry first (more energy at 120 to 400 Hz).
- `Dive_Block_Hit_a`: default: one thud, centroid 259 Hz, most of it at 120 to 400 Hz (-3 dB of the take), lead-in 2.7 ms, 40 dB down after 0.17 s; 0.25 s.
- `Dive_Block_Hit_b`: deeper and heavier (centroid 133 Hz, under 120 Hz -2 dB), 40 dB down after 0.23 s; the loudest of the three at equal peak.
- Rejected: `DiveHit_2` (not dull: a click over a sub-bass thump (under 120 Hz -1 dB with as much over 1.5 kHz as between), centroid 646 Hz).

**Slide_Tackle** (asked for: a hip and leg sliding fast on ice with a stick sweeping, then a clatter of stick and skates at 0.17 s; 0.6 s). No generation holds a clatter: all three are a slide only (no sharp event after the start). A second round gave the clatter alone. Default: the slide whose swell peaks at 0.17 s with the two-knock clatter laid on it there. Alternate: that slide alone.
- `Slide_Tackle_a`: default, assembled: the SlideTackle_1 slide, 5 dB under, and the Clatter_2 knocks starting at 0.167 s (the take's contact frame 10).
- `Slide_Tackle_b`: the slide alone: a scrape that swells to its loudest at 0.17 s and falls away over 0.35 s (centroid 3.1 kHz); no clatter; the raw was very wide stereo (correlation 0.13).
- Rejected: `SlideTackle_2` (a slide at full level from the first frame for 0.3 s, no swell and no clatter; the raw decodes to +0.5 dBFS); `SlideTackle_3` (a shorter slide (0.39 s), loudest at 0.12 s, no clatter); `Clatter_1` (second round, a clatter alone: one woody knock only (centroid 1.0 kHz, 40 dB down after 0.16 s); used as the clack of Giant_Sweep_a); `Clatter_2` (second round, a clatter alone: two knocks 0.11 s apart, the second 17 dB under; used as the clatter of Slide_Tackle_a).

### The speed boost (dash)

**Dash_Start** (asked for: blades digging in hard twice, fast; 0.25 s). Pass: two events. One of three passes.
- `Dash_Start_a`: default: the only take with two events, at 0 and 0.135 s (the second 8 dB louder), three bursts in all; the take's crouch is at frame 3 and its drive at frame 6 to 10, so the second dig lands on the drive if the sound fires at frame 0.
- `Dash_Start_b`: one dig only: a single crunch, loudest at 0.05 s, 40 dB down by 0.2 s; 0.25 s; fails the test, kept as the cleaner single event.
- Rejected: `DashStart_2` (one burst of bright hiss (centroid 5.0 kHz), flagged tone-ish (1066 Hz)).

**Dash_Loop (the 0.4 s cycle)** (asked for: quick short strides at twice a sprint's rate for a 0.4 s cycle that loops). Pass for a loop: a measurable push rhythm (regularity 0.3 or more; plain noise reads 0.07 to 0.18) and a seam level step under 2 dB. None of the three generated cycles passes (regularity 0.13 to 0.14, the level swinging 32 to 73 dB inside the file, seam steps 6.5 to 37 dB). Both kept loops are built from the single strides: a push at 0 ms and one at 200 ms in a file of exactly 0.400 s, which is the take's own cycle (pushes at frames 0 and 12 of 24). Order: the smaller level step at the wrap first.
- `Dash_Loop_a`: default, assembled: the same stride twice, 0.2 s apart, the second 1.5 dB under; what runs past 0.4 s is folded back onto the start; the level at the wrap steps 2.4 dB, about what it does between the two pushes inside the loop, and the sample step there is 0.05 of the mean step (no click); it is the most even and the most likely to sound mechanical.
- `Dash_Loop_b`: assembled: two different strides for the two skates, at the same peak; the second is shorter (0.1 s against 0.18 s), so the loop ends in 0.1 s of near-silence before it wraps (level step at the wrap 33 dB, sample step 0.01: no click, but an uneven gait).
- Rejected: `DashCycle_1` (not a loop: silent for 0.3 s, then one swell; regularity 0.14, seam step 37 dB); `DashCycle_2` (not a loop: bursts with gaps of 0.3 s; regularity 0.13, seam step 27 dB); `DashCycle_3` (not a loop: two swells 0.3 s apart; regularity 0.14, seam step 6.5 dB).

**Dash_Stride (a single stride to retrigger)** (asked for: a single fast stride the engine can retrigger). Pass: one event, over inside 0.3 s. Order: no tone flag first. Level: -9 dBTP, as the stumble.
- `Dash_Stride_a`: default: one push, swelling for 0.1 s and 20 dB down 0.13 s later; no tone flag; centroid 2.0 kHz; the sound ran 0.385 s and is faded at 0.3 s.
- `Dash_Stride_b`: a sharper rasp: full level at once for 0.1 s, 40 dB down by 0.14 s; flagged tone-ish (2549 Hz, prominence 28 dB): a squeal of the blade may stand out.
- Rejected: `DashStride_2` (three scrapes (events at 15, 70 and 130 ms), not one stride).

**Dash_Skid** (asked for: a long two-skate skid, longer and throatier than the hard stop, with a spray tail; 0.9 s with a body the engine can hold). Pass: lower than the hard stop Brake_a (centroid under 2180 Hz); a body that holds (0.2 s or more within 4 dB of the loudest); an end inside the file (20 dB down or more before the cut). Order: the longest body first.
- `Dash_Skid_a`: default: the longest body: 0.65 s within 4 dB of the loudest (35 to 680 ms, level step between its ends 0.4 dB), then 40 dB down in 0.2 s; the lowest (centroid 1410 Hz against 2180 Hz for Brake_a); 0.93 s, ends on its own; the raw decodes to +0.8 dBFS, so the service may have limited it.
- `Dash_Skid_b`: a rougher skid with low rumble (energy spread from under 120 Hz to over 5 kHz), a body of 0.2 s (135 to 340 ms), then a slow fall over 0.4 s with a tail that gets brighter (1091 to 1764 Hz); flagged tone-ish (1113 Hz); 27 dB down at the 0.95 s cut.
- Rejected: `DashSkid_1` (does not end: only 8 dB down at the 0.95 s cut (faded there); otherwise an even scrape, centroid 2.0 kHz).

**Dash_Wipeout** (asked for: a tumble: a slap of the body, a bounce, a slide; 0.8 s; comic, not hurt). Pass: two impacts and sound after the second. One of three passes.
- `Dash_Wipeout_a`: default: a slap at 0 s, 40 dB down by 0.24 s; a second impact at 0.30 s, as loud as the first, and a smaller one at 0.35 s; then 0.35 s of tail, 20 to 50 dB under; 0.74 s, ends on its own.
- `Dash_Wipeout_b`: one flat slap only (centroid 2.2 kHz), 40 dB down after 0.23 s, silence after; fails the test, kept as a plain slap for a shorter tumble.
- Rejected: `Wipeout_2` (one deep thump only (centroid 388 Hz, under 120 Hz -1 dB), no bounce, no slide).

**Boost_Take** (asked for: a short bright tick of a blade and a glove clap, not a jingle, no synthesiser; 0.3 s). Pass: bright (centroid over 2 kHz), no weight under 400 Hz, a raw level that needs under 10 dB of gain. Order: a clap body (0.4 to 5 kHz) first, then the tick alone.
- `Boost_Take_a`: default: the clap: full level at once, 0.1 s of body at 0.4 to 5 kHz (centroid 2.3 kHz), 20 dB down after 0.19 s; a small second event at 0.17 s, 16 dB under; cut at 0.3 s (60 dB down).
- `Boost_Take_b`: the tick: almost all over 1.5 kHz (centroid 6.0 kHz), loudest at 30 ms, 40 dB down after 0.22 s; the raw was very wide stereo (correlation 0.13); the two can be layered where the sound is played.
- Rejected: `BoostTake_3` (a sub-bass bump (under 120 Hz -1 dB), raw peak -14 dBFS).

### The toy weapons

**Giant_Sweep** (asked for: a long wooden stick whooshing low over ice and clacking a puck; 0.5 s). No generation shows a clack after its whoosh (no sharp event). Default: the whoosh with the single woody knock of the second round laid on it at 0.20 s (the take's contact window is frames 10 to 14, 0.17 to 0.23 s). Alternate: the whoosh alone. GiantSweep_1, a whoosh that rises and stops, went to Mallet_Swing.
- `Giant_Sweep_a`: default, assembled: the GiantSweep_3 whoosh, 4 dB under, and the Clatter_1 knock (centroid 1.0 kHz) at 0.20 s.
- `Giant_Sweep_b`: the whoosh alone: swells to its loudest at 0.16 s and falls over 0.3 s, getting darker (2479 to 669 Hz) with its low end strongest at 0.27 s; no sharp clack; the sound ran 0.67 s and is faded at 0.5 s (32 dB down).
- Rejected: `GiantSweep_2` (no whoosh: a hollow low thump (centroid 119 Hz, a tone at 246 Hz); used as the thoomp of the two cannon defaults).

**Giant_Poke** (asked for: a long jab: a short whoosh and a wooden tick; 0.35 s). No generation has a whoosh before its knock. Default: a short swell of air (the first 0.12 s of the rising whoosh MalletSwing_2) with the first knock of GiantPoke_3 placed on the take's contact frame 8 (0.133 s). Alternate: a single knock, to fire on contact.
- `Giant_Poke_a`: default, assembled: 0.125 s of the MalletSwing_2 swell, 8 dB under, then the first knock of GiantPoke_3 (woody, 0.4 to 1.5 kHz), its peak at 0.133 s.
- `Giant_Poke_b`: one knock, no whoosh: onset 20 ms in, lower and rounder (centroid 361 Hz), gone in 0.1 s; 0.3 s file; fire it 0.11 s after the use so it lands on the contact frame.
- Rejected: `GiantPoke_1` (a sub-bass thump (centroid 142 Hz, under 120 Hz -0.3 dB)); `GiantPoke_3` (two knocks, at 55 and 215 ms, and a raw with left and right out of phase (correlation -0.05); its first knock is the tick of Giant_Poke_a); `Swish_1` (second round, a swish alone: came back as a sub-bass bump (centroid 173 Hz), raw out of phase (correlation -0.51)).

**Mallet_Swing** (asked for: a heavy soft object swung through air, a deep whoosh rising over 0.3 s). Pass: a whoosh (no tonal flag, not sub-bass only: centroid over 150 Hz) that rises (loudest point later than a third of its length). None of the five mallet generations passes: they are short thumps, hums or sub-bass. Default: the one take of the night that rises and stops, generated for the giant sweep.
- `Mallet_Swing_a`: default: a whoosh that rises for 0.19 s to its loudest and is 40 dB down 0.16 s later; 0.41 s; centroid 2.8 kHz; fire it 0.06 s after the use so the loudest point meets the contact frame 15.
- `Mallet_Swing_b`: a short swell at 0.4 to 1.5 kHz (centroid 602 Hz), loudest at 0.07 s, 40 dB down by 0.22 s; flagged tone-ish (574 Hz); fails the rise test.
- Rejected: `MalletSwing_1` (a hum, not a whoosh: tonal (ratio 0.86 at 229 Hz)); `MalletSwing_3` (a low thump at 0 ms, then a decay: nothing rises); `MalletSwing_4` (second round: sub-bass only (centroid 52 Hz)); `MalletSwing_5` (second round: sub-bass only (centroid 45 Hz)).

**Mallet_Bonk** (asked for: a big felt-wrapped mallet landing on padding: a deep soft thump with a wooden knock under it, comic and round, no squeak toy; 0.4 s). Four of five generations are sub-bass only (centroid 65 to 79 Hz) and nearly silent in the raw (-20 to -39 dBFS); the fifth is a woody knock with no thump. Default: the two laid together. Alternate: the knock alone.
- `Mallet_Bonk_a`: default, assembled: the MalletBonk_2 thump (centroid 66 Hz) with the MalletBonk_5 knock 6 dB under it, starting together.
- `Mallet_Bonk_b`: the knock alone: one round woody tock, almost all at 0.4 to 1.5 kHz, 40 dB down after 0.28 s; tonal (1113 Hz); loud in the raw.
- Rejected: `MalletBonk_1` (sub-bass only (centroid 68 Hz), raw peak -33 dBFS); `MalletBonk_2` (sub-bass only (centroid 66 Hz), raw peak -20 dBFS; the thump of Mallet_Bonk_a); `MalletBonk_3` (sub-bass only (centroid 65 Hz), raw peak -30 dBFS); `MalletBonk_4` (second round: sub-bass only and nearly silent (raw peak -39 dBFS)).

**Cannon_Fire** (asked for: a spring-loaded stovepipe launcher: a hollow metallic thoomp with a spring's twang and a short rattle, clearly a toy, no gunshot, no explosion; 0.5 s). No generation of four has a low thoomp (energy under 400 Hz is 13 dB or more under the take in all). Default: the take with the ringing twang, with a hollow low thump of the night laid under it. Alternate: the twang take alone.
- `Cannon_Fire_a`: default, assembled: the CannonFire_2 hit with its ring at 820 Hz, and under it the hollow low thump of GiantSweep_2 (centroid 119 Hz), 8 dB lower in peak, starting together; the thump swells for 0.08 s, so it follows the hit.
- `Cannon_Fire_b`: one hit at 0.4 to 1.5 kHz with a ring at 820 Hz (prominence 38 dB), which is the spring by the numbers; 40 dB down after 0.44 s; no low end; raw peak -8 dBFS; cut at 0.5 s (44 dB down).
- Rejected: `CannonFire_1` (a bright rattle with no body (centroid 2.4 kHz, under 400 Hz -21 dB), wide raw (correlation 0.34)); `CannonFire_3` (three bursts, at 0, 0.05 and 0.12 s, the second the loudest: a rattle with no single shot); `CannonFire_4` (second round: a bright click (centroid 3.6 kHz), 40 dB down after 0.18 s, no thoomp).

**Cannon_Kick** (asked for: the same thoomp, heavier, with a clatter as the carrier sits down; 0.8 s). Pass: more than one event (the shot, then the clatter). One of three passes, and it has no low end. Default: that take over the same hollow thump as Cannon_Fire_a. Alternate: the take alone.
- `Cannon_Kick_a`: default, assembled: CannonKick_2 (metallic hits at 0, 0.15 and 0.33 s) over the same hollow thump as Cannon_Fire_a, here 4 dB under in peak where the fire has it 8 dB under; the take sits down at frames 15 to 17, 0.12 to 0.15 s after the shot, and lies from frame 24.
- `Cannon_Kick_b`: the take alone: bright metallic hits at 0, 0.15 and 0.33 s (centroid 3.0 kHz), 40 dB down by 0.46 s; flagged tone-ish (2531 Hz); no low end; the sound ran 1.05 s and is cut at 0.8 s (50 dB down).
- Rejected: `CannonKick_1` (one hit only, 40 dB down after 0.27 s; no clatter); `CannonKick_3` (one hit, then a noise floor 30 dB under for 0.7 s; no separate clatter).

## 5. What falls short of its description, by the numbers

- **Dive_Block:** The default lands as a swell over 0.1 s, not as a slap; its slide is short (0.2 s) and 21 to 31 dB under the landing.
- **Slide_Tackle:** No single generation has the clatter; the default is assembled from two. The clatter is two knocks 0.11 s apart, the second 17 dB under the first: thin for "stick and skates".
- **Dash_Start:** The two digs of the default are 0.135 s apart and the second is the louder; the file is 0.30 s, cut 45 ms short of its natural end (45 dB down there).
- **Dash_Loop (the 0.4 s cycle):** No generated cycle loops. The loops are assembled from the single strides, so "twice a sprint's rate" is exact (a push every 0.2 s) but the strides are one or two takes repeated.
- **Dash_Skid:** The default's tail gets darker, not brighter (centroid 1281 Hz at the start, 896 Hz at the end): a spray of ice does not show in it.
- **Dash_Wipeout:** In the default the bounce comes 0.30 s after the slap and is as loud as the slap; the take's belly and bounce are 0.05 s apart (frames 13 and 16). No voice shows in the numbers, but a cry cannot be ruled out without an ear.
- **Boost_Take:** No take measurably holds both a tick and a clap. The tonality test is weak on files this short (it flags both kept takes on persistence, from two or three frames).
- **Giant_Sweep:** No single generation has the clack; the default is assembled from two.
- **Giant_Poke:** No single generation has the whoosh; the default is assembled from two. The whoosh is borrowed from a mallet take and is 0.4 to 1.5 kHz, not airy.
- **Mallet_Swing:** No deep rising whoosh was obtained in five tries. The default rises over 0.19 s but is bright (centroid 2.8 kHz), not deep; pitching it down where it is played is the cheap fix. The default's raw has left and right out of phase (correlation -0.32): the mono fold cancelled part of it.
- **Mallet_Bonk:** No single generation has both the thump and the knock; the default is assembled from two. The knock is strongly pitched (a tone at 1113 Hz, prominence 44 dB): by the numbers it is closer to a wood block than to felt on padding. The thump is under 120 Hz: small speakers will carry only the knock.
- **Cannon_Fire:** No single generation has the thoomp; the default is assembled from two, and its thump was generated as a wooden knock, not as a pipe. No rattle shows in the default (one event, then a ring).
- **Cannon_Kick:** Assembled, as Cannon_Fire_a. "Heavier" is only the same thump 4 dB louder.

Also: the tonality test flags several low thuds as tonal (a thud has a pitch); I used it only where a tone would be a fault (the boost take, the mallet swing, the stride).

## 6. For the import lane

Suggested MetaSounds, in lane AU11's pattern: gain = a level times the file's factor against `Hit_a`; a sound that is a hit on a body carries the hit factor 0.80 inside its gain (x 0.8 / 0.7, as AU11 did for the hip check) where its MetaSound's trim is `FxTrim` 0.70. The levels are my suggestions by kind (AU11 used 1.75 to 1.97 for the shotgun and the hip check, 0.5 to 0.9 for movement): the operator's mix rule decides.

| sound (default take) | MetaSound | inputs | when it fires | suggested gain (level x file factor) | trim |
|---|---|---|---|---|---|
| `Dive_Block_a` | `MS_dive_block` (new) | `Strength`, `Variant` (side, unused), `DiveBlock` | on `PlayDiveBlock`, at the take's frame 0: the landing is 0.15 s into the file (frame 9; full cover is frame 10) | 1.71 (1.5 x 1 x 0.8/0.7) | `FxTrim` 0.70, the hit factor 0.80 inside the gain |
| `Dive_Block_Hit_a` | `MS_dive_block_hit` (new) | `Strength`, `DiveBlockHit` | on `PlayDiveBlockHit` (the puck strikes the lying blocker), no delay: the thud is at 0 ms and the jolt peaks at frame 2 | 3.67 (1.5 x 2.138 x 0.8/0.7) | `FxTrim` 0.70, the hit factor 0.80 inside the gain |
| `Slide_Tackle_a` | `MS_slide_tackle` (new) | `Strength`, `SlideTackle` | on `PlaySlideTackle`, at frame 0: the clatter is at 0.17 s, the contact frame 10. The victim's `Trip` keeps the existing hit sounds | 1.42 (1.2 x 1.035 x 0.8/0.7) | `FxTrim` 0.70, the hit factor 0.80 inside the gain |
| `Dash_Start_a` | `MS_dash` (new), trigger `Start` | `Start`, `Loop` (bool), `Rate`, `EndKind` | on `PlayDash` / `SetBoost(true)`, at `Dash_Start` frame 0: digs at 0 and 0.135 s (the take drives from frame 6 and joins the loop at frame 10) | 0.72 (0.8 x 0.902) | `FxTrim` 0.70 |
| `Dash_Loop_a` | `MS_dash`, the looping layer | `Rate` = the loop take's play rate (0.9 to 1.3) as pitch and speed | from `Dash_Start` frame 10 (0.167 s) while the dash lasts; one file cycle = the take's 24 frames at rate 1; stop with a 50 ms fade on `EndDash` | 0.65 (0.6 x 1.084) | `FxTrim` 0.70 |
| `Dash_Stride_a` | `MS_dash`, in place of the loop if retriggered | - | on each push key of `Dash_Loop` (frames 0 and 12), if the engine would rather fire strides than loop a file | 0.95 (0.6 x 1.585) | `FxTrim` 0.70 |
| `Dash_Skid_a` | `MS_dash_skid` (new) | `Hold` (bool), `DashSkid` | on `EndDash` reason 1, at `Dash_Skid` frame 2 (the spray starts). The file is 0.93 s and the take only 0.33 s unless held: loop the body (100 to 600 ms of the file, level step 0.4 dB) while the take holds frames 6 to 10, then jump to 680 ms for the 0.25 s tail; with no hold, start the tail at frame 10 | 0.27 (0.8 x 0.335) | `FxTrim` 0.70 |
| `Dash_Wipeout_a` | `MS_dash_wipeout` (new) | `Strength`, `DashWipeout` | on `EndDash` reason 3, at the take's frame 13 (belly on the ice, 0.217 s): the slap is at 0 ms. The file's bounce comes 0.30 s later; the take's is at frame 16 | 1.56 (1.5 x 0.912 x 0.8/0.7) | `FxTrim` 0.70, the hit factor 0.80 inside the gain |
| `Boost_Take_a` | `MS_boost_take` (new), or a layer in `MS_pickup_take` | `BoostTake` | on `PlayBoostPickup`, at frame 0 (the fist is up at frame 5); the existing pickup chime plays on the same event today | 0.32 (0.6 x 0.537) | `FxTrim` 0.70 |
| `Giant_Sweep_a` | `MS_weapon` (new), `Variant` 1, action sweep | `Strength`, `Variant` (weapon kind), `Action` | on `PlayWeapon(1)` aimed to a side, at `Giant_Sweep` frame 0: the clack is at 0.20 s, inside the contact window (frames 10 to 14) | 0.98 (1.2 x 0.813) | `FxTrim` 0.70 |
| `Giant_Poke_a` | `MS_weapon`, `Variant` 1, action poke | as above | on `PlayWeapon(1)` aimed ahead, at `Giant_Poke` frame 0: the tick peaks at 0.133 s, the contact frame 8 | 0.76 (1 x 0.759) | `FxTrim` 0.70 |
| `Mallet_Swing_a` | `MS_weapon`, `Variant` 2, action swing | as above | on `PlayWeapon(2)`, 4 frames (0.06 s) after `Mallet_Swing` frame 0, so that its loudest point (0.19 s in) meets the contact frame 15; also for `Mallet_Miss` | 0.49 (0.8 x 0.617) | `FxTrim` 0.70 |
| `Mallet_Bonk_a` | `MS_weapon`, `Variant` 2, action hit | as above | on the mallet's contact (frame 15 of the swing, when the victim gets `SetKnockedDown` with cause mallet): the bonk is at 0 ms | 1.84 (1.5 x 1.072 x 0.8/0.7) | `FxTrim` 0.70, the hit factor 0.80 inside the gain |
| `Cannon_Fire_a` | `MS_weapon`, `Variant` 3, action fire | as above | at `Cannon_Fire` frame 8, the frame the puck leaves: the shot is at 0 ms | 2.16 (1.9 x 1.135) | `FxTrim` 0.70 |
| `Cannon_Kick_a` | `MS_weapon`, `Variant` 3, action kick | as above | at `Cannon_Kick` frame 8 in place of the fire sound: the shot at 0 ms, metal hits at 0.15 and 0.33 s (the take sits down at frames 15 to 17 and lies from 24) | 1.47 (1.9 x 0.676 x 0.8/0.7) | `FxTrim` 0.70, the hit factor 0.80 inside the gain |

- **One event per sound is needed from the other device:** `PlayDiveBlock`, `PlayDiveBlockHit`, `PlaySlideTackle`, `PlayDash` / `SetBoost` / `EndDash(Reason)`, `PlayBoostPickup`, `PlayWeapon(Kind, ..., bHit)`, as the three set reports ask. Until they exist, the test trigger the operator asked for can fire each MetaSound with its take.
- **Dash loop:** it is a 0.400 s file to loop; tie its pitch and speed to the take's play rate. The alternative without a new wave is the existing `SFX_SkateStrideLoop_01` pitched up.
- **Mallet swing:** the default is bright (centroid 2.8 kHz); a pitch shift of -5 to -7 semitones in the MetaSound is the cheap way to make it deeper (it also lengthens it by a third to a half, so fire it at frame 0 then).
- **Boost take:** `Boost_Take_a` is the clap and `_b` the tick; layering both at 0 ms gives what the brief describes.
- **Large factors:** the files peak at -3 dBTP, so a gain times trim over 1.41 passes full scale at the peak. That is the case for `Dive_Block_Hit_a` (3.67 x 0.70 = 2.57) and `Cannon_Fire_a` (1.51): both are short, peaky sounds that the loudness factor over-raises. Cap them near 1.4 after the trim, or use `Dive_Block_Hit_b` (factor 0.668).
- **Variation:** every sound has two takes; where a sound fires often (the stride, the block hit) alternate them.
- Import settings as AU11 used for effects (mono, Bink quality 40, `SC_SFX`); `Dash_Loop_*` looping.

Ledger rows (proposed names `SFX_<file name>`; `shipped` is `y` for the defaults):

| path | kind | size_mb | origin | tool | licence | url | date | device | shipped | note |
|---|---|---|---|---|---|---|---|---|---|---|
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Dive_Block_a.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | y | dive block, a padded body landing flat and sliding; mono 48 kHz 0.615 s, -22 LUFS, -3 dBTP; from sets/Dive_Block_a.wav (59084 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Dive_Block_b.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | n | dive block, a padded body landing flat and sliding; mono 48 kHz 0.7 s, -27.4 LUFS, -3 dBTP; from sets/Dive_Block_b.wav (67244 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Dive_Block_Hit_a.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | y | a puck striking the lying blocker; mono 48 kHz 0.25 s, -28.6 LUFS, -3 dBTP; from sets/Dive_Block_Hit_a.wav (24044 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Dive_Block_Hit_b.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | n | a puck striking the lying blocker; mono 48 kHz 0.295 s, -18.5 LUFS, -3 dBTP; from sets/Dive_Block_Hit_b.wav (28364 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Slide_Tackle_a.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | y | slide tackle, slide and clatter; assembled from SlideTackle_1 and Clatter_2; mono 48 kHz 0.575 s, -22.3 LUFS, -3 dBTP; from sets/Slide_Tackle_a.wav (55244 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Slide_Tackle_b.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | n | slide tackle, slide and clatter; mono 48 kHz 0.575 s, -16.1 LUFS, -3 dBTP; from sets/Slide_Tackle_b.wav (55244 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Dash_Start_a.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | y | dash start, two blade digs; mono 48 kHz 0.3 s, -21.1 LUFS, -3 dBTP; from sets/Dash_Start_a.wav (28844 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Dash_Start_b.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | n | dash start, two blade digs; mono 48 kHz 0.25 s, -18.3 LUFS, -3 dBTP; from sets/Dash_Start_b.wav (24044 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Dash_Loop_a.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | y | dash stride loop, 0.4 s cycle; assembled from DashStride_3; mono 48 kHz 0.4 s, -22.7 LUFS, -9 dBTP, looping; from sets/Dash_Loop_a.wav (38444 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Dash_Loop_b.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | n | dash stride loop, 0.4 s cycle; assembled from DashStride_3 and DashStride_1; mono 48 kHz 0.4 s, -21.1 LUFS, -9 dBTP, looping; from sets/Dash_Loop_b.wav (38444 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Dash_Stride_a.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | y | a single dash stride; mono 48 kHz 0.3 s, -26 LUFS, -9 dBTP; from sets/Dash_Stride_a.wav (28844 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Dash_Stride_b.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | n | a single dash stride; mono 48 kHz 0.3 s, -21.6 LUFS, -9 dBTP; from sets/Dash_Stride_b.wav (28844 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Dash_Skid_a.uasset | audio | 0.1 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | y | dash skid, long two-skate skid; mono 48 kHz 0.925 s, -12.5 LUFS, -3 dBTP; from sets/Dash_Skid_a.wav (88844 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Dash_Skid_b.uasset | audio | 0.1 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | n | dash skid, long two-skate skid; mono 48 kHz 0.95 s, -17.7 LUFS, -3 dBTP; from sets/Dash_Skid_b.wav (91244 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Dash_Wipeout_a.uasset | audio | 0.1 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | y | dash wipeout, slap, bounce, slide; mono 48 kHz 0.735 s, -21.2 LUFS, -3 dBTP; from sets/Dash_Wipeout_a.wav (70604 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Dash_Wipeout_b.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | n | dash wipeout, slap, bounce, slide; mono 48 kHz 0.7 s, -24.7 LUFS, -3 dBTP; from sets/Dash_Wipeout_b.wav (67244 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Boost_Take_a.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | y | boost pickup, glove clap or blade tick; mono 48 kHz 0.3 s, -16.6 LUFS, -3 dBTP; from sets/Boost_Take_a.wav (28844 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Boost_Take_b.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | n | boost pickup, glove clap or blade tick; mono 48 kHz 0.3 s, -20.3 LUFS, -3 dBTP; from sets/Boost_Take_b.wav (28844 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Giant_Sweep_a.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | y | giant stick sweep, whoosh and clack; assembled from GiantSweep_3 and Clatter_1; mono 48 kHz 0.5 s, -20.2 LUFS, -3 dBTP; from sets/Giant_Sweep_a.wav (48044 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Giant_Sweep_b.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | n | giant stick sweep, whoosh and clack; mono 48 kHz 0.5 s, -16.3 LUFS, -3 dBTP; from sets/Giant_Sweep_b.wav (48044 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Giant_Poke_a.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | y | giant stick poke, whoosh and tick; assembled from MalletSwing_2 and GiantPoke_3; mono 48 kHz 0.288 s, -19.6 LUFS, -3 dBTP; from sets/Giant_Poke_a.wav (27692 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Giant_Poke_b.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | n | giant stick poke, whoosh and tick; mono 48 kHz 0.3 s, -21.8 LUFS, -3 dBTP; from sets/Giant_Poke_b.wav (28844 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Mallet_Swing_a.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | y | mallet swing, a rising whoosh; mono 48 kHz 0.41 s, -17.8 LUFS, -3 dBTP; from sets/Mallet_Swing_a.wav (39404 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Mallet_Swing_b.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | n | mallet swing, a rising whoosh; mono 48 kHz 0.3 s, -17.6 LUFS, -3 dBTP; from sets/Mallet_Swing_b.wav (28844 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Mallet_Bonk_a.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | y | mallet bonk, thump and woody knock; assembled from MalletBonk_2 and MalletBonk_5; mono 48 kHz 0.4 s, -22.6 LUFS, -3 dBTP; from sets/Mallet_Bonk_a.wav (38444 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Mallet_Bonk_b.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | n | mallet bonk, thump and woody knock; mono 48 kHz 0.4 s, -17.8 LUFS, -3 dBTP; from sets/Mallet_Bonk_b.wav (38444 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Cannon_Fire_a.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | y | toy puck cannon firing; assembled from GiantSweep_2 and CannonFire_2; mono 48 kHz 0.5 s, -23.1 LUFS, -3 dBTP; from sets/Cannon_Fire_a.wav (48044 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Cannon_Fire_b.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | n | toy puck cannon firing; mono 48 kHz 0.5 s, -25 LUFS, -3 dBTP; from sets/Cannon_Fire_b.wav (48044 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Cannon_Kick_a.uasset | audio | 0.1 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | y | toy puck cannon firing with the carrier sitting down; assembled from GiantSweep_2 and CannonKick_2; mono 48 kHz 0.8 s, -18.6 LUFS, -3 dBTP; from sets/Cannon_Kick_a.wav (76844 bytes WAV; set the uasset bytes after import) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Cannon_Kick_b.uasset | audio | 0.1 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | n | toy puck cannon firing with the carrier sitting down; mono 48 kHz 0.8 s, -17.4 LUFS, -3 dBTP; from sets/Cannon_Kick_b.wav (76844 bytes WAV; set the uasset bytes after import) |

## 7. Previews and page

Stereo, each with 0.20 s of silence first; mono sounds centred at -3 dB per channel; no gain changes. The times follow the takes' frames.

| file | timeline (time zero is 0.20 s into the file) | length s | LUFS | peak dBTP |
|---|---|---|---|---|
| `preview_dive.wav` | Dive_Block_a at 0.00 s; Dive_Block_Hit_a at 0.90 s; Slide_Tackle_a at 1.60 s | 2.475 | -23.1 | -6 |
| `preview_dash.wav` | Dash_Start_a at 0.00 s; Dash_Loop_a at 0.17 s; Dash_Loop_a at 0.57 s; Dash_Loop_a at 0.97 s; Dash_Skid_a at 1.37 s; Boost_Take_a at 2.60 s; Dash_Wipeout_a at 3.20 s | 4.235 | -16.3 | -6 |
| `preview_weapons.wav` | Giant_Poke_a at 0.00 s; Giant_Sweep_a at 0.80 s; Mallet_Swing_a at 1.66 s; Mallet_Bonk_a at 1.85 s; Cannon_Fire_a at 2.80 s; Cannon_Kick_a at 3.80 s | 4.9 | -19 | -1.1 |

In `preview_weapons.wav` the swing starts 0.06 s late and the bonk 0.25 s after the use, as section 6 proposes.

`C:/Users/kazda/jam/audio-lab/sets/pick.html`: the three previews, six existing sounds for comparison, then the three sets, a section per sound, a player per take (loops play looped) with its numbers, a loudness curve and the reason; the default pre-selected; "Copy picks". All 70 audio paths exist and the inline script passes `node --check`. **Not opened in a browser.** `manifest.json` holds every number, layer, prompt and credit.

## 8. What nobody has heard, and what I could not verify

- **All of it.** In particular: that the cannon is a toy and nothing like a firearm; that the wipeout and the dive hold no cry or voice; that the assembled takes sound like one event and not two pasted sounds; that the mallet is comic and not a wood block; that anything here suits clay toys in a timber barn.
- **The loops by ear:** a 0.4 s loop of one or two strides may sound mechanical.
- **Mono colour:** three sources had left and right out of phase in the raw (`GiantSweep_1`, now `Mallet_Swing_a`, correlation -0.32; `GiantPoke_3`, the tick of `Giant_Poke_a`, -0.05; `Swish_1`, rejected); the mono fold cancels part of such a take.
- **Limiting at the source:** several raws decode to or just over full scale (`DashSkid_3` +0.8 dBFS, `SlideTackle_1` +1.2).
- **Timing in the engine:** every frame number here is from the three set reports; nothing was tried against a take in motion.
- **The page in a browser.** The pick rules are calibrated on these takes only.

## 9. Decisions for the operator

1. **Assembled or single, sound by sound** (the page): the assembled defaults match the description by construction; the single takes are one recording each.
2. **The wipeout's rhythm:** the kept take has 0.30 s between slap and bounce, a slower, more comic beat than the take's 0.05 s. Keep it, or ask for a tighter one.
3. **The mallet:** accept the pitched woody knock as the "bonk", or spend more generations on a duller one; pitch the swing down or not.
4. **The cannon's tone:** it is a metallic hit with a ring over a hollow thump. If it reads as a gun at all, it goes.
5. **Boost take against the existing pickup chime:** replace, layer, or keep the chime only.
6. **The dash:** a looped file, retriggered strides, or the existing stride loop pitched up.

## 10. The next three evolutions

1. **Timing proved in the engine.** Fire each of the 15 sounds with its take from the test trigger, record the output, and measure the main transient against the take's key frame. The number: 15 of 15 within one frame (17 ms) of the frame named in section 6 (today: 0 measured).
2. **Single takes in place of assemblies, and a deep swing.** A third round aimed only at what failed. The numbers: defaults that are one generation and pass their own rule, from 8 of 15 to 12 of 15; a mallet swing that rises for 0.25 s or more with its centroid under 800 Hz (today 0.19 s and 2841 Hz); a cannon shot with three bursts after a thump under 400 Hz in one take (today none).
3. **The sets inside the mix rule.** A recorded mode match with the weapons and the dash in use. The numbers: the effects bus at least 6 LU under the commentator and under crowd plus music over the match; and in 20 triggers of one sound, no take twice in a row (two takes alternate).
