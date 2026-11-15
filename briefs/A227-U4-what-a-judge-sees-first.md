# Lane A227, track U4 (the screen), Sunday, the last lane: five things a judge sees in the first minutes (device A, Opus)

You are lane A227 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md`, then `briefs/NIGHT-COMMON.md`: their rules are yours in full.**
Then **R-0069 and R-0070** at the end of `contracts/RULINGS.md` (nothing technical is drawn on or in front of a skater; at a hit the
figure's animation is the whole message), lane A226's faults with their stills (`status/A226.md` in the Director's message's path, sheet
`faults.jpg`), `status/A222.md` (the controls panel; the clamp of the puck arrow), `status/A224.md` (the chevrons' clamp, the boxed ring),
`status/A219.md` (the hit layer's switches).

Your worktree and the commit it stands on are named in the Director's message that starts you. Status file `status/A227.md`. Two heavy
processes at most; ONE film at a time. Off screen only. Nothing of yours runs from the main checkout. **The last landing of the jam is at
18:30; the final package is cut at 19:00.** Do the jobs in this order; stop adding at 17:55 and prove what you have.

## 1. No effect at a hit but the figures (F6, and device B's pink balls on Glow Ice)
Lane A226 saw white streaks fly over the ice at a contact on clay for about nine frames (four times in one match), and lane A223 a ring of
opaque pink balls round the bodies at a knockdown on Glow Ice. Both are Niagara effects A spawns at a check and a knockdown
(`UJamEffectsBinder`: find which events spawn `NS_check`, `NS_knockdown` and their kin). His word: "the player animation should reflect the
crosscheck movement and that should be all". By default no visual effect is spawned at a check, a cross-check, a knockdown or a board
hit, on any look; their SOUNDS stay; the goal's effect, the puck's trail, the ability's and the pickups' effects stay. Behind
`jam.fx.HitEffects` (0; 1 = today). Film a hit on clay and on Glow Ice before and after.

## 2. While he sits in the box the camera watches the game (F3)
On Glow Ice a body check is a foul: he sits ten seconds, and for eight of them the screen showed a boost beacon and an empty sheet, his
figure behind the scoreboard: twice in the scripted hand's first forty seconds, in the mode the trailer ends on. While the human is boxed
the camera follows the puck (its carrier, or the loose puck) from the usual height and angle, eased in over about 20 frames and back to
him as he steps out; the HUD says why and for how long in the front end's type (one plate, centre top under the score: `IN THE BOX  8`
counting down): no control legend rows are lit, the panel dims as it does when he is down. Behind `jam.cam.BoxFollowsPlay` (1; 0 =
today). Film it; every look.

## 3. The sprint bar leaves the ice (F5)
The stamina bar lies across the lower edge of his ring: on film it reads as a bar under the skater (R-0069). By default it is not drawn
on the ice; the controls panel's SPRINT row carries it instead: the row lit while he sprints, its word dimmed while the legs are spent
(no bar, no meter, no number). If the panel is switched OFF in SETTINGS he has no stamina display: accept that. Behind `jam.hud.SprintOnIce`
(0; 1 = today). A still before and after at pro.

## 4. Two small ones
a. **The edge triangle on the controls panel (F7):** an orange triangle (an off-screen marker of a skater, find which) sits on the panel's
   BRAKE row and E keycap. The same clamp above the panel as the puck arrow and the chevrons have.
b. **Glow Ice's plate "speed 2.8s" under the scoreboard (F8)** reads as a debug line: in the front end's type and case (`SPEED  2.8`), or
   remove it if the boost's own HUD slot already says it. Say which.

## 5. Only if 1 to 4 are filmed and green by 17:40: the camera at the side boards (F4)
At the side boards the camera goes straight down and close: he is cut by the bottom edge with his rings over his own back. Do not retune
lane A214's camera: add one floor (the camera never closer to him than the distance at which his whole figure and ring fit the frame with
a tenth of the height to spare) behind `jam.cam.BoardFloor`, and show the same frame before and after. If it is not a ten-line change,
leave it and say so.

## Prove
A still or strip before / after for each job (`docs/experiments/first/`), tests (`Jam.Hit.*`, `Jam.Hud.*`, `Jam.Camera.*`), the full `Jam`
filter in a cleared environment, Shipping compiles, the sim's hash equal on a 60-second scripted match on clay and on Glow Ice
(presentation only: none of this may touch the sim).

`SEEN` lines as each job is filmed (the Director looks himself); `FIRST READY` with the files by name, the stills and the gate **by 18:10
by the machine clock, not later**. Keep the files closed after the line. What is not done by then is named, not rushed.
