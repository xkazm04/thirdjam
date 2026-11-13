"""tools/blender/arena/engine_stills.py: the arena pieces rendered by the engine, off screen, without a game (Third Jam, track F3, lane A119).

Why this exists: a match locks the mouse to its viewport (AHokejGameMode::EnsureMatchInputMode, EMouseLockMode::LockAlways), which Slate turns into a
ClipCursor on the one system cursor; a lane that does not own the cursor cannot start a match. This script starts NO game: no player controller, no input
mode, no window on screen. It opens device B's rig map in an off-screen editor, spawns the real ARinkActor (transient, nothing is saved), builds it for a
rink and a look tier, sets MPC_Look.Clay as the rig's BP_LookParams would at BeginPlay, and takes stills from fixed cameras at the game camera's height
(3.5 m) and field of view (70 degrees). It is the lane's own look at the pieces under B's materials and lights; it is NOT a still from the game
(no figures, no HUD, no tier switcher).

Run (Git Bash, from the worktree root; one process for every still):
  "C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "$PWD/Game/ThirdJam.uproject" -ExecCmds="py $(pwd -W)/tools/blender/arena/engine_stills.py" \
      -RenderOffScreen -Multiprocess -unattended -nopause -nosplash -log -abslog="$PWD/Game/logs/a119-stills.log"
(-ExecCmds, not -ExecutePythonScript: that one quits the editor as soon as the script returns, before a frame is drawn. The rig map is changed in memory
only: it is loaded again with the Discard policy between jobs and the editor quits without saving; the script never saves anything.)
Writes PNG files to tools/blender/arena/out/engine/ and prints "A119 STILLS ..." lines; quits the editor when done or after its frame budget.

Evolution 3 (lane A127): ARENA_STILLS_REACT=1 shoots the arena answering the game instead, to tools/blender/arena/out/engine-e3/: six frames after a goal
(sim frames 0, 3, 8, 18, 45, 120) of the scored net with its goal light and of the scorer's table's lamp, the game camera's view lit and at rest, and a
penalty box at rest and on the frame of a hard board hit. No game runs: the script calls ARinkActor.ReactToEvent / AdvanceReactions (what the event bus
and the actor's tick call in a match) with the frame it wants to look at.
"""
import os

import unreal

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "out", "engine")
RIG_LENGTH_M = 90.0          # ULookTierSwitcher::RigRinkLengthM: the rig is built for a 90 m rink and scaled with the rink's length
WARM = 70                    # editor ticks between placing the camera and the shot (exposure, Lumen, virtual shadow maps settle)
RES = (1600, 900)
FOV = 70.0
ONLY = os.environ.get("ARENA_STILLS_ONLY", "")            # e.g. "l2_night-90" to render one job
SUFFIX = os.environ.get("ARENA_STILLS_SUFFIX", "")        # appended to the file names (an A/B)
NO_STENCIL = os.environ.get("ARENA_STILLS_NO_STENCIL", "").split(",")   # component properties whose custom depth is switched off (an A/B of the outline)
JOBS = [("p3_clay", "LR_P3_Clay", 1.0, 90, 45), ("p3_clay", "LR_P3_Clay", 1.0, 120, 60), ("l2_night", "LR_L2_Night", 0.0, 90, 45), ("l2_night", "LR_L2_Night", 0.0, 120, 60)]
REACT = os.environ.get("ARENA_STILLS_REACT", "") in ("1", "sweep", "place", "caps")   # lane A127: the reactions' stills
# Lane A136 (evolution 4). ARENA_STILLS_REACT=place: the goal's frame from the game camera on p3_clay with the scored end's lamp light at several places
# (ARENA_STILLS_PLACES="dx,y,z;..." in metres from the goal line, "cur" = where Build put it) and candelas (ARENA_STILLS_LIGHT), with stand-ins for a puck
# and two figures (engine cylinders on B's instances, transient) so a washed-out picture shows. ARENA_STILLS_REACT=caps: the board caps from the game
# camera's angle on both long sides and both ends, at rest and on the frame of a hard hit, and (ARENA_STILLS_GPU=1) ProfileGPU pairs with jam.rink.caps 0 / 1.
PLACE = os.environ.get("ARENA_STILLS_REACT", "") == "place"
CAPS = os.environ.get("ARENA_STILLS_REACT", "") == "caps"
PLACES = [None if t.strip() == "cur" else tuple(float(v) for v in t.split(",")) for t in os.environ.get("ARENA_STILLS_PLACES", "cur;1.0,0,5.0;-2.5,0,6.5").split(";")]
PLACE_CDS = [float(v) for v in os.environ.get("ARENA_STILLS_LIGHT", "3000,6000,12000,24000").split(",")]
GPU = os.environ.get("ARENA_STILLS_GPU", "") == "1"
SWEEP = [float(v) for v in os.environ.get("ARENA_STILLS_LIGHT", "50,200,800,3000,12000").split(",")] if os.environ.get("ARENA_STILLS_REACT", "") == "sweep" else []
REACT_FRAMES = [0, 3, 8, 18, 45, 120]
if REACT:
    OUT = os.path.join(HERE, "out", "engine-e3")
    JOBS = [("p3_clay", "LR_P3_Clay", 1.0, 90, 45), ("l2_night", "LR_L2_Night", 0.0, 90, 45)]
if PLACE:
    OUT = os.path.join(HERE, "out", "engine-e4-place")
    JOBS = [("p3_clay", "LR_P3_Clay", 1.0, 90, 45)]
if CAPS:
    OUT = os.path.join(HERE, "out", "engine-e4-caps")
    JOBS = [("p3_clay", "LR_P3_Clay", 1.0, 90, 45), ("p3_clay", "LR_P3_Clay", 1.0, 120, 60), ("l2_night", "LR_L2_Night", 0.0, 90, 45)]
if os.environ.get("ARENA_STILLS_OUT", ""):
    OUT = os.path.join(HERE, "out", os.environ["ARENA_STILLS_OUT"])


def log(msg):
    unreal.log("A119 STILLS " + msg)


def views(length, width):
    gl = length * 0.5 - 4.0
    bx = 0.0833 * length
    if REACT:   # (name, eye, target, action): action = ("goal", frames since the goal) | ("rattle", frames since the hit) | None (the still arena)
        hw = width * 0.5
        if PLACE:   # lane A136: the game camera's view of the scored end: at rest, then the goal's frame for every place and candela
            eye, tgt = (gl - 11.0, 1.9, 3.5), (gl - 1.5, 0.6, 0.6)
            out = [("place-rest", eye, tgt, None)]
            for pi, pos in enumerate(PLACES):
                for cd in PLACE_CDS:
                    out.append(("place-p%d-cd%05d" % (pi, cd), eye, tgt, ("goal", 0, cd, pos)))
            if os.environ.get("ARENA_STILLS_PLACE_FRAMES", ""):   # the last place and candela at later frames of the flash
                for f in [int(v) for v in os.environ["ARENA_STILLS_PLACE_FRAMES"].split(",")]:
                    out.append(("place-final-f%03d" % f, eye, tgt, ("goal", f, PLACE_CDS[-1], PLACES[-1])))
            return out
        if CAPS:    # lane A136: the board caps. The game camera stands 6 m behind its skater at 3.5 m; both long sides, both ends, a hit, a close look
            hx, hy = length * 0.25, hw - 0.4
            out = [("caps-side-pos", (-6.0, hw - 12.0, 3.5), (2.0, hw - 1.0, 0.6), None),          # a skater 1 m off the +y boards, play going +x
                   ("caps-side-neg", (-length * 0.25 - 6.0, -(hw - 12.0), 3.5), (-length * 0.25 + 2.0, -(hw - 1.0), 0.6), None),
                   ("caps-near-pos", (0.0, hw + 2.6, 3.5), (6.0, hw - 6.0, 0.6), None),             # the camera pushed outside the +y boards (a skater on the near boards)
                   ("caps-near-neg", (length * 0.2, -(hw + 2.6), 3.5), (length * 0.2 + 6.0, -(hw - 6.0), 0.6), None),
                   ("caps-end-pos", (gl - 11.0, 1.9, 3.5), (gl - 1.5, 0.6, 0.6), None),
                   ("caps-end-neg", (-(gl - 11.0), -1.9, 3.5), (-(gl - 1.5), -0.6, 0.6), None),
                   ("caps-own-end", (length * 0.5 + 2.4, -3.2, 3.5), (gl - 4.0, -0.8, 0.3), None),   # the camera behind the end boards, beside the goal light (at y -0.7 it stood inside the lamp)
                   ("caps-hit-rest", (hx - 5.0, hw - 6.5, 2.4), (hx, hw + 0.1, 1.3), None),
                   ("caps-hit-f000", (hx - 5.0, hw - 6.5, 2.4), (hx, hw + 0.1, 1.3), ("cap", 0, (hx, hy))),
                   ("caps-hit-f003", (hx - 5.0, hw - 6.5, 2.4), (hx, hw + 0.1, 1.3), ("cap", 3, (hx, hy))),
                   ("caps-hit-game-rest", (hx - 8.0, hw - 12.0, 3.5), (hx, hy, 0.6), None),
                   ("caps-hit-game-f000", (hx - 8.0, hw - 12.0, 3.5), (hx, hy, 0.6), ("cap", 0, (hx, hy)))]
            return out
        if SWEEP:   # the lamp light's candelas (jam.rink.react.light), on the goal's frame: the game camera's view and the table
            out = []
            for cd in SWEEP:
                out += [("react-game-f000-cd%05d" % cd, (gl - 11.0, 1.9, 3.5), (gl - 1.5, 0.6, 0.6), ("goal", 0, cd)),
                        ("react-table-f000-cd%05d" % cd, (-3.6, -(hw - 7.5), 2.9), (0.2, -(hw + 0.9), 1.9), ("goal", 0, cd))]
            return out
        out = [("react-net-f%03d" % f, (gl - 6.5, 1.7, 2.9), (gl + 1.6, 0.0, 1.1), ("goal", f)) for f in REACT_FRAMES]
        out += [("react-table-f%03d" % f, (-3.6, -(hw - 7.5), 2.9), (0.2, -(hw + 0.9), 1.9), ("goal", f)) for f in REACT_FRAMES]
        out += [("react-game-f%03d" % f, (gl - 11.0, 1.9, 3.5), (gl - 1.5, 0.6, 0.6), ("goal", f)) for f in (0, 120)]
        out += [("react-box-rest", (-8.5, -(hw - 5.5), 2.0), (-3.0, -(hw + 0.8), 1.0), None), ("react-box-hit-f000", (-8.5, -(hw - 5.5), 2.0), (-3.0, -(hw + 0.8), 1.0), ("rattle", 0))]
        return out
    return [
        ("goal", (gl - 11.0, 1.9, 3.5), (gl - 1.5, 0.6, 0.6)),                               # 6 m behind a skater 5 m from the +x net
        ("goal-close", (gl - 5.0, 1.3, 2.2), (gl + 0.5, 0.0, 0.3)),                          # closer than the game camera comes: the frame and the cords
        ("goal-own-end", (gl + 3.3, -0.7, 3.5), (gl - 1.6, -0.2, 0.3)),                      # defending: the camera behind the own net, looking through its top
        ("boxes", (-7.0, -(width * 0.5 - 13.0), 3.5), (0.5, -(width * 0.5 + 0.8), 1.1)),     # the penalty side from the ice
        ("benches", (bx - 13.0, width * 0.5 - 12.0, 3.5), (bx - 2.0, width * 0.5 + 0.6, 0.9)),
        ("over", (-length * 0.30, width * 0.12, 14.0), (length * 0.18, -width * 0.10, 0.0)),  # not a game camera: everything in one frame
    ]


class Run:
    def __init__(self):
        self.job = -1
        self.view = 0
        self.wait = 0
        self.state = "next_job"
        self.frames = 0
        self.cam = None
        self.rink = None
        self.pending = None
        self.done = []
        self.gpu = []
        os.makedirs(OUT, exist_ok=True)
        self.les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        self.eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
        self.handle = unreal.register_slate_post_tick_callback(self.tick)

    def world(self):
        return unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()

    def start_job(self):
        tier, rig, clay, length, width = JOBS[self.job]
        # a fresh rig map for every job (the scale below is applied once); what the last job changed in memory is DISCARDED, never saved
        policy = [getattr(unreal, n) for n in dir(unreal) if "UnsavedChangesPolicy" in n]
        if self.job == 0 or not policy:
            ok = self.les.load_level("/JamAssets/Rigs/" + rig) if self.job == 0 else False
        else:
            ok = self.les.load_level("/JamAssets/Rigs/" + rig, policy[0].DISCARD)
        log("load %s -> %s (policy %s)" % (rig, ok, [str(x) for x in policy]))
        if not ok:
            raise RuntimeError("rig map not loaded")
        s = length / RIG_LENGTH_M
        if abs(s - 1.0) > 1e-6:      # what the level instance's transform does in the game: every actor's place and scale times s about the origin
            for a in self.eas.get_all_level_actors():
                root = a.get_editor_property("root_component")
                if not root:
                    continue
                loc = a.get_actor_location()
                sc = a.get_actor_scale3d()
                a.set_actor_location(unreal.Vector(loc.x * s, loc.y * s, loc.z * s), False, False)
                a.set_actor_scale3d(unreal.Vector(sc.x * s, sc.y * s, sc.z * s))
        mpc = unreal.load_asset("/JamAssets/Look/MPC_Look")
        if mpc:
            unreal.MaterialLibrary.set_scalar_parameter_value(self.world(), mpc, "Clay", clay)
        self.rink = self.eas.spawn_actor_from_class(unreal.RinkActor, unreal.Vector(0, 0, 0))
        cfg = unreal.JamGameConfig()
        r = cfg.get_editor_property("rink")
        r.set_editor_property("length_m", float(length))
        r.set_editor_property("width_m", float(width))
        cfg.set_editor_property("rink", r)
        look = cfg.get_editor_property("look")
        look.set_editor_property("tier", tier)
        cfg.set_editor_property("look", look)
        self.rink.build(cfg)
        for prop in [n for n in NO_STENCIL if n]:
            self.rink.get_editor_property(prop).set_render_custom_depth(False)
        self.cam = self.eas.spawn_actor_from_class(unreal.CameraActor, unreal.Vector(0, 0, 350))
        cc = self.cam.get_editor_property("camera_component")
        cc.set_editor_property("field_of_view", FOV)
        cc.set_editor_property("constrain_aspect_ratio", False)
        self.les.editor_set_game_view(True)
        self.les.pilot_level_actor(self.cam)
        if PLACE:   # stand-ins: pucks in the crease and the slot, two figures (engine cylinders, B's instances by reference, transient)
            gl = length * 0.5 - 4.0
            for nm, mat, loc, sc in (("puck", "MI_Toon_Dark", (gl - 1.6, 0.7, 0.04), (0.16, 0.16, 0.05)), ("fig0", "MI_Toon_Blue", (gl - 3.2, -1.6, 0.9), (0.55, 0.55, 1.8)),
                                     ("fig1", "MI_Toon_Orange", (gl - 5.5, 2.3, 0.9), (0.55, 0.55, 1.8)), ("puck2", "MI_Toon_Dark", (gl - 6.0, -0.8, 0.04), (0.16, 0.16, 0.05))):
                a = self.eas.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(loc[0] * 100.0, loc[1] * 100.0, loc[2] * 100.0))
                c = a.static_mesh_component
                c.set_mobility(unreal.ComponentMobility.MOVABLE)
                c.set_static_mesh(unreal.load_asset("/Engine/BasicShapes/Cylinder"))
                m = unreal.load_asset("/JamAssets/Look/" + mat)
                if m:
                    c.set_material(0, m)
                a.set_actor_scale3d(unreal.Vector(sc[0], sc[1], sc[2]))
                c.set_render_custom_depth(True)
                c.set_editor_property("custom_depth_stencil_value", 1)
        log("job %s %dx%d: rink built, arena shown=%s goals shown=%s clay=%s scale=%.3f" % (tier, length, width, self.rink.is_arena_dressing_shown(), self.rink.is_arena_goal_shown(), clay, s))
        self.view = 0
        self.place()

    def place(self):
        tier, rig, clay, length, width = JOBS[self.job]
        view = views(length, width)[self.view]
        name, eye, target = view[0], view[1], view[2]
        if REACT:
            self.react(view[3], length, width)
        e = unreal.Vector(eye[0] * 100.0, eye[1] * 100.0, eye[2] * 100.0)
        t = unreal.Vector(target[0] * 100.0, target[1] * 100.0, target[2] * 100.0)
        self.cam.set_actor_location_and_rotation(e, unreal.MathLibrary.find_look_at_rotation(e, t), False, False)
        self.les.pilot_level_actor(self.cam)
        self.wait = WARM
        self.state = "warm"

    def react(self, action, length, width):
        """Lane A127: puts the rink at one frame of a reaction (or at rest). The same two calls the event bus and the actor's tick make in a match."""
        self.rink.advance_reactions(10 ** 9)          # whatever the last still left live ends: the still arena
        if not action:
            return
        pl = unreal.JamEventPayload()
        if action[0] == "goal" and len(action) > 2:
            unreal.SystemLibrary.execute_console_command(self.world(), "jam.rink.react.light %g" % action[2])
        if action[0] == "cap":    # lane A136: a hard skater `board` event at (x, y) metres
            pl.set_editor_property("who", 0)
            pl.set_editor_property("where", unreal.Vector(action[2][0] * 100.0, action[2][1] * 100.0, 4.0))
            pl.set_editor_property("strength", 1.0)
            pl.set_editor_property("kind", "skater")
            self.rink.react_to_event("board", pl, 1000)
        elif action[0] == "goal":
            pl.set_editor_property("where", unreal.Vector((length * 0.5 - 4.0 + 0.5) * 100.0, 0.0, 4.0))   # the puck in the +x net
            pl.set_editor_property("strength", 1.0)
            self.rink.react_to_event("goal", pl, 1000)
        else:
            pl.set_editor_property("who", 0)
            pl.set_editor_property("where", unreal.Vector(-300.0, -width * 50.0, 4.0))                     # a skater into the boards at the -x penalty box
            pl.set_editor_property("strength", 1.0)
            pl.set_editor_property("kind", "skater")
            self.rink.react_to_event("board", pl, 1000)
        self.rink.advance_reactions(1000 + action[1])
        if len(action) > 3 and action[3]:      # lane A136: the scored end's lamp light moved to a trial place (metres: from the goal line along x, y, z)
            lights = self.rink.get_editor_property("arena_lamp_lights")
            lights[2].set_relative_location(unreal.Vector((length * 0.5 - 4.0 + action[3][0]) * 100.0, action[3][1] * 100.0, action[3][2] * 100.0), False, False)
            if len(action[3]) > 3:
                lights[2].set_attenuation_radius(action[3][3] * 100.0)
            log("light +x moved to %s" % (action[3],))
        log("light cd %s" % [round(self.rink.get_lamp_light_intensity(i), 1) for i in range(3)])
        log("react %s frame %d: lamp %.2f (rest %.2f) light+x %.2f net+x scale %s rattle %.2f deg %s" % (action[0], action[1], self.rink.get_lamp_emissive(),
            self.rink.get_lamp_rest_emissive(), self.rink.get_goal_light_emissive(1), self.rink.get_net_shake_scale(1), self.rink.get_rattle_deg(), self.rink.get_rattled_piece_name()))

    def shoot(self):
        tier, rig, clay, length, width = JOBS[self.job]
        name = views(length, width)[self.view][0]
        path = os.path.join(OUT, "engine-%s-%dx%d-%s%s.png" % (tier, length, width, name, SUFFIX)).replace("\\", "/")
        if os.path.exists(path):
            os.remove(path)
        unreal.AutomationLibrary.take_high_res_screenshot(RES[0], RES[1], path, self.cam)
        self.pending = path
        self.wait = 400
        self.state = "saving"

    def tick(self, dt):
        self.frames += 1
        try:
            if self.frames > 20000:
                log("frame budget spent")
                self.finish()
            elif self.state == "next_job":
                self.job += 1
                if self.job >= len(JOBS):
                    self.finish()
                else:
                    self.start_job()
            elif self.state == "gpu":       # lane A136: ProfileGPU pairs, the caps off and on, from one game-camera view
                self.les.editor_invalidate_viewports()
                self.wait -= 1
                if self.wait <= 0:
                    if not self.gpu:
                        self.state = "next_job"
                    else:
                        cmd, wait = self.gpu.pop(0)
                        log("gpu %s" % cmd)
                        unreal.SystemLibrary.execute_console_command(self.world(), cmd)
                        if cmd.startswith("jam.rink.caps"):
                            self.rink.set_construction_visible(True)     # what the cvar sink does at the end of the frame
                        self.wait = wait
            elif self.state == "warm":
                self.les.editor_invalidate_viewports()      # the off-screen viewport is not realtime: ask for every frame of the warm-up
                self.wait -= 1
                if self.wait <= 0:
                    self.shoot()
            elif self.state == "saving":
                self.wait -= 1
                if os.path.exists(self.pending) and os.path.getsize(self.pending) > 0:
                    self.done.append(self.pending)
                    log("saved %s" % self.pending)
                    self.advance()
                elif self.wait <= 0:
                    log("NOT SAVED %s" % self.pending)
                    self.advance()
        except Exception:
            import traceback
            unreal.log_error("A119 STILLS exception: %s" % traceback.format_exc())
            self.finish()

    def advance(self):
        tier, rig, clay, length, width = JOBS[self.job]
        self.view += 1
        if self.view >= len(views(length, width)):
            if CAPS and GPU and self.job == 0:
                hw = width * 0.5
                e = unreal.Vector(-600.0, (hw - 12.0) * 100.0, 350.0)
                t = unreal.Vector(200.0, (hw - 1.0) * 100.0, 60.0)
                self.rink.advance_reactions(10 ** 9)
                self.cam.set_actor_location_and_rotation(e, unreal.MathLibrary.find_look_at_rotation(e, t), False, False)
                self.les.pilot_level_actor(self.cam)
                self.gpu = []
                for i in range(10):
                    self.gpu += [("jam.rink.caps 0", 24), ("ProfileGPU", 10), ("jam.rink.caps 1", 24), ("ProfileGPU", 10)]
                self.wait = 40
                self.state = "gpu"
                return
            self.state = "next_job"
        else:
            self.place()

    def finish(self):
        if self.state == "done":
            return
        self.state = "done"
        unreal.unregister_slate_post_tick_callback(self.handle)
        log("DONE %d stills" % len(self.done))
        unreal.SystemLibrary.quit_editor()


if ONLY:
    JOBS = [j for j in JOBS if ("%s-%d" % (j[0], j[3])) == ONLY]
RUN = Run()
