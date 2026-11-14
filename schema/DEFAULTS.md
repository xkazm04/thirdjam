# Engine defaults for absent keys

The validator only checks what is written. It never fills anything in. The C++ engine (lane A1) applies these defaults when a key is absent
("absent value = omit the key": an omitted key means the default, never 0 and never null). Presets in `Game/Content/Cartridges/` write most
of them out explicitly, but a Designer or hand-written cartridge may leave them out.

| Key | Default | Note |
|---|---|---|
| `rink.corner_radius_m` | 5 | |
| `rink.board_restitution` | 0.4 | arcade bounce, not measured, A3 sweep may retune |
| `rink.ice_friction` | 0.015 | glide friction coefficient (0.015 x 9.8 = 0.147 m/s^2) |
| `rink.goal_width_m` | 4 | |
| `rink.goal_height_m` | 0.61 | R-0033 point 2 (lane A50): the half-height net of a game without goalies; a real net is 1.22. Net frame, netting, goal triggers and the sim's lifted-puck goal test read it |
| `rink.obstacles` | none | |
| `skater.accel` | 12 | |
| `skater.turn_rate` | 5 | |
| `skater.lateral_grip` | 3 | |
| `skater.mass` | 80 | |
| `skater.glide_drag` | 0.147 | |
| `skater.brake_decel` | 6 | |
| `skater.check_enabled` | true | R-0013: the body check is in every cut (was false before H7) | |
| `skater.check_speed_min` | 5 | only read when `check_enabled` is true |
| `stick.reach_m` | 1.2 | lane A16 (R-0013); 1.2 by R-0022 point 3; the range 0.6..1.6 by R-0034 (lane A51): the carried puck sits at the blade, forehand to backhand as the aim sweeps (the brief's 1.1 halved the regular player's passes received at pro: kept 1.2, STICK.md section 6) |
| `stick.poke_window_frames` | 8 | lane A16 (R-0013) |
| `stick.lift_range_m` | 1.0 | lane A16 (R-0013) |
| `stick.lift_cone_deg` | 70 | lane A95 (B-to-A-0037): The cone of the stick lift and of the cross-check's shove, in degrees off the attacker's facing: a carrier farther off than this is neither lifted nor shoved (the swing is a miss), bots and the human alike (B-to-A-0037, lane A95). 180 = no cone (before A95). |
| `stick.stumble_frames` | 20 | lane A16 (R-0013); 0 in `hockey.raw` |
| `stick.mouse_sensitivity` | 1 | lane A16 (R-0013) |
| `stick.foul_grace_frames` | 12 | lane A16 (R-0029 point 3): no `foul_no_puck` on a skater that carried within the last 12 frames |
| `stick.deke.offset_m` | 1.0 | lane A39 (R-0029 point 4): a deke tap (`bDeke`) moves the carried puck this far to the blade side (the side the stick is aimed to; the middle of the ice when it is not aimed) within 6 frames; a shot inside the commit window aims at the post on that side; a second tap inside 30 frames does nothing; 0 = off |
| `stick.deke.commit_frames` | 15 | lane A39: the crease back's stance and blade follow the puck's old side this long |
| `stick.sweep.min_deg_per_frame` | 6 | lane A51 (R-0034): an aim turning more than this in one frame on the carrier is a sweep (read-out flag `bSweepThisFrame`) |
| `stick.sweep.shield_frames` | 12 | lane A51: frames after a sweep (or a deke tap) in which an opponent's poke or lift misses, from either side of the body (R-0044 as amended, lane A93; only the side the puck left, before); 0 = off |
| `stick.carry_reach_mult` | 0.75 | lane A93 (R-0045 law 4): the carried puck rides at this fraction of the blade's full distance unless the carrier sprints (1.2 m against 1.6 m at full aim); 1 = always full (neutral) |
| `stick.sweep.sprint_shield_mult` | 0.5 | lane A93 (R-0045 law 4): a sprinting carrier's sweep shield lasts this fraction of `shield_frames` (6 of 12); 1 = no difference (neutral) |
| `stick.body_shield` | true | lane A102 (track F2): puck protection with the body, no button. While the carrier's body is between a defender's stick and the puck (the defender within 2 m, the puck on the far side of the carrier's body, the carrier turned more than 60 degrees away from the defender), that defender's poke or stick lift does not take the puck; a carrier that faces the defender shields nothing, and a body check is never shielded. true or absent = on; false = the neutral value (the game before the lane). |
| `stick.passive_take` | false | lane A102 (R-0048): false or absent = a carried puck is not taken by an unpressed blade or by a body's touch: it leaves its carrier only by a shot, a pass, a poke, a lift, a check or stumble that loosens it, or the carrier's own release. true = the neutral value (the game before the lane): an aimed opposing blade within 0.6 m deflects a carried puck moving faster than 6 m/s and any body's touch takes it. A teammate never takes a carried puck by touch under either value. |
| `stick.take_grace_frames` | 30 | lane A121 (R-0049 point 2): the take grace. A skater that has just won the puck (by a poke, a lift, a loose puck, a pass or the draw: the puck came to it from somebody else) cannot be poked or stick-lifted for this many physics frames; a body check still loosens the puck, and a poke that runs out on it costs the poker no stumble. Leave the key out for 30 (half a second); 0 = off, the neutral value (the game before the lane). |
| `stick.poke_assist_deg` | 60 | lane A152 (track F2, R-0059 point 4): the poke reaches. Inside the open poke window of the skater a person steers, a puck an opponent carries is taken when it is within the stick's full reach of the poker's body (as far as a perfectly aimed blade takes from, never further) and within this many degrees of the direction the blade is aimed in; the body between, the sweep's shield, the spin and the take grace refuse it as before; bots keep the blade rule. 0 = off, the neutral value (the blade itself within 0.35 m of the puck, for everybody) |
| `stick.miss_recovery_frames` | 20 | lane A152: a miss costs a recovery, not a fall. A person's poke whose window runs out next to an opposing carrier locks its stick for this many frames (no poke, no lift, no check; it skates and steers). 0 = the neutral value: the miss costs `stick.stumble_frames` without control. A bot's miss stays the stumble; with `stick.stumble_frames` 0 a miss costs nothing |
| `stick.stop_grace_frames` | 0 | lane A163 (track F2 evolution 6): the stop tucks the puck. A person's reverse brake begun with the puck at 6 m/s or more gives its puck the take grace for this many frames (no poke, no lift; a body check still loosens it). 0 = off, the neutral value and the shipped one until the curve track sets it (measured: `docs/experiments/MOVEMENT-7.md`) |
| `stick.sweep.bot_react_frames` | absent | lane A51: a defending bot reads the puck's place on the carrier this many frames late; absent = the difficulty profile (rookie 14, pro 8, all_star 5) |
| `stick.reception_m` | 0.9 | lane A46 (PAYOFFS.md section 6): a teammate's pass reaching the body within this radius at 6-14 m/s seats on the blade with a 4-frame settle; faster passes bounce off the body unless the blade meets them; 0 = blade reception only |
| `skater.knockdown_frames` | 90 | only read when `check_enabled` is true |
| `skater.pass_speed` | 14 | lane A16 (C-0005) |
| `skater.pass_lead_frames` | 20 | lane A16 (C-0005) |
| `skater.breakaway.speed_bonus` | 0.2 | lane A39 (R-0029 point 4): a carrier past centre ice with no opponent between it and the net gets this fraction more top speed and acceleration for `frames`, once per possession (event `breakaway`); 0 = off |
| `skater.breakaway.frames` | 60 | lane A39 |
| `skater.carry_speed_mult` | 0.92 | lane A82 (R-0041 point 6): the carrier's top speed and acceleration as a fraction of a free skater's, bots and the human alike; the carrier cannot sprint and its bar does not drain; 1 = no carry penalty |
| `skater.free_speed_mult` | 1.15 | lane A185 (track F2 evolution 9, R-0064 point 2): a skater without the puck, bots and the human alike, has this much more top speed and acceleration (the higher top in the same time), in a glide and in a sprint; the carrier skates as without it (0.92 of the base, 0.994 sprinting), so it is that much slower against everyone; 1 = the neutral value (the game before the lane) |
| `skater.sprint.bonus` | 0.15 | lane A49: Shift / LT held adds this fraction to top speed and acceleration while the skater moves and the stamina lasts; bots only with `ai.sprint` (lane A72); 0 = off |
| `skater.sprint.drain_frames` | 180 | lane A49: frames of sprint from full to empty (3 s) |
| `skater.sprint.refill_frames` | 360 | lane A49: frames at rest from empty to full (6 s) |
| `skater.sprint.carry_bonus` | 0.08 | lane A93 (R-0045 law 4, replacing R-0041 point 6): the carrier may sprint for this much on top of `carry_speed_mult` (0.92 x 1.08 = 0.994 of the base speed against a free sprint's 1.15); its bar drains as anyone's; 0 = the carrier cannot sprint (neutral) |
| `skater.reverse_brake_decel` | 26 | lane A93 (R-0045 law 3): move input against the velocity is a hard stop at this rate, then the input accelerates the other way; lane A121 (track F2): 26, the stop's grip (8 m/s to still in 19 frames, 11 m/s in 26); 20 = the value before that lane (its neutral value); 0 = off (lane A93's neutral value) |
| `skater.burst.frames` | 20 | lane A93 (R-0045 law 3): the first-step burst after a stop (under 2 m/s) or a sharp cut (the input turned more than 60 degrees inside 10 frames); 0 = off (neutral) |
| `skater.burst.accel_mult` | 1.8 | lane A93: acceleration multiplier while the burst runs; 1 = none (neutral) |
| `skater.burst.own_stop` | true | lane A121 (track F2): the first step is earned. true or absent = being slower than 2 m/s arms the burst only after a stop of the skater's own (it braked on the way down, by reversing or with the brake button); a skater that merely stands (a waiting defender, the faceoff, a skater getting up) starts at the normal acceleration. A sharp cut arms the burst under either value. false = the neutral value (the game before the lane): any skater slower than 2 m/s has the burst. |
| `skater.burst.rest_frames` | 6 | lane A128 (track F2, evolution 3): the standing start. A skater that has been slower than 1 m/s for this many physics frames in a row (after a faceoff, after getting up, after a full stop) and is not pushing has earned its first step: its burst is armed while it stands and runs burst.frames from its first push (a gentle push spends it as a hard one does). A charge already under way never has it. Leave the key out for 6; 0 = the neutral value (the game before the lane: only a stop or a cut of the skater's own arms the burst). |
| `skater.burst.rest_kept_mps` | 3 | lane A147 (track F2, evolution 4): the first step is not lost to what is not the skater's own doing. While it does not push and is slower than this, a rested skater keeps its armed burst (a bump, a shove); a skater lying on the ice or held by a stumble counts its rest whatever it slides at (it gets up with its first step); turning on the spot is not the push-off. 0 = the neutral value (lane A128's rule: the rest is lost at 1 m/s). `docs/experiments/MOVEMENT-5.md` |
| `skater.goalie.reach_m` | 0.9 | lane A52 (R-0035): a skater in the crease stance (within 3 m of its own goal centre, the human included) saves a loose shot passing within this of it; 0 = off |
| `skater.goalie.block_height_m` | 0.4 | lane A52 (R-0035), Director 01:30: a shot at or above this height passes the crease skater; below the 0.61 bar so a half charge (0.45) is the skill shot: over the pads, under the bar (0.7 saved everything the net could take; the novice gate failed 8 of 20) |
| `skater.spin.frames` | 24 | lane A51 (R-0034): the spin on E / gamepad B: a full turn of the figure over this many frames, the puck on the far side of the body from the nearest opponent (a poke never reaches it); a check landing mid-spin knocks down 1.5x longer; 0 = off |
| `skater.spin.cooldown_frames` | 90 | lane A51: frames before the next spin |
| `skater.spin.speed_mult` | 0.7 | lane A51: top speed while spinning as a fraction of the skater's |
| `skater.spin.stamina_cost` | 0.25 | lane A51: the share of the sprint bar a spin takes; refused with an empty bar |
| `skater.spin.exposed_frames` | 5 | lane A128 (track F2, evolution 3): the price of the spin. For the last this many physics frames of a spin the body has come round and is no longer between: neither the body nor a sweep shields the carried puck from a stick, so a poke or a lift in reach takes it (a puck just won keeps its take grace). A spin begun too early ends in front of the defender. Leave the key out for 5; 0 = the neutral value (the game before the lane: the whole spin shields). |
| `skater.visual` | `humanoid_b1` | which figure is drawn: `toy_skeletal` (the Blender toy figure as a skeletal mesh, lane A26, the default since H11.7), `toy` (the code-built figure), `humanoid_b1`, `generated_b2`; gameplay is identical; the Designer may not change it (R-0001) |
| `puck.radius_m` | 0.12 | |
| `puck.max_speed` | 30 | |
| `puck.drag` | 0 | |
| `puck.carry_assist` | 0.6 | lane A16 (C-0005): pull towards the stick socket while the carrier turns under 90 deg/s |
| `puck.count` | 1 | lane A16 (C-0005) |
| `puck.colour_hex` | `#111111` | near-black; readability comes from trail and shadow disc |
| `puck.gravity_mps2` | 9.81 | lane A52 (R-0035): gravity on a lifted puck |
| `puck.ice_restitution` | 0.3 | lane A52 (R-0035): the bounce on the ice; a hop under 2 cm dies. The glass is 2.4 m (fixed, no key): a puck over it at the boards is out of play (free faceoff) |
| `shot.reach_m` | 1.5 | |
| `shot.speed` | 18 | |
| `shot.cooldown_frames` | 24 | |
| `shot.aim_cone_deg` | absent = no aim assist | |
| `shot.charge_frames_max` | 30 | lane A16 (C-0005): hold shoot up to this many frames for the bonus |
| `shot.charge_speed_bonus` | 8 | lane A16 (C-0005): m/s at full charge |
| `shot.deflect` | true | lane A16 (C-0005): a shot into a skater within 0.6 m of its stick keeps 70 % of its speed |
| `shot.one_timer.window_frames` | 10 | lane A39 (R-0029 point 4): a shot inside this many frames of a received pass is a one-timer (full charge without the hold, `aim_cone_deg` + `cone_bonus_deg`, `shot` event kind `one_timer`); 0 = off |
| `shot.one_timer.cone_bonus_deg` | 8 | lane A39 |
| `shot.lift.tap_m` | 0 | lane A52 (R-0035): the height a tap arrives at over the goal line; R-0042 (lane A85): 0, a tap stays on the ice the whole flight (0.05 made it a 0.5 m lob from 10 m that cleared every blade) |
| `shot.lift.half_m` | 0.45 | lane A52: the height of a shot held `half_frames` |
| `shot.lift.full_m` | 0.47 | lane A52 (0.95), Director 01:40 (0.6), R-0044 (lane A91; the ruling said 0.5, the measurement 0.47): the height of a full charge: the top shelf clearly under the 0.61 bar, whose ring starts at 0.51. At 0.6 every full charge from 4 to 24 m rang the bar; at 0.5 those from 15 to 24 m; 0.47 scores from every metre of 3..30 at every hold; a cartridge may still choose 0.95 for a sailing full charge |
| `shot.tap_frames` | 4 | R-0044 (lane A91): a hold of this many frames or fewer is a tap (charge 0: no speed bonus, flat at `tap_m` 0); a human's click is 2 to 4 frames long; the human and the bots alike; 0 = every held frame charges |
| `shot.lift.half_frames` | 8 | lane A52: frames of hold that reach `half_m`; linear tap -> half -> full |
| `pass.saucer.half_frames` | 8 | lane A77 (R-0036): pass (F / X) held this many frames with the puck in reach is a saucer; a shorter hold or the tap is the flat pass; 0 = off |
| `pass.saucer.lift_m` | 0.35 | lane A77: the saucer's apex height; it clears a blade (0.2 m) in the lane and lands three quarters of the way to the receiver; 0 = off |
| `contact.momentum` | true | lane A92 (R-0045): false = the neutral value: the check rule before lane A92 (a pressed check lands when the checker's own speed along the contact passes skater.check_speed_min and always knocks down for skater.knockdown_frames; the cross-check is the lift only); true or absent = contact by momentum (impulse against stability, three outcomes). |
| `contact.carrier_can_hit` | false | lane A92 (R-0045): true = the neutral value: a skater carrying the puck may body-check as before lane A92; false or absent = the carrier's check press is refused (read-out bCheckRefused). |
| `contact.lunge_mps` | 1.5 | lane A92 (R-0045): Speed a pressed body check adds to the closing speed along the contact line, in m/s; the checker gives half of it away. |
| `contact.thrust_mps` | 3 | lane A92 (R-0045): Speed a cross-check (the lift at an opposing carrier in range) adds to the closing speed, in m/s; 0 = the cross-check has no body hit (the lift only). |
| `contact.cross_reach_m` | 0.5 | lane A92 (R-0045): The gap between the two bodies, in metres, inside which the lift at a carrier is a cross-check (the stick's shaft at arm's length: the thrust lands); farther out, up to stick.lift_range_m, it is the stick lift only. stick.lift_range_m or more = every lift in range shoves. |
| `contact.stability_mps` | 6 | lane A92 (R-0045): The stability of a skater of the reference mass that moves slowly, sees the hit and is not braced, as the closing speed (m/s) of an equal-mass hit that puts it down at down_ratio 1. |
| `contact.stand_speed_mps` | 1 | lane A92 (R-0045): Under this speed a skater stands braced (stand_brace); above it the stability falls with the speed (speed_stability). |
| `contact.stand_brace` | 1.5 | lane A92 (R-0045): Stability factor of a standing skater against a hit it can see; a cross-check inside its front arc ignores it; 1 = no brace. |
| `contact.speed_stability` | 0.85 | lane A92 (R-0045): Stability factor at skater.speed; linear from 1 at stand_speed_mps; 1 = pace costs nothing. |
| `contact.sprint_stability` | 0.7 | lane A92 (R-0045): Stability factor while sprinting; 1 = the sprint costs nothing. |
| `contact.spin_stability` | 0.8 | lane A92 (R-0045): Stability factor during a spin or with a full shot wind-up; 1 = no cost. |
| `contact.unseen_stability` | 0.8 | lane A92 (R-0045): Stability factor against a hit from behind the victim's facing (the push runs along its facing); such a hit also finds a standing skater unbraced; 1 = no cost. |
| `contact.bump_ratio` | 0.4 | lane A92 (R-0045): Impulse over stability under this is a bump: both keep their feet and their control. |
| `contact.loose_ratio` | 0.7 | lane A92 (R-0045): A stumble at this ratio or more knocks the carried puck loose. |
| `contact.down_ratio` | 1 | lane A92 (R-0045): At this ratio or more the victim goes down; between bump_ratio and it the victim stumbles (half its speed, stagger_frames without control). |
| `contact.down_full_ratio` | 3 | lane A92 (R-0045): The ratio at which the knockdown lasts down_frames_max; linear from down_frames_min at down_ratio. |
| `contact.down_frames_min` | 60 | lane A92 (R-0045): Frames down at down_ratio, at skater.knockdown_frames 90 (the frames scale with skater.knockdown_frames / 90). |
| `contact.down_frames_max` | 150 | lane A92 (R-0045): Frames down at down_full_ratio and above, at skater.knockdown_frames 90; never under down_frames_min. |
| `contact.stagger_frames` | 16 | lane A92 (R-0045): Frames a stumbling skater glides without thrust, steering or stick. |
| `contact.board_bonus` | 1.25 | lane A101 (track F1): The boards are a body: the impulse of a hit that pushes a victim standing within 1.5 m of the boards into them counts this much higher (the full bonus inside 60 degrees of straight in, none along the boards), and the victim's board read-out fires. 1 = the neutral value: no bonus, no read-out. |
| `contact.release_grace_frames` | 120 | lane A113 (track F1; R-0051): The release grace (R-0049 point 1): for this many frames after a knockdown ends, a hit on the released skater is a bump at most (no stumble, no knockdown; the attacker pays as for any bump) and the brain starts no check on it. Part of the momentum model (contact.momentum true). 0 = the neutral value: no grace. |
| `contact.check_reach_m` | 0 | lane A125 (track F1; off until the Director turns it on: 1.0 was the measured value): The check finds its man: while a human skater's check window is open (12 frames) its velocity is turned onto the collision course with the nearest standing opponent whose body is within this many metres of its own and inside 60 degrees of where it is going or facing. The speed is kept and none is added (the hit's strength stays the contact law's), the position is untouched; a bot aims its own check. Part of the momentum model (contact.momentum true). 0 = the neutral value: the press goes where the skater was going. |
| `contact.downed_radius_m` | 0 | lane A157 (track F1; off until the Director turns it on: 0.6 is the measured value): Nobody stands in a man who is down (B-to-A-0041 point 2): with this above 0 a downed skater's body has this radius for every skater on its feet (roots kept 0.4 m + this apart, never less than the two 0.4 m body circles): the skater on its feet alone gives way (eased out at 6 m/s at most), the speed it had into the body turns along it (it goes round), and the downed skater is neither moved nor given any speed. The puck sees the same body as before; two downed skaters and the hit that fells a skater collide as before. 0 = the neutral value: a downed skater is an ordinary body (others stop 0.8 m from its root, push it along the ice and bounce off it). |
| `contact.check_cooldown_frames` | 0 | lane A157 (track F1; off until the Director turns it on): A check press has a cooldown (R-0060 point 4): for this many frames after a check press that opened a window, the same skater's next check press is dropped (no window, no cue), so a mashed button holds no window open: at the human's 12-frame window a button mashed without pause has one open 12 frames in every this-many. Bots and the human alike (the brain spaces its own presses 30 frames apart). 0 = the neutral value: every press opens a window. |
| `contact.check_cooldown_restart` | false | lane A165 (track F1; off until the Director turns it on): A mashed button lands nothing: true = a check press inside the cooldown (check_cooldown_frames) starts the cooldown again from that press, so a button pressed faster than the cooldown opens one window and no other until the hand has rested a whole cooldown; a press made after the cooldown never notices. Read only with check_cooldown_frames above 0. false = the neutral value: a dropped press costs nothing more. |
| `contact.board_slam_frames` | 0 | lane A165 (track F1; off until the Director turns it on): The board slam as a play. Earned: a hit that fells its victim while the victim's centre is within 1 m of the boards and the attacker closes on it along the boards' normal at 4 m/s or more (driven into the wall, not along it). Paid: this many frames down on top (at skater.knockdown_frames 90; they scale with it), the victim's board read-out and the board event. 0 = the neutral value: no slam exists. |
| `contact.take_shield_frames` | 0 | lane A177 (track F1; off until the operator has played): A hit inside a carry's first frames is capped at a stumble (REVIEW-3 item 2): for this many frames after a skater wins the puck (it comes to it from somebody else; not a placed puck), a hit that would fell it is a stumble: it keeps its feet, loses half its speed, its control for the stumble and the puck; the attacker pays as for any stumble; an armed hip check keeps its rule. Part of the momentum model (contact.momentum true). 0 = the neutral value: the hit lands as the contact law says. |
| `contact.take_shield_human_side` | false | lane A177 (track F1; off until the operator has played): true = the take shield (take_shield_frames) only for skaters of a side that has a human on it; a bot of the other side that wins the puck is felled as before. false = the neutral value: every skater. Read only with take_shield_frames above 0. |
| `contact.take_shield_stagger_max` | 0 | lane A177 (track F1; off until the operator has played): The capped hit's stumble scales with the hit: from stagger_frames at down_ratio to this many frames at down_full_ratio (the line the frames down follow), so a bigger hit still costs more. 0 (or not above stagger_frames) = the neutral value: the flat cap, every capped hit is stagger_frames. Read only with take_shield_frames above 0. |
| `contact.lunge_frames` | 0 | lane A157 (track F1; off until the Director turns it on): The lunge as a motion: a check press that opens a window commits the body for this many frames: lunge_step_mps is added along its facing at once, it turns at a quarter of its turn rate and cannot brake by reversing. A lunge that ends in a body ends there; one that runs out on nobody costs twice lunge_step_mps of speed (the miss). Part of the momentum model (contact.momentum true). 0 = the neutral value: a press moves nothing. |
| `contact.lunge_step_mps` | 0 | lane A157 (track F1; off until the Director turns it on): The lunge's forward step in m/s, added to the velocity along the facing when a lunge starts (lunge_frames above 0). It is real closing speed in the hit, so the impulse's own lunge_mps is reduced by it while the lunge runs (at lunge_step_mps = lunge_mps a head-on hit is the hit it was). 0 = the neutral value. |
| `rules.goal_pause_frames` | 120 | |
| `rules.offside` | false | only false is accepted |
| `rules.penalties` | true | R-0013: fouls (from behind, or on a skater without the puck) = 300 frames in the box + a free faceoff at the fouler's end; `hockey.raw` sets false |
| `rules.penalty_frames` | 300 | lane A72 (ask A8): frames a hard foul sits in the box; read only when `penalties` is true |
| `ai.difficulty` | `pro` | lane A3; the numbers below (and `react_frames`, `body_check`), when absent, come from the level's profile (`docs/experiments/BALANCING.md`), not from this table |
| `ai.yield_margin_m` | 2 (pro) | |
| `ai.yield_radius_m` | 15 | |
| `ai.hold_radius_m` | 0 (pro; 4.5 before the difficulty sweep) | |
| `ai.speed_scale` | 1 | |
| `ai.red_skill` | 1 | |
| `ai.react_frames` | the level's profile (0 at rookie, pro and all_star) | lane A72 (ask A4): the opposing bots' steering delay in frames; the back reacts at once |
| `ai.sprint` | 0 | lane A72 (ask A2): the share of the time a bot sprints on a chase from more than 6 m (both sides' bots, same stamina bar as the human); 0 = bots never sprint |
| `ai.body_check` | the level's rule (rookie never; pro and all_star from 5 m/s of closing) | lane A72 (ask A5): when written, the share of the chances an opposing bot checks, at `skater.check_speed_min` instead of 5 m/s; 0 never |
| `pickups` | none | |
| `pickups[].max_on_ice` | 1 | lane A14 (C-0005) |
| `pickups[].zone` | absent = anywhere on the ice | |
| `abilities` | none (nobody holds an ability; F does nothing) | lane A84 (R-0041 point 3) |
| `abilities[].charges` | unlimited for `trigger` mode, 1 for `trigger` pickup | lane A84 |
| `abilities[].cooldown_frames` | 1200 (20 s) | lane A84 |
| `abilities[].duration_frames` | 360 shield (6 s), 120 slow_puck (2 s); unused by shotgun and hip_check | lane A84 |
| `abilities[].magnitude` | 1.5 shotgun (shot speed x), 1.5 hip_check (knockdown frames x), 0.5 slow_puck (the puck's speed share); unused by shield | lane A84 |
| `effects` | none (the five built-ins are always available to pickups) | lane A14; `Game/Content/Designer/builtin-effects.json` |
| `effects[].kind` | `custom` | the visual family; built-ins carry their own |
| `effects[].stack` | `refresh` | |
| `effects[].icon` | by kind: bolt (speed), crosshair (precision), weight (heavy_puck), circle (big_puck), swirl (reverse_other), star (custom) | |
| `effects[].stats[].multiply` / `add` | 1 / 0 | |
| `look.trim_colour_hex` | `#38BDF8` | |
| `look.fog_density` | 0 | |
| `meta` (whole object) | absent | lane A81 (A67 ask A11): the menu's words for a Director's choice (`pitch`, up to three `facts`; the card's name is the envelope `title`); informational, never read by the sim; a mode file without it gets no card |
| `look.trim_emissive` | by tier: 30 (`l1_day`, `l2_night`, `l3_neon`, `p4_broadcast`), 12 (`p3_clay`) | B-to-A-0018 point 5, B-to-A-0025 (lane A31); the beacon stays at 40 |
| `look.wet_ice` | false | |
| `camera` (whole object) | see below | measured by lane E4, `docs/experiments/CAMERA-120M.md` |
| `barks.enabled` | true | lane A12, ruling R-0007; read by `UJamBarkSystem` from the cartridge's raw JSON |
| `rink.obstacles[].kind` `bumper` | restitution 1.6 for the puck | lane A15 (C-0005); `goal_shield` is a wall `size` m wide and 0.3 m deep |
| `rink.zones` | none | lane A15 (C-0005) |
| `rink.zones[].strength` | 0.5 | slow_ice at 1 halves speed and makes the glide 4x stickier; fast_ice at 1 adds half the speed; a bumper zone at 0.5 has the bumper obstacle's 1.6 |
| `rink.zones[].visible` | true | |
| `rules.hooks` | none | lane A15 (C-0005); evaluated in id order |
| `rules.hooks[].after_frame` | 0 | the hook is armed from this frame |
| `rules.hooks[].team` | `scoring` | the team the `score` condition reads and the actions apply to; an action's own `team` wins |
| `rules.hooks[].then[].team` | the hook's team | |
| `rules.variants` | none | the loader expands each variant into a hook named `variant-*` after the explicit hooks (`expandVariants` in validate.mjs, `ExpandVariants` in JamCartridge.cpp): `sudden_death` = on_goal after frame 600 end_match; `last_goal_double` = on_goal at score_to_win add_score 1 (first_to only); `mercy_goals` = on_goal lead_at_least N end_match; `period_count` = on_frame every timer_frames / N (3600 in first_to) end_period; `shrink_per_goal` = on_goal shrink_rink by the fraction |
| `barks.intensity` | `normal` | `quiet` doubles the cooldowns and never mocks, `loud` halves them |
| `commentary.enabled` | true | lane A32 (B-to-A-0026); read by `UJamCommentarySystem` from the cartridge's raw JSON, console `jam.Commentary -1|0|1` overrides |
| `commentary.intensity` | `normal` | `quiet` plays priority 3 and up only and doubles the situation cooldowns, `loud` halves them; the 2.5 s gate between lines never changes |

## Camera defaults (when `game.camera` or any of its keys is absent)

| Key | Default |
|---|---|
| `camera.mode` | `third_person` |
| `camera.distance_m` | 6 |
| `camera.height_m` | 3.5 |
| `camera.look_ahead_m` | 5 |
| `camera.lag_frames` | 4 |
| `camera.fov_deg` | 70 |
| `camera.radar` | true |
| `camera.boards_margin_m` | 0.8 | lane A40: the camera never crosses the boards' inner face minus this, corner arcs included |
| `camera.end_height_m` | 3 | lane A49 (was 5, lane A40): the lift when the boards take the arm (proportional to the arm lost), looking along the rink at an end board |
| `camera.far_net_bias` | 0.2 | lane A49 (was 0.5, lane A40): the yaw lead toward the far net when heading for it, fading in from 8 to 20 m |

Wide option (not a default): 8 / 4 / 6 / 6 frames / fov 85. Pitch is derived, never stored: `-atan(height_m / (distance_m + look_ahead_m))`.
In `arena` mode the distance, height, look-ahead and lag keys are ignored.

## Derived by the engine, not in the cartridge

- Goal line at `length_m / 2 - 4` m from the centre on each end (the Godot rink: 60 m long, nets at x = +-26).
- Pickup `magnitude` absent: 1.5 for `speed`, 2 for `precision` (cone multiplier), 2 for `heavy_puck` (drag multiplier, max speed divided by it), 1.5 for `big_puck` (radius multiplier); `reverse_other` ignores it. For a pickup naming a custom effect, `magnitude` scales the effect's deviation from neutral (`multiply' = 1 + (multiply - 1) * magnitude`, `add' = add * magnitude`, then clamped to 0.25..4 and the stat range); absent = 1 = as written.
- Effect modifiers compose as `base * product(multiply) + sum(add)` per stat, clamped to the stat's schema range; the same effect on the same target while active: `stack` decides (refresh restarts the timer, extend adds the duration up to 7200 frames, ignore keeps the running one).
- A `values` entry overrides the game field a tunable is named after (README.md, "Tunables"); an absent entry leaves the game field alone.
