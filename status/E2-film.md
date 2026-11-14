# E2 film script: the figure's three new calls, for the GPU lane

Tree `C:\Users\kazda\jam\wt\b-fig` (branch `lane/b-fig`; assets saved, not committed). Use the lead's GPU slot, with one editor only. Turn sound off in memory first
(`LevelEditorPlaySettings.enable_game_sound` false) and back on at the end. `r.Editor.Viewport.OverridePIEScreenPercentage 0` for the stills, back to 1 after.

**Harness** (the same one lanes SK28 to FILM3 used; E2's copy has its paths re-pointed):
`C:\Users\kazda\AppData\Local\Temp\claude\6d8eaecb999746c2\1f5c7b1e-557b-4c70-b752-8370d9dbef0b\scratchpad\e2\`.
- Runner: `tx.sh <name> T`. It uses the figures and acts in `fa_T.figs` / `fa_T.acts`, which `gen.py` writes.
- Clay is the default figure; for night, set `BPN=BP_SkaterVisual_TB1`. Run with `NREC=300`.
- For stills, add `(tick, None, 'cam', side(x))` and `(tick, None, 'shot', label)` acts the way FILM3's `gen3.py` does. `w3.py` already records the props and the hold weight (`ex.hw`, `ex.boost`).
- Ticks are frames at 60 fps from the `go` call.

Film each shot on the clay figure (`BP_SkaterVisual_TB1_P3`), then on the night figure. Camera: side-on at 4 m, hip height, 50 deg, unless the shot says otherwise.
Then film one more time **from the game camera's height and angle** (behind and above, about 6 m back and 3.5 m up). That view is what the operator sees.

| # | Shot | Figures and acts (from `fa_T`) | What must be seen | What must never be seen |
|---|---|---|---|---|
| 1 | Hold while skating | H1 at 6 m/s: `SetCrossCheckHold(true)` on tick 30, `(false)` on tick 150; control H0 beside it at 6 m/s | The upper body goes into the braced pose over 6 frames (hold weight 0.17 a frame, 1.0 from tick 36). The stick is held across in front of the chest, both hands about 94 cm up and 60 cm ahead of the hips. The legs keep striding exactly as H0 (measured 0.00 cm apart). It goes back out over 6 frames (0 at tick 156). | Legs that change. A pop at tick 30 or 150: the largest one-frame step of the arms in the ramp is 19.7 cm (clay) / 21.6 cm (night), against H0's 2.2 cm. Judge whether that reads as a quick brace or as a snap. |
| 2 | Hold at a sprint | H5 at 9 m/s, the same calls | The same pose over the faster stride | The stick crossing the head. The blade is 110 cm up at its highest: check that it does not read as a raised stick. |
| 3 | Release from the hold | H2 standing: hold on at tick 30, `PlayCrossCheck(0)` and `PlayLift()` on tick 90 | `CrossCheck_Release` (stand-in: `Lift` from its frame 2) plays from tick 90 to 104. The hold fades under it in 3 frames. `PlayLift` on the same frame is skipped. | A double take. A 13 cm hand drop on tick 90 was measured (the release's first frame against the held pose): look at ticks 89 to 91 frozen. |
| 4 | Cross-check without a hold (control) | H3: `PlayCrossCheck(0)` and `PlayLift()` on tick 30 | Today's `Lift_Light` (clay) / `Lift` (night), unchanged | Anything new |
| 5 | Hold, then a knockdown | H4: hold on at tick 30, knockdown on tick 60 (110 frames), hold off on tick 64, up on tick 170 | The fall, the lie and the get-up exactly as today. The hold weight is 0 from tick 61. | The braced arms during the fall or the get-up |
| 6 | Celebrations | C0 to C3: `PlayCelebrate(0..3)` on tick 30 | 58 frames each (the stand-in `Celebrate` take under four names) | A take that starts while down (C4: knocked down on tick 20, `PlayCelebrate(0)` on tick 40 is skipped) |
| 7 | Equipped props | E1 `SetBoostEquipped(shotgun, true)` on tick 20, knockdown on tick 60, up on tick 170, off on tick 250. E2 hip_check standing. E3 shield at 6 m/s. E4 slow_puck standing. E5: slow_puck on at 20, off at 100, shotgun on at 120. | The keg on the back (shotgun), the mallet hanging at the right hip (hip_check), a round timber disc on the back (shield), the rolling pin across the lower back (slow_puck). Through E1's whole knockdown the keg stays on the back (9.05 to 9.16 cm from `spine_03` at every frame). | A prop through the body. The placement was measured against the joints, not against the skin, so this shot decides it. A prop floating away from the back. A prop left after its off call. |

Stills to keep (`devlog\shots\<yyyymmdd-hhmmss>-B-e2-<what>.png`, each with a caption):
- 1 at tick 60, side-on and from the game camera
- 2 at tick 100
- 3 at ticks 89, 90 and 94
- 6: one frame per take at its middle (tick 60)
- 7: each prop from behind and from the game camera, plus E1 at tick 100 (lying)

**Not filmable yet:** the brace's hit, where a `check` bus event for the skater arrives while held. The figure plays `CrossCheck_Release` instead of a check take.
The game's scripted human does not brace on this tree, because no human input reaches `bCrossHold` before the controls lane lands.
In a match with braces on, look for the log line `E2 cross release why 2`.

## Job 5 additions: the pickups and the equipped set (14:55 onward)

**The pickups at 25 m and 45 m from the real game camera.** Scripts are in the E2 lab folder above. Never save the map.
1. Start a play session on `Rink` with the clay cartridge, the game's own camera, and no test camera.
2. Run `run pickcam.py` and keep its output (`[[x,y,z],pitch,yaw,fov]`; headless gave `[[77,996.5,350],-17.65,38.12,70]`).
3. Stop the session.
4. In the editor world, run `run pickput.py "MODE='put'" "CAM=<that output>" "DISTS=[25,45]"`. It places two rows of the six `SM_Boost_*` (speed, precision, heavy_puck, big_puck, reverse_other, custom) across the ice. The rows are 25 m and 45 m from the camera down the rink's long axis, spaced 2.5 m, each pickup turned 30 deg more than the last. Use `DIRX=-1` for the other way.
5. Start the session again and capture from the game camera with the human facing down the rink.
   - From a camera near centre ice, the 45 m row may reach the end boards. Start with the human near his own blue line facing the far end.
   - A188 puts the real pickups on a pedestal of light, scaled to 0.8 m. These rows are the bare meshes at scale 1 (64 to 83 cm tall), standing on the ice.
6. Clean up with `run pickput.py "MODE='clear'"`.

What must be seen: each of the six told apart by shape and colour at 25 m, and at 45 m. What must never be seen: two kinds that read alike, or a pickup reading as a weapon.

**The four equipped props on a skating figure.** Session `fa_Q`, runner `tx.sh <name> Q` with `NREC=280`.
- Figure `M` skates at 6 m/s with all four equipped from tick 10; the shield goes off at tick 250.
- Figures G0 to G3 loop `Glide`, S0 to S3 loop `Stride`, one kind each.
- K0 to K3 wear one kind each and are knocked down on tick 40 (110 frames, up on 150).

What must be seen:
- the keg high on the upper back, about 1 to 2 cm off the jersey
- the yellow bumper on the left back of the hips
- the green round shield on the back
- the snowflake crest on the helmet
- every prop gone from the knockdown's second frame until after the get-up (measured: hidden 41 to 150, shown again on 151)

What must never be seen: a prop through the jersey or the arms in Glide and Stride; a prop under the ice.
Stills: M from behind, from the side and from the game camera; K0 on tick 80, which must show no keg.
