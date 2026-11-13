// Generates briefs/commentary/lines.json from the tables below (lane N2, device B).
import { writeFileSync } from 'node:fs';
const out = [];
// S(situation id, event, kind|null, when, team, priority, cooldown_s, texts[], speaker)
function S(sid, event, kind, when, team, priority, cooldown_s, texts, speaker = 'commentator') {
  texts.forEach((text, i) => {
    const e = { id: `${sid}.${String(i + 1).padStart(2, '0')}`, speaker, event };
    if (kind) e.kind = kind;
    Object.assign(e, { when, team, priority, cooldown_s, text });
    out.push(e);
  });
}
const D = (sid, event, kind, when, priority, cooldown_s, texts) => S('dir.' + sid, event, kind, when, 'any', priority, cooldown_s, texts, 'director');

// ---------------- COMMENTATOR ----------------
// Faceoff
S('faceoff.drop', 'faceoff', 'drop', 'the puck drops at any faceoff', 'any', 2, 8, ['Puck is down.', 'Here comes the drop.', 'And we are underway.']);
S('faceoff.blue', 'faceoff', 'won', 'a blue centre wins the draw', 'blue', 2, 6, ['Blue wins the draw.', 'Clean win for blue.', 'Blue wins the faceoff.']);
S('faceoff.orange', 'faceoff', 'won', 'an orange centre wins the draw', 'orange', 2, 6, ['Orange wins the draw.', 'Clean win for orange.', 'Orange wins the faceoff.']);
S('faceoff.early', 'faceoff', 'lost', 'a centre moves before the drop and loses the draw', 'any', 3, 20, ['Too early on the draw.', 'Jumped the drop. Faceoff lost.', 'Anticipated that one a bit much.']);
S('faceoff.after_goal', 'faceoff', 'drop', 'the first drop after a goal', 'any', 2, 30, ['Back to centre ice.', 'Reset, and we go again.', 'Fresh scene at centre ice.']);
// Shots
S('shot.blue', 'shot', null, 'a blue skater shoots', 'blue', 2, 6, ['Blue shoots!', 'Shot from blue!', 'Blue lets it go!']);
S('shot.orange', 'shot', null, 'an orange skater shoots', 'orange', 2, 6, ['Orange shoots!', 'Shot from orange!', 'Orange lets it go!']);
S('shot.hard', 'shot', null, 'a shot with Strength 0.8 or more', 'any', 3, 10, ['What a rocket!', 'That one was loaded.', 'Full wind-up, full power!']);
S('shot.soft', 'shot', null, 'a shot with Strength under 0.3', 'any', 1, 15, ['Just a little flip.', 'Soft one on net.', 'A gentle try.']);
S('shot.long', 'shot', null, 'a shot from the shooter\'s own half', 'any', 2, 20, ['From a long way out!', 'Long range attempt.', 'Trying it from distance.']);
S('shot.close', 'shot', null, 'a shot from inside the crease area', 'any', 3, 10, ['Right on the doorstep!', 'Point blank!', 'In tight!']);
S('shot.human_streak', 'shot', null, 'the human\'s team takes its third shot in ten seconds', 'any', 2, 30, ['The pressure is building.', 'They keep coming.']);
// Goals: colour call
S('goal.blue', 'goal', null, 'the puck goes in for blue', 'blue', 5, 0, ['Blue scores!', 'In! Blue scores!', 'It\'s in, for blue!']);
S('goal.orange', 'goal', null, 'the puck goes in for orange', 'orange', 5, 0, ['Orange scores!', 'In! Orange scores!', 'It\'s in, for orange!']);
// Goals: from the human's side
S('goal.for', 'goal', null, 'the human\'s team scores (second line in the goal pause)', 'any', 4, 0, ['What a finish!', 'Picture perfect.', 'Right where it had to go.']);
S('goal.against', 'goal', null, 'the other team scores (second line in the goal pause)', 'any', 4, 0, ['That one stings.', 'Credit where it is due.', 'Nothing anyone could do.']);
S('goal.first', 'goal', null, 'the first goal of the match', 'any', 4, 0, ['First one on the board.', 'And the first one is in the can.']);
S('goal.tie', 'goal', null, 'the goal ties the score', 'any', 4, 0, ['All square!', 'And we are level again.', 'Tied up! Start the scene over.']);
S('goal.lead', 'goal', null, 'the goal puts the scoring team ahead after a tie', 'any', 4, 0, ['And that is the lead!', 'Out in front now.', 'A go-ahead goal!']);
S('goal.comeback', 'goal', null, 'the scoring team was down by two or more before this goal', 'any', 4, 0, ['The comeback is on!', 'They are clawing back!', 'Not done yet, not at all.']);
S('goal.blowout', 'goal', null, 'the goal makes the lead three or more', 'any', 4, 0, ['This is getting lopsided.', 'Somebody call for a rewrite.', 'Running away with it.']);
S('goal.match_point', 'goal', null, 'the scoring team is now one goal from winning (first-to mode)', 'any', 4, 0, ['One more and that is the match.', 'One goal from a wrap.', 'Match point.']);
S('goal.long', 'goal', null, 'the goal was shot from the shooter\'s own half', 'any', 4, 0, ['From all the way out there!', 'A long-distance special!']);
S('goal.own', 'goal', null, 'the last touch was by the conceding team', 'any', 4, 0, ['Off their own stick. Ouch.', 'An unlucky bounce in.']);
S('goal.late', 'goal', null, 'a goal in the last ten seconds of a take', 'any', 5, 0, ['Right at the end!', 'With seconds to spare!', 'Just before the cut!']);
S('goal.pause', 'goal', null, 'during the goal pause, after the first goal line, about one second in', 'any', 2, 20, ['Let us see that one again. Lovely placement.', 'The counter clicks over, and the set resets.', 'A clean take. The director will keep that one.']);
// Saves by the last man
S('save.any', 'save', null, 'the last man blocks a shot', 'any', 3, 6, ['Blocked!', 'Turned away!', 'Kept out!']);
S('save.big', 'save', null, 'the last man blocks a shot with Strength 0.8 or more', 'any', 4, 10, ['What a stop!', 'Robbed!', 'Huge block!']);
S('save.blue', 'save', null, 'a blue last man blocks a shot', 'blue', 3, 10, ['Blue holds the line.', 'Blue gets a stick on it.', 'Blue says not today.']);
S('save.orange', 'save', null, 'an orange last man blocks a shot', 'orange', 3, 10, ['Orange holds the line.', 'Orange gets a stick on it.', 'Orange says not today.']);
// Post
S('post.any', 'post', null, 'the puck hits a post', 'any', 4, 6, ['Off the post!', 'Iron! So close!', 'Rings the post!']);
S('post.late', 'post', null, 'the puck hits a post in the last ten seconds', 'any', 5, 10, ['The post says no!', 'Inches from the tying goal.']);
// Board
S('board.hard', 'board', null, 'the puck hits the boards with Strength 0.7 or more', 'any', 1, 20, ['Off the boards and out.', 'Rattles the boards.', 'Loud one off the wall.']);
S('board.rim', 'board', 'rim', 'the puck runs along the boards behind a net', 'any', 1, 25, ['Rimmed around the back.', 'Wrapped around the boards.', 'Around the wall it goes.']);
S('board.bank', 'board', 'bank', 'the puck banks off the boards into a goal within one second', 'any', 3, 20, ['Banked it in off the boards!', 'Used the boards like a teammate.']);
// Checks and knockdowns
S('check.blue', 'check', null, 'a blue skater lands a body check', 'blue', 2, 8, ['Blue finishes the check.', 'Big hit from blue.', 'Blue lays the body.']);
S('check.orange', 'check', null, 'an orange skater lands a body check', 'orange', 2, 8, ['Orange finishes the check.', 'Big hit from orange.', 'Orange lays the body.']);
S('check.big', 'check', null, 'a body check with Strength 0.8 or more', 'any', 3, 10, ['Oh, that is a big hit!', 'Felt that one up here.', 'Crunching check!']);
S('check.strip', 'check', null, 'a check takes the puck from the carrier', 'any', 3, 10, ['Hit, and the puck is loose!', 'Separated from the puck.', 'Check, and a turnover.']);
S('check.boards', 'check', null, 'a body check within two metres of the boards', 'any', 2, 15, ['Big hit along the boards!', 'Into the glass!', 'Squeezed against the wall.']);
S('knockdown.any', 'knockdown', 'check', 'a skater is knocked down by a check', 'any', 2, 10, ['Down goes the carrier.', 'Flat on the ice.', 'Off the feet!']);
S('knockdown.boost', 'knockdown', 'boost', 'a skater is knocked down by a boost', 'any', 2, 15, ['Knocked over by the boost.', 'That boost packs a punch.']);
S('knockdown.human', 'knockdown', null, 'the human\'s skater is knocked down', 'any', 3, 15, ['Back up, quickly now.', 'Shake it off and get back in.', 'Up you get. Play on.']);
// Poke and lift
S('poke.clean', 'poke', 'clean', 'a poke check takes the puck', 'any', 3, 6, ['Poked away!', 'Quick stick, puck gone.', 'Clean poke check.']);
S('poke.blue', 'poke', 'clean', 'a blue skater pokes the puck away', 'blue', 2, 10, ['Blue picks the pocket.', 'Blue steals it.']);
S('poke.orange', 'poke', 'clean', 'an orange skater pokes the puck away', 'orange', 2, 10, ['Orange picks the pocket.', 'Orange steals it.']);
S('poke.miss', 'poke', 'miss', 'a mistimed poke and a stumble', 'any', 2, 10, ['Swing and a miss.', 'Mistimed, and a stumble.', 'Too early with the stick.']);
S('lift.clean', 'lift', 'clean', 'a stick lift strips the carrier', 'any', 3, 8, ['Lifts the stick, takes the puck!', 'Textbook stick lift.', 'Lifted and gone.']);
S('lift.miss', 'lift', 'miss', 'a stick lift that misses', 'any', 1, 15, ['Reached for the lift, nothing there.', 'Lift attempt, no luck.', 'Missed the stick.']);
// Fouls
S('foul.behind', 'foul', 'foul_from_behind', 'a check from behind is called', 'any', 4, 5, ['From behind! That is a foul.', 'Hit from behind. Whistle.', 'You cannot do that from behind.']);
S('foul.no_puck', 'foul', 'foul_no_puck', 'a check on a skater without the puck is called', 'any', 4, 5, ['No puck there. That is a foul.', 'Hit away from the play. Called.', 'Late hit. That one is called.']);
S('foul.blue', 'foul', null, 'a blue skater is sent to sit', 'blue', 3, 10, ['Blue takes a seat.', 'Blue is a skater short.', 'Blue to the bench for a while.']);
S('foul.orange', 'foul', null, 'an orange skater is sent to sit', 'orange', 3, 10, ['Orange takes a seat.', 'Orange is a skater short.', 'Orange to the bench for a while.']);
S('faceoff.after_foul', 'faceoff', 'drop', 'the free faceoff after a foul drops', 'any', 3, 10, ['Free faceoff after the foul.', 'Draw at their end. A real chance.', 'Free draw. Make it count.']);
// Breakaways and carries
S('breakaway.blue', 'breakaway', null, 'a blue carrier is past the last defender', 'blue', 4, 10, ['Blue is in alone!', 'Breakaway for blue!', 'Blue on the break, nobody back!']);
S('breakaway.orange', 'breakaway', null, 'an orange carrier is past the last defender', 'orange', 4, 10, ['Orange is in alone!', 'Breakaway for orange!', 'Orange on the break, nobody back!']);
S('breakaway.human', 'breakaway', null, 'the human is the carrier on a breakaway', 'any', 4, 15, ['All yours now!', 'Just the last man to beat.', 'Clear ice. Pick a spot.']);
S('carry.long', 'carry', null, 'a carrier keeps the puck for thirty metres or more', 'any', 2, 20, ['Still carrying it.', 'Nobody can catch up.', 'Long carry up the ice.']);
S('carry.coast', 'carry', null, 'a carrier takes the puck from its own net area to the far blue line', 'any', 3, 30, ['End to end!', 'Coast to coast!']);
S('carry.protect', 'carry', null, 'a carrier keeps the puck while being checked', 'any', 2, 20, ['Strong on the puck.', 'Shrugs off the check.', 'Cannot knock it loose.']);
// Passing
S('pass.any', 'pass', null, 'a completed pass', 'any', 1, 15, ['Nice feed.', 'Puck moves quickly.', 'Tape to tape.']);
S('pass.long', 'pass', null, 'a pass with Strength 0.8 or more', 'any', 2, 20, ['Long stretch pass!', 'Hits the far skater in stride.', 'Stretch pass, right on target.']);
S('pass.to_human', 'pass', null, 'a teammate passes to the human', 'any', 2, 20, ['Into your lane.', 'Set up nicely.']);
S('pass.blue', 'pass', null, 'blue completes a third pass in a row', 'blue', 2, 30, ['Blue moving it around.', 'Blue playing keep-away.']);
S('pass.orange', 'pass', null, 'orange completes a third pass in a row', 'orange', 2, 30, ['Orange moving it around.', 'Orange playing keep-away.']);
// Pickups appearing
S('pickup_spawn.any', 'pickup_spawn', null, 'a pickup appears (any kind)', 'any', 1, 20, ['A pickup on the ice.', 'The props team drops one in.', 'Something new out there.']);
S('pickup_spawn.speed', 'pickup_spawn', 'speed', 'a speed pickup appears', 'any', 2, 30, ['Speed boost on the ice.', 'Speed is up for grabs.']);
S('pickup_spawn.precision', 'pickup_spawn', 'precision', 'a precision pickup appears', 'any', 2, 30, ['Precision boost out there.', 'Sharpshooter boost on the ice.']);
S('pickup_spawn.heavy_puck', 'pickup_spawn', 'heavy_puck', 'a heavy-puck pickup appears', 'any', 2, 30, ['Heavy puck pickup out there.', 'The heavy puck is on the ice.']);
S('pickup_spawn.big_puck', 'pickup_spawn', 'big_puck', 'a big-puck pickup appears', 'any', 2, 30, ['Big puck pickup is out.', 'Someone wants a bigger puck.']);
S('pickup_spawn.reverse_other', 'pickup_spawn', 'reverse_other', 'a reverse-controls pickup appears', 'any', 2, 30, ['The reverse pickup is out.', 'Watch that one. It flips the other side.']);
// Pickups taken
S('pickup_take.blue', 'pickup_take', null, 'a blue skater takes a pickup', 'blue', 2, 15, ['Blue grabs it.', 'Blue takes the pickup.', 'Blue helps itself.']);
S('pickup_take.orange', 'pickup_take', null, 'an orange skater takes a pickup', 'orange', 2, 15, ['Orange grabs it.', 'Orange takes the pickup.', 'Orange helps itself.']);
S('pickup_take.speed', 'pickup_take', 'speed', 'a skater takes a speed pickup', 'any', 3, 15, ['Speed boost! Off they go.', 'Turbo on.', 'Now that is quick.']);
S('pickup_take.precision', 'pickup_take', 'precision', 'a skater takes a precision pickup', 'any', 3, 15, ['Precision boost. Aim is locked.', 'Sharper shooting now.', 'Dead-eye mode.']);
S('pickup_take.heavy_puck', 'pickup_take', 'heavy_puck', 'a skater takes a heavy-puck pickup', 'any', 3, 15, ['The puck just got heavy.', 'Heavy puck. Good luck lifting it.', 'Puck weighs a ton now.']);
S('pickup_take.big_puck', 'pickup_take', 'big_puck', 'a skater takes a big-puck pickup', 'any', 3, 15, ['Big puck! Hard to miss.', 'Look at the size of that puck.', 'Puck the size of a pizza.']);
S('pickup_take.reverse_other', 'pickup_take', 'reverse_other', 'a skater takes a reverse-controls pickup', 'any', 3, 15, ['Controls reversed on the other side!', 'Left is right now. Good luck.', 'The other team is all turned around.']);
// Effects ending
S('effect_start.any', 'effect_start', null, 'an effect starts that has no kind-specific line', 'any', 2, 15, ['Something just changed out there.', 'A new effect is rolling.']);
S('effect_end.any', 'effect_end', null, 'any effect wears off', 'any', 1, 15, ['That boost has worn off.', 'Back to normal.', 'And the effect fades.']);
S('effect_end.speed', 'effect_end', 'speed', 'a speed effect ends', 'any', 2, 20, ['Speed boost is done.', 'Back to regular pace.']);
S('effect_end.reverse_other', 'effect_end', 'reverse_other', 'a reverse-controls effect ends', 'any', 2, 20, ['Controls are the right way round.', 'Back to normal steering.']);
// Zones (Kind = zone id; the binder maps it to the zone kind)
S('zone_enter.slow_ice', 'zone_enter', 'slow_ice', 'a carrier enters a slow-ice zone', 'any', 1, 30, ['Into the slow patch.', 'Sticky ice there.']);
S('zone_enter.fast_ice', 'zone_enter', 'fast_ice', 'a carrier enters a fast-ice zone', 'any', 1, 30, ['Onto the fast ice!', 'Zooming through the slick patch.']);
S('zone_enter.bumper', 'zone_enter', 'bumper', 'the puck or a skater enters a bumper zone', 'any', 1, 30, ['Off the bumper!', 'Bumper sends it flying.']);
S('zone_enter.no_shoot', 'zone_enter', 'no_shoot', 'a carrier enters a no-shoot zone', 'any', 1, 30, ['No shooting from there.', 'In the quiet zone. No shots.']);
S('zone_enter.boost_only', 'zone_enter', 'boost_only', 'a skater enters a boost-only zone', 'any', 1, 30, ['Into the boost zone.', 'Power up territory.']);
S('zone_exit.any', 'zone_exit', null, 'a carrier leaves any zone', 'any', 1, 30, ['Out of the zone.', 'Back on regular ice.']);
// Rule announcements
S('announce.rule', 'announce', 'rule', 'a rule hook announces text on the scoreboard', 'any', 3, 10, ['New rule on the board.', 'Check the scoreboard, everyone.', 'The script just changed.']);
// Director's Call: generic
S('directors_call.any', 'directors_call', null, 'any Director\'s Call is applied (fallback when no key line fits)', 'any', 4, 0, ['The director has made a call.', 'A rule change, live on set.', 'New direction for this take.']);
// Director's Call per tunable (Strength = new value's position in its range)
const DC = [
  ['skater_speed', 'skater speed', ['Everybody just got faster.', 'Skaters on fast forward.'], ['Slowing everybody down.', 'Skaters in slow motion.']],
  ['skater_turn_rate', 'skater turn rate', ['Sharper turns now.', 'Skaters turn on a dime.'], ['Wide turns from here.', 'Turning like a cargo ship.']],
  ['shot_speed', 'shot speed', ['Shots get heavier now.', 'Shot power way up.'], ['Shots are softer now.', 'Less power behind the shots.']],
  ['puck_max_speed', 'puck top speed', ['The puck can really fly now.', 'Puck speed limit raised.'], ['Puck speed limit lowered.', 'The puck will not go far now.']],
  ['ice_friction', 'ice friction', ['Sticky ice from here on.', 'More grip, less glide.'], ["Director's call! The ice is now glass.", 'Slippery out there now.']],
  ['goal_pause_frames', 'goal pause', ['Longer celebrations, then.', 'More time to enjoy a goal.'], ['Short breaks after goals.', 'Quick resets after a goal.']],
  ['board_restitution', 'board bounce', ['Bouncy boards! Bank it in.', 'The boards are springy now.'], ['Dead boards now.', 'The boards soak it up.']],
  ['ai_speed_scale', 'opponent speed', ['The opposition just sped up.', 'Faster opponents. Bold call.'], ['The opposition slows down.', 'Opponents on a slower clock.']],
  ['ai_red_skill', 'opponent skill', ['Smarter opponents. Brave call.', 'The other team just got sharper.'], ['The other team gets a little sloppier.', 'Easier opponents for this take.']],
];
for (const [key, label, up, down] of DC) {
  S(`directors_call.${key}_up`, 'directors_call', key, `the Director's Call sets ${label} in the upper half of its range (Strength 0.5 or more)`, 'any', 5, 0, up);
  S(`directors_call.${key}_down`, 'directors_call', key, `the Director's Call sets ${label} in the lower half of its range (Strength under 0.5)`, 'any', 5, 0, down);
}
// Takes (periods) starting
S('period.take_1', 'period', 'take_1', 'the first take of a match starts (stoppage line, may run long)', 'any', 4, 0, ['Welcome to the sound stage: blue against orange, take one.', 'Lights are up, the ice is fresh. Take one is rolling.', 'Good evening from the stage. Let us see what this cut can do.']);
S('period.take_2', 'period', 'take_2', 'the second take starts', 'any', 4, 0, ['Take two. Same set, new chances.', 'Back for take two.', 'Second take is rolling.']);
S('period.take_3', 'period', 'take_3', 'the third take starts', 'any', 4, 0, ['Take three. This is where it gets decided.', 'Third take. No more rehearsals.', 'Final take of the match. Here we go.']);
S('period.take_n', 'period', null, 'a take after the third starts', 'any', 4, 0, ['Another take. They are not done yet.', 'Still rolling. Another take.']);
S('period.take_tied', 'period', null, 'a take after the first starts with the score tied', 'any', 3, 0, ['Level at the start of this take.', 'All square as we go again.']);
// Takes ending
S('period_end.any', 'period_end', null, 'a take ends', 'any', 4, 0, ['And that is the end of the take.', 'Cut. End of the take.', 'That take is in the can.']);
S('period_end.tied', 'period_end', null, 'a take ends with the score tied', 'any', 4, 0, ['End of the take, and still level.', 'All square at the cut.']);
S('period_end.lead', 'period_end', null, 'a take ends with one team ahead', 'any', 4, 0, ['End of the take. One side in front.', 'A lead going into the break.']);
// Clock and score states
S('clock.last_10', 'clock', 'last_10s', 'ten seconds left in a take', 'any', 4, 0, ['Ten seconds left in the take.', 'Ten seconds left!', 'Into the final ten!', 'Clock is running down!']);
S('clock.last_10_tied', 'clock', 'last_10s', 'ten seconds left and the score tied', 'any', 5, 0, ['Ten seconds, all square!', 'Tied with ten to go!', 'Somebody needs a hero!']);
S('clock.last_10_close', 'clock', 'last_10s', 'ten seconds left and the gap is one goal', 'any', 5, 0, ['Ten seconds to find an equaliser!', 'One goal in it, ten seconds left!']);
S('clock.sudden', 'clock', 'sudden_death', 'sudden death starts', 'any', 5, 0, ['Next goal wins it.', 'Sudden death. Do not blink.']);
// Win and loss (the human's team)
S('win.any', 'win', null, 'the human\'s team wins', 'any', 5, 0, ['And that is the match!', 'Victory! What a performance.', 'They pulled it off!']);
S('win.blue', 'win', null, 'blue wins the match', 'blue', 4, 0, ["And that's a wrap! Blue takes it.", 'The night belongs to blue.']);
S('win.orange', 'win', null, 'orange wins the match', 'orange', 4, 0, ["And that's a wrap! Orange takes it.", 'The night belongs to orange.']);
S('win.shutout', 'win', null, 'the human\'s team wins without conceding', 'any', 5, 0, ['A clean sheet. Not a thing got through.', 'Shutout! A spotless take.']);
S('win.comeback', 'win', null, 'the human\'s team wins after trailing by two or more', 'any', 5, 0, ['From behind to the win! What a script.', 'A comeback for the ages.']);
S('win.close', 'win', null, 'the human\'s team wins by one goal', 'any', 5, 0, ['By a single goal. Nerves of steel.', 'Close, but they got there.']);
S('lose.any', 'lose', null, 'the human\'s team loses', 'any', 5, 0, ['Not this time.', 'The other side takes it.', 'A tough night on the ice.', "We'll need a retake on that one."]);
S('lose.close', 'lose', null, 'the human\'s team loses by one goal', 'any', 5, 0, ['So close. One goal in it.', 'Just short. That one hurts.']);
S('lose.blowout', 'lose', null, 'the human\'s team loses by three or more', 'any', 5, 0, ['A night to forget. Back to practice.', 'That one needs a full rewrite.']);
// Season take intros (one line each; stoppage, may run long)
const SEASON = [
  ['The Read-Through', 'Take one of the season. The read-through. Nobody is judging yet.'],
  ['The Big Room', 'The big room tonight. More ice, and the clock is rolling.'],
  ['Rough Cut', 'Rough cut. Checks are in, manners are out.'],
  ['Set Pieces', 'Set pieces tonight. Watch the props out there.'],
  ['Rubber Boards', 'Rubber boards tonight. Expect a few strange bounces.'],
  ['One Goal', 'One goal tonight. Whoever scores takes the scene.'],
  ['Wide Shot', 'Wide shot. That far net is a long way off.'],
  ['Everything We Shot', 'The final cut of the season. Everything they have, right here.'],
];
SEASON.forEach(([title, text], i) => S(`season_take.level_${i + 1}`, 'season_take', `level_${i + 1}`, `season take ${i + 1}, "${title}", starts (before the first drop)`, 'any', 4, 0, [text]));
S('win.season', 'win', null, 'the human wins the last take of the season', 'any', 5, 0, ['That is the season. A wrap on the whole thing.']);

// ---------------- DIRECTOR ----------------
D('action', 'faceoff', 'drop', 'the puck drops at the first faceoff of a take', 5, 0, ['Action!', 'And... action!', 'Rolling. Action!']);
D('cut_goal', 'goal', null, 'a goal, at most once a minute (the pause starts)', 5, 60, ['Cut!', 'Cut! Print it.', 'Cut! Reset the puck.']);
D('from_the_top', 'goal', null, 'the other team scores, at most once a minute (alternative to cut_goal)', 5, 60, ['From the top.']);
D('take_2', 'period', 'take_2', 'the second take starts', 5, 0, ['Take two.', 'Places, everyone.']);
D('take_3', 'period', 'take_3', 'the third take starts', 5, 0, ['Take three.', 'Quiet on the set. Take three.']);
D('take_end', 'period_end', null, 'a take ends', 5, 0, ['Cut! Moving on.', 'Cut. Reset the set.']);
D('call_open', 'directors_call_open', null, 'the human opens the Director\'s Call screen', 5, 0, ['Director\'s call!', 'Your call. Change one thing.']);
D('call_apply', 'directors_call', null, 'a Director\'s Call is applied', 5, 0, ['Rolling.', 'New pages. Action!']);
D('foul', 'foul', null, 'any foul', 5, 0, ['Cut! That\'s a foul.', 'Cut! Not in my picture.']);
D('win', 'win', null, 'the human\'s team wins', 5, 0, ['That\'s a wrap!', 'Cut, print, that\'s a wrap!']);
D('lose', 'lose', null, 'the human\'s team loses', 5, 0, ['Retake?', 'Again. Retake.']);
D('menu', 'menu', null, 'the main menu opens', 5, 0, ['Quiet on the set.', 'Choose your cut.']);

// ---------------- HOCKEY FIRST (R-0021 points 2 and 3, 2026-11-12) ----------------
// The operator: "it should be hockey first theme, keeping director cut in the menu". Commentator lines that leaned on film vocabulary are replaced here by id,
// so ids, events, priorities and cooldowns stay as generated above. The director's lines keep their film words: they are menu-only now (see README).
const HOCKEY_FIRST = {
  'faceoff.after_goal.03': 'Fresh start at centre ice.',
  'goal.first.02': 'And the first one is on the board.',
  'goal.tie.03': 'Tied up! We start again.',
  'goal.blowout.02': 'Somebody call a timeout.',
  'goal.match_point.02': 'One goal from the win.',
  'goal.late.03': 'Just before the horn!',
  'goal.pause.02': 'The counter clicks over, and they line up again.',
  'goal.pause.03': 'A clean finish. That one goes on the highlights.',
  'pickup_spawn.any.02': 'A pickup drops onto the ice.',
  'effect_start.any.02': 'A new effect is in play.',
  'announce.rule.03': 'The rules just changed.',
  'directors_call.any.01': 'The rules have changed.',
  'directors_call.any.02': 'A rule change, live on the ice.',
  'directors_call.any.03': 'New rule for this period.',
  'directors_call.ice_friction_down.01': 'New rule! The ice is now glass.',
  'directors_call.ai_red_skill_down.02': 'Easier opponents for this period.',
  'period.take_1.01': 'Welcome to the rink: blue against orange, first period.',
  'period.take_1.02': 'Lights are up, the ice is fresh. First period is under way.',
  'period.take_1.03': 'Good evening from the rink. Let us see what these two can do.',
  'period.take_2.01': 'Second period. Same ice, new chances.',
  'period.take_2.02': 'Back for the second period.',
  'period.take_2.03': 'Second period is under way.',
  'period.take_3.01': 'Third period. This is where it gets decided.',
  'period.take_3.02': 'Third period. No more warm-ups.',
  'period.take_3.03': 'Final period of the match. Here we go.',
  'period.take_n.01': 'Another period. They are not done yet.',
  'period.take_n.02': 'Still going. Another period.',
  'period.take_tied.01': 'Level at the start of this period.',
  'period_end.any.01': 'And that is the end of the period.',
  'period_end.any.02': 'There is the horn. End of the period.',
  'period_end.any.03': 'That period is in the books.',
  'period_end.tied.01': 'End of the period, and still level.',
  'period_end.tied.02': 'All square at the horn.',
  'period_end.lead.01': 'End of the period. One side in front.',
  'clock.last_10.01': 'Ten seconds left in the period.',
  'win.blue.01': 'And that is the game! Blue takes it.',
  'win.orange.01': 'And that is the game! Orange takes it.',
  'win.shutout.02': 'Shutout! A spotless night.',
  'win.comeback.01': 'From behind to the win! What a finish.',
  'lose.blowout.02': 'That one will sting for a while.',
  'season_take.level_1.01': 'Match one of the season. The opener. Nobody is judging yet.',
  'season_take.level_2.01': 'The big rink tonight. More ice, and the clock is running.',
  'season_take.level_3.01': 'Rough stuff tonight. Checks are in, manners are out.',
  'season_take.level_4.01': 'Tricky ice tonight. Watch what is lying out there.',
  'season_take.level_6.01': 'One goal tonight. Whoever scores takes the game.',
  'season_take.level_7.01': 'Wide rink tonight. That far net is a long way off.',
  'season_take.level_8.01': 'The final match of the season. Everything they have, right here.',
  'win.season.01': 'That is the season. What a run it was.',
};
const seen = new Set();
for (const l of out) if (l.speaker === 'commentator' && HOCKEY_FIRST[l.id]) { l.text = HOCKEY_FIRST[l.id]; seen.add(l.id); }
for (const id of Object.keys(HOCKEY_FIRST)) if (!seen.has(id)) throw new Error('HOCKEY_FIRST: no such commentator line ' + id);

writeFileSync(process.argv[2], JSON.stringify(out, null, 2) + '\n');
console.log('wrote', out.length);
