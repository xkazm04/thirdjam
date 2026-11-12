# Bark bank review (R-0015 register, hockey first per R-0021 point 2, model claude-sonnet-5-5, generated 2026-11-12T12:50:40.384Z)

Skim every line; delete any you dislike from barks.json, then run node tools/barks/embed.mjs and node --test tools/barks/*.test.mjs.
Two sonnet runs (26 + 6 calls, $0.36): 1,582 raw lines; the filter rejected the hard list (25 + 4), over-length (28), duplicates (12),
one "ass" outside a kick-your-ass phrase and four off-world words (goalie, rent...), then capped every combination at 20 and the mild words at one in ten.
Lane A20 (R-0021 point 2, 2026-11-12 13:44): 118 lines that leaned on film vocabulary (cut, take, director, script, scene, reel...) were rewritten by hand as rink talk,
same id, situation, speaker, tone, register and mild budget; they are marked (rewritten) below. The filter now rejects director, cut, scene, script, reel, take two, take three.
Shipped: 1340 lines, 56 with a mild word (damn, hell, crap, kick your ass: 4.2%, quota 10%), 89 combinations of 12..20 lines, 38 lines with a soft film word in its hockey sense (take the puck, the lead, the frame of the net).
Tones: opponent mock / savage / complain / bravado, teammate encourage / complain / praise. Intensity: quiet = complain (opponents) and encourage/praise/complain (teammates),
normal = mock and the rest, loud = savage allowed. Regular ships normal, Raw ships loud.

## check_landed / opponent / bravado (20)
- Cute shove, now watch real hockey
- Down for a second, up for the win
- Enjoy it, my line has owned this ice since warm-up (rewritten)
- Enjoy the moment, it's the last one you'll get
- Hit all you like, the scoreboard stays ours
- I'll get up, and then we'll finish the shift (rewritten)
- Knock me down, I'll still be scoring first
- My line has seen better rookies get lost at center
- My line makes that hit look like an accident
- One check doesn't change who runs this rink (rewritten)
- One hit won't save you from a line this good
- Our line doesn't flinch, it just collects
- Take your hit, my line still owns this ice
- That's the best you've got? We're barely warm
- We make this look easy, you make it look accidental
- We'll be back up before your next lucky swing
- We're three steps ahead and you're still lacing up
- We've done this longer than you've had a stick
- You swing once, we score four, that's the math
- You won a moment, we're winning the whole period (rewritten)

## check_landed / opponent / complain (20)
- Fine, hit me, see if the scoreboard cares (rewritten)
- Great, now I have to get up off the ice again
- Great, now I'm eating ice because of that clown
- I didn't see you coming, nobody ever does
- I'm too old to be flattened by a rookie
- My knees have filed a complaint about you
- Next time I'm bringing a map of where you skate
- Of course the bounce goes his way again
- Of course the one time I look away
- Oh come on, that one was clean as mud
- Perfect, flat on the ice and the puck is gone
- Somebody tell me how that even connected
- That bounce was garbage and you know it
- That one was all luck and bad timing
- That's what I get for trusting the ice today
- This ice grabs my skates every single time
- Tired of cleaning up after your wild swings
- Typical, the ice always favors the clumsy one
- Unreal, I got hit by the guy who can't steer
- Unreal, the boards were practically on his side

## check_landed / opponent / mock (20)
- Big hero moment, shame the net is still empty
- Bold move, considering you skated the long way around
- Bold, hitting me after three minutes of hiding
- Cute, now go back to skating in circles
- Even your body check had to ask for directions
- Finally, a decision that wasn't a total disaster
- Great, one hit, and the puck is still not yours
- Look at you, using the boards as a plan
- Nice hit, did you plan it or just lose the brakes?
- Nice hit, now try that near the puck
- One hit and you skate like you invented it
- That's the first thing you've hit all night
- That's your whole highlight, all four seconds of it (rewritten)
- Thrilling stuff, maybe next you'll find the net
- Was that a check or did you trip on purpose?
- Was that a check or did you trip on the ice?
- Wow, a body check, and only three tries to find me
- You hit him like you were apologizing in advance
- You hit like a man apologising in advance
- You lined that up for ten seconds, I watched

## check_landed / opponent / savage (20)
- Cute hit, now go lose the puck again like always
- Enjoy that hit, it's the only highlight you'll get
- Enjoy that hit, it's the only thing you'll land
- Great, you can hit people and still can't score
- Hit me again, it still won't help your aim
- Hitting is easy, rookie, playing well is the hard part
- I've seen better aim from a puck sliding downhill
- Keep celebrating, I'll be scoring in ten seconds
- One lucky bump does not fix that skating
- One lucky check doesn't fix how badly you skate
- Sure, celebrate, you're still the worst skater out here
- That hit was a fluke and everyone here knows it
- That was your whole career, and it's already over
- That's the peak of your career, enjoy the view
- You couldn't hit the net with a map and a week
- You got one hit in, I got your number
- You hit like a closing door and skate like a stuck one
- You hit like that and still can't find the net
- You're a speed bump with a stick, nothing more
- Your whole game is one accident per period

## check_landed / teammate / complain (20)
- Cute check, but who's covering the net now?
- Every time you hit someone I lose the play
- Great check, shame the puck rolled the other way again.
- Great, now the whole rink is leaning your way
- Hit him all you want, the bounce still hates us.
- I asked for a pass, not a demolition
- I'd cheer, but the post just ate our best chance.
- Nice hit, pity it changed nothing on the scoreboard.
- Nice, now he's down and we're out of position
- Of course you hit him and the puck goes the other way
- Of course you hit him now, with the puck ten feet behind us.
- Sure, hit him, while I skate back for the whole play.
- The ice is slick and so is your timing
- This ice never gives us anything, not even a clean pickup.
- Typical, all that effort and the bounce hates us
- Wonderful hit, shame it bought us nothing
- Wonderful, he's down and we're still out of position.
- You flatten him and I'm the one chasing the puck, as usual.
- You hit like a wrecking ball with no map
- You knock him down and leave me the dirty work, great.

## check_landed / teammate / encourage (20)
- Do that again, but this time near the net.
- Don't admire it, grab the puck
- Don't admire it, he's done for the shift, you're not. (rewritten)
- Don't get sentimental, line up the next one.
- Fine, that worked. Now repeat it with some purpose.
- Fine, you can hit, now try skating at the same time
- Good hit, now hit the next one before he gets up
- Good, now get up the ice before the puck forgets you.
- Great, you found someone to hit. Now find the net.
- He's down, so use the space, that's the whole point.
- He's on the ice, you're not, so go go go.
- Less staring, more skating, the puck's loose.
- Nice hit. Now skate like you meant it.
- One hit doesn't make a career, so keep moving
- One hit. Now do it again, and try facing the puck.
- One lucky check doesn't make a career, keep moving.
- Stop posing and chase the pickup
- Stop posing and grab the puck, rookie.
- That was the easy part, rookie, now score
- That's the easy part, now try finishing something.

## check_landed / teammate / praise (20)
- Clean hit, I almost clapped
- Decent. Don't let it go to your skates.
- Fine, I'll admit that hit had some weight.
- Fine, that was proper hockey
- He felt that, and so did the boards
- Huh, that actually landed
- Huh. That one actually landed.
- I blinked and he was on the ice, nicely done.
- Not bad, I've seen worse from guys who meant it.
- Not bad, I've seen worse from veterans
- Not terrible, rookie, not terrible at all.
- Okay, that one I'd show the whole bench. (rewritten)
- Okay, that one I'll remember
- Solid, don't let it go to your head
- Surprised me, and that's not easy anymore.
- That was almost on purpose, wasn't it?
- That's the first thing you've done I'd call hockey.
- Took you long enough, but that was a proper check.
- Well, look at that, the rookie has a pulse.
- Well, look at that, you meant it

## check_received / opponent / bravado (13)
- Class is in session and you're the demonstration
- Every shift you take only proves our point
- Fast, mean and ready, that's our top line (rewritten)
- Our line doesn't need luck, only your open ice
- That's how a real line plays, take notes
- The scoreboard will agree with me soon
- This is the first line, and you're not on it (rewritten)
- Three of us, zero doubts, one of you down
- We hit, we score, you watch from the floor
- We play the whole rink, you play the floor part
- You can't match our pace, so you can't match our hits
- You're the warm-up cones we skate around (rewritten)
- Your line's a scrimmage, ours is the playoff (rewritten)

## check_received / opponent / complain (14)
- Can you at least fall somewhere useful?
- Crap, now I have to skate around your wreckage
- Every time I get a clean hit, it's wasted on you
- Great, you're on the ice and I'm out of position
- I did all the work and you took the fall
- Ice is rough tonight, and so is your defence
- Of course you drop right in my lane
- That puck bounced wrong and you still lost it
- This rink keeps handing me rookies, I'm worn out
- Tired of hitting someone who doesn't fight back
- Ugh, took forever to find you, you're so slow
- Why do they always fall right where I'm skating?
- You block my path even when you're flat
- Your teammates will sulk and I'll hear about it

## check_received / opponent / mock (13)
- Another rookie angle, another nap on the ice
- Bold of you to skate where the hitting happens
- Mark that fall, it's your best move all night (rewritten)
- Gravity again, huh, you two should get a room
- Great stance, shame it ended horizontal
- Nice landing, very committed to the ice
- Stay down, it's the only play you've mastered
- That's the fastest you've moved all period
- The ice called, it wants its rookie back
- Try standing up, it's the trick after falling
- Was that a dive or just your usual skating?
- You saw me coming and chose the floor anyway
- You went down like you practised it in warm-up (rewritten)

## check_received / opponent / savage (12)
- Crawl back to the bench, rookie, the net is mine
- Damn, you folded like a cheap pylon (rewritten)
- Get up, I want to do that again
- Hell, I barely touched you and you quit
- I'd say you're a speed bump, but speed bumps slow me down
- I've hit bigger threats than you in warmups
- Stay down there, the view suits your game
- That check was free, you should be embarrassed
- That fall was the best backcheck you'll ever do (rewritten)
- You bring nothing but a landing pad to this rink
- You skate like a dropped stick, and land like one (rewritten)
- You're the spare pylon nobody asked for (rewritten)

## check_received / teammate / complain (14)
- Another check, another turnover, same old show
- Damn, you saw him coming from across the ice
- Every shift, the same fall and the same excuse
- Great, now we're playing two on three again
- Hell of a way to hand them the puck
- I pass you the puck and you give it to the boards
- Of course you went down, you always lead with your face
- One job: stay upright. Just one
- That bounce was rotten, and so was your positioning
- This ice hates us, but you're not helping
- Tired of watching you decorate the ice
- Typical. The check lands and the puck goes with it
- Why do I even bother passing to you
- You skate straight at the biggest guy, every time

## check_received / teammate / encourage (14)
- Back on your skates, this is a shift, not a nap (rewritten)
- Get up and make him regret it, if you can
- Get up, the puck won't wait for your feelings
- Hit him back, or at least hit something
- Learn the lesson and get off my ice
- Look up before you skate, it's free advice
- Next time see the check coming, they're not subtle
- Quit napping and get back in the play
- Shake it off, then go hit someone back
- Skates under you, rookie, that's the whole trick
- Stop admiring the ice and stand up
- The ice is cold, so move your feet
- Up you get, we're short a skater down there
- Up. You're lying on my passing lane

## check_received / teammate / praise (14)
- Alright, you've got some nerve after all
- Didn't think you'd take that and still hold the puck
- Fine, that was a decent hit to survive
- Good fall. I've seen far worse from veterans
- Huh, you might actually belong on this ice
- Huh. You went down well, almost graceful
- Not bad, you took that one like a pro
- Not the worst way to eat a body check
- Respectable. The ice only won on points
- Surprised you didn't fold, I'll admit that
- That check should've ended you, and it didn't
- Took it on the chin and kept the line, fair enough
- You bounce well, rookie, I'll give you that
- You got up faster than I expected

## directors_call / opponent / bravado (13)
- Any rule set, same result, we win
- Change whatever you want, my line owns every version
- Class doesn't care about the settings
- Go on, tweak it, we'll still look good
- Make it faster, make it slower, we're still ahead
- Our line has seen harder rule changes than this (rewritten)
- This is our rink, whatever you call it
- We adapt faster than you can pause
- We don't need luck, just a puck and your defense
- We were winning before the change and we'll win after
- You change the rules, we change the scoreboard (rewritten)
- You touched the dial, we kept the lead
- Your new rule just gave us a better one-timer (rewritten)

## directors_call / opponent / complain (14)
- Can we play one full period without a new rule? (rewritten)
- Crap, that new setting ruined a perfectly good rush
- Every time I read the ice, you change it (rewritten)
- Fine, new rules, but the bounces still owe me
- Funny how the rules only change when you're losing
- Great, another rule change, just what this shift needed
- I was fine with the old physics, thanks
- I'm too old to relearn the rink every five minutes
- My skates just learned this ice and now it's gone
- Nothing bounces right anymore, and I blame your tweak (rewritten)
- Pausing again, really? My legs were just waking up
- This is the third change and I've barely warmed up
- Tired of chasing a puck that behaves differently each minute
- Who keeps touching the rules mid-shift?

## directors_call / opponent / mock (13)
- Bold new rule, same footwork (rewritten)
- Changing the rules won't change the skater, rookie (rewritten)
- Did the new rule include learning to aim? (rewritten)
- Fresh rules, same hesitation at the blue line
- I'd say nice try, but I watched it
- Keep editing, maybe the puck will start liking you
- Nice tweak, shame about the skater (rewritten)
- Oh good, a new rule for you to ignore
- Pause all you like, the net stays the same size
- Sure, tweak the ice, that was clearly the problem
- New rule and you still turn like a parked truck (rewritten)
- The rules changed, your skating didn't (rewritten)
- You paused to think and that's what you came up with?

## directors_call / opponent / savage (14)
- Tweak it however you like, you're still the turnover (rewritten)
- Damn, you can't even lose consistently
- Tweak the ice all you want, I'll still flatten you (rewritten)
- Every rule you pick makes my job easier
- Go ahead, change the rule, I'll still bury you (rewritten)
- Hell, the posts are embarrassed to know you
- I'll kick your ass under any setting you pick
- I've scraped better players off the boards
- Keep stalling, the boards are saving a spot for you
- New rule, same loser, same ending
- You couldn't hit the net if it hugged the puck
- You pause like a man hiding from the next check
- Your aim is the real special effect, it vanishes
- Your stick works great as a decoration

## directors_call / teammate / complain (12)
- Damn, every time I get comfortable, you touch something.
- Every pause costs me a perfectly good grudge.
- Hell of a time to change the rulebook, boss. (rewritten)
- I was open for six seconds and you called a pause.
- I've lost count of the rule changes. Not the bad passes. (rewritten)
- My stick hated the old rules and it hates these more.
- Same rookie, new physics, worse results.
- The ice felt fine until you got your hands on it.
- This rink was a mess before you started tinkering.
- Tweak, tweak, tweak. Meanwhile, nobody can hit the net.
- Who asked for that? Not my knees.
- You fix the rules, I fix your mistakes. Fair deal?

## directors_call / teammate / encourage (14)
- Adapt or sit on the boards, rookie.
- Your tweak, you asked for this. Now go use it. (rewritten)
- Eyes up, stick down, and stop drifting.
- New rule, fresh chance to miss the net. Try not to. (rewritten)
- Hit someone. The rule change won't do it for you.
- I carried worse rookies through harder seasons. Skate. (rewritten)
- Lead the play or get out of my lane.
- Less thinking, more checking. Go.
- Move your skates, the puck won't chase itself.
- New rule, same excuse. Go get the puck.
- Quit admiring the scoreboard and start skating.
- Shoot early, shoot often, and aim this time.
- The rules changed, your skating didn't. Fix that.
- You changed the rules, so now earn them.

## directors_call / teammate / praise (14)
- Better than your skating, anyway.
- Decent. Do it again and I might nod.
- Even the boards looked impressed. Barely.
- Fair enough, rookie. That one worked.
- Fine, you earned that one. Don't let it go to your head.
- Huh, the new rules suit you. Suspicious.
- Huh. That rule change actually helped.
- I'll admit it, that was a decent call.
- I've seen worse rule changes. Not many, but a few. (rewritten)
- Not bad, that tweak. Almost looked planned. (rewritten)
- Okay, I didn't hate that new rule. (rewritten)
- That's the first smart pause I've seen from you.
- Well, look at that. You can skate and think. (rewritten)
- You surprised me. Annoying, but true.

## faceoff / opponent / bravado (13)
- Class is in session, and you're sitting at the front.
- Drop it. We'll finish before you find your stick.
- Nobody on my line is worried. Look around, rookie.
- Pick any of us. You'll still be second to the puck.
- This is our ice. You're just renting the view.
- Three of us, one plan, and you're not in it.
- Watch closely. This is what a real line looks like.
- We beat you in warm-up. The rest is just the clock. (rewritten)
- We don't chase pucks. Pucks come looking for us.
- We skate in sync. You skate in circles, apologetically.
- Win the faceoff? Please. We win the game before it starts.
- You're the opening faceoff, we're the whole season. (rewritten)
- Your best play is our warm up.

## faceoff / opponent / complain (14)
- Again? We just did this, and it was dull the first time.
- Every bounce goes your way. Nobody earned it.
- Fine. Reset the shot. Let's pretend that never happened.
- Honestly, I expected more from my own line there.
- I'm tired of cleaning up after this whole line.
- I've lost to better players than you. Just not today.
- Is the net really that hard to defend for us? Apparently.
- One lucky shot and the rookie thinks he's a star.
- Someone fix this ice before my skates file a complaint.
- That last one was luck, and I'm sick of your luck.
- The posts hate me, the puck hates me, and you're smug.
- This ice is garbage and your line is worse.
- We gave that away like a free pickup. Real smooth.
- Why do I always get the bad side of this faceoff?

## faceoff / opponent / mock (12)
- Ah, the bench's favourite pylon. Blink and you're done. (rewritten)
- Another faceoff, another rookie on the wrong foot. (rewritten)
- Big breath in. Now forget everything you practised.
- Bold of you to show up with a stick like that.
- Centre ice, where your confidence goes to retire.
- I'll go easy. You'll still find a way to lose the puck.
- I've seen scoreboards with more attack in them.
- Is that your skating face or are you just thinking?
- Line up, rookie, try to face the right net this time.
- Nice stance. Shame it's the only part that moves right.
- Sure, take the faceoff. Somebody has to lose it.
- Try hitting the puck first. The net is step two.

## faceoff / opponent / savage (14)
- Crap, I almost forgot you were on the ice.
- Damn, you look scared. Good instincts, honestly.
- Drop the puck, I'm already bored of watching you lose.
- Enjoy the faceoff. It's the last time you'll touch the puck.
- Hell, I'd feel bad if you ever made it hard.
- I came for a game and got a rookie. Disappointing.
- I'll body check you clean into next season. (rewritten)
- I'll kick your ass to the boards and back, twice.
- Keep the stick up, it'll help you hide behind it.
- Stand still, that's the closest you get to defending.
- You're not a threat, you're a speed bump with a stick.
- You're the turnover, and we haven't even started. (rewritten)
- Your skating makes the ice want to melt out of shame.
- Your whole line is a bad warm-up for real hockey. (rewritten)

## faceoff / teammate / complain (14)
- Damn, the ice gave you that bounce and you still blew it
- Every faceoff starts with hope and ends with you (rewritten)
- Great, back to centre, just like old times
- Hell of a way to waste a perfectly good puck
- I'm too old to watch you miss that open net
- I've seen posts with better timing than you
- Lucky bounces skip us and find you doing nothing
- Next time pass it, the puck didn't deserve that
- Nobody dumps it like you do, and not in a good way
- Same faceoff, same sad ending, I've lost count
- Tell me again how that was the ice's fault
- That puck had a mind of its own, same as your stick
- The bounce was bad, but your skating was worse
- We keep restarting because somebody can't finish

## faceoff / teammate / encourage (14)
- Be useful for ten seconds, I'm begging you
- Body check somebody, it's the only thing you do well
- Don't think, you're bad at it, just hit someone
- Get the puck, then get it to me, in that order
- Hold the centre and try not to wave at the puck
- Just once, aim at the net instead of the boards
- Keep up, or I'll start passing to the posts
- Less drifting, more checking, we're on the clock
- Move your feet, the ice isn't getting any younger
- Skate like you mean it, rookie
- Stick on the ice, head in the game, in that order
- Stop admiring the scoreboard and go get the puck
- Second period is already looming, so wake up (rewritten)
- Win the faceoff this time, or stay out of my way

## faceoff / teammate / praise (14)
- Decent shot, I'd have said so sooner if I'd expected it
- Didn't see that coming, and neither did they
- Fine, that one was clean
- Huh, so you can find the net after all
- I've seen worse, mostly from you
- Keep that up and I might learn your name
- Look at that, the stick does more than hold your hands
- Not bad, for someone who skates like that
- Okay, that check had some mean in it
- One good play, don't make it a habit I have to explain
- That one's a highlight, on a very short list (rewritten)
- That was almost professional, don't let it go to your head
- Well, the radar finally had something worth showing
- You did the thing, I'll allow it

## goal_against / opponent / bravado (13)
- Class shows, and you just saw it.
- Get used to the sound of that horn.
- Keep up, or enjoy the view of our backs.
- Next one is already on the scoreboard. (rewritten)
- Nobody on your side can touch what we do.
- One shift, one goal, no sweat.
- That's what a real line looks like.
- This is our rink, you're just renting the ice.
- We don't chase the puck, it comes to us.
- We make it look easy because it is.
- We score, you watch, and the cycle continues.
- Write it down, rookie, we're the first line. (rewritten)
- Your line is the warm-up, ours is the game. (rewritten)

## goal_against / opponent / complain (13)
- Crap, I had a cleaner shot a minute ago and you stole it.
- Every shift it's the same, and I'm sick of it.
- Fine, it counts, but nobody enjoyed that one.
- I skated my legs off for that one tap-in.
- I'm tired of carrying this line through your mess.
- I've scored uglier goals, but not many.
- That bounce off the post nearly ruined a perfect play.
- That puck rolled like it was scared to score.
- The boards ate my first pass, so that's on them.
- This ice has a mind of its own, and so do you.
- Ugh, the ice was bad there, and the puck was worse.
- Who keeps changing these rules mid-shift? It's exhausting.
- You made that harder than it needed to be, somehow.

## goal_against / opponent / mock (13)
- Bold strategy, leaving the whole net to me.
- Cute little glide there, right past the whole play.
- I'd call that defense, but the word has standards.
- Impressive, you found the one spot the puck wasn't.
- Keep that angle, it works great for us.
- Nice effort, if effort meant standing near the puck.
- Oh, you were defending? I thought that was a scrimmage. (rewritten)
- Take your time, we only scored while you thought about it.
- That was a scoring chance, and you were the escort.
- Was that a check or a polite wave?
- You skate like the ice owes you an apology.
- You turned the wrong way with real confidence.
- Your stick is lovely. Does it ever touch the puck?

## goal_against / opponent / savage (12)
- Another goal for my collection, and you're the giveaway. (rewritten)
- Come get the puck back, if you can find it.
- Damn, I've seen cones put up a better fight.
- Hell, we didn't even need a plan against you.
- I'd kick your ass on defense, but you're not even on it.
- That goal had your name on it, and your skates were nowhere.
- That was not a defense, that was a welcome mat.
- Try hitting something that isn't your own teammate.
- We walked through you like you were a painted line.
- You blinked and the scoreboard changed. Typical.
- You got burned so fast the ice steamed.
- Your whole line is a speed bump with a stick.

## goal_against / teammate / complain (13)
- Cool, so that's how we're playing it tonight
- Damn, that was our net, not a loading dock
- Every time I turn around, it's behind us again
- Great, another puck in our net, just like last shift (rewritten)
- I carry the whole zone and you wave at the puck
- I've seen better defense from a stack of cones
- Of course it went in, nobody was home to argue
- That post should've saved it, even it gave up on us
- The ice is bad, the bounce is bad, and you're worse
- This period is cursed, and I'm blaming your skating (rewritten)
- Tired of this, we keep giving them free practice
- Unreal, we got out-skated by guys who barely tried
- Where were you? Never mind, I saw where you were

## goal_against / teammate / encourage (13)
- Back to center, and try arriving before the puck does
- Cover your man this time, it's not a hard concept
- Get the puck back, that's the whole job
- Hustle back, the shift isn't over until I say it is (rewritten)
- Less staring at the scoreboard, more skating at the puck
- Move your skates, the next one is not going to stop itself
- One mistake is fine, you're working on a collection
- Shake it off and get back on defense, rookie
- Stop admiring the replay and body check someone
- Tighten up, this is not a guided tour of our net
- Use your stick, it's not just for decoration
- Win the next pickup or get out of my lane
- You can sulk later, right now go hit somebody

## goal_against / teammate / praise (14)
- Decent stick work, even a broken clock gets lucky
- Fine, that was a decent shift, don't get cocky
- Huh, you actually checked someone, noted
- I'd clap, but my gloves are busy
- Not bad, the ice almost looked impressed
- Not terrible out there, and I'm as surprised as you
- Okay, rookie, that pickup was clean
- One good play, now try making it a habit
- Respectable, for someone who usually skates in circles
- So you can hit, who knew, keep doing it
- That was almost the right read, almost counts
- That was good, and I hate saying that out loud
- You covered back, I may have to change my notes (rewritten)
- You skated like you meant it, welcome to the job

## goal_for / opponent / bravado (14)
- Cute goal, now watch how it's done
- Don't mistake one goal for a comeback
- Enjoy the lead, it has an expiry date
- I've already got the next three goals planned
- Our line has done this a thousand times
- Our line skates circles around yours, even now
- Take your moment, I'll take the period
- That was a warmup, nothing more
- That's your one goal, we'll take the rest
- This match ends our way, it always does (rewritten)
- We score when we feel like it, you'll see
- We're better and you both know it
- You got lucky, we get paid in skill
- You scored, we'll answer before you stop smiling

## goal_for / opponent / complain (13)
- Bad ice, bad luck, same old story
- Crap ice, crap angle, crap everything
- Every single bounce goes their way, every time
- I told you to close the lane, nobody listens
- I'm tired of covering for this mess
- My legs are done with this nonsense
- Of course it went in, nothing else ever does for us
- That shouldn't have gone in, it just shouldn't
- The ice is chewed up and that puck knew it
- Typical, the human gets the lucky bounce
- Unreal, that bounce was pure garbage
- We had that one, and the puck just quit
- Who let that one through, seriously

## goal_for / opponent / mock (14)
- Bold move, standing near the net for once
- Careful, you might start thinking you can skate
- Congratulations, even a broken clock gets a goal
- Cute, the puck did all the work
- Enjoy it, your highlight list is one goal long (rewritten)
- I'd clap, but my stick is busy
- Look at you, doing the bare minimum
- Nice shot, shame about the other fifty
- One goal, and you still look lost out there
- Pretty sure the post helped you with that one
- That went in by accident, and you know it
- Was that on purpose, or did the puck get lost?
- Wow, you touched the puck and it went in
- You celebrate like the net moved by itself

## goal_for / opponent / savage (14)
- Damn, that was ugly, and somehow it counted
- Don't get comfortable, I'm coming for you next shift
- Enjoy it, because I'm about to kick your ass
- Hell, even a lucky bounce looks better than your game
- I've seen better hands on a snowman
- Next time try scoring on purpose
- One goal does not fix a whole career of bad ice
- Savor it, it's the only thing you'll hit all night
- That goal was a fluke and your skating is the proof
- That was luck wearing your jersey
- The net was open and you still looked scared
- You skate like a folding chair on wheels
- You're the filler between my real highlights (rewritten)
- Your shot was bad, the puck just took pity

## goal_for / teammate / complain (13)
- Don't get used to it, the boards did half of that.
- Every goal with you feels like a coin toss.
- Hell of a way to score, sideways and by accident.
- I did all the work and you got the credit.
- I set that up and you almost wasted it.
- I'm tired of winning ugly with you.
- Ice is chewed up and we still scored, go figure.
- Of course it goes in off the post, never off a plan.
- That bounce was the only thing on our side.
- That puck went in by pure luck, not by your aim.
- Took you long enough, my stick was getting cold.
- Typical, the bounce finally helps and it's an accident.
- You scored, and I'm still the one skating back.

## goal_for / teammate / encourage (14)
- Again, and this time aim on purpose.
- Don't celebrate, hustle back and cover your side.
- Don't frame it, just go get the next one.
- Fine, now skate like you meant it.
- Good, now do it without needing a miracle.
- Great, now find the puck before they do.
- Keep that up and we might call it a plan.
- Less staring, more skating, we're not done.
- Nice, now repeat it before you forget how.
- One goal. Try doing it again before the ice melts.
- One good shot doesn't fix the rest of your game.
- Quit grinning and get back on the ice.
- Shot's in, so stop admiring it and keep moving.
- That's one. The scoreboard wants more than one.

## goal_for / teammate / praise (14)
- Clean enough, I'll allow it.
- Decent shot, don't let it go to your skates.
- Didn't think you had that in you, rookie.
- Fine, you earned that one.
- Good, you found the net and the net was cooperative.
- Huh, so the stick does more than hold your hands.
- I take it back, a little.
- Keep doing that and I might stop sighing.
- Not bad. I've seen worse from better skaters.
- Not the worst finish I've watched today.
- Okay, that was a real shot, I noticed.
- Passable. Mark it down, it won't happen often.
- That one even looked planned.
- Well, that was almost on purpose.

## lose / opponent / bravado (13)
- Admire us from the bench, it's the best angle
- Come back when you can keep up with the puck (rewritten)
- Our line doesn't win, it simply finishes
- Our line wrote the scoreboard and you just read it (rewritten)
- Smooth, cold, finished, that's how we close a match (rewritten)
- Take it as a lesson, we do it nightly
- That's what a real line looks like, rookie
- This ice has our name on it
- Three of us, no wasted motion, no mercy
- We skate, you watch, and the scoreboard agrees
- We were the goals and you were the turnovers (rewritten)
- You brought a stick, we brought a clinic
- You were never in the play, we owned the slot (rewritten)

## lose / opponent / complain (13)
- Hell of a way to waste my evening
- I carried this match and you still got in the way (rewritten)
- I skated three miles for one lazy shot
- I've had smoother ice and smarter opponents
- Next time pick a side of the ice and stay there
- Nobody told me this was a charity match
- That bounce was nothing, you were the real problem
- That whole match felt like pushing a parked sled (rewritten)
- The ice was fine, so that's not your excuse
- Tired of dragging a rookie through every shift (rewritten)
- Wasted a whole period on someone who skates sideways
- Why does your bench keep handing you the stick (rewritten)
- Your wild shots kept ruining my clean checks

## lose / opponent / mock (13)
- Bold strategy, arriving after the puck left
- Brave of you to skate near the action, never into it
- Careful out there, the boards bruise easy and so do you
- Great hustle, shame it pointed at the wrong end
- I'd say good game, but you weren't in most of it
- I've seen cardboard cutouts cover more ice
- Nice effort, if effort were measured in wrong turns
- Third period is where you find the puck, right? (rewritten)
- That was a highlight night, for the wrong team (rewritten)
- The scoreboard asked me to stop laughing
- You passed to the ice. It caught it, at least
- You skated all that way to miss the net by a mile
- Your stick spent the night as decoration

## lose / opponent / savage (14)
- Crap, I forgot you were even out there
- Damn, you made the net look like a stranger
- Even your stick wants a trade
- Hell, I've had warmups tougher than you
- Keep skating, maybe the ice will finally let you win
- Pucks flew past you like you were a traffic cone
- That wasn't a loss, that was a demonstration
- We played keep-away and you never found the keys
- We'd kick your ass again, but you barely showed up
- You bring the effort and we take the points
- You got skated around so much I got dizzy
- You hit the boards more than the puck, congratulations
- You lost so hard the radar gave up tracking you
- Your shot had the courage of a wet sock

## lose / teammate / complain (14)
- Another loss, and the ice still takes the blame
- Cold ice, cold luck, colder passing from you
- Damn, I've carried heavier teams than this one
- Even the posts looked embarrassed for us
- Every time I looked up, you were somewhere else
- I passed you the puck and you passed it to nobody
- I've seen better defense from a stack of sticks
- Thanks for the tour of the boards, very scenic
- That bounce was bad, but your read was worse
- That was a final buzzer, and not the good kind (rewritten)
- Three skaters, one brain cell, guess who had it
- Unreal, the puck bounced everywhere but in
- We lost that one before the first faceoff, I swear
- You gave the puck away like a free sample

## lose / teammate / encourage (14)
- Again, and this time skate like you mean it
- Back on the ice, and bring some nerve this time
- Fix your angles or fix your hobby
- Get in the corners, the puck won't come to you
- Get your head up and find the net, it's not hiding
- Hit someone, anyone, it's allowed here
- Less sightseeing, more checking, rookie
- Next period, try shooting before the puck gets bored (rewritten)
- Pick up the puck first, then be brilliant
- Quit drifting and go finish a check for once
- Shoot sooner, think later, it's not a chess game
- Skate harder next match, we're not paying you to glide (rewritten)
- Stop admiring the replay and win the next one
- You've got legs, so use them for more than standing

## lose / teammate / praise (14)
- Didn't expect that pickup, color me mildly surprised
- Fair play, you earned one nod and that's it
- Fine, that was a real shot, I noticed
- Hell of a hit, shame about the rest
- Huh, you actually skated like you belonged there
- Nice stick work, kid, now do it twice
- Not terrible, and I mean that as a compliment
- Okay, you've got a pulse after all
- Respectable, and I don't say that often
- That check was decent, don't let it go to your head
- That pass found me, so the day wasn't a total loss
- That was almost a highlight, keep going
- You almost made me look good, almost
- You showed up for once, I'll allow it

## near_miss / opponent / bravado (20)
- Close shots are for those who can't finish, we finish
- Enjoy the near miss, it's the best you'll get this shift (rewritten)
- I'll show you how that shot ends, with a red light
- Keep hitting iron, we'll keep hitting the lead
- My line makes that shot look easy, yours makes it look lost
- My line makes the net sing, yours makes the post ring
- My line scores on that without even looking
- My line would have buried that, and then yawned
- Next time we'll show you where the net actually is
- Nice try, kid, my line doesn't miss from there
- Real shooters hit the twine, you hit the hardware
- That shot clanged off, and my line would've buried it
- That's the difference between us, I hit the net on purpose
- The post is the only one on your team doing its job
- The post was the closest you came to a good play
- We don't need luck, the post only helps the weak
- We hit posts on purpose, you do it by accident
- We'd have scored and skated off without a glance
- You can have the post, we'll take the net
- You rang the post, I'd have rung the scoreboard

## near_miss / opponent / complain (20)
- Another loose puck and nobody's home, great teamwork
- Another post, another reason I hate this rink
- Every bounce goes the wrong way and nobody fixes it
- Every shot rings off something, and it's always loud
- Fine, the post took it, but we keep wasting these chances
- I skated all the way back for that nonsense
- I'm tired of hitting iron and getting nothing for it
- Ice is chewed up, the bounces are garbage, and now this
- Long shift, bad ice, worse shots, I'm over it
- Nobody on this line picks up the rebound, I'm done asking
- Of course it hits the post and bounces out, typical
- Of course it rang off the post, nothing goes my way tonight
- One more inch and I'd be done for the night, but no
- One more inch and I'd have had to watch a goal, ugh
- Rattled the post and got a faceful of nothing, great
- That bounce was a coin flip and we lost it anyway
- That puck should have gone in for us and you know it
- The post keeps getting more action than my linemates
- This rink hates all of us equally and I'm tired of it
- Why do the posts always side with the human?

## near_miss / opponent / mock (20)
- Almost a goal, in the same way a nap is almost a career
- Another masterpiece, signed in ringing metal
- Bold choice, shooting at the frame instead of the hole
- Close only counts in horseshoes, and that was the post
- Even the post flinched, and it's been hit by better
- Hit the iron, huh? Even the net looked relieved
- I'd clap, but the post already did
- Nice shot, if the plan was to scare the post
- So close, and yet so very you
- Ten feet wide would've been more honest
- That post is the only one who's talking to you now
- That shot had speed and no sense of direction
- The post sends its regards and a polite refusal
- The post stopped that one, so you don't have to feel bad
- Was that a shot, or an introduction to the post?
- Wide of the net, and still the best thing you did all night
- You aimed at the net like it owed you something, then missed
- You found the post. Tell me when you find the net
- You hit the post like it owed you a pass
- You're a natural at hitting everything but the net

## near_miss / opponent / savage (20)
- Damn, you missed a net the size of a barn door
- Embarrassing, and that's the nice word for it
- Even the post wants nothing to do with you
- Hell, I almost felt sorry for you, then you shot again
- Hit the post, and you still looked bad doing it
- Hit the post? Damn, even your luck has bad aim
- I'll kick your ass to the net and you still won't score
- I've seen better aim from a puck stuck in the corner
- Keep shooting at the post, it's the only thing you can beat
- Keep shooting like that and I'll kick your ass at my leisure
- Nearly scoring is how you lose slowly
- That shot was so bad the boards flinched
- That shot was so bad the post sent it back
- That was your big moment and the post wiped its feet on it
- The post has more offense than your entire line
- The post saved you from my laughing, not from the scoreboard
- You couldn't score if I held the net open for you
- You had the whole net and picked the iron, brilliant
- You miss like it's a job and you're good at it
- You skate in, you shoot, you fail, and we all get to watch

## near_miss / teammate / complain (20)
- Another inch and I'd have something to smile about
- Another inch and I'd have stopped being tired
- Every shot rings off something, never into anything
- Great, a fancy noise and no goal
- Half the rink is posts and you hit one
- Hit the post and called it a shot, huh
- I carried that puck across the ice for the post?
- I passed you that for a bang, not a goal
- I'm too old for near misses
- I've watched that bounce ruin better shots than yours
- Of course it hit the post, nothing here goes our way
- Of course the post got in the way again
- Ringing the post, the loudest way to fail
- Tell the post it's not supposed to be there
- That bounce was as unlucky as your aim
- That ice is lying to us again
- The bounce hates us, and you're not helping
- The ice hates us today, and so do you
- This net has the posts and you found them all
- Why does the puck always pick the board?

## near_miss / teammate / encourage (20)
- Adjust your aim, the net didn't move
- Again, and this time hit something that counts
- Again, and this time hit the part with no metal
- Close counts for nothing, so make it count
- Close only counts on the scoreboard if it goes in
- Don't sulk, steal the puck and try again
- Get the rebound, don't just stand there admiring it
- Get the rebound, or are you busy?
- Good angle, bad finish, fix the finish
- Half an inch left, rookie, find it
- Post is not the net, try the other part
- Quit admiring it and get back in the play
- Same shot, two inches over, go
- Shoot sooner, the net won't wait for your feelings
- That one only needed a little courage
- The post is a hint, take the hint
- The post is not the target, the net is
- You found the iron, now find the twine
- You found the post, now find the hole next to it
- You were close, now be accurate

## near_miss / teammate / praise (20)
- Almost good, and that's a start for you
- Close, and I mean that, which surprises me
- Closer than I expected, and I expected little
- Decent power, shame about the geometry
- Fine, that was a real attempt
- Fine, that's the first aim I've respected
- Hit the post, fine, at least it was the right area
- Hm, that was nearly a shot, I'll allow it
- Huh, you found the net's neighbourhood
- Look at you, threatening the net
- Not bad, the post felt that one
- Not bad, you almost scared the post
- Not terrible, that nearly counted
- Okay, that one had some snap
- Post, but a respectable post
- That one had some real life in it
- That ping had some class, I'll allow it
- That was almost a shot, I'm impressed
- The post flinched, give yourself a moment
- You almost made me stop yawning

## praise_reply / teammate / complain (14)
- Damn ice grabbed my skates all night, so spare me
- Don't flatter me, the last shift cost us plenty
- Every bounce goes against me, and you want to chat?
- Great, the one bounce that went our way and it's yours
- I carried that play and you're handing out medals
- I've skated this shift on bad ice and worse support
- Kind words, bad timing, worse passing
- Nice one, he says, after the third lost puck
- Nice one, now where was that all game?
- Sure, nice one, shame about everything before it
- Thanks, I'd enjoy it more if we weren't losing
- The ice is rotten and you pick now to wake up
- This rink hates me and you hate the pass
- You clap now? I've been open for ten minutes

## praise_reply / teammate / encourage (14)
- Compliments don't put pucks in the net, hustle does
- Do it again, and try not to look surprised
- Don't get comfortable, the next shift is already here
- Don't thank me, get back on the play
- Fine, now do it with your eyes open
- Good. Now stay off the boards for a change
- Keep your head up, the boards are coming for you
- Less talking, more skating, rookie
- Nice one was one. Go find another
- One good play, and now you want a parade?
- Save the thank-you, skate faster
- Stop admiring it and chase the puck
- That was luck until you repeat it
- You got one. The scoreboard wants more

## praise_reply / teammate / praise (13)
- Acceptable. From you, that's practically a trophy
- Decent work. Now do it twice
- Didn't think you had that in you
- Fine, I'll admit it, that was clean
- Huh. So you can skate after all
- I've seen worse. Today, that's a compliment
- Keep that up and I might stop sighing
- Look at that, a decision that paid off
- Not bad, rookie. Don't let it go to your head
- Respectable, and I barely had to yell
- That was almost professional
- Well, the stick does work after all
- You surprised me, and nobody does that

## signal_ack_defend / teammate / complain (14)
- Back again, the ice hates me and you don't help
- Back to the net, where your mistakes pile up
- Defend, defend, you could've said that before the turnover
- Defend, he says, after losing the puck twice
- Every puck you lose ends up in my lap
- Falling back, because someone skated into a wall
- Great, now I clean up after you again
- Heading back, same mess, different shift
- I fall back every time you wander off, damn it
- I was warm up front, now I'm babysitting the posts
- Of course it's me back here, it's always me
- Sure, I'll guard the net you left wide open
- This bounce, this ice, this teammate, all terrible
- Tired of covering for that stick of yours, honestly

## signal_ack_defend / teammate / encourage (14)
- Back at the net, your turn to look busy
- Back I go, now do something useful up there
- Covering the net, so quit gifting them the open ice
- Defend, sure, now earn the rebound I'm about to make
- Defending, yes, now win it back before I get bored
- Dropping back, and you bring that puck home
- Falling back, so try keeping up for once
- Fine, I defend, you stop skating in circles
- Going back, and try not to feed them the puck
- Heading back, so check somebody for a change
- I'll hold the posts, you hold your own for a change
- I'm back, so make your next pass count
- I'm on the net, you're on the puck, try it
- Shout less, skate more, I'm already on it

## signal_ack_defend / teammate / praise (14)
- Back I go, and credit where it's due
- Defend, yes, finally a shout worth hearing
- Falling back, and for once I agree with you
- Fine, that was a veteran move, almost
- Good call, that one was actually smart
- Huh, you read that right, I'm almost impressed
- I'd have called it too, so, well done
- Not bad, that's the first useful thing you've said
- Okay, that was decent, don't let it go to your head
- Right call, I'll admit it, just this once
- Smart, I didn't think you had that in you
- Somebody's been watching the replays, good
- That's the right shout, took you long enough
- Well, look at that, the rookie has instincts

## signal_ack_pass / teammate / complain (14)
- Coming your way, damn, I hate this part
- Fine, but if you trip, I'm filing that under tradition
- Here, and spare me the stumbling this time
- Here, try not to feed it straight to the boards
- I pass, you panic, and we call it teamwork
- Passing again, I must enjoy disappointment
- Passing it, and yes, I already regret it
- Passing, because shouting at me is apparently the plan
- Passing, though I know how this shift ends (rewritten)
- Puck's yours, guess I'm the one doing the hoping
- Sending it over, hope your stick shows up too
- Sure, here, I'll hand you another chance to lose it
- Sure, I'll pass, the ice is cold and so is my hope
- Ugh, fine, it's yours, break it gently

## signal_ack_pass / teammate / encourage (14)
- Catch it this time, I'm not doing a second pass (rewritten)
- Coming your way, try looking at the puck
- Fine, puck's coming, try not to fumble it
- Heads up, you wanted it, now go earn it
- Here it comes, don't make me regret it
- Here you go, and shoot before the ice melts
- Open your stick, rookie, it's not a decoration
- Passing it, so don't skate into the boards again
- Passing now, so be somewhere useful
- Passing, so aim for the net, not the scenery
- Puck's on its way, move your skates for once
- Sending it, now skate like you meant to be there
- Taking the risk on you, so make it worth the shift (rewritten)
- You called for it, so do something with it

## signal_ack_pass / teammate / praise (14)
- Coming your way, that's the first good shout all night
- Fine, you read that one right
- Good call, that was almost smart
- Here it comes, you've stopped skating in circles, nice
- Here you go, you've been less awful lately
- Huh, you were actually open, I'll allow it
- Not bad, rookie, puck's yours
- Okay, that shout was justified, don't let it go to your head
- Passing it, you may be getting the hang of this
- Passing, guess you do look up sometimes
- Passing, you found the open ice, color me surprised
- Passing, you've earned it, barely
- Right place, right time, who would have guessed
- Sending it, your positioning has improved, shockingly

## signal_ack_space / teammate / complain (13)
- Bad bounces, bad ice, and now you want me open
- Do I get a pass or just the exercise
- Every time you yell, I have to fix your mess
- Funny how space is only my job
- Great, another sprint for a pass that won't come
- I do all the running and you get the credit
- I'm out here wearing a groove for you, rookie
- I've skated this lane ten times, and not one pass
- Open ice, as if you'll find me with that aim
- Space, space, space, and nobody ever sends the puck
- Sure, shout space, then lose the puck anyway
- This ice is slush and I'm still the one hustling
- Your calling is louder than your passing, damn it

## signal_ack_space / teammate / encourage (14)
- Don't just stand there, a pass is a free gift
- Eyes up, stick down, and try not to miss me
- Fine, I'm open, now do one thing right with it
- Hit me in stride, not in the skates
- I moved, now you move the puck, simple as that
- I'm open, so try passing before you panic again
- Look up, I'm right here and I'm not getting younger
- Make the pass before the defender finds his legs
- Open ice, rookie, now put the puck where I'm skating
- Pass it clean this time, or don't bother calling
- Quit admiring me and get the puck over here
- Shouted for space, so use it, hotshot
- Space is made, so don't waste it like the last one
- You asked, I skated, now earn it

## signal_ack_space / teammate / praise (14)
- Clean call, I might even keep skating for you
- Decent call, rookie, the old guys noticed
- Fine, I'll admit that was a good shout
- Huh, that was almost a smart call
- Not bad, you called for space and meant it
- Not terrible, and from me that's high praise (rewritten)
- Okay, that's the right read, don't let it go to your head
- Somebody's been watching film, nicely done
- Space well asked for, I'll give you that
- That's a usable instinct, keep it handy
- That's how it's done, try doing it twice
- Well, look who learned to read the ice
- You found the gap, I'm mildly impressed
- You spotted it before I did, don't tell anyone

## stall / opponent / bravado (13)
- Any time you want to lose faster, go ahead
- Class is permanent, and you are plainly temporary
- Let the puck sit. We'll still take it from you
- My linemates play better asleep than you awake
- Take your time, our lead isn't going anywhere
- This is our rink, you're just in the shot
- Watch us wake up and finish this in ten seconds
- We could nap and still outplay you three to one
- We don't circle, we wait for you to blink
- We're patient because we know how this ends
- We've already won this shift, you're just standing in it (rewritten)
- You stall, we score. It's a simple game (rewritten)
- Your whole line is a warm-up for ours

## stall / opponent / complain (13)
- Another dull stretch, and I know whose fault it is
- Do something, anything, I'm bored of my own skates
- Every time I get set, you drift off somewhere
- Hard to hustle when nobody else is even trying
- I didn't come out here to watch you wander
- I've skated more laps than plays in this stretch
- Nothing bounces right on this sheet, nothing at all
- Somebody touch the puck, I'm aging out here
- The puck stuck again. Great. Just great
- This shift is dragging and you're the anchor (rewritten)
- Tired of circling around a player who won't commit
- Why is the puck always hiding in that corner?
- Worst pace I've seen, and I've seen rough ones

## stall / opponent / mock (14)
- Ah, the famous patient wait for a mistake. Yours
- Brave stuff, standing near the puck and fearing it
- Circling again? Bold choice for a man with a stick
- Don't rush, the scoreboard loves a long dull shift (rewritten)
- Fast skater, shame it's in no direction at all
- I'd cheer, but you haven't done anything yet
- Is this a strategy or are you just lost?
- Keep gliding, maybe the puck will apologize first
- Nice loop. Do it again, I'm almost impressed
- Take your time, the puck isn't going anywhere either
- That corner has seen more action than your stick
- Wow, a corner. Nobody has ever thought of that
- You could touch the puck, it's allowed
- Your plan is working, nothing is happening at all

## stall / opponent / savage (14)
- Come get it. Oh right, you can't
- Crap skating, crap nerve, and the puck knows it
- Damn, I've seen statues forecheck harder than you
- Go on, hit me. I'll wait until you find the nerve
- Hell, even the corner wants you to leave
- I could kick your ass from the other end of the ice
- Keep circling, it's the only thing you finish
- Stand there, I'll take the puck and your dignity
- The boards have more offense than you do
- Waiting for your courage? It's still in the locker room (rewritten)
- You skate like a lost shopping cart with a stick
- You're so scared of the puck it's starting to notice
- You're the reason this shift needs a line change (rewritten)
- Your whole game fits inside that one lonely corner

## stall / teammate / complain (14)
- Bounces like that only happen when you're involved
- Damn, even the puck gave up and went to sleep
- Every shift with you is a long, quiet wait
- Everyone's circling like it's a warm-up nobody wanted (rewritten)
- Great, another shift where nothing happens (rewritten)
- I carried this team all night and you carried the water
- I've waited for a play longer than a full intermission (rewritten)
- Nobody wants the puck, and honestly I get it
- Stuck in the corner again, like the rest of this match (rewritten)
- The posts have done more work than you this shift (rewritten)
- This corner has seen more action than your stick
- This ice is slow, but you're somehow slower
- Tired of watching you skate in perfect circles
- Twenty seconds of skating and not one useful thing

## stall / teammate / encourage (13)
- Body check someone, it's the one thing that's free
- Go get the puck, it's not going to bite you
- Hit somebody, the boards are getting lonely
- Less thinking, more skating, you're bad at the first one
- Nobody scores from the bench, and you're close to it
- Pick a direction and commit, even a wrong one helps
- Shoot something, anything, I'll even clap
- Skate somewhere, anywhere, I'm not picky
- Stop admiring the ice and move your skates
- Take the corner, rookie, it's just a puck in a corner
- Wake up, the bench is watching and it's not impressed (rewritten)
- We're circling like a bad warm-up drill, break it up (rewritten)
- You wanted ice time, so use some of it

## stall / teammate / praise (14)
- Alright, you earned a nod, just the one
- Even I'd have kept that one, and I'm picky
- Fair enough, that one belongs on the first line (rewritten)
- Fine, that one counts, don't let it go to your head
- Huh, that was almost competent
- I've seen worse, and today I'm saying it kindly
- Keep that up and I'll stop sighing at you
- Nice hands, who knew you came with a pair
- Not bad, I may have blinked and missed the luck
- Okay, rookie, that was clean, don't ruin it
- That was a real play, I'm as shocked as you are
- That's the first smart thing you've done all shift (rewritten)
- Took you long enough, but it was decent
- Well, look who found the puck

## steal / opponent / bravado (12)
- Class is in session, and you're late again
- Enjoy the view from behind us, it never changes
- I could do that blindfolded and still beat you
- Pucks come to us, they run away from you
- That's what a real line does with a loose puck
- The scoreboard knows who owns this ice
- Try to keep up, it'll be educational
- We don't chase pucks, they find us
- We make this look easy because it is
- We take it, we keep it, you watch
- Your best shift is our warm-up
- Your line is the practice squad. Ours is the real thing (rewritten)

## steal / opponent / complain (14)
- Another scrappy shift, another puck rolling away
- Can we get one clean bounce before the buzzer? (rewritten)
- Cover the lane next time, you let that slide by
- Every bounce lately goes the wrong way, tired of it
- Hell of a time for the ice to go soft
- I called for it, but sure, ignore me
- I'm too old to chase loose pucks like these
- Nobody on this line reads a play anymore
- Of course the puck bounced there, it hates us
- That rebound off the boards was pure spite
- This ice is garbage, and so is that last pass
- Whoever laid that ice should have to skate on it
- Why does the puck always find the human first?
- You turned a simple play into a headache

## steal / opponent / mock (13)
- Bold strategy, handing it over before we asked
- Careful, you almost kept it for a full second
- I almost felt bad. Almost
- Nice touch, if the goal was to give it away
- Oh, you wanted that? Should have said something
- Slow as a line change with a broken bench door (rewritten)
- Stickhandling, or an interpretive dance about losing?
- Take the puck back? Sure, when you learn to carry it
- That turn took longer than most intermissions (rewritten)
- Was that a pass, or did you just lose the puck politely?
- You saw me coming and still chose that. Fascinating
- You skate like the plan is to be a spectator
- Your backcheck arrives right after the buzzer (rewritten)

## steal / opponent / savage (13)
- Damn, I thought you were fighting for it
- Even the posts are embarrassed to stand near you
- Every touch you take is a gift to my line
- Hell, I barely had to move, you just handed it over
- I'd feel sorry for you, but you keep doing it
- I've taken pucks from statues with more resistance
- Keep skating in circles, it's the only thing you finish
- Kick your ass? You'd have to show up first
- That was less a turnover, more a donation
- You couldn't hold that puck with glue and a prayer
- You got robbed by your own stick, and I just watched
- You lose pucks like it's your whole career
- Your whole game is a turnover and I'm the forecheck (rewritten)

## steal / teammate / complain (14)
- Another turnover, another long skate for the rest of us
- Damn, it was right on your stick and you still missed
- Every shift the same, I carry you and you drop it
- Great, now I'm the one skating back again
- Hell of a giveaway, I almost admired the commitment
- I'd blame the ice, but you did that one yourself
- I've seen better puck control from a broken stick
- Nice carry, shame it ended at their stick
- Of course you lost it, the ice looked too easy
- That bounce was bad, but your stick was worse
- That puck was safer in my hands, wasn't it
- The puck bounced wrong, and you bounced right along with it
- Tired of watching you donate pucks to the other side
- Wonderful, you handed it over with a bow on it

## steal / teammate / encourage (14)
- Don't hug it, just pass it or shoot it
- Don't stand there admiring it, go take it back
- Fast, rookie, faster than whatever that was
- Forget the apology and get back on defense
- Get it back, rookie, nobody's going to ask twice
- Good, you've got it, now do something with it
- Hands up, head up, and chase that puck down
- Hit him now, while he still thinks he's clever
- Move your skates, the puck won't wait for you
- Next time protect the puck like it matters
- Quit sulking and make him regret that pass
- Skate through him if you have to, the boards can wait
- Stick on the ice, rookie, it's not a decoration
- You gave it away, so go and earn it back

## steal / teammate / praise (14)
- Fine, that was a decent steal
- Good pickup, don't let it go to your head
- Huh, you actually took it clean
- I've seen worse hands on better players
- Keep that up and I might stop watching you
- Nice stick work, I'd say it twice but I won't
- Not bad, rookie, that almost looked planned
- Not terrible, and from you that's a medal
- Okay, that one I won't complain about
- That check was clean, I'll allow it
- That was almost professional, almost
- Well, look at that, the toy figure can skate
- You held on to it, I'm mildly shocked
- You took it back and nobody had to ask

## win / opponent / bravado (20)
- Call it a fluke, because my line already does
- Don't confuse a scoreboard with talent, rookie
- Don't get comfortable, my line has plenty of ice left
- Enjoy the trophy moment, we'll be back for the next match (rewritten)
- Enjoy the win, my line will remember it for you
- Hold that scoreboard tight, it won't be yours long
- My line could beat you with our sticks tied together
- Next match, we skate through you like open ice (rewritten)
- One win doesn't make you a veteran, it makes you lucky
- One win, and you act like it was earned
- Our worst night beats your best on any ice
- Rookies win a match, veterans win the season (rewritten)
- That was our mercy match, don't expect a rematch (rewritten)
- This win doesn't change who owns the ice
- We lose one and still skate circles around you
- We lost the score, not the standing
- We were warming up, you just didn't notice
- You beat us once, we'll beat you every time after
- You celebrate, we study, and next time we win
- Your line got lucky, ours gets results

## win / opponent / complain (20)
- Another match ruined by a lucky deflection (rewritten)
- Every rebound landed on your side, every single one
- Every rebound went their way, every single one
- Forty shots of effort and the post still said no
- I passed perfectly and my line forgot how to finish
- I'll be thinking about that last miss until next match (rewritten)
- I'm done with these bounces, they never bounce for me
- I'm tired of losing to guys who can't aim
- My stick feels cursed after that mess
- My stick felt like wet cardboard all night
- That last puck had no business going in
- The one skater who can't aim, and he wins anyway
- The puck keeps picking favorites, and it's never us
- This ice has hated me since the first face-off
- Three bad bounces in a row, that's not luck, that's spite
- Unreal, the ice gave you every bounce tonight
- Unreal, we outplayed them and still lose to this
- We deserved that one, and the scoreboard disagrees
- We did everything right and the scoreboard disagrees
- We played the better game and still got robbed

## win / opponent / mock (20)
- Careful, a win like that might go to your head
- Congratulations, you won by accident again
- Do you always celebrate before you touch the puck?
- Even a broken clock scores twice a day, and you scored once
- Fine, take the win, the puck probably felt sorry for you
- Good game, I mean that about everyone but you
- Great match, shame none of it was on purpose (rewritten)
- I'd clap, but the boards already did it for me
- Lovely win, the net was just standing there
- Nice trophy run, you only fell over four times
- Slow, scared, and somehow ahead, quite the combination
- Take that win home and ask your teammates how it happened
- That was a comeback? Your team did the work, you watched
- That was a win? I blinked and missed the effort
- Well, even a broken clock scores once in a while
- You found the net once, I assume it was a typo
- You skated in circles and the puck felt sorry for you
- You skated like a rumor and still got the win
- You won the way a lost puck finds the net
- Your aim was awful, luckily so was the rest of the ice

## win / opponent / savage (20)
- Crap, did you even know where the net was?
- Damn, I've seen cones with more nerve than you
- Enjoy it, rookie, it's the only scoreboard you'll ever lead
- Enjoy it, rookie, it's the only win you'll steal this week
- Every pass you made was a prayer, and a bad one
- Every time you touched the puck, it begged to leave
- I've scraped better plays off my skate blades
- Savor it, because that was pure charity on our part
- Take the win, it's the only thing you took all night
- That win has your name on it, and everyone else's work
- Three of us lost to two skaters and one decoration
- You couldn't hit a net if it body checked you
- You didn't win, your teammates carried you like luggage
- You hide behind your line like a puck behind the boards
- You skate like a dropped glove, and just as useful (rewritten)
- You won because your team carried you, we all saw it
- You're the reason the posts look so relaxed
- Your aim is a crime and your team covered for you
- Your best play was staying out of the way
- Your teammates did the work, you just wore the jersey

## win / teammate / complain (20)
- Even the posts rang out in protest at your shots.
- Every time you spun in circles, I aged a year.
- Every time you touched the puck, I aged a little.
- Half those bounces were pure luck, and none were yours.
- Half those passes went to the boards. Great teamwork.
- I called for the pass three times. Three. Hello?
- I did the checking, you did the posing.
- I did the work and you got the credit. Typical.
- I'm winning games and still feel like I lost something.
- My skates are tired and your aim is why.
- My skates are worn out from fixing your mistakes.
- Nice win. Shame I spent it covering for your turnovers.
- That puck bounced like it hated us, and it still went in.
- The posts did more work than you did out there.
- We won by accident, and the accident was me.
- We won in spite of that last period, not because of it.
- We won, and I still lost patience somewhere in the second.
- Won the game, lost a year of my life watching you.
- Won ugly, and you were most of the ugly.
- You hogged the puck like it owed you a favor.

## win / teammate / encourage (20)
- Don't get comfortable, rookie, that was mostly us.
- Don't get comfortable, rookie, the next match is harder. (rewritten)
- Enjoy it for a second, then get back on the ice.
- Fine, we won. Now earn the spot you're standing on.
- Good. Now find the net without three tries first.
- Good. Now try skating before the puck gets there.
- Keep your head up, and your stick down, for once.
- Next match, try skating toward the puck on purpose. (rewritten)
- Next match, shoot sooner. The net isn't going anywhere. (rewritten)
- One win and you're humming? Sit down and watch the replay.
- One win doesn't fix that backhand, so go practice it.
- Pass the puck. It's not a family heirloom.
- Tighten up your turns, you take corners like a lost bus.
- We won. Now do it again without the near misses.
- We won. Now do it again without the sightseeing.
- Winning is fine, but your positioning still needs work.
- Winning is the minimum. Show me something extra next time.
- You can do better, and we both know it.
- You got lucky once. Make it a habit.
- You got lucky today, so make it a habit next time.

## win / teammate / praise (20)
- Didn't think you had that shot in you.
- Fine, you earned that one. Quietly, though.
- Good match. Don't make me say it twice. (rewritten)
- Huh, the rookie can actually play a little.
- Huh. Not bad, rookie.
- Huh. You actually knew where the net was.
- I'd skate with you again. Don't let it go to your head.
- I've seen worse rookies. Slightly worse. (rewritten)
- Not a disaster. Honestly, that's high praise from me.
- Not bad. I've seen worse from players with more practice.
- Okay, I'll admit it, nice skating out there.
- Okay, that was a real play. Almost looked intentional.
- That check was decent. Do it again.
- That last goal was almost professional.
- That last shot was decent. Don't let it go to your head.
- Well, I didn't expect that, but I'll take it.
- Well, that was cleaner than your usual mess.
- You finally found the net. Good for you.
- You read that play better than I expected.
- You skated like you meant it. Noted.
