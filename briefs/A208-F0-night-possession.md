# Lane A208, track F0 (the puck and the stick), the night: the puck belongs to whoever touches it (device A, Opus)

You are lane A208 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md`, then `briefs/NIGHT-COMMON.md`: their rules are yours in full.**
Then `docs/VISION.md`, the stick and possession docs (`docs/experiments/STICK*.md`, `CONTACT-5.md` and what `schema/README.md` says of `game.stick`
and `game.puck`: the carry reach, the take, the take grace, the poke, "the puck is yours until someone presses for it" of R-0048), the possession
loop and the puck's collisions in `JamSim.cpp` (who becomes the carrier and when; what the puck collides with: boards, nets, bodies, sticks), and
the carry's presentation (where the puck is drawn against the blade).

Your worktree and the commit it stands on are named in the Director's message that starts you. Status file `status/A208.md`. Port 4808. Family: the
possession loop, the loose puck's pickup, the puck's collision with skaters and sticks, the carry (how the puck follows the blade) in `JamSim.cpp/.h`,
their keys and tests, the puck's drawing on the stick, `docs/experiments/POSSESSION.md`. Not yours: the pass's flight and aim (A207: but the RECEIVE
of a pass meets your pickup: agree it through the status files), the brain (A206), hits (A209).

## His words: "Puck does not stick with players, it moves through them without locking"
1. **See it** (30 minutes, no code): PLAY NOW at rookie and pro, 20 matches traced, and staged scenes: a loose puck sliding at 2 / 6 / 12 / 20 m/s
   into a standing skater's feet, body, stick blade, from the front, the side, behind; a skater skating over a resting puck at walk, glide, sprint; a
   pass arriving at a mate's feet, a metre off his stick, at his skates; a shot hitting a skater. For each: does the puck stop, bounce, pass through,
   become his? How many frames is it inside his body? One table and stills. Then the match counts: how often a puck passes within 1 m of a skater
   (human, mate, opponent) and is not picked up, and why (the rule that says no: the unpressed-blade rule, the take grace, a cooldown after losing it,
   a reach that is the blade's point only, a speed limit, facing).
2. **The rule a hand expects**: a loose puck that touches a skater who can play it becomes his: stick first (a generous arc in front of him: say the
   metres and degrees), then skates and body (it stops at him and is his a few frames later), at any speed up to a hard shot (a shot that hits a
   body bounces off and stings: a rebound he or anyone can take); skating over a resting puck picks it up always. A puck CARRIED by someone else is
   still only taken by a press (the poke, a check, a cross-check: R-0048 holds: say where your rule ends and that one begins). The puck never passes
   through a skater: it collides with bodies (a circle) as it does with boards: check tunnelling at the shot's new speed (A207 doubles it tonight:
   sub-step the puck or sweep the segment).
3. **The lock he should feel**: once his, the puck goes to his blade in a few frames and stays there through turns, stops and sprints (the carry's
   spring: measure the puck's distance from the blade over a carry at full speed and through a hard turn: under 15 cm, never behind his skates), and
   the screen says it at once (the puck's ring takes his team's colour, the carrier's marker, a short sound line: use what the HUD and the binder
   have). The half second of grace after winning it stays or goes by what you measure (he must not be stripped in the same frame he takes it).
4. **Receiving**: a pass from a mate that reaches his stick's arc is his at once at any pass speed A207 ships (tell A207 the arc; read its file).
5. **Bars** (staged, then 100 matches at rookie and pro): every staged scene above ends with the skater carrying or the puck bouncing off his body
   (0 pass-throughs in 1,000 staged arrivals at up to 40 m/s); in matches a loose puck passing within the pickup arc of a skater who is not down is
   picked up 98 % or more; the carried puck stays within 15 cm of the blade 99 % of carry frames; no pickup through the boards or the net's back.
6. Law 3 and the take's rules: the stop-and-go drill and `Jam.Core.Stick*` tests green; say each test you re-staged and why.

`STICK READY 1` by 23:15 (items 1 to 3: file list, the staged table before and after, the bars, the hash proof at neutral, the six cells at 600
for the curve lane), then waves 2 and 3 as NIGHT-COMMON says. Three measurement processes at most. Time box: the night.
