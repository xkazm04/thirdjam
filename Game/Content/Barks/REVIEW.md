# Bark bank review (model claude-haiku-5-5, trimmed to 20 lines per combination)

Skim every line; delete any you dislike from barks.json and run node tools/barks/embed.mjs, then node --test tools/barks/*.test.mjs.
Three generation runs: 1518 raw lines, 48 rejected by tools/barks/filter.mjs (22 word-list hits: cheat, dead/died, drunk, useless, money, punch, blood; 26 duplicates), 63 removed by the reviewer regex (goalies, crowd, referees, offside, penalty box, coach), surplus over 20 per combination trimmed, 1225 shipped.

## check_landed / opponent / bravado (14)
- our line never misses a shift, watch and learn
- I'm the lead in this picture, sit down
- this team plays like a sold out premiere
- every check I throw is a headline
- we're the cast everyone wants on the poster
- steady hands, steady nerves, steady wins
- I hit harder than your highlight reel
- our bench is the best seat in the building
- we write the ending, you just read the lines
- a clean cut from us every single time
- I'm the one they cast when it matters
- our top line is the feature, the rest is trailers
- I don't miss my mark, ever
- we're the blockbuster, you're the matinee

## check_landed / opponent / complain (13)
- the ice is greasy, that is not a real hit
- he slipped, the ice tripped him, obviously
- unlucky, the boards jumped right in the way
- who sanded this rink, it's a trap
- I was set up, the lighting was wrong
- my skate caught a bad bounce, that's all
- the whistle should have blown ages ago
- they always let the big ones get away with it
- that check was cheap and the replay proves it
- the rink hates me, I can feel it
- not fair, the puck wandered over on purpose
- I want a rematch, this cut was rigged
- the boards moved, I'd swear it on my stick

## check_landed / opponent / mock (13)
- nice hit, did you rehearse that one
- that check had the subtlety of a dropped anvil
- you hit him like a door slamming in a cheap film
- bold move, shame the script wanted a clean play
- wow, a whole scene just to knock him over
- careful, you almost looked like you meant that
- great hit, now do it again without the fanfare
- he's still doing the stunt walk back to the bench
- that was a dramatic exit for someone who just stood there
- you call that a check or a sneeze
- a slow motion replay would be kinder to you
- bravo, now get back on your mark
- that fall was better acted than your entire shift

## check_landed / teammate / complain (13)
- the ice is so slick today, unbelievable
- did that bounce have to happen now
- the puck always finds the worst spot
- who greased the boards out there
- my stick just gets tangled every shift
- this rink is a total mess tonight
- the lighting makes everything blurry
- you always hit like a freight train
- of course the puck took that funny hop
- the skates feel wobbly, not my fault
- the whole play was rigged from the start
- why does the boards always get in the way
- that wasn't even a real hit, come on

## check_landed / teammate / encourage (13)
- shake it off, you're still in the scene
- good hit, now get back in the play
- that's just a bump, get your skates under you
- back on your feet, we need you
- stay with it, the next shift is yours
- you got this, take a breath and go
- down but not out, get up and skate
- one bad bounce, that's all it was
- pick it up, we're counting on you
- still in this, let's roll
- get up, the puck still needs you
- brush it off, we're rolling again
- dust off and drive it back up the ice

## check_landed / teammate / praise (13)
- that hit was clean, great timing
- nice check, you flattened them
- wow, that was a perfect body check
- you cut right through their defense
- that's how you take the play back
- great read, you shut that down
- you just rewrote the scene, nice
- now that's a director's cut moment
- textbook check, couldn't have been better
- you sent them flying, impressive
- that was a power move, well done
- you own that corner now
- smooth hit, you read that perfectly

## check_received / opponent / bravado (14)
- we run this rink, the scoreboard agrees
- my line hits harder than your whole bench
- I'm the reason this cut is worth watching
- three checks, three skaters on the ice
- our defense never takes a day off
- watch us finish, we always close scenes
- I hit like a closing credits theme
- this line is the star of the show
- we don't rehearse, we just win
- my stick is the sharpest on the set
- our team wins the final take every time
- I'm the headliner, you're the extra
- we command the ice, so take notes
- one more shift and this game is ours

## check_received / opponent / complain (13)
- the ice is slick and I'm not even mad
- that bounce was rigged, obviously
- my skate caught a ridge out of nowhere
- why does the puck always hop right then
- the lights glared at me during that play
- the boards reached out and grabbed me
- that was not even a real hit, honest
- my edge got stuck on a bad patch
- somebody sprinkled wax on this rink
- the wind swirled right when I checked
- I swear the net shifted by an inch
- the timing buzzer went off too early
- this rink hates me, I can feel it

## check_received / opponent / mock (13)
- that check was a little too personal, rookie
- down you go, nice view of the ice
- the ice missed you, I did not
- you went down like a dropped prop
- stay down, the scene is still rolling
- bold move, falling in the spotlight
- that landing needs a second take
- I've seen better falls in the blooper reel
- careful, the ice is ticklish today
- you're the reason we shoot twice
- nice dive, want a pillow for that
- keep the stick, lose the balance
- that's a wrap on your footwork

## check_received / teammate / complain (12)
- the ice is slick and somebody greased it
- unbelievable, the puck bounced off your shin
- did you see that, the boards hit back
- my skates are ruined by that scrape
- the lights in this arena are blinding me
- that bounce hates us personally today
- why does the ice always pick on you
- ugh, the net moved again during that whistle
- this scene is rigged against our line
- somebody left a crack in the glass right there
- my stick snapped in sympathy with you
- this game keeps booting us off the rink

## check_received / teammate / encourage (13)
- get up, we need you on this shift
- up you go, the puck is still ours
- shake it off, that check was loud not real
- one more lap, the cut is not over yet
- stay down and the director yells cut
- grab your stick, we still have a power play
- they hit you once, now hit them back
- breathe, stand up, we are still in this
- that was a bump, not a bruise, go
- the net is still open, get moving
- lift the stick, the next take starts now
- you got this, one clean pass and we tie it
- back on your feet, the scene needs you

## check_received / teammate / praise (13)
- nice stick work, that pass was pure poetry
- clean dangle, the defense never saw it coming
- that backhand was a perfect close up shot
- you read that play like a script
- that spin was the best take of the period
- flawless setup, you gave me an open lane
- you made that look easy, director style
- great vision, that pass hit my tape exactly
- that shot was pure cinema, print it
- you stole that puck like it was yours
- beautiful give and go, you owned that cut
- those hands are special, keep rolling
- that was a highlight reel, nice work

## directors_call / opponent / bravado (19)
- our line adjusts faster than any director can edit
- we read this new cut like a script we wrote
- give us the rink and we'll make it our stage
- three skaters, one plan, zero fear of a rewrite
- change the ice all you want, we already know the ending
- our passes stay sharp even when the physics shift
- we thrive on bad cuts and better shots
- the slower ice just gives our hands more time
- every new rule is another chance to shine for us
- we are the main feature and the cut proves it
- adjust the friction, we will still skate circles around you
- our team looks sharpest when the director gets nervous
- we turned your edit into our highlight reel
- our line skates like the rink was built for us
- faster ice just means more highlight reel for us
- we were made for this cut, watch us shine
- bigger rink, bigger stage, bigger finish
- I read the new rules and smiled
- our stick work is the feature film

## directors_call / opponent / complain (20)
- the ice got slicker and somehow it's my fault
- who approved this friction, because it's not fair
- every time the director pauses, my legs get cursed
- the new rink is tiny and my stick is still huge
- I was set up for a shot and the rules moved
- the puck is heavier now and nobody warned me
- why does the cut always favour the one with the remote
- I lost my footing before the whistle even blew
- the ice keeps changing and my line never does
- every rule shift lands right on my helmet
- I swear the boards moved closer just for me
- this ice is cursed, who greased the rink
- faster ice, slower feet, perfect
- the bounce hates me personally
- why does the rink always shrink on us
- every cut lands on our side somehow
- that shot power is a joke, nobody can aim
- the rules changed mid shift, not fair
- I had the puck and the ice ate it
- this friction is wild, my skates are gone

## directors_call / opponent / mock (18)
- cut that rink again and you still skate like it's wet cement
- the director moved the ice and forgot to move your feet
- rule change and still the same slow tape to the puck
- you rewrote the physics and kept the bad angles
- new friction, same sad slide into the boards
- the rink shrank and so did your confidence
- bold edit, shame the footage still shows you lost
- you changed the cut but not the ending
- that power boost went straight into the glass
- faster ice for you, still a backward turn
- you made the rink smaller, now hide from me
- good cut, bad performance, same final score
- the director changed the ice and still cannot skate
- new cut, same result: you are still losing
- rink shrank and your aim still did not
- nice rule change, shame it does not help you
- you asked for faster ice and still fell down
- the director blinked and you still missed

## directors_call / teammate / complain (20)
- who approved this ice, honestly
- the puck is sliding like it's on vacation
- that bounce hated us personally
- great, the rink got longer and slower
- my skates have no grip and no mercy
- the shot power went up and so did my doubt
- why does the ice keep changing its mind
- I was set for a sprint, not a slide
- this cut is making everything feel wrong
- the luck just went to the other bench
- I'd complain louder if I could stop sliding
- whose idea was this rink, anyway
- the new rules are a bit too dramatic
- I'm all for drama, not for this
- this ice is greased, somebody tell the director
- the puck hates me, I swear it moved
- my skates have no grip, none at all
- that bounce came out of nowhere, unbelievable
- I'm sliding into the boards like a dropped prop
- the rink got bigger, now I'm running a marathon

## directors_call / teammate / encourage (20)
- the ice changed, not your nerve
- new cut, same captain, let's go
- shake it off, we still have the puck
- you read this rink better than anyone
- adjust, skate, we're still in this
- one bad bounce is not the whole film
- stay calm, the new rules favor patience
- hey, we have done this before, go
- keep your feet moving and trust them
- that's a rough take, now take the next one
- you've got this, watch the boards
- the new friction is ours to figure out
- breathe, then press them hard
- we're with you, make it count
- keep your head up, the next shift is ours
- shake it off, we have the legs for this
- new cut, same us, let's go get it
- skate it off, rookie, the ice still likes you
- eyes up, pass to me and watch it work
- we adjust, we adapt, we score

## directors_call / teammate / praise (19)
- that pass was pure director's vision
- you cut that play like a pro
- gorgeous shot, the camera loved it
- you read the new ice perfectly
- that's a scene stealer right there
- clean stick work, very well directed
- you turned that chaos into a highlight
- textbook move, print that one
- you made the new rules look easy
- that save was pure cinema
- smooth as a final cut, great finish
- you just stole the scene from them
- that deke deserves a standing ovation
- that move was pure highlight reel material
- nice read, you saw that gap before anyone
- clean finish, that was pure instinct
- you adapted faster than the new cut did
- that was sharp, hard to believe you did it
- slick hands, that pass was perfect

## faceoff / opponent / bravado (14)
- we're the cast nobody can replace
- our line is the main feature tonight
- this is our rink, sit down and watch
- we don't rehearse, we just score
- my line plays like a sold out premiere
- we own this ice, every inch of it
- give us the spotlight, we'll steal it
- nobody out-skates this three-man crew
- I'm the star and the stunt double both
- we write the ending, you just watch it
- our passing could win an award
- every shift here is a headline
- we don't miss our marks, ever
- I came to play and to take the lead

## faceoff / opponent / complain (12)
- the ice is slick, the lights are wrong
- that bounce was rigged, I'm sure of it
- the puck hit a rut, that's not fair
- my skates feel like they're on the wrong set
- the boards moved, I swear they did
- who sets the ice this slippery anyway
- luck is a bad co-star today
- that deflection came from nowhere, unfair
- the lighting on this rink is blinding me
- the puck is cursed, I think
- another stick break, the gear is cheap
- the camera angle hid that hit

## faceoff / opponent / mock (13)
- nice stick, shame about the aim
- that's a wrap, rookie
- you skated like a deleted scene
- your passing plan was a blooper reel
- cut! that one goes on the floor
- bold move, shame the script said no
- you just starred in your own flop
- was that your stunt double skating
- take the dolly back to the studio
- you'd make a great extra in this game
- director's cut: you're the outtake
- nice try, that one's a bloopers reel
- you blinked and the puck left the set

## faceoff / teammate / complain (13)
- this ice is slower than my patience
- that bounce had a personal grudge
- the puck hates me personally today
- my stick is cursed, I swear it
- who waxed this ice, it's sticky
- unbelievable, the net moved again
- the puck has a flight plan against us
- that goal was a lucky dog bounce
- why does every bounce go sideways
- this rink tilts toward their bench
- I was open, the pass was not
- my skates have feelings about this ice
- fine, blame the ice like always

## faceoff / teammate / encourage (14)
- you've got this, stay low and steal it
- one clean win here, let's take it
- keep your feet moving, we're with you
- pucks bounce for those who chase them
- watch the blue line, we'll feed you
- hold the stick steady and stay sharp
- stay calm, we run this faceoff together
- next shift is ours, just keep moving
- breathe, pick your spot, then strike
- trust your edges, the ice is ours
- you're the best pick in this lineup
- dig in, we've got your back out here
- clean ice ahead, go make some noise
- play it smart and we'll finish it

## faceoff / teammate / praise (12)
- that spin move was pure cinema
- you read that pass before it existed
- textbook steal, now that's directing
- absolute wizardry with that release
- you own this ice, keep rolling
- perfect angle, that was a masterpiece
- sharp stick work, the aim was brutal
- that was a scene stealer right there
- you made that look effortless
- that goal was poetry on skates
- crisp passing, the lane was yours
- you're stealing the whole show

## goal_against / opponent / bravado (13)
- our top line is the best cut in town
- we've got hot hands and a cold glare
- this team blocks shots like a fortress
- I skate faster than my own reflection
- every shift is a sold out premiere for us
- call me the lead, steady as a long take
- our blue line is a wall nobody scores through
- the scoreboard will catch up, it always does
- three of us, one plan, zero doubts
- we run a shift like a well oiled machine
- the replay shows us winning every time
- our passing lanes are wider than the rink
- give me the puck and watch the highlight reel

## goal_against / opponent / complain (13)
- the ice is slanted and nobody told me
- that bounce was rigged against my blades
- the puck hit a seam I swear it saw
- my skates squeaked and that threw my timing
- that net moved, I'm putting it on record
- the boards grabbed my stick like a fan
- the lighting in here hides every puck
- why does the puck always find the crease
- that deflection came off a ghost, I'm sure
- this ice is too thin for proper cutting
- the lamp above us blinked on my shot
- I'd have scored if the seats stayed still
- unlucky again, the universe hates my wrists

## goal_against / opponent / mock (13)
- you skated like a rumor that got cancelled
- was that a pass or a sneeze
- even the puck wanted out of your zone
- bold of you to call that a screen
- you held that stick like a rental
- the replay will be kinder to me
- that save was slower than your reaction
- bad lighting, rookie, bad lighting
- rookie energy, veteran stumble
- skate to the puck, not the bench
- I'd ask for a retake but you blew it
- your stick work needs a script rewrite
- you chased that like a lost prop

## goal_against / teammate / complain (12)
- that bounce was cursed, the rink hates us
- the puck hit the boards and chose them
- my skates are slower than my patience
- the lighting is making me dizzy
- the net shrank, I swear it did
- that deflection came from nowhere, rude
- the ice is greasy, I keep slipping
- unbelievable luck, the puck loves them
- rewind that, the bounce was fake
- I'd call a reshoot if I could
- who wrote this script, it's rigged
- my stick is cursed, I need a new one

## goal_against / teammate / encourage (14)
- shake it off, we still have the cut
- one goal down, the scene is not over
- reset your feet and find your lane
- keep your head up, they're rattled too
- next shift is ours, stay with me
- still time to rewrite this ending
- breathe, then pressure their defense
- good shifts coming, stay sharp
- we roll again, same energy
- this is a long take, plenty of footage left
- wipe the frost off your visor and go
- they got lucky, we get the rematch
- back to work, the director wants a comeback
- we score next, I promise

## goal_against / teammate / praise (13)
- nice read, you saw that lane coming
- that pass was clean, perfect timing
- you froze their defense with one feint
- sharp work, that's a highlight reel
- that check was beautiful, you're on fire
- wow, you steal the puck like a pro
- that shot had real velocity, good aim
- you read their line change like a book
- brilliant footwork, do that again
- quick hands, you're making this look easy
- great vision, you found the open man
- that's the cut we needed, well done
- you carried that shift, thank you

## goal_for / opponent / bravado (20)
- our top line never misses a cue
- we do the scoring here, sit down and watch
- one more goal and this cut is a wrap
- my stick is hotter than the rink lights
- we have the best closer on this ice
- our defence shuts doors like a pro director
- we are the main feature, not the trailer
- one goal does not rewrite the script for us
- our line plays like it owns the set
- watch us hit the next beat, same energy
- we pull ahead and keep rolling, no mercy
- my hands are the reason this rink has lights
- our squad never blinks when the spotlight hits
- we came to win the final cut, not the warm up
- our line cuts cleaner than anyone on this ice
- my team plays like the final take of a blockbuster
- we don't miss twice, that's the whole script
- I was born to finish, that goal was typed in
- our passing looks rehearsed because it is perfect
- this line is the director's favorite cast

## goal_for / opponent / complain (20)
- that bounce was rigged, the ice hates us
- the puck jumped right into the net, unfair
- my stick snapped, that's why I'm sulking
- the boards pushed that puck, I saw it
- lighting on this rink is blinding, I swear
- that deflection came off a skate, not fair
- why does the wind only help the other side
- I had the angle, the net just moved
- the zamboni should have resurfaced slower
- that was a cheap goal and I'm not over it
- the puck hit a pebble, nothing else explains it
- this ice is slower on our side, I'm sure
- the ice was wet on purpose, I can feel it
- that bounce came from nowhere, unfair scene
- my stick snapped right when I needed it
- who wrote this luck, I want a rewrite
- the board moved, I swear it moved
- every puck hits my skate tonight, come on
- I was set, the ice just slid out from under me
- that deflection was pure coincidence, not a play

## goal_for / opponent / mock (20)
- that goal was a rerun, rookie, we saw it coming
- nice finish, shame the net had to save it
- did you practise that or did it just happen
- bold move, sloppy follow through though
- you look thrilled for a lucky bounce
- that was a fluke with a good haircut
- you scored? someone check the replay
- smooth hands, shaky plan, a rare combo
- enjoy it, director, the edit comes later
- that goal gets a one star review from me
- that goal was a gift, you just unwrapped it
- nice finish, shame it was on our side of the ice
- lucky bounce, want me to reshoot the scene
- that shot had the aim of a sleepy cameraman
- you scored, now please stop doing that little dance
- good hands, shame the rest of you wasn't in frame
- that was the director's luck, not your skill
- you finally found the net, want a map next
- pretty goal, zero chance of a repeat take
- you call that a finish, we call it a rehearsal

## goal_for / teammate / complain (20)
- that bounce was pure ice spite
- unbelievable, the puck just stopped
- my skates slipped right at the wrong time
- lucky shot, the net moved, I swear
- who keeps painting the ice this slick
- that deflection came out of nowhere
- my stick blade snapped on the pass
- that ice is slower than my patience
- the crossbar owes me one
- fine, the puck found you, good luck
- my visor fogged right before that
- the ice was slick, that was not my fault
- the puck took a bad bounce, pure luck
- who painted these boards, they are sticky
- my skate slipped, the ice is cursed today
- that bounce hated me, I swear
- the stick is too short for this rink
- why does the puck always ride the boards
- the camera angle hid my good move
- the lights are glaring, I could not see

## goal_for / teammate / encourage (20)
- shake it off, we're still rolling
- you're the reason this cut works
- again, same energy, one more
- keep your stick low and your head up
- big moment, now stay locked in
- reset your edges, we need another
- you've got the puck, now own it
- they're rattled, press the advantage
- stay sharp, the next shift is ours
- breathe, then get back in the play
- that's the spirit, keep the pressure on
- we're in this, let's make it louder
- fresh legs, let's go finish this
- shake it off, the next one is ours
- keep your head up, we are just warming up
- nice bounce back, go again
- you have the feel now, stay on it
- one more cut like that
- eyes up, the next puck is yours
- we are right there, keep pushing

## goal_for / teammate / praise (20)
- that finish was pure highlight reel
- clean pass, clean shot, clean goal
- you read that play before it happened
- nice stick, and the aim was perfect
- textbook move, you ran that cut
- you made that look way too easy
- smooth hands, that was pure skill
- that's a goal you'll replay all night
- you turned that defense into a cameo
- perfect timing on that one-timer
- that was a director's cut-worthy finish
- you owned that corner like a pro
- great vision, you found the open lane
- that shot was a masterpiece, rookie
- that finish was pure director level stuff
- clean cut, you made that look easy
- perfect timing, the net never saw it coming
- that was a masterpiece, print it
- sharp hands, sharp eyes, sharp finish
- that shot had real style

## lose / opponent / bravado (20)
- we run this rink, every shift
- my line is the main event tonight
- I win the close ones, always have
- our top line is a headline act
- watch us, we set the pace from here
- this team has the best timing on ice
- I am the sharpest blade in this building
- we own the third period, every time
- our passing plays like a sold out show
- I read the play before it starts
- give us the puck and we finish the story
- my stick does the talking, and it talks loud
- we are the cast everyone wants back
- next match we rewrite the whole ending
- our line is the cut everyone wants to watch
- we skate like we own the final edit
- three skaters, one script, zero wasted shifts
- I'm the lead role and I never miss my mark
- our passing was a masterpiece, sorry about the score
- we rehearse harder than anyone on this ice

## lose / opponent / complain (20)
- that bounce was scripted against us
- the ice is slick today, not my fault
- the puck hit a rut, that's a robbery
- lucky stick, lucky break, nothing more
- they got the rebound by pure accident
- the boards sprang the wrong way on purpose
- the lights blinded me for that whole shift
- the net shifted right as I shot
- the puck came off a skate, that doesn't count
- the rink is tilted toward their bench
- I was held, nobody saw it but me
- the wind in this arena is unfair
- the ice was sticky and nobody warned me
- that bounce was rigged, I saw it clearly
- my skates never betrayed me until today
- the puck hit a rut, it's obviously the rink
- somebody sanded the boards before my shift
- that gust off the ice was pure bad luck
- the lights were in my eyes all period
- my stick was fine until that deflection

## lose / opponent / mock (20)
- your passes went on a longer vacation than you did
- cut! print that one, the blooper reel needs it
- take two, same ending for you
- even the puck ducked your shot
- you rushed that like the director yelled action early
- maybe try the other end of the rink next time
- your footwork needs a rewrite
- nice effort, wrong script
- did you practice that move or just hope
- you're the lead role in our highlight reel
- wrap party is over, director, pack your sticks
- great hustle, shame the net had no idea
- your edit was shorter than your lead
- you scripted that loss all by yourself
- even the boards dodged that shot
- was that the plot or just your skating
- your playbook has one page and it's blank
- bold script, terrible casting for you
- who wrote this, a puck with a plan
- rewind it, we still won the rematch

## lose / teammate / complain (20)
- the ice was sliding like a dodgy set
- who painted the boards so slippery
- the puck hit a rut and jumped away
- we got robbed by a lucky deflection
- the net must have shrunk during play
- the ice is faster than my patience
- that goal had no business going in
- the lights were in my eyes all match
- who keeps moving the blue line
- the puck has a grudge against us
- that ice was slower than a rewind
- who painted this rink, it's all wrong
- the bounce hated us, plain and simple
- that puck had a mind of its own
- unreal, the post just laughed at us
- we got robbed by the lights, obviously
- that check came out of nowhere, nowhere
- why does the puck always skip for them
- the ice got greasy right when we needed it
- that bounce was scripted, I know it

## lose / teammate / encourage (20)
- one bad period is not the whole film
- shake it off, we reshoot tomorrow
- you still got the good stick, rookie
- keep your head up, the next cut is ours
- we lost the match, not the story
- tough loss, but you were our best take
- chin up, we have the second act
- that was a rough scene, let's fix it
- they got lucky, we got better
- back on the ice, same crew, new plan
- every director loses a cut or two
- you held the line, that counts
- okay, reset, we run it again
- heads up, we are still in this series
- shake it off, we still have the next cut
- heads up, rookie, the next take is ours
- we lost this one, not the whole picture
- reset your stick and get back in the scene
- one bad take does not sink the film
- grab a breath, we are rolling again soon

## lose / teammate / praise (20)
- that backcheck was pure director's cut
- your stick work was the best scene
- the save you made was the highlight reel
- nice shift, you carried the whole line
- that deke was pure cinema, rookie
- you kept us alive with that defense
- best pass of the match, no question
- your speed made their wings look slow
- that bodycheck was a perfect camera shot
- you owned the faceoff circle all game
- the goal you set up was a masterpiece
- sharp hands, sharp eyes, sharp play
- you were the star of this cut, rookie
- that deke was pure director's cut magic
- you carried the whole line tonight
- textbook pickup, you made it look easy
- nobody else saw that lane but you
- your backhand was the best scene today
- clean check, the other guy never got up
- you read that play like a pro

## near_miss / opponent / bravado (14)
- our line is just warming up
- we run this ice, not you
- my stick never misses a cue
- watch us script the next goal
- three of us, one clean story
- I am the lead in this cast
- our passes write the better plot
- the top shelf has our name on it
- we don't miss second takes
- our line is the main event
- you can't handle this much skill
- I'm the star of this rink
- next shot is a sure thing, rookie
- our team owns every cut tonight

## near_miss / opponent / complain (13)
- that post jumped in front of it
- this ice is cursed, I swear
- bad bounce, total rigged bounce
- the boards are on their side, obviously
- that puck hates me personally
- unbelievable, the net moved right there
- the ice is slanted, I'm sure of it
- that was a robbery by the wood
- who builds posts this close anyway
- zero luck, all bounce, no justice
- the puck bounced off my frustration
- that rebound had a grudge against me
- the rink is clearly tilted today

## near_miss / opponent / mock (13)
- that post had more accuracy than you
- wow, the boards got the assist there
- you almost scored, almost counts for nothing
- nice shot, the net just wasn't invited
- aim for the net next time, director
- the post is the only one cheering you
- close only counts in horseshoes, rookie
- that puck took one look at you and left
- bold cut, terrible framing
- your shot needs a stunt double
- you missed by an inch, the post is smug
- is that a shot or a suggestion
- you hit metal and still lost the scene

## near_miss / teammate / complain (14)
- that post has it out for us
- the ice is sticky, I swear it
- unbelievable, the boards grabbed that one
- who painted this net so skinny
- the bounce hates us today
- that puck has a mind of its own
- every rebound is cursed by this rink
- I was sure that was going in
- the wood of this rink is rigged
- bad luck, not bad hands, promise
- the net is shrinking, I'm sure of it
- that puck took a wrong turn
- clearly the post is on their side
- the frame needs a director's note

## near_miss / teammate / encourage (14)
- that post owes you one, keep firing
- close enough to count, next one's ours
- shake it off, the net is still there
- that was almost it, go again
- good shot, the puck just wants a rematch
- keep that wrist loose, it'll go in
- we're getting closer, stay on it
- post got lucky, you didn't miss by much
- pick it up, we're still in this scene
- same shot again, the net will move
- breathe, reset, the goal is coming
- one more look like that and it's in
- don't sulk, that was the right idea
- the boards got in the way, not you

## near_miss / teammate / praise (14)
- great shot, the post just got lucky
- that snap was pure poetry on ice
- you read that angle perfectly
- gorgeous release, shame about the wood
- nice aim, the post was in the way
- that was a shot worth the replay
- you put that right where it belongs
- sharp shooting, the boards stole it
- your timing on that was spot on
- that's the kind of shot I love to see
- clean wrist work, the net just dodged
- best shot of the period so far
- you had that net beat, great effort
- smooth shot, that frame got lucky

## praise_reply / teammate / complain (19)
- the ice is slicker than my confidence
- that bounce had it out for me
- my stick chose the wrong moment
- that rebound was personal, I swear
- the boards moved, I'm sure of it
- the lighting in here is ruining my shot
- we'd have scored if the net were bigger
- this rink's cold, my fingers are gone
- the puck jumped right off my blade
- my skates got an unfair kick off the wall
- the ice is slick and nobody warned me
- my stick turned into a noodle mid play
- the puck has a grudge against my blade
- why does the boards always win
- the lighting on this rink is terrible
- that deflection came straight out of nowhere
- my skates are fighting me today
- the puck jumped like it had somewhere better to be
- unbelievable luck, truly a cursed sheet

## praise_reply / teammate / encourage (19)
- keep your stick on the puck, director
- next cut is yours, go get it
- one good shift and we own this ice
- shake it off, we still have the frame
- you saw the lane, now take it
- reset your feet and run it back
- they don't see you coming, use that
- stay sharp, this scene is far from over
- we need your hands on the next play
- breathe, then go find the gap
- you made the play once, do it again
- keep the pressure on, they're tiring
- our top line is ready when you are
- shake it off, the next one is yours
- one good shift and we are back in it
- that was a warm up, the real cut starts now
- stay with me, we have plenty of ice left
- you have the touch, trust it again
- reset your feet and go find the puck

## praise_reply / teammate / praise (19)
- that move was pure director's cut
- clean finish, the whole rink saw it
- sharp hands, sharper eye, nice work
- you made that look easy, cheers
- that pass landed like it was rehearsed
- perfect timing, you stole the scene
- brilliant read, you own this ice
- that was a highlight reel moment
- you put that top corner on a plate
- smooth as glass, what a finish
- you turned that into art out there
- best shift you've had, keep it rolling
- that move was pure cinema, director
- flawless read, you saw the lane early
- clean finish, the net never had a chance
- you carried that whole shift on your own
- sharp stick work, that was a masterclass
- perfect timing on that pickup
- you turned that rebound into gold

## signal_ack_defend / teammate / complain (13)
- the ice is greased tonight, unbelievable
- that bounce hated us specifically
- their stick found the only gap
- the puck has a grudge against me
- who put that rut in the crease
- my skates just slid for no reason
- that deflection came from nowhere
- unlucky, the net jumped into it
- this rink is faster than it looks
- the boards ate that pass whole
- the puck took the scenic route
- that wobble was pure bad luck
- the ice keeps changing its mind

## signal_ack_defend / teammate / encourage (14)
- hold the line, we've got this
- stay low, the net needs you
- good read, keep that stick down
- breathe, we're square on the play
- you're the wall, stay wide
- eyes up, they're coming through
- steady feet, nobody gets past
- tight gap, make them earn it
- we ride this one out together
- slow it down, make them cut back
- that's the spot, sit right there
- one more block and we're golden
- lean in, the puck's ours
- calm hands, clean clear coming

## signal_ack_defend / teammate / praise (14)
- that sweep was textbook, nice work
- you read that play before it started
- great stick check, that stopped them cold
- perfect positioning, the net thanks you
- clean block, you're a brick wall
- that poke check was pure instinct
- you shut that lane down fast
- slick footwork, they never saw you
- that save was worth the price of admission
- you kept your cool and held the crease
- sharp reads, director's cut worthy
- nice lift on that pass, well timed
- you own the crease right now
- that's the defense we needed, well done

## signal_ack_pass / teammate / complain (19)
- that bounce was personal
- lucky deflection, unlucky timing
- somebody sand this rink, please
- was that a stick or a sneeze
- my skates are sliding like jelly
- that rebound hopped like it was cursed
- this puck hates everyone on our side
- my blade caught a crack, what a joke
- the ice is wet and so is my mood
- why does every bounce go their way
- the ice is sticky today, great
- why does the puck always hate me
- I was open for half a second
- that rebound came out of nowhere
- my skate caught a crack, unreal
- the boards grabbed it, of course
- I'm skating in circles here
- that pass came in slower than my nap
- the lighting is glaring on the ice

## signal_ack_pass / teammate / encourage (20)
- you got this, take the shot
- I've got your back, fire away
- come on, make them pay for that
- eyes up, you're open, let's go
- trust your stick, the net is waiting
- one more rush, we can do this
- shoot it now, don't overthink it
- you're the one I want on this
- keep your head up, the lane is clear
- come on, pick it up and go
- stay with it, the goal is coming
- our line needs you, let's roll
- back you up, go make a play
- shake it off, next shift is ours
- you got this, keep your stick on the puck
- heads up, I'm wide open, send it
- one touch and we're rolling
- ice is yours, make the move
- steady now, the net is waiting
- come on, go take the shot

## signal_ack_pass / teammate / praise (20)
- that pass was pure poetry
- textbook move, I'm taking notes
- you made that look easy
- clean finish, nobody else could do that
- best feet on the ice right now
- now that's how a director cuts a play
- smooth hands, smoother finish
- you're on fire, keep rolling
- that was a masterpiece, take a bow
- perfect read, you saw it before it happened
- more of that, please, that was great
- wow, you just stole the scene
- you didn't just score, you made art
- that move was pure poetry on ice
- clean dish, great hands, nice work
- that was a masterclass, director
- perfect pass, you saw everything
- wow, that spin left them spinning
- sharp pivot, you owned that lane
- that deke was a real showstopper

## signal_ack_space / teammate / complain (13)
- the ice is slicker than my patience
- rigged, the puck just swerved away
- who greased this rink, seriously
- unreal luck, the puck took a detour
- the boards jumped in front of me
- I was open, the puck had other plans
- the crease got my skate, great
- ice took that one, not my fault
- the puck chose the worst angle again
- my stick snapped on a flat pass
- that bounce was a villain's move
- the rink is working against us
- why does the puck hate my blade

## signal_ack_space / teammate / encourage (13)
- pick it up, the ice is still ours
- one more take, we can flip this
- keep your head up and find the lane
- good call, I'm open, feed me now
- reset, breathe, the next rush is ours
- that miss happens, we roll again
- we got time, stay calm and skate
- stick on the ice, we still got this
- eyes up, the play is still alive
- come on, one clean pass and we score
- forget it, the next shift is ours
- you're fine, I'm right here, go
- tighten up, we're still in this cut

## signal_ack_space / teammate / praise (12)
- that was a clean move, director
- you read that lane like a pro
- smooth stop, perfect timing on that
- that pass was pure poetry on ice
- great vision, you saw the gap early
- sharp cut to the net, well played
- clean stick work, keep that energy
- you own this shift, keep rolling
- that snap shot was a thing of beauty
- quick call, great read, wonderful skating
- that's a print-worthy play, director
- you're the best cut on this ice

## stall / opponent / bravado (13)
- our line is the whole show, sit back
- we own this corner, watch us reset it
- pure class on this ice, nobody matches us
- I could do this all day and still win
- three of us, one plan, zero worries
- we run this rink, you just skate on it
- watch the master at work, learn something
- our best scene is coming, get ready
- I am the lead, the rest of you are extras
- we make the plays, you make the excuses
- this is our house, polite guests only
- stick skills in every single frame
- hold the puck and the title, easy

## stall / opponent / complain (12)
- this ice is sticky and somebody greased it
- the puck bounced off nothing and hit me anyway
- my stick has a mind of its own today
- the boards are rigged, I swear they moved
- unlucky again, the puck hates my skates
- why does the puck always pick the wrong side
- this corner is cursed, I am sure of it
- I had the angle and the bounce ate it
- the lighting is in my eyes, that's the problem
- fine, the ice is slow and I am not to blame
- that deflection was a fluke, a cheap fluke
- stuck again, the universe owes me one

## stall / opponent / mock (13)
- the puck is stuck and so is your plan
- still circling? the rink is not a carousel
- you are directing a very long silent scene
- is this the slow motion part of the cut
- we could all retire before you make a move
- two-word jab: stand still, director
- your big play is a loop, try a new scene
- the puck wants out. so do we
- all that cornering and no goal to show
- bold choice, doing nothing for thirty seconds
- you call this a plot twist? it is a standstill
- lights up, camera on, and still no action
- nobody told the puck it was a silent film

## stall / teammate / complain (13)
- this ice is slower than a bad script
- that bounce hated us, I swear
- seriously, is the puck glued to the boards
- we've been circling longer than the credits
- who painted this rink, it grabs every skate
- that was a bounce, not a play, ugh
- the puck has a grudge against this corner
- we're going in circles like a bad sequel
- that one was on the ice, not on you
- every time we touch it, it slides away
- this shift is dragging like a rough cut
- unbelievable, the boards keep eating our pass
- I could've sworn that was a clean pass

## stall / teammate / encourage (14)
- come on, grab that puck and skate it out
- we're still in this, let's go get it back
- shake it off, the next shift is ours
- you're the one who can break this open
- dig in, the net is right there
- one good rush and we're back in the cut
- don't stop now, keep your feet moving
- okay reset, we go again
- you've got the angle, use it
- hey, find the gap and I'll find you
- stay with it, the bounce will turn
- we need you on this one, go
- wake up, this puck is still live
- take a breath and attack the corner

## stall / teammate / praise (13)
- nice hands, that was pure director stuff
- perfect read, you cut right through them
- that pass was spotless, keep it coming
- you made that look easy, nice work
- that's a wrap on their defense, nice
- beautiful feet, you owned that corner
- clean finish setup, you saw it early
- that was a masterclass in puck control
- you stole that shift, well done
- sharp thinking, you opened the lane
- pure poise right there, nice one
- that's the cut we wanted, great job
- look at you, slicing through like a pro

## steal / opponent / bravado (14)
- we run this line, you just watch
- our top line never loses a battle
- that puck belongs to us, always
- I am the lead in this whole production
- we win the big scenes, every time
- my stick is the star of the rink
- I steal pucks like I own the arena
- our team plays the good ending
- nobody out-skates this line today
- I was built for the final cut
- watch how we turn defence into offence
- the spotlight follows our blades
- we are the premiere, not the rehearsal
- my hands are the best in the house

## steal / opponent / complain (12)
- the ice is slick and I hate it
- lucky stick, lucky ice, lucky everything
- why does the ice always favour the other side
- that was a fluke and a half
- my skates slipped, that is on the rink
- the boards grabbed it, unfair
- this rink needs a new director
- I had that, the ice took it away
- my stick was fine, the puck was not
- the wind on this rink is a menace
- that puck has a grudge against me
- I want a retake, this scene is ruined

## steal / opponent / mock (15)
- that puck saw you coming from the blue line
- nice pass, shame it went to me
- you handed that over like a free sample
- my grandmother skates faster than that
- the ice just laughed at your turn
- you gave it away like a bad review
- careful, that stick is getting nervous
- was that a pass or a surrender
- that was a cut nobody asked for
- you lost that one before the faceoff
- say goodbye to that puck, director
- your footwork is in the blooper reel
- that puck left you like a bad date
- still waiting on your highlight reel
- even the boards looked away

## steal / teammate / complain (13)
- the ice is slicker than a bad review
- that bounce had no business being real
- unreal, the puck just hates us today
- who sold this rink a crooked boards
- the script was ours, the ice rewrote it
- seriously, the puck took a wrong turn
- my stick hates this rink, no question
- that was a lucky bounce and nothing else
- the puck picked the wrong skate again
- that rebound had a plot twist nobody wanted
- the whistle should have caught that one
- every rink has a lucky side, not ours
- the ice moves when you blink, I swear

## steal / teammate / encourage (14)
- shake it off, we still own this cut
- next shift is yours, get back on it
- one bounce doesn't end the scene
- eyes up, the puck comes back
- reset your feet and hunt it down
- they got lucky, now get it back
- stick on ice, head in the game
- we run this line, get back in it
- that was a read, now make the next one
- hold your lane, the puck finds you
- still with us, keep your legs moving
- fair steal, now steal it back
- walk it off, we go again
- chin up, the ice owes us one

## steal / teammate / praise (13)
- clean takeaway, that was pure film work
- you read that pass like a pro
- stick work worthy of the final cut
- that steal was the scene stealer
- perfect timing on that poke check
- the puck never saw you coming, nice
- that intercept was pure directing
- you took that one like you owned it
- textbook lift, the cut is yours
- that pickpocket was seriously sharp
- you stole that puck with style
- beautiful read, you cut right through
- that's how you reclaim a puck, director

## win / opponent / bravado (20)
- our line is the real main event tonight
- we own this ice and every frame of it
- three of us, one story, and we wrote it
- our defense is a wall with better hair
- we cut the scene and kept the best take
- watch us, we were born for this rink
- our passing looks like choreography
- the scoreboard agrees with the director
- we play like the headline act every shift
- nobody on this ice skates cleaner than us
- this win was scripted and we starred in it
- we crushed it and we know it
- call us the feature, not the extras
- we run this rink, plain and simple
- our line stays sharp under any spotlight
- we make the big saves look easy
- my squad never misses a cue
- give us the puck and watch us cook
- we came, we skated, we owned the cut
- our power play is a feature film

## win / opponent / complain (20)
- the ice was slick and somebody greased it
- that bounce was clearly rigged against us
- our skates are cursed, I swear it
- that puck hit a rut and changed its mind
- the lights were in my eyes the whole period
- we lost the cut because of one lucky bounce
- the boards moved, I felt it happen
- that was a fluke and everyone knows it
- my stick is too short for this rink
- the goal judge needs glasses, obviously
- they got a break and we got a shadow
- the whistle came too early, just wrong
- this rink hates visitors, I am certain
- the ice was sticky, that's on the ice
- that bounce was clearly rehearsed
- the puck hit a seam, pure bad luck
- the lighting was in their favor
- my stick had a bad day, not me
- the rink felt slower than usual today
- that shot deflected off a ghost

## win / opponent / mock (20)
- that stick had a better day than you did
- nice finish, shame it was the wrong net
- did you rehearse that celebration or just panic
- you looked lost out there, rookie
- your footwork needs a script and a director
- you scored, but the replay is hilarious
- i saw more grace in a dropped puck
- take a bow, you earned one bad take
- you swung like you were hunting a fly
- even the boards were laughing at that turn
- lucky goal, director's cut will not be kind
- our net had more action than your highlight reel
- cut! print the part where you tripped
- you won, but the replay will be brutal
- your passing was a two-act tragedy
- come back next take, bring a better script
- you celebrated like it was a final cut
- the puck was looking for a better lead
- took the trophy, lost the plot
- that goal needed a rehearsal

## win / teammate / complain (20)
- this ice is slicker than a bad script
- who waxed this rink, my skates keep wobbling
- the puck took a sneaky wrong bounce right there
- I called that puck to the corner and it ignored me
- my stick has more luck than my shot
- the boards jumped in front of me, total setup
- that deflection was a plot twist nobody asked for
- this rink keeps stealing my edges, it is personal
- this rink has a grudge against my blades
- that goal was a fluke and a cheap one
- the fog machine got me, I can't see a thing
- the lights up there are blinding, I'm squinting
- that was a rigged take, I want a reshoot
- That ice is slicker than a freshly waxed floor
- Their bounce off the boards was pure luck
- The puck hopped like it had a grudge
- My stick got tangled in the net again
- Who painted this ice, it grips like soup
- That bounce owed me one and never paid
- Whoever set those boards low should fix them

## win / teammate / encourage (20)
- you held the blue line like a pro, keep rolling
- one bad shift is just a bloop reel, we move on
- hey director, the next cut is ours, let's go
- sharp reads out there, keep that energy up
- you got this, same energy for the next take
- still got the wheels, ride that momentum
- rough patch? nah, just a blooper for the reel
- stick up, chin up, we are not done filming
- next period, same squad, same swagger
- you were in the zone, go find it again
- lean on me, we make this cut together
- one more win like that and we shoot a sequel
- keep your head in the game, rookie
- We won so take a bow, director
- Nice work out there, let's run it back
- You carried that cut, thanks for the lift
- Rest up, we shoot the rematch soon
- Good call on that last shift, boss
- Keep that energy, the next match is ours
- You kept us in it, that was huge

## win / teammate / praise (20)
- that backhand dish was pure gold, director
- clean cut through the middle, nice work
- you read that pass before it even existed
- textbook one timer, print it and frame it
- that stop was a masterpiece, encore please
- you carried us, I owe you a sandwich, seriously
- pure poise under pressure, what a finish
- three slick passes, that was a full feature
- you made that look easy, show off
- your timing was perfect on every single take
- best shift of the night, no retakes needed
- that top corner shot was pure cinema
- you are the reason we cut this match clean
- That backhand was a perfect cut, director
- Textbook pass, you saw the whole rink
- That spin move deserves an encore
- You threaded that puck like a pro
- Clean finish, the net never saw it coming
- Your read on that rebound was unreal
- That body check was pure poetry
