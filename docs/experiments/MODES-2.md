# Director's choices, pass 2: twelve modes, the card words in the cartridge, two rows of six (lane A81, R-0036, R-0041, device A)

Status: 2026-11-13 13:25. Owner: lane A81. Pass 1: `MODES.md` (lane A67), keys: `MODE-KEYS.md` (lane A72). Files: six new `Game/Content/Cartridges/modes/*.cartridge.json`,
`modes/MODES.md`, `game.meta` (schema, loader, menu), `tools/playtest/modes.mjs`, stills `docs/experiments/img/modes2/`. No sim code changed.

## How it was measured

`node tools/playtest/modes.mjs --seeds 1-20 --max-frames 48000` on **this worktree's build** (main c45face plus this lane, no A77 / A79 changes): every mode cartridge
validated (`schema/validate.mjs`), then played by lane A34's **regular** playtester at the mode's level for seeds 1-20, next to `hockey.regular` at the same level
on the same binary. A mode passes when its character number differs from regular's by a third or more and all 20 matches end. Lines and table:
`tools/playtest/out/modes2-final.{jsonl,md}` (all twelve), `modes2-bruisers.{jsonl,md}` (Bruisers as shipped, against the same regular cells), the first round
`modes2-r1.*`, the retunes `tools/playtest/out/a81-variants/` (cartridge per variant). Lanes A77 (pass rule) and A79 (all_star) change the sim and the profile on
main at the same time: these numbers are against today's regular cartridge on my build and move when those land.

Automation on the same build: `Jam.Core + Jam.Menu` 41 / 41 Success (`Jam.Menu.DirectorsChoices` now expects twelve cards and loads each, `Jam.Core.CartridgeRoundTrip`
loads `modes/`, `Jam.Core.CartridgeParity`, `Jam.Core.Determinism` unchanged). `node --test schema/*.test.mjs` 98 / 98.

## The twelve modes against regular (20 seeds each)

| mode | lane | level | W-L | win % (regular) | GF / GA (regular) | checks landed / taken (regular) | character number | mode | regular | change | verdict |
|---|---|---|---|---|---|---|---|---|---|---|---|
| Pond | A67 | rookie | 13-7 | 65 (100) | 1.7 / 1.2 (3 / 0.4) | 0 / 0 (1.6 / 0) | passes + received / play min | **1.79** | 1.11 | +61 % | pass |
| Cage | A67 | pro | 6-14 | 30 (50) | 3.4 / 5 (2.3 / 2) | 4.8 / 4.3 (5 / 2.7) | checks landed / match | **4.75** | 4.95 | -4 % | **fail** |
| Sniper | A67 | pro | 16-4 | 80 (50) | 2.6 / 1.6 (2.3 / 2) | 2.7 / 1.9 (5 / 2.7) | human goals per human shot | **0.32** | 0.13 | +157 % | pass |
| Marathon | A67 | pro | 7-13 | 35 (50) | 4.3 / 4.8 (2.3 / 2) | 11.3 / 5.3 (5 / 2.7) | minutes per goal (no `--sprint`) | **1.19** | 0.87 | +36 % | pass |
| Power Play | A67 | pro | 13-7 | 65 (50) | 3.4 / 2.3 (2.3 / 2) | 6.2 / 3 (5 / 2.7) | passes + received / match | **5.2** | 2.1 | +148 % | pass |
| Blind Side | A67 | pro | 15-5 | 75 (50) | 2.6 / 1.8 (2.3 / 2) | 2.2 / 2.6 (5 / 2.7) | dekes / match | **1.45** | 2.2 | **-34 %** | pass, wrong way |
| **Sudden Death** | A81 | all_star | 4-16 | 20 (10) | 0.2 / 0.8 (0.9 / 2.8) | 0.3 / 0.7 (1.6 / 2.1) | match seconds | **54.6** | 161.7 | **-66 %** | pass |
| **Outnumbered** | A81 | rookie | 11-9 | 55 (100) | 2.3 / 1.8 (3 / 0.4) | 6.8 / 0 (1.6 / 0) | shots against / play min | **4.57** | 3.02 | **+52 %** | pass |
| **Glass House** | A81 | pro | 6-14 | 30 (50) | 1.9 / 2.5 (2.3 / 2) | 3.0 / 2.4 (5 / 2.7) | human box seconds / match | **21.6** | 8.5 | **+154 %** | pass |
| **Pinball** | A81 | pro | 8-12 | 40 (50) | 1.8 / 2.2 (2.3 / 2) | 3.0 / 1.4 (5 / 2.7) | human touches / min | **15.4** | 26.3 | **-42 %** | pass |
| **Bruisers** | A81 | rookie | 13-7 | 65 (100) | 2.1 / 1.3 (3 / 0.4) | 9.7 / 15.1 (1.6 / 0) | hits (landed + taken) / play min | **3.80** | 1.02 | **+274 %** | pass |
| **Fog of War** | A81 | pro | 16-4 | 80 (50) | 2.6 / 1.2 (2.3 / 2) | 2.5 / 2.0 (5 / 2.7) | human breakaways / play min | **1.80** | 0.96 | **+87 %** | pass |

Other numbers from the same lines: Sudden Death 1.2 human shots a match (regular all_star 5.0); Outnumbered 17.1 shots against a match in 4:04 (regular rookie 4.6
in 1:43), 6.8 checks landed by the human; Glass House 2.3 fouls a match at 10 s each (regular 1.7 at 5 s); Pinball 29 s of the human's idle per match and a longest
no-touch stretch of 60 s (regular 7 s / 33 s); Bruisers matches last 6:42, the human is knocked down 15.1 times and lands 9.7, 38 shots a match; Fog of War 4.0
breakaways a match (regular 3.3) in shorter matches (2:26 against 3:43).

### What a player feels (one sentence each)

- **Pond**: the puck moves by pass on soft, quiet ice, and nobody ever hits you.
- **Cage**: a crowded box where hitting is the only defence and the bots hit back.
- **Sniper**: fewer shots, but a lifted one goes in twice as often as on regular ice.
- **Marathon**: long ice and a long clock: the match is paced, stamina is the resource.
- **Power Play**: the extra teammate means the puck comes back to you; you play give-and-go.
- **Blind Side**: dekes that fool the back and checks that knock you down for four seconds.
- **Sudden Death**: under a minute of dread: one mistake against all-star bots on big ice ends it, and they score first four times in five.
- **Outnumbered**: five bodies swarm you; you spend the match blocking shots and hitting your way out, and you win only one in two even against rookies.
- **Glass House**: every check is a gamble: a foul puts you in the box for ten seconds, two and a half times regular's box time, and the bots take every chance to hit you.
- **Pinball**: the puck caroms off eight bumpers and live boards and keeps running; you touch it 40 % less and chase it twice as long.
- **Bruisers**: five a side on small ice with heavy bodies and no whistle: almost four hits a minute, a knockdown every half minute, a brawl you can still win.
- **Fog of War**: the bots react a fifth of a second late in the fog, so you slip past them on breakaways nearly twice as often.

## What the measurement changed (round 1 -> shipped)

- **Pinball**: first cut (6 bumpers size 1.5, puck 45 m/s, shot 26) with the human's carry share as the number: 10.2 % against 12.3 % (-17 %), **fail**. 8 bumpers
  size 2, ice 0.005, puck 50 m/s, shot 30: carry 9 % (-27 %, still short), but touches / min 15.4 against 26.3 (-42 %); shipped with touches as the number (the
  puck that runs away is the character). Variant with `carry_assist` 0.2 instead of bumpers: touches 17.5 (-33 %, borderline), dropped.
- **Fog of War**: `react_frames` 40 gave breakaways +230 % and 95 % wins (a walkover); 28 / 20 / 12 gave 90 / 95 / 80 % wins and +165 / +116 / +88 %. Shipped at 12.
  The human's checks landed fall (2.5 against 5.0) although `check_speed_min` is 1: the human carries more and chases less.
- **Bruisers** (R-0041, added for Tap-In, below): first cut at pro on 90 x 45 with checks from 2 m/s: hits +36 % but 10 % wins (5 v 5 heavy pro bots out-score
  the human); 80 x 40 at pro: 5 % wins. Rookie bots with `ai.body_check` 1 (any share turns the rookie's rule on, MODE-KEYS.md) on 80 x 40 from 1.5 m/s: hits +274 %,
  65 % wins: the beating is two-way and winnable.
- **Tap-In** (net 6 x 1.5 m, no crease saves, shot 10 m/s, 3:00) measured goals (both) / play min 1.80 against 1.24 (+45 %, pass, 65 % wins), then **parked** for
  Bruisers after R-0041 asked for aggression where it fits: the twelve slots are full. File: `tools/playtest/out/a81-variants/parked-tap-in.cartridge.json`
  (the variant with passes seated at 1.4 m did not raise passing: 0.6 / 2.2 a match against 0.6 / 2.3).
- Sudden Death, Outnumbered and Glass House passed in round 1 as written in the brief (Outnumbered with `ai.sprint` 0.75).

## Findings on A67's modes against today's regular (not retuned here: A67's files, only `game.meta` added)

- **Cage fails** its own number: regular pro now lands 5.0 checks a match (A67: 3.2, A72: 3.6: main moved under the mode; R-0039's pro bots checking from 4 m/s is one
  change in between, not isolated here). Cage's human is checked down 4.3 times against 2.7 (+59 %): moving its number to "checks taken" or "hits / play min" (as Bruisers) would pass today.
- **Blind Side passes the wrong way**: dekes 1.45 against regular's 2.2 (-34 %); regular pro's dekes rose from 0.75 (A72) to 2.2 on this build. The mode no longer
  gives more dekes; its checks-taken half (2.6 against 2.7) has flattened too. It needs a retune or a new number after A77 / A79 land.
- **Marathon** without `--sprint` is +36 % minutes per goal (A67's 51 %); its A72 number needs `--sprint` and was not re-run here.

## The card words in the cartridge (A67's ask A11): `game.meta`

`game.meta { pitch, facts[] }`, an informational object (the schema rule allows it: unknown keys stay errors, the object is declared like any other): `pitch` 1..48
characters, `facts` 1..3 items of 1..20 characters; the card's name is the envelope `title`, so there is no `meta.title` (the brief's shape had one; it would
duplicate a required key). Landed in `schema/game.schema.json`, `limits.json` (`meta.facts[]` 1..3), `DEFAULTS.md`, `README.md`, the loader
(`UJamCartridge::MetaPitch`, `MetaFacts`; never copied into `FJamGameConfig`, so the sim and the hash cannot see it), the three generators re-run (schema data,
parity 81 cases, designer data 36 cuts). `UJamMenuFlow::ModeCards` keeps only the order of the twelve and reads the words from each file once; a mode file not in the
order is appended in file order, a file without `meta` gets no card. The lengths are the layout: at 48 / 20 the pitch is two lines and a fact one line at 1080p
(two facts wrap at 720p: "burst +60 % for 4 s", "8 bumpers, bouncy").

## The menu: two rows of six

Choose your cut widens the slate card from 880 to 1240 slate units on that screen only (the other screens keep 880); the Director's choices grid is 6 x 2, the card
text starts at the top (names line up), the name has no tracking so OUTNUMBERED fits a sixth. The cut list keeps its 250-unit box below. Stills (PIE in a floating
window of the stated size on my editor, MCP 8040, `jam.menuscreen cuts` as a console command, no widget call from Python; the editor stopped cleanly afterwards):

- `docs/experiments/img/modes2/directors-choices-1080p.png`
- `docs/experiments/img/modes2/directors-choices-720p.png`

Both show all twelve cards, the cut list, the status line, BACK and the controls strip inside the frame. **Eyes `readability` did not run**: the Gemini key's prepaid
credits are depleted (`generateContent 402 RESOURCE_EXHAUSTED`); my own read: every name and pitch legible at both sizes, facts at 720p are the smallest text and read, Pond's focus tint shows.

## Asks that remain

| ask | what | for |
|---|---|---|
| A12 (R-0041, lane A84) | `game.abilities[]`: the human's ability on F. Shape I would want: `[{ "kind": "shotgun" \| "hip_check" \| "shield" \| "slow_puck", "charges": 1..9 (absent = unlimited), "cooldown_frames": 60..3600 (absent = the kind's default), "duration_frames": 1..600 (shield, slow_puck; absent = the kind's default), "magnitude": 0.1..4 (absent = 1) }]`, 0..1 items while F is one key (a list so a pickup or a second key can add one later), unknown keys errors, a bark and a HUD slot per kind | the four modes below |
| A12a | **Sudden Death** grants `{ "kind": "shotgun", "charges": 1 }`: one shot through the crowd per match; against all-star bots on big ice it is the one way to force the single goal | Sudden Death |
| A12b | **Outnumbered** grants `{ "kind": "hip_check", "cooldown_frames": 600 }`: one burst every 10 s to clear a body when five swarm you | Outnumbered |
| A12c | **Pinball** grants `{ "kind": "slow_puck", "duration_frames": 120, "cooldown_frames": 900 }`: two seconds of a slow puck to catch it out of the bumpers (its idle of 29 s a match is the cost the ability buys back) | Pinball |
| A12d | **Bruisers** grants `{ "kind": "shield", "duration_frames": 180, "charges": 3 }`: three times a match the human cannot be knocked down for 3 s | Bruisers |
| A13 | a cross-check count in the playtest line (`cross_checks`), once R-0041's cross-check exists: Bruisers and Glass House would read it | Bruisers, Glass House |
| A14 | Cage and Blind Side retuned or renumbered against the post-A77 / A79 regular (above) | A67's modes |
| A15 | Gemini credits for `tools/eyes` (402) so the menu stills get their readability pass | the stills |
| A6, A7, A9, A10 | still open from MODES.md: `teams[].skater_scale`, offside, `pond_day` look, `puck.count` in the sim | as in MODES.md |
