## H4 Four hours dark: the night Windows Update took

Windows Update restarted device B at 02:34, one minute after it restarted device A at 02:33 (`status/B.md` H4, `status/A.md`). The machine then sat in standby, nobody logged on, until 09:56. The 06:00 task `ThirdJam-H0-B` never ran, and the interactive session with wake-ups set for H0 died in the restart. Device B did not exist for four hours.

The task would not have been a clean start anyway. `register-h0.ps1` passes the date to `schtasks` in one format, and `schtasks` reads it in the system's regional one: on a day/month locale it registered **11 December**, then printed a PASS line with the date we asked for, not the date it had registered (`handoff/B-to-A-0002.md`). We corrected device B's trigger by hand just after registering it at 01:05. Device A's task had the same bug and was never corrected (A's response), so A's start would not have fired even with someone logged on. B's would have fired, but only with a logon, and there was none.

The note about the bug was uncommitted when the restart hit, so A never saw it.

## H4.1 The machine that was not the machine in the plan

The plan (`docs/TRACKS.md`) assumed 128 GB of memory and a discrete GPU. The audit we ran between 00:00 and 00:15 found an ASUS ProArt P16 running Windows 11 on **ARM64**, with an NVIDIA RTX Spark N1X (20-core CPU, 6144-core GPU) and **64 GB of unified memory** shared between CPU and GPU. The Unreal editor is an x64 binary, so it ran under emulation (editor log: `Architecture: x64`, `CPU: Virtual CPU`).

The night before, a ComfyUI server held 51 of the 64 GB and left 12 to 14 GB free. The operator had said we could close it from 06:00; the restart did it for us, and at 10:02 we had 50.6 GB free.

The Director's lane counts (three editors, seven builders, a 24 GB free rule) had been written for the bigger machine. We planned one editor and a second only after measuring (`status/B.md`, pre-jam and H4).

## H4.2 Exit code -2147450730, and the fix that could not live in the repo

On the first try `tools/run/build.ps1` died after 2 seconds with exit -2147450730 and an empty log. The engine's `GetDotnetPath.bat` picks a `win-arm64` .NET when the processor is ARM64, but the launcher engine ships only `DotNet/10.0/win-x64`. `dotnet` then resolved to the system .NET 8 runtime with no SDK, and UnrealBuildTool, which needs .NET 10, could not start.

The batch file skips its own choice when `UE_DOTNET_VERSION` is already `10.0`, so seven environment variables set first were enough. We put them in a script outside the repository (`tools/**` is device A's) and dot-sourced it in every engine shell. We did not set them machine-wide: a global `DOTNET_ROOT` would break native ARM64 .NET programs.

We opened `B-to-A-0001` with a proposed patch. Device A answered at 11:25 and landed it in `tools/run` (H5.5). We had proved the fix on UE 5.8.4 as a stand-in, because 5.9 was still downloading.

## H4.3 A smoke script that failed while everything worked

On UE 5.9, from 10:15 to 10:22, the numbers came in slower than device A's (`status/B.md` H4): a blank C++ project built in 104.6 s against A's 64.6 s, the real project in 76.5 s against 56 s, and the editor answered MCP 45 s after start against 40 s.

Three things in the scripts did not survive this machine, and none was a machine fault:

- `smoke.ps1` calls the first tool 1 to 7 s after MCP answers. The editor was still in its start-up slow task and refused (`Not run: the Unreal Editor is running a slow task`). Tools worked 7 to 8 s later, so the script reported FAIL on a healthy editor.
- After a forced stop, port 8000 stayed in LISTENING with owner pid 0 for about 100 s, and `editor.ps1` refused to start.
- The first 5.9 start sat on a modal box, **"Wait for ZenServer?"**: the cache server took 38 s and the editor asks after 20. The second start had no box.

## H4.4 The painted-cel look in about ten minutes

Lane B1 (`status/B1-report.md`) built five master materials, seventeen instances and the night rig `LR_L2_Night` through MCP calls, with no editor clicks. The five masters went in one call that took 7.7 s. Two post-process passes do the style: a toon ramp that bands light into steps, and an ink outline, both keyed on custom-depth stencil values. Each costs 0.04 ms.

`status/B.md` says ten minutes; the report's own times, 10:23 to 10:35, make it twelve. PIE GPU time was 10.5 ms median with a second editor view in the frame, a device B number and not the gate's.

The one slip was a process one. Adding a map as a streaming sub-level makes it the current level, so whatever is spawned next lands in it and is saved with it. That rule went into `briefs/B-machine-notes.md`.

## H4.7 A puck you can find, and sounds nobody had heard

Lane B3 worked in a second tree with its own editor (port 8001) from 10:38. Part 1 was the puck readability kit (`status/B3-report.md`): `SM_PuckDisc`, a glowing 60 cm by 4 mm disc made by scaling a duplicated engine cylinder, and `NS_puck_trail`, a ribbon that is off under 4 m/s and full from 8 m/s.

One thing nobody would have guessed: with Lumen on, a moving emissive disc left fading ghost copies of itself on the ice until `bAffectDynamicIndirectLighting` was switched off. That is a component flag, so only device A's code can set it, and `B-to-A-0007` says so. It was still open at 14:20.

Then came one effect and one sound for every event. The MetaSound graphs were checked by recording the output to a wav and reading its peak and length. Nobody listened to them (`status/B.md` H5.3).

## H4.8 The arena becomes a sound stage

The theme is Director's Cut, so the night arena became a film sound stage. Lane B2 placed 289 actors of engine basic shapes (`Dress_L2_Night_1..289`) around the ice: lighting truss, eight film lights on tripods, a director's chair, a clapper slate, a camera crane, dolly track, 75 flight cases (`status/B2-report.md`). The total was 48.8k triangles and about +0.3 ms.

Everything had to stand outside a 140 by 70 m footprint so it never covered the ice or the far net. Two things did not work. The doors in the boards were not built, because the boards belong to device A. The cables were black on black and effectively invisible. The truss beams had to change material for the same reason: dark on the black room disappeared in the first capture.

## H4.9 Three editors, then two: the cap that arrived late

At about 10:30 the operator asked for more builders in parallel. We started three lanes at 10:40. The Director's cap (R-0002 point 6, built on our own pre-measurement note that B plans one editor) reached us at 10:46, after the lanes were running. We measured and wrote it up straight away: free memory 37.2 GB with three editors, but GPU utilisation 93%. Memory was not the limit; the GPU was (`handoff/B-to-A-0006.md`).

The handoff says it plainly: "B did not know the cap when it started them and has not stopped them."

Then at 10:55 the operator decided, in the words quoted in the same file: "Keep 2 editors and expand builders so we can swiftly experiment with different skater approaches." We closed the main-tree editor at 10:57 through `ExitEditor`, which freed the port at once.

## H5 The skater lab: four experiments, no editor

The decision opened the lab. Four lanes never started Unreal, each asking one question of the skater, under a shared rubric (`briefs/B-skater-lab.md`, `status/B.md` H5.1). S1: do other figure styles beat the baseline? S2: can text-to-motion clips cover nine animation needs? S3: does a mesh made in Hyper3D Rodin, rigged in Meshy, work? S4: can a Blender script alone build an original figure with exact looping clips?

S1 to S3 ran on Sonnet and S4 on Opus, per the lane table. A fifth lane, V1, wrote a plan for tomorrow's visual upgrade with ready-to-run test scripts. The lab's credit caps added up to 375 of 1204 Meshy credits. CPU was at 13% before the lab started, which is why no-editor lanes were nearly free.

## H5.1 Four figure styles, and the texture that could not be swapped

Lane S1 generated three new Meshy styles against the baseline figure: a big-head mascot, a lean athlete and a wooden toy (`status/B-skater-s1.md`). It spent 65 of its 150 credits in about ten minutes. The first mascot preview had a head only a quarter of the body, and one retry fixed it.

At 40 pixels tall, the size the game camera mostly shows, the lean athlete failed: its head was 3 px and its thin limbs vanished. All four rigs had the same 28 bones.

The finding that cost us a design: blue and orange refines of the same figure had identical geometry but a different UV layout and vertex order, so **the orange texture cannot simply be swapped onto the blue rigged mesh**. Team colour became a hue-key in the material instead.

The lane ranked the wooden toy first and the mascot second; `status/B.md` H5.3 reports the mascot as the pick. The two disagree.

## H5.2 A jersey number nobody asked for

Lane B5 took the baseline figure all the way through Unreal (`status/B5-report.md`): mesh, a 29-bone skeleton (the importer added a root), clips, a hue-key team-colour material, an animation blueprint and `BP_SkaterVisual_TB2` answering all eight calls of the visual interface.

Then the verdict: **figure 1 cannot ship.** Its texture carried numerals ("29" and "25") on the jersey back and sleeve, plus small marks on the socks and helmet. Our rules say no text, letters or numbers in any asset (`briefs/B-machine-notes.md`). Lane S1's contact sheet had already caught the "29" at about 11:01. We marked every row `shipped = n`, "test only".

The swap into a test cartridge could not be checked: at that moment nothing in the game read `game.skater.visual`, and no bot test existed.

## H5.2 The Rodin route: 1.5 credits and under three minutes

Lane S3 tried the route the lead liked: mesh in Hyper3D Rodin, rig in Meshy (`status/B-skater-s3.md`). It worked first try. Rodin made a 19,282-triangle skater in 37 s for 0.5 credit. Meshy's rig step accepted Rodin's download link directly (5 credits, 28 bones with the same names), and the existing stride played on it for 3 more.

The whole route cost 1.5 Rodin and 16 Meshy credits, in under three minutes of API time. `status/B.md` says "0.5 Rodin credit"; the report counts three generations.

The lane still ranked it a second source, not a replacement: three 2K maps for a figure that needs one, mitten hands, and the same stride clip with the same faults. The `TAPose` flag barely changed anything and made gloves intersect the torso. The same lane made the first prop, a studio spotlight, for 0.5 credit.

## H5.3 Generated strides do not loop

Lane S2 asked Meshy for clips for nine needs (`status/B-skater-s2.md`). Skating looked right in stills, but no generated stride looped: seams of 53 to 85 degrees between first and last frame, and the figure drifted 1.8 to 6.04 m across one clip. A wrist shot barely moved and had no stick, and the body check was weak. Meshy's library of 678 presets had nothing for skating, gliding or shoulder checks.

The lane made one mistake of its own: it started the generation script twice by accident, so the stride prompt ran twice for 13 extra credits. It kept both. The same prompt gave 3.76 m of drift one time and 6.04 m the next, a free sample of how much generation varies. The lane spent 138 of 150 credits.

The workable answer was a Blender script that strips drift, trims and loops.

## H5.4 The best-moving figure came from a script and cost nothing

Lane S4 asked whether Blender Python alone could build an articulated skater with looping clips (`status/B-skater-s4.md`). Yes: `build_skater.py` and `build_clips.py` rebuilt the whole figure from nothing in 82 s with one command. No service, no download, no credits.

The figure had 5,344 triangles, 24 bones with a real `root`, and ten clips at the 11:25 report (fifteen after a polish at 11:32). Loop seams were 0.000 cm and pelvis travel was 0.000 cm on every frame. It went through four passes: the first was slim and let the stick sink 14 cm into the ice.

The lane called it "programmer art" in its own rubric (no face, no hands, no cloth folds) and first rated it a fallback. It became track T-B1.

## H5.4 A lane crashed the editor with another lane's script

At 11:23 lane B1b ran the plan's test script T1, written by the planning lane V1, on the main editor. It duplicated a map asset and then loaded a level while Python still held the new world. Unreal raised the fatal error `World Memory Leaks` (`EditorServer.cpp:2543`) and the editor died (`status/B1b-report.md`).

We restarted it at 11:24 on port 8000 with 44 GB free. Nothing was dirty, so nothing was lost. B1b fixed the script: save the duplicate, drop the Python reference, run garbage collection, then load. It also found that `execute_python` rejects an `import` inside a function.

The script had been written by a lane with no editor, so it had never run against one until B1b ran it on the lead's.

## H5.6 The black ice between the pools

The first look stills showed the ice between the four light pools as pure black, and the real rink confirmed it: side-band luma 0.7 of 255 (`status/B1b-report.md`). Painted pools look fine; the rest of the rink looked like a hole.

Three fixes failed first. A higher sky light did nothing, because Lumen occluded it completely. Two house-fill spots reached only 10.8. A noise-modulated ambient term was cut by the toon ramp into blotches, and its dimmer setting dropped the puck below the contrast bar.

The one that held was a small `IceAmbient` term in `M_Ice` that follows the base colour: band luma 0.7 to 46.5 (46.9 on the real rink), pool centre unchanged within 0.3%, the blue-line ripple down from 74 to 21. Every frame number was marked unconfirmed, because two editors shared the GPU.

## H5.8 Generated props replace 144 basic shapes

Dressing pass 2 (`status/B2b-report.md`) swapped our shape-built set pieces for seven Hyper3D Rodin props: film light, director's chair, slate, camera dolly, crane, flight cases and truss lamps. The import route worked first try, no Blender: Unreal's Interchange through a plain import task, then renaming and a 1K re-export of each 2K texture.

**144 actors were removed**: 80 film-light parts, 13 chair, 14 slate, 13 crane and 24 case cubes. In their place went 35 prop actors. To stay inside the triangle budget we set Nanite to keep 50% of triangles; without it the total would have been about 335k. With it the dressing came to 188,742 triangles against 250k.

The weakest piece was the truss: it came out as a pale lattice whose lamps point sideways, and we kept it anyway. The seven props cost 4.5 Rodin credits.

## H5.9 The hybrid: a generated mesh, scripted motion

Lane S5 asked whether lane S4's exact clips could drive the Rodin mesh, so the game gets the soft generated shapes and the zero-drift motion (`status/B-skater-s5.md`). They could. It used direct matrix maths, with a leg-length ratio of 0.586 between the two figures, and all fifteen takes played with 0.000 cm seams. The lower skate sat on the ice within 0.29 cm. It used no credits and no network, and rebuilt in about three minutes.

Lane S6 then cleaned lace-like marks off the Rodin texture: 130,234 of 4,194,304 pixels changed. That may have been wasted work, and the reports disagree. S6 found the marks as about 160 thin strips in the texture. In the engine, lane B6s concluded the three dark strokes were outlined lace bars modelled in the mesh, made no texture patch, and noted that the outlines do not show at game-camera distance (still `b6s-boot`).

## H6.2 The FBX files Unreal refused

At 12:09 and again at 12:19 the two lab lanes exported FBX files that Unreal 5.9's importer rejected with "Cannot load the FBX file. There was an error when parsing the file." (`status/B4-report.md`). Lane B4 wrote a note for the lead at 12:16.

The cause was one step in both lab scripts, `strip_tracks.py`, which rewrote the files after export. The same scripts run without that step imported cleanly. We disabled it in both and told the labs to publish the importable file.

Earlier lane B4 had already learned three other things Blender could not show. Two-frame lean holds imported as copies of the glide pose. The skeleton root arrived with scale 100 and a 90 degree roll, so sockets needed the inverse. An animation-only re-import keyed the root at scale 1 and shrank the figure to a 1.7 cm dot.

## H6.4 Stage light, and a test of haze, shafts and a painted backdrop

Lane B2c moved the stage pieces into groups at the two ends and added eight shadowless practical lights, one beside each film light (`status/B2c-report.md`). The ice stayed the same: the largest brightness change was 0.45% against a 3% limit.

On a scratch copy it tested three atmosphere ideas. The script's haze density was too weak to see (far end +1.0 luma), and 0.0003 gave +13.7. A painted backdrop flat at brightness 4.5 gave a theatre-sky ridge with stars and a beacon at 244 luma against 36 beside it. The first light-shaft version looked washed out, and the second added a fade near the camera.

The lane proposed a ship set and applied none of it. The atmosphere pass waits for the operator's veto window, open until 21:00 (`status/B.md` H7.7).

## H6.7 A request the other way: `main` did not compile

At 12:38 we merged `main` into `dev-b` and the build failed (opened 12:40): `BarkWidget.cpp` includes `MenuStyle.h`, and that header was in no commit. We checked the git tree and also found the default map, `Menu`, was not in git either. On a clean checkout the game's first map would not exist (`handoff/B-to-A-0010.md`).

The file compiled on device A only because it sat on A's disk, uncommitted. Until about 12:55 `main` did not build on B, which also held up the microphone test A had asked for (`A-to-B-0005`). The status says the break was later fixed on `main`.

Device B's tree held only what git held, so it was the first clean-checkout build. We opened the handoff with the exact error line, `main` commit `706f232` and `exit 6`.

## H6.9 Both figures play real matches through a key that already worked

Lane B6s set out to bolt both skater visuals onto device A's pawns from Python. It did not have to. `AHokejGameMode::ResolveVisualClass` already mapped `game.skater.visual` to the figure classes, so setting one console variable ran the real path with no code change (`status/B6s-report.md`). We withdrew the question we had asked A in `B-to-A-0008` at 12:55.

Both ran full bot matches. Verdict in play: the hybrid, for charm and team colour in the light pools. T-B1 read better between pools, because the hybrid's orange went brown-red in the dark gaps (its hue key multiplies team colour by texture brightness). Lane B4 had leaned the other way earlier: T-B1 "reads cleaner and moves better". Both stay selectable by cartridge key.

Still wrong: the figure leaned fully at 0 m/s, and the puck sat left of the blade.

## H7 Two hours of requests we had not read

At 13:00 we noticed a miss: `handoff/A-to-B-0001..0006` and rulings R-0006 to R-0013 had gone unread on device B from 11:10 to 13:00. We had read only the answers to our own handoffs (`status/B.md` H7.7).

Two of the unread items mattered. `A-to-B-0003` reported that the night rig hung device A's GPU within 4 to 7 s of PIE, and said it blocks the shipped look; it had been open since about 11:40. And R-0010 had moved the default rink from 120 by 60 to 90 by 45 m, after our rig had been built for 120.

We answered `A-to-B-0001` with the words "late: B did not read `handoff/A-to-B-*` between 11:10 and 13:00; it does at every fetch from now on." That rule is now number 13 in the machine notes.

## H7.1 105 audio takes that nobody had heard

Lane AU1 used ElevenLabs outside the repository (`status/B-audio-au1.md`). It made a director's voice in two stock voices, 29 effect files (26 planned and three retries), two music tracks, and then five designed commentator voices with ten lines each. Counting the `audio-lab` folder gives 105 mp3 files.

The lane could not listen, so every judgement is a number. Both goal horns clip at about +0.1 dBTP. Both music tracks end in a fade, so neither loops as delivered. Four of five clapperboard takes were nearly silent, one of them entirely empty. The docs promise 40 credits per second for effects; the account paid about 11.

Spend was 2,231 credits, then 1,455 more for the commentators. On the five voices the lane wrote: "I could not tell the previews apart by ear." It chose among them by duration.

## H7.7 The GPU hang we could not reproduce

Lane B1c ran 7 PIE runs, 11.5 minutes, with one editor and four mid-run rig loads, on the default 90 by 45 cut and the 120 by 60 one (`status/B1c-report.md`). It never saw a GPU crash, `DEVICE_HUNG` or a virtual-shadow-map warning. Device A's dump was a page fault with the same hash twice, under four or five editors on one GPU. The answer says that fits driver or contention better than a cost problem, and adds: "I cannot prove it from here."

We hardened anyway. No dressing piece casts a shadow, spot attenuation went from 40 to 30 m, shadow resolution to 0.5, and ray-traced shadows off. Shadow-casting primitives fell from 67 to 54, and `ShadowDepths` from 1.09 to 0.84 ms. Pools were re-laid at x = plus or minus 11.25 and 33.75 m. First single-editor numbers on B: GPU 11.5 to 12.1 ms median. The 8 ms gate is A's.

## H7.8 Phase 2: master the toy game, run clay in parallel

Device A's `A-to-B-0006` quoted the operator: master the painted toy-figure game, "while in parallel making effort to upgrade into P3 and P4". P3 is a clay miniature look and P4 a broadcast look, where the operator said they would be glad just to get regular hockey working.

Our reply put a table in `status/B.md` by time and editor: two editors plus up to three lanes without one; P2 polish and P3 clay in parallel from 15:15; P3 complete by H36. We also corrected one premise. P4 wanted humanoid figures, and both already existed: T-B1 and the hybrid played real matches that morning. The GPU hang came first by the Director's rule, and we answered it first.

## H7.95 A texture setting that sits in the other machine's file

`A-to-B-0004` asked for import defaults that keep the shipped zip small: BC1 with Oodle RDO at Medium on colour textures. Lane B8 set them on all eight textures (`status/B8-report.md`). On disk each `.uasset` grew by 294 bytes and nothing else visible changed.

The report says why: Medium does nothing unless `bFinalUsesRDO=True` is set in `DefaultEngine.ini`, and that is device A's file. So our change is a no-op until A adds a line. Shipped sizes are known only after a cook; our estimate (699,050 bytes per 1K texture) is arithmetic, not a measurement.

Two inputs were missing from our tree: `handoff/A-to-B-0004.md` and `docs/experiments/SIZE.md`. We rebuilt the handoff header from A's brief, flagged it inside the file, and answered at 13:57.

## H8.1 One build at a time, on the whole machine

Our first package attempt on ARM64 hung in UnrealBuildTool at a compile step after 16 minutes and was stopped (`status/B.md` H7.7). At 14:05 we wrote a new rule into the machine notes (`briefs/B-machine-notes.md`, rule 11).

`RunUAT BuildCookRun` starts UnrealBuildTool without `-WaitMutex`. If any other UnrealBuildTool is running anywhere, even a `build.ps1` in another worktree, the package dies in 12 s with `ConflictingInstance` and a misleading exit, `Error_SDKNotFound`. `build.ps1` itself waits for the lock. The sources do not date the failed run.

Rule 12 sits beside it: one editor in PIE measured 87% of the GPU, with 41 GB of RAM free and CPU at 28%. At 14:20 the package lane was running again, and the first Development package on device B was still unproven.

## H8.2 What only the human could do

Some of the work could not be done by an agent. The operator had to listen to 105 audio takes on `listen.html` and choose a voice; the audio import waits for the picks. The operator had to speak into the microphone for `A-to-B-0005`: "an agent cannot speak into the microphone." The test is ten minutes of work and was still waiting at 14:20.

The operator also had to read licence terms we could not settle: Hyper3D's wording on shipping Rodin output (the hybrid's mesh and seven props depend on it; T-B1 and the shape-built dressing do not), and ElevenLabs' music terms. The lane's reading, that a jam game is an indie game, is marked not verified.

And the decisions were theirs: two editors, more builders, and at 14:10 "push lanes requiring no editor and track in the github so Device A is informed and can sync."

## H8.3 How the two machines talk

Requests cross the ownership line as files in `handoff/`: one per request, the requester writing above a `## Response` heading and the receiver writing below it. The statuses are `accepted`, `done` or `declined`, and the README adds: "A decline is a counter-proposal, not a refusal: measurement outranks the brief." By 14:20 there were eleven requests from B to A and six from A to B.

Two lines show the register. From A, on the hang: "It is the shipped tier; if it hangs on a judge's machine the entry is dead." From B, in answer: "I cannot prove it from here."

Rulings come from the Director on device A; merge windows give both sides a time to sync, the next at 15:00. At 14:20 four of our requests were still open: `0007` (effects and puck kit), `0008` (T-B1), `0009` (five look defects on A's side) and `0011` (scale the rig with the rink).

## Screenshots worth showing

- 20261112-103012-B-b1-l2night-mid-ice.jpg: the painted-cel look at its first moment, banded pools and ink outlines, with the black between the pools.
- 20261112-113701-B-b1b-before-band.jpg: the real rink before the fix, with the ice between the pools pure black.
- 20261112-113701-B-b1b-after-band.jpg: the same view after the ambient term, with a navy floor and the pools still banded.
- 20261112-104742-B-b2-dressing-own-end.png: dressing pass 1, the sound stage built from basic shapes.
- 20261112-115044-B-b2b-own-end.png: the same end after the Rodin props, with the crane, truss and slate.
- 20261112-115046-B-b2b-close-chairs-slate.png: generated chairs, flight cases and slate close up, outlined by the post pass.
- 20261112-104734-B-b3-puck-dark-2400.jpg: the puck at 24 m on dark ice, a thin yellow-green line between the pools.
- 20261112-110308-B-b3-fx-goal-s10.jpg: the goal sparks and the puck's halo disc.
- 20261112-110132-B-lab-s1-turntable.png: four figure styles in rows, with the baseline's printed "29" visible in the bottom row.
- 20261112-110633-B-lab-s2-stride2b.png: one generated stride, eight frames; it reads as skating and does not loop.
- 20261112-110742-B-lab-s3-rodin-turntable.png: the Rodin skater above the Meshy baseline, with the "29" on the baseline sleeve.
- 20261112-112323-B-lab-s4-skating.png: the scripted figure in Blender frames, stick and all.
- 20261112-115434-B-lab-s5-compare.png: scripted original, hybrid and generated clips side by side for stride and slap shot.
- 20261112-122758-B-tb1-vs-tb2h.png: the two shipped figures from above in the real game.
- 20261112-125215-B-match-tb2h-1.jpg: the hybrid in a live bot match.
- 20261112-125216-B-match-tb1-1.jpg: the scripted original in a live bot match.
- 20261112-123256-B-b2c-t2b-far-ship.jpg: the proposed atmosphere set with painted backdrop, stars and film set.
- 20261112-133445-B-b1c-after-pool.jpg: the pools re-laid for the 90 by 45 rink, with figures casting shadows.
- 20261112-124227-B-b6s-mitts.jpg: the mitt colour key, before on the left and after on the right, in blue and orange.
