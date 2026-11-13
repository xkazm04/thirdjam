# The voice: rough, funny, never cruel (lane A89, device A, 2026-11-13, R-0041 point 4)

Owner: lane A89. Inputs: R-0021 (hockey first, teen rating), R-0033 point 3 (commentator random, no repeats), R-0041 (raw and brutal brand), `docs/PLAN-RECONCILED.md`.
Applies to every word the player reads or hears: menu, mode cards, match cards, HUD, commentator, barks. Device B owns the logo and the banner; this file owns the words.

## The one sentence

The game is a bar fight between clay figures that happens to have a puck in it. It brags, it winces, it laughs at the guy on the ice, and it never wishes anyone harm.

## Ten rules (a do and a don't each)

1. **Short and loud.** A line is one beat; the verb carries it.
   Do: "Flattened!" / "Nobody is getting up fast." Don't: "That was a very substantial body check by the blue skater."
2. **Hit words are hockey words.** Check, hit, lay the body, cross-check, flatten, rattle, rock, level, send flying, bounce off the glass.
   Do: "Leveled at the boards." Don't: "Murdered." "Destroyed his spine." "Crushed your bones."
3. **The ice is the victim, never the person.** The skater went down; the figure is clay; the joke is about the fall, never about the player's skill or body.
   Do: "Down he goes. Gravity wins again." Don't: "You are terrible." "Pathetic." Never call the human a noob, an idiot or a loser.
4. **Brag up, wince sideways.** When the human hits, the booth cheers; when the human is hit, the booth winces and tells them to get up, never mocks.
   Do: "That one left a mark. Up you get." Don't: "Stay down." "Give up."
5. **Funny by exaggeration, not by cruelty.** Physics jokes, clay jokes, the glass, the puck going somewhere it should not.
   Do: "He is wearing the boards now." Don't: jokes about injuries, hospitals, health, bodies, ages or anyone's looks.
6. **One mild word in ten lines, at most, and never two in a line.** The bank's own cap (R-0015): damn, hell, crap, and the kick-your-ass phrases. Every other line is clean.
   Do: "Hell of a hit." (once in ten) Don't: stacking two in a line, or any word the filter bans.
7. **Hockey first, film words only in the menu.** Director's choices, takes and cuts live on the menu, the Designer and the season screen (R-0021 point 3); the ice says PERIOD, FACEOFF, MATCH, GOAL.
   Do: "MATCH 3 OF 8" on the ice, "CHOOSE YOUR CUT" in the menu. Don't: "TAKE 3" on the ice, "scene", "reel", "script" in a bark.
8. **No names.** No real players, teams, brands, countries or cities; the teams are blue and orange.
   Do: "Orange lays the body." Don't: any name in the filter's list, any capitalised word mid-sentence.
9. **Numbers are facts, not decoration.** A mode card's fact must be true of its cartridge; a bark never states a score or a count (the filter bans long numbers).
   Do: "down for 4 s a hit" (the cartridge says 240 frames). Don't: "130 kg of pure pain".
10. **Never twice the same.** The booth chooses at random and never repeats a situation's last lines (R-0033 point 3); write three or more alternates per situation, each with a different joke, not the same line reworded.
    Do: three different images of one hit (glass, floor, gravity). Don't: "Big hit!" "Huge hit!" "Giant hit!"

## The line the rating draws (R-0021, teen)

A knockdown may be called: down, flat, flattened, leveled, laid out (on the ice only), flipped, rattled, rocked, sent flying, bounced, folded, planted, sat down hard, wearing the boards, "feeling the glass".
A knockdown may not be called: anything with blood, bones, a body part breaking, pain beyond a wince, a hospital, a stretcher, an injury, death or killing, a weapon (a stick is a stick), "crush your", "break your", "end you", "wreck you", fists and punching a person (the checks are the game; fighting is not).
Also never: slurs, insults about real traits, sexual words, drugs, real names, brands, teams, places. "Sudden death" stays as the hockey phrase (the one allowed use). The bark filter (`tools/barks/filter.mjs`) is the machine form of this section and decides when in doubt. Figures are clay: nothing on screen ever bleeds, breaks or stays down.

Cross-checks and shotgun knockdowns are described as what they do to the *play*: "sent sprawling", "bowled over", "the whole line goes down like pins".

## Glossary (the game's own words)

| Word | Meaning here | Not |
|---|---|---|
| hit, check | a body check; the game's main verb | "attack", "assault" |
| cross-check | the stick-across check, a bigger knockdown | "stab", "jab" |
| flatten, level, rattle, rock | a check that knocks a skater down | "kill", "crush" |
| down, flat, sprawled | knocked down (240 frames at pro) | "hurt", "out cold" |
| the glass, the boards | the rink wall | "the wall of pain" |
| carrier | the skater holding the puck (slower, cannot sprint) | "ball carrier" |
| lift | the puck leaving the ice | "flip" |
| shotgun | the one-shot ability: a shot that knocks down every skater it passes | "gun", "weapon" |
| massacre shift | three or more knockdowns in about twenty seconds, said as a brag about the *hits*, never about people | "slaughter", "bloodbath" |
| rookie / pro / all-star | the aggression ladder: rookies rarely hit, pros hit as a habit, all-stars hunt the carrier | "easy / normal / hard" |
| period, faceoff, match | the ice's words | "take", "scene" on the ice |
| Director's choice, cut, take | the menu's words (modes, saved rule sets, season matches) | on the ice, in a bark |
| Director's Call | the one live change; the name stays, the card wording is hockey | "ACTION!" |
| blue, orange | the two teams | any team name |

## Where each surface speaks

| Surface | Register | Limits |
|---|---|---|
| Menu | cheeky, a little theatre, the film words allowed | one line per button, labels as long as the label they replace plus 20 % |
| Mode cards | a pitch (48 chars) that sells the fight, three true facts (20 chars each) | `game.meta` limits, facts match the cartridge |
| Match cards | plain hockey words: MATCH, PERIOD, FACEOFF | pinned by `Jam.Menu` tests |
| HUD | one or two words, hockey, no jokes in the numbers | pinned strings: YOU WIN, REMATCH?, PAUSED |
| Commentator | the full voice, rules 1 to 10 | bark filter: 60 chars, no digits, ASCII, mild at most one in ten |
| Barks | the skaters' own mutterings, same rules | R-0015 register |

## Proposals this lane did not edit (owners named)

See `status/A89.md` and the report: HUD end cards ("YOU WIN", "REMATCH?"), the controls cards (A84), the commentary bank (A84), the new lines in `tools/commentary/proposed/a89-lines.json`.
