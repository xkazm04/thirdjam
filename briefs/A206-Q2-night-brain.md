# Lane A206, track Q2 (the brain), the night: ten skaters who play hockey (device A, Opus)

You are lane A206 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md`, then `briefs/NIGHT-COMMON.md`: their rules are yours in full.**
Then `docs/VISION.md`, the track's `docs/experiments/BRAIN-6.md` to `BRAIN-8.md` with `status/A202.md` (the watch instrument `Bar.Brain.Watch` and its
detectors), the curve's `docs/experiments/CURVE-12.md` (what the levels' numbers are: rookie's opponents at skill 0.80, shot reach 20 m, no poke; pro's
pressure time 0.6 s), `JamAICore.cpp/.h`, `DifficultyProfile.cpp`.

Your worktree and the commit it stands on are named in the Director's message that starts you. Status file `status/A206.md`. Port 4806. Family as the
track's: `JamAICore.cpp/.h`, the brain's tests, the levels' brain numbers in `DifficultyProfile.cpp` (tonight they are yours too: the curve lane sets the
ladder after you), `docs/experiments/BRAIN-9.md`. Not yours: the puck's flight and the pass's physics (lane A207), the pickup (A208), contact (A209).

## His words: "AI is the main flaw, it does not try to play hockey to achieve a goal, all teammates/oponents are not interested in game, disengaged"
1. **See what he saw** (first 40 minutes; no code): PLAY NOW at rookie, regular rink, then pro: 20 matches a level traced with the watch instrument,
   extended with what "interested" means, per skater and per match: the share of play time each skater spends (a) moving at more than half his top
   speed, (b) closing on the puck or a man with the puck, (c) standing or drifting with no job; the time from a puck coming loose to the first bot
   touching it and how many bots moved toward it; shots a minute by side and from where; the seconds the bots' side holds the puck in the attacking
   third; how often a carrier bot skates to the net and shoots against how often it stops, circles or passes back; the mates: how often they are open
   in front of the human carrier, how often they skate away from the play. Ten off-screen stills of the worst moments. **One table that names the
   disinterest: which rule or number makes a skater stand** (rookie's zeroed poke and slow skill, the hold radius, the yield rules that make mates
   give the human room, the last man playing goalie, the pressure time). Write it as `SEEN` in the status file by 21:20.
2. **Every skater has a job every frame** (the order of work): the nearest two of each side go for a loose puck at full speed; the carrier bot goes to
   the net and shoots from inside the circles, or passes to a mate who is nearer the net and open; the man away from the puck goes to the net front or
   the open lane, not to a parking spot; on defence one man on the carrier, one in the slot, the rest between their man and the net; the human's mates
   skate ahead of him into open ice and call for the puck by going to the net; nobody stands for more than a second except the last man in his crease.
   Use what the brain already has (roles, lanes, the hunt, the yield) and remove what parks them.
3. **Rookie is easy because rookies are clumsy, not because they are asleep**: the same jobs, with a slower release, a wider aim, a later read, a
   shorter reach. Give rookie back its poke and its speed if they are what makes it stand; ease it with execution numbers and say which.
4. **The bots use what the other lanes build tonight**: a pass that arrives (A207: lead the receiver, choose the pass by the lane's own flight time),
   a shot twice as fast (shoot from further when the lane is open), a puck that sticks (A208). Read their status files at each wave; do not wait.
5. **The bars a person would agree with** (pro and rookie, 100 matches): no skater's "no job" share over 10 % of play; a loose puck is touched by a
   bot within 1.5 s when one is within 15 m; each side shoots at least 1.5 times a minute; the bots complete at least 60 % of their passes once A207
   lands; the human's mates give him an open man in the attacking half at least half the time he carries there. And the old detectors stay down
   (downed-body stand, own-zone carry, clump, long shots with a mate open).
6. Law 3 with every wave (pro and all_star: 8 / 8.5 / 9 / 9.5 m, the straight charge).

`BRAIN READY 1` by 23:15 (items 1 to 3: the file list, the bars before and after, the hash proof at neutral, the six cells at 600 for the curve
lane), then waves 2 and 3 as NIGHT-COMMON says. Three measurement processes at most. Time box: the night.
