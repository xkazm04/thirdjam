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
REACT = os.environ.get("ARENA_STILLS_REACT", "") in ("1", "sweep", "place", "caps", "e5", "e6")   # lane A127: the reactions' stills
# Lane A160 (evolution 6): ARENA_STILLS_REACT=e6: every prop the A side places, from the game camera's own pose (UJamFollowCamera::ComputeThirdPersonPose:
# 6 m behind the skater, 0.7 m to its right, 3.5 m up, pitch -atan(3.5 / 11) = -17.65 degrees, FOV 70) with the skater at the faceoff and in each corner,
# beside each piece, on the regular (90 x 45), the large (120 x 60), the wide (140 x 70) and the pond (60 x 30, a 6 m goal), 1920 x 1080; the goal's first
# frames; Glass House (90 x 45, l3_neon, the one mode that whistles) with a boxed stand-in. ARENA_STILLS_E6 picks parts: props,rinks,goal,glass.
E6 = os.environ.get("ARENA_STILLS_REACT", "") == "e6"
E6_PARTS = [p for p in os.environ.get("ARENA_STILLS_E6", "props,rinks,goal,glass").split(",") if p]
E6_GOAL_FRAMES = [int(v) for v in os.environ.get("ARENA_STILLS_GOAL_FRAMES", "0,1,2,3,9").split(",")]
EXTRA = {}       # (tier, length, width) -> what the cartridge of that rink sets beside its size
# The lane's experiment before any C++ (it decided D4 and D5): on a bare tier, show from Python what the build hides (the Blender goals instead of the code
# frame, the caps, the boxes with doors and hands) and hang a trial light in the used box. With the lane's build the rink does this itself: leave it unset.
FORCE_BARE = os.environ.get("ARENA_STILLS_FORCE_BARE", "") == "1"
BOX_CDS = [float(v) for v in os.environ.get("ARENA_STILLS_BOX_LIGHT", "").split(",") if v]     # trial candelas of the box's light (jam.rink.react.box)
# Lane A145 (evolution 5). ARENA_STILLS_REACT=e5: what a player sees of a hard board hit from the game camera 12 m off the boards (at rest, on the hit's
# frame at Strength 0.45 / 0.8 / 1.0, frames 3 and 6, with `jam.rink.react.hit 0`), the goal's frames with the wave (ARENA_STILLS_WAVE="0,0.5": the shares
# of `jam.rink.react.wave`) at ARENA_STILLS_GOAL_FRAMES, the penalty box in use (a foul's frames), and the round caps. ARENA_STILLS_E5 picks the parts
# ("sweep,hit,goal,box,caps"; "sweep" = the hit's frame for every tint of ARENA_STILLS_TINTS and candela of ARENA_STILLS_LIGHT). ARENA_STILLS_GPU=1 adds
# ten ProfileGPU pairs from the 12 m view, the arena at rest then a Strength 1 hit's frame held.
E5 = os.environ.get("ARENA_STILLS_REACT", "") == "e5"
E5_PARTS = [p for p in os.environ.get("ARENA_STILLS_E5", "hit,goal,box,caps").split(",") if p]
HIT_TINTS = [int(v) for v in os.environ.get("ARENA_STILLS_TINTS", "0,1,2").split(",")]
HIT_CDS = [float(v) for v in os.environ.get("ARENA_STILLS_LIGHT", "1500,3000,6000").split(",")] if E5 else []
WAVES = [float(v) for v in os.environ.get("ARENA_STILLS_WAVE", "0.12").split(",")]
GOAL_FRAMES = [int(v) for v in os.environ.get("ARENA_STILLS_GOAL_FRAMES", "0,9,17,45").split(",")]
HIT_RADII = [float(v) for v in os.environ.get("ARENA_STILLS_HIT_RADIUS", "").split(",") if v]          # metres; empty = the build's radius (the sweep only)
HIT_STRENGTHS = [float(v) for v in os.environ.get("ARENA_STILLS_STRENGTHS", "0.8").split(",")]
E5_CVARS = ("jam.rink.react.hit", "jam.rink.react.hit.tint", "jam.rink.react.wave", "jam.rink.react.box")
E5_DEFAULTS = {}
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
if E5:
    OUT = os.path.join(HERE, "out", "engine-e5")
    JOBS = [("p3_clay", "LR_P3_Clay", 1.0, 90, 45), ("l2_night", "LR_L2_Night", 0.0, 90, 45)]
if E6:
    OUT = os.path.join(HERE, "out", "engine-e6")
    RES = (1920, 1080)
    JOBS = []
    if "props" in E6_PARTS or "goal" in E6_PARTS:
        JOBS.append(("p3_clay", "LR_P3_Clay", 1.0, 90, 45))
    if "rinks" in E6_PARTS:
        JOBS += [("p3_clay", "LR_P3_Clay", 1.0, 120, 60), ("p3_clay", "LR_P3_Clay", 1.0, 140, 70), ("l1_day", "LR_L1_Day", 0.0, 60, 30)]
    if "glass" in E6_PARTS:
        JOBS.append(("l3_neon", "LR_L3_Neon", 0.0, 90, 45))
    EXTRA = {("p3_clay", 90, 45): {"corner": 3.75}, ("p3_clay", 120, 60): {"corner": 5.0}, ("p3_clay", 140, 70): {"corner": 5.83},
             ("l1_day", 60, 30): {"corner": 3.0, "goal_w": 6.0}, ("l3_neon", 90, 45): {"corner": 3.75, "penalties": 600}}
if os.environ.get("ARENA_STILLS_OUT", ""):
    OUT = os.path.join(HERE, "out", os.environ["ARENA_STILLS_OUT"])


def game_cam(px, py, yaw_deg):
    """The game camera's pose for a skater at (px, py) metres facing yaw (UJamFollowCamera::ComputeThirdPersonPose with the cartridge's camera block:
    distance 6, height 3.5, look-ahead 5, off axis 0.7): the eye and a point on its axis (the pitch is the engine's own: the axis meets the ice 11 m on)."""
    import math
    c, s = math.cos(math.radians(yaw_deg)), math.sin(math.radians(yaw_deg))
    ex, ey = px - c * 6.0 - s * 0.7, py - s * 6.0 + c * 0.7
    return (ex, ey, 3.5), (ex + c * 11.0, ey + s * 11.0, 0.0)


def log(msg):
    unreal.log("A119 STILLS " + msg)


def views(length, width):
    gl = length * 0.5 - 4.0
    bx = 0.0833 * length
    if REACT:   # (name, eye, target, action): action = ("goal", frames since the goal) | ("rattle", frames since the hit) | None (the still arena)
        hw = width * 0.5
        if E6:
            out = []
            hl = length * 0.5

            def gc(name, px, py, yaw, action=None):
                eye, tgt = game_cam(px, py, yaw)
                out.append((name, eye, tgt, action))
            if tier_now() == "l3_neon":     # Glass House: the boxes from the ice, at rest and with a boxed stand-in while the penalty runs
                gc("glass-faceoff", 0.0, 0.0, 0.0)
                gc("glass-goal", gl - 6.0, 1.0, 0.0)
                gc("glass-goal-f000", gl - 6.0, 1.0, 0.0, ("goalw", 0, None))
                gc("glass-corner-pos-neg", hl - 7.0, -(hw - 4.0), 0.0)
                gc("glass-box-rest", -6.0, -(hw - 6.0), -35.0)
                gc("glass-box-foul-f030", -6.0, -(hw - 6.0), -35.0, ("foul6", 30))
                gc("glass-box-foul-f300", -6.0, -(hw - 6.0), -35.0, ("foul6", 300))
                gc("glass-box-far-f300", -2.0, 2.0, -90.0, ("foul6", 300))          # from the middle of the ice, the play going to the box side
                gc("glass-box-along-f300", -14.0, -(hw - 3.0), 0.0, ("foul6", 300))  # skating along the box side's boards
                for cd in BOX_CDS:
                    gc("glass-box-foul-f300-cd%05d" % cd, -6.0, -(hw - 6.0), -35.0, ("foul6", 300, cd))
                    gc("glass-box-far-f300-cd%05d" % cd, -2.0, 2.0, -90.0, ("foul6", 300, cd))
                return out
            small = not (tier_now() == "p3_clay" and length == 90)
            if "props" in E6_PARTS or small:
                gc("faceoff", 0.0, 0.0, 0.0)                                         # the skater on the centre dot, the play going +x
                gc("corner-pos-pos", hl - 7.0, hw - 4.0, 0.0)
                gc("corner-pos-neg", hl - 7.0, -(hw - 4.0), 0.0)
                gc("corner-neg-pos", -(hl - 7.0), hw - 4.0, 180.0)
                gc("corner-neg-neg", -(hl - 7.0), -(hw - 4.0), 180.0)
                gc("goal", gl - 6.0, 1.0, 0.0)                                       # 6 m off the +x net
                if small:
                    gc("goal-f000", gl - 6.0, 1.0, 0.0, ("goalw", 0, None))          # the goal's frame on this rink and tier (no stand-ins)
            if "props" in E6_PARTS and not small:
                gc("boxes", -9.0, -(hw - 5.0), -25.0)                                # the penalty side: both boxes and the table, the play drifting to them
                gc("boxes-along", -12.0, -(hw - 2.5), 0.0)                           # skating along their boards
                gc("benches", -bx - 9.0, hw - 5.0, 25.0)
                gc("benches-along", -4.0, hw - 2.5, 0.0)
                gc("box-facing", 3.0, -(hw - 8.0), -90.0)                            # facing the +x box square on (a pass back to the boards)
                gc("goal-own", -(gl - 14.0), -1.0, 180.0)                            # attacking -x: the -x goal, its light, the end caps
                gc("goal-side", gl - 2.0, hw * 0.55, -60.0)                          # the +x goal's joints from the side
                gc("end-boards", gl + 0.5, -6.0, 20.0)                               # behind the goal line: the end boards' caps and the goal light close
                gc("box-facing-foul-f300", 3.0, -(hw - 8.0), -90.0, ("foul6", 300))  # the +x box in use (a stand-in at its seat, the box's light on)
                gc("boxes-foul-f300", -9.0, -(hw - 5.0), -25.0, ("foul6", 300))
            if "goal" in E6_PARTS and not small:
                gc("goalfx-rest", gl - 6.0, 1.0, 0.0)
                for f in E6_GOAL_FRAMES:
                    gc("goalfx-f%03d" % f, gl - 6.0, 1.0, 0.0, ("goalw", f, None))
            return out
        if E5:      # lane A145: the hit from the game camera 12 m off the boards, the goal with its wave, the box in use, the round caps
            hx, hy = length * 0.25, hw - 0.4
            eye, tgt = (hx - 8.0, hw - 12.0, 3.5), (hx, hy, 0.6)
            geye, gtgt = (gl - 11.0, 1.9, 3.5), (gl - 1.5, 0.6, 0.6)
            out = []
            if "sweep" in E5_PARTS:
                out.append(("sweep-rest", eye, tgt, None))
                for radius in (HIT_RADII or [None]):
                    for strength in HIT_STRENGTHS:
                        for tint in HIT_TINTS:
                            for cd in HIT_CDS:
                                out.append(("sweep-r%03d-s%03d-t%d-cd%05d" % (round((radius or 0) * 10), round(strength * 100), tint, cd), eye, tgt, ("hit", 0, (hx, hy), strength, tint, cd, radius)))
            if "hit" in E5_PARTS:
                out.append(("hit-rest", eye, tgt, None))
                out += [("hit-s080-f%03d" % f, eye, tgt, ("hit", f, (hx, hy), 0.8)) for f in (0, 4)]
                out += [("hit-s045-f000", eye, tgt, ("hit", 0, (hx, hy), 0.45)), ("hit-s100-f000", eye, tgt, ("hit", 0, (hx, hy), 1.0))]
                out.append(("hit-s080-f000-quiet", eye, tgt, ("hit", 0, (hx, hy), 0.8, None, 0.0)))     # jam.rink.react.hit 0: evolution 4's hit
                beye, btgt = (-hx - 8.0, hw - 12.0, 3.5), (-hx, hy, 0.6)
                out += [("hitblue-rest", beye, btgt, None), ("hitblue-s080-f000", beye, btgt, ("hit", 0, (-hx, hy), 0.8))]
            if "goal" in E5_PARTS:
                out.append(("goal-rest", geye, gtgt, None))
                for share in WAVES:
                    out += [("goal-w%03d-f%03d" % (round(share * 100), f), geye, gtgt, ("goalw", f, share)) for f in GOAL_FRAMES]
            if "box" in E5_PARTS:   # the +x box (team 1's, orange); both boxes take a foul in these stills
                beye, btgt = (3.0 - 5.5, -(hw - 5.5), 2.0), (3.0, -(hw + 0.8), 1.0)
                ceye, ctgt = (-4.0, -(hw - 13.0), 3.5), (3.5, -(hw + 0.8), 1.1)
                out += [("box-rest", beye, btgt, None)] + [("box-foul-f%03d" % f, beye, btgt, ("foul", f)) for f in (11, 150)]
                out += [("boxgame-rest", ceye, ctgt, None), ("boxgame-foul-f011", ceye, ctgt, ("foul", 11))]
            if "caps" in E5_PARTS:
                out += [("caps-side-pos", (-6.0, hw - 12.0, 3.5), (2.0, hw - 1.0, 0.6), None),
                        ("caps-close", (-3.2, hw - 3.6, 2.1), (0.0, hw + 0.1, 1.3), None)]     # the centre line: blue caps to the left, orange to the right
            return out
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


TIER_NOW = [""]


def tier_now():
    return TIER_NOW[0]


class Run:
    def __init__(self):
        self.job = -1
        self.boxed = None
        self.trial_light = None
        self.standins = []
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
        TIER_NOW[0] = tier
        self.boxed = None
        self.trial_light = None
        self.standins = []
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
        extra = EXTRA.get((tier, length, width), {})
        if "corner" in extra:
            r.set_editor_property("corner_radius_m", float(extra["corner"]))
        if "goal_w" in extra:
            r.set_editor_property("goal_width_m", float(extra["goal_w"]))
        cfg.set_editor_property("rink", r)
        if E6:      # lane A160: the cartridge's whistle (Glass House carries it, every other cut has it off)
            rules = cfg.get_editor_property("rules")
            rules.set_editor_property("penalties", "penalties" in extra)
            if "penalties" in extra:
                rules.set_editor_property("penalty_frames", int(extra["penalties"]))
            cfg.set_editor_property("rules", rules)
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
        if E6 and (tier == "l3_neon" or (tier == "p3_clay" and length == 90)):     # lane A160: a boxed stand-in where the figure would be drawn: at the box's seat when the rink serves one, else at the sim's BoxPos
            seat = self.rink.get_penalty_box_seat(1)
            loc = None
            if seat and not isinstance(seat, bool):
                loc = seat[0] if not isinstance(seat[0], bool) else seat[1]
            if loc is None and FORCE_BARE:
                loc = self.rink.get_editor_property("arena_box_pos").get_socket_location("Seat")
                lamp = self.eas.spawn_actor_from_class(unreal.PointLight, unreal.Vector(loc.x, loc.y + 70.0, loc.z + 220.0))
                lc = lamp.get_editor_property("point_light_component")
                lc.set_mobility(unreal.ComponentMobility.MOVABLE)
                lc.set_editor_property("intensity_units", unreal.LightUnits.CANDELAS)
                lc.set_intensity(0.0)
                lc.set_light_color(unreal.LinearColor(1.0, 0.92, 0.78, 1.0))
                lc.set_attenuation_radius(420.0)
                lc.set_cast_shadows(False)
                self.trial_light = lc
            where = (loc.x, loc.y, loc.z + 90.0) if loc is not None else (300.0, -(width * 50.0 - 75.0), 90.0)     # UJamSim::BoxPos: on the ice at the boards, a skater radius and 0.3 m in
            log("e6 boxed stand-in at %s (%s)" % ([round(v, 1) for v in where], "the box's seat" if loc is not None else "the sim's BoxPos, no seat served"))
            a = self.eas.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(where[0], where[1], where[2]))
            c = a.static_mesh_component
            c.set_mobility(unreal.ComponentMobility.MOVABLE)
            c.set_static_mesh(unreal.load_asset("/Engine/BasicShapes/Cylinder"))
            m = unreal.load_asset("/JamAssets/Look/MI_Toon_Orange")
            if m:
                c.set_material(0, m)
            a.set_actor_scale3d(unreal.Vector(0.55, 0.55, 1.8))
            c.set_render_custom_depth(True)
            c.set_editor_property("custom_depth_stencil_value", 1)
            self.boxed = a
            a.set_is_temporarily_hidden_in_editor(True)
        if PLACE or E5 or (E6 and tier == "p3_clay" and length == 90 and "goal" in E6_PARTS):   # stand-ins: pucks in the crease and the slot, two figures (engine cylinders, B's instances by reference, transient)
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
                self.standins.append(a)
        if E6 and FORCE_BARE and tier in ("l1_day", "l3_neon"):
            for prop in ("arena_goal_neg", "arena_goal_pos", "arena_goal_net_neg", "arena_goal_net_pos", "arena_caps", "arena_caps_pos",
                         "arena_box_neg", "arena_box_pos", "arena_box_door_neg", "arena_box_door_pos", "arena_box_hand_neg", "arena_box_hand_pos"):
                self.rink.get_editor_property(prop).set_visibility(True)
            self.rink.get_editor_property("nets").set_visibility(False)
            log("e6 FORCE_BARE: goals, caps and boxes shown from Python on %s" % tier)
        log("job %s %dx%d: rink built, arena shown=%s goals shown=%s clay=%s scale=%.3f" % (tier, length, width, self.rink.is_arena_dressing_shown(), self.rink.is_arena_goal_shown(), clay, s))
        self.view = 0
        self.place()

    def place(self):
        tier, rig, clay, length, width = JOBS[self.job]
        TIER_NOW[0] = tier
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
        if getattr(self, "trial_light", None):
            self.trial_light.set_intensity(0.0)
        if E6:                                         # lane A160: the goal's stand-ins (two figures, two pucks) show in the goal's views only
            for a in self.standins:
                a.set_is_temporarily_hidden_in_editor(not (action and action[0] == "goalw"))
        if getattr(self, "boxed", None):               # lane A160: the boxed stand-in shows only while a penalty runs
            self.boxed.set_is_temporarily_hidden_in_editor(not (action and action[0] == "foul6"))
        if E5 or E6:      # lane A145: the cvars a still before may have changed go back to what the build has
            for name in E5_CVARS:
                if name not in E5_DEFAULTS:
                    E5_DEFAULTS[name] = unreal.SystemLibrary.get_console_variable_float_value(name)
                    log("default %s %g" % (name, E5_DEFAULTS[name]))
                unreal.SystemLibrary.execute_console_command(self.world(), "%s %g" % (name, E5_DEFAULTS[name]))
        if not action:
            return
        pl = unreal.JamEventPayload()
        if action[0] == "goal" and len(action) > 2:
            unreal.SystemLibrary.execute_console_command(self.world(), "jam.rink.react.light %g" % action[2])
        if action[0] == "hit":    # lane A145: a hard skater `board` event at (x, y) metres with a Strength, the board's normal in the payload
            if len(action) > 4 and action[4] is not None:
                unreal.SystemLibrary.execute_console_command(self.world(), "jam.rink.react.hit.tint %d" % action[4])
            if len(action) > 5:
                unreal.SystemLibrary.execute_console_command(self.world(), "jam.rink.react.hit %g" % action[5])
            if len(action) > 6 and action[6]:      # a trial radius for the flash (the sweep): the build's own stays otherwise
                for light in self.rink.get_editor_property("arena_hit_lights"):
                    light.set_attenuation_radius(action[6] * 100.0)
            pl.set_editor_property("who", 0)
            pl.set_editor_property("where", unreal.Vector(action[2][0] * 100.0, action[2][1] * 100.0, 4.0))
            pl.set_editor_property("strength", action[3])
            pl.set_editor_property("kind", "skater")
            pl.set_editor_property("dir", unreal.Vector(0.0, -1.0 if action[2][1] > 0 else 1.0, 0.0))
            self.rink.react_to_event("board", pl, 1000)
        elif action[0] == "goalw":   # lane A145: a goal in the +x net with the wave's share
            if action[2] is not None:
                unreal.SystemLibrary.execute_console_command(self.world(), "jam.rink.react.wave %g" % action[2])
            pl.set_editor_property("where", unreal.Vector((length * 0.5 - 4.0 + 0.5) * 100.0, 0.0, 4.0))
            pl.set_editor_property("strength", 1.0)
            self.rink.react_to_event("goal", pl, 1000)
        elif action[0] == "foul6":   # lane A160: a foul by a skater of team 1 (no teams in this config: every index is team 1's): the +x box
            if len(action) > 2:
                unreal.SystemLibrary.execute_console_command(self.world(), "jam.rink.react.box %g" % action[2])
            if getattr(self, "trial_light", None):
                self.trial_light.set_intensity(action[2] if len(action) > 2 else 400.0)
            pl.set_editor_property("strength", 1.0)
            pl.set_editor_property("kind", "foul_no_puck")
            pl.set_editor_property("who", 0)
            self.rink.react_to_event("foul", pl, 1000)
        elif action[0] == "foul":    # lane A145: a foul by a skater of each team (skater 0 is team 0's, skater 99 is past any team 0): both boxes are used
            pl.set_editor_property("strength", 1.0)
            pl.set_editor_property("kind", "foul_no_puck")
            for who in (0, 99):
                pl.set_editor_property("who", who)
                self.rink.react_to_event("foul", pl, 1000)
        elif action[0] == "cap":    # lane A136: a hard skater `board` event at (x, y) metres
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
        if E5 or E6:
            log("e5 %s frame %d: hit lights cd %s wave lights cd %s box door %s hand %s caps round=%s reacting=%s" % (action[0], action[1],
                [round(self.rink.get_hit_light_intensity(i), 1) for i in range(4)], [round(self.rink.get_wave_light_intensity(i), 1) for i in range(2)],
                [round(self.rink.get_box_door_deg(t), 1) for t in range(2)], [round(self.rink.get_box_hand_deg(t), 1) for t in range(2)],
                self.rink.are_caps_round(), self.rink.is_reacting()))
        if action[0] == "goal" and len(action) > 3 and action[3]:      # lane A136: the scored end's lamp light moved to a trial place (metres: from the goal line along x, y, z)
            lights = self.rink.get_editor_property("arena_lamp_lights")
            lights[2].set_relative_location(unreal.Vector((length * 0.5 - 4.0 + action[3][0]) * 100.0, action[3][1] * 100.0, action[3][2] * 100.0), False, False)
            if len(action[3]) > 3:
                lights[2].set_attenuation_radius(action[3][3] * 100.0)
            log("light +x moved to %s" % (action[3],))
        log("light cd %s" % [round(self.rink.get_lamp_light_intensity(i), 1) for i in range(3)])
        log("react %s frame %d: lamp %.2f (rest %.2f) light+x %.2f net+x scale %s rattle %.2f deg %s" % (action[0], action[1], self.rink.get_lamp_emissive(),
            self.rink.get_lamp_rest_emissive(), self.rink.get_goal_light_emissive(1), self.rink.get_net_shake_scale(1), self.rink.get_rattle_deg(), self.rink.get_rattled_piece_name()))

    def caps_in_view(self, eye, tgt):
        """Lane A160: the board caps inside the game camera's picture (horizontal FOV 70, 16:9): their count and their triangles at the level the rink uses."""
        import math
        fx, fy, fz = tgt[0] - eye[0], tgt[1] - eye[1], tgt[2] - eye[2]
        fl = math.sqrt(fx * fx + fy * fy + fz * fz)
        fx, fy, fz = fx / fl, fy / fl, fz / fl
        rl = math.hypot(fx, fy)
        rx, ry = -fy / rl, fx / rl                       # right, level
        ux, uy, uz = -fz * fx / rl, -fz * fy / rl, rl    # up = forward x right, normalised by construction
        th = math.tan(math.radians(FOV * 0.5))
        tv = th * RES[1] / RES[0]
        n = self.rink.get_cap_count()
        seen = 0
        for k in range(n):
            p = self.rink.get_cap_place_m(k)
            dx, dy, dz = p.x - eye[0], p.y - eye[1], 1.3 - eye[2]
            d = dx * fx + dy * fy + dz * fz
            if d <= 0.1:
                continue
            if abs(dx * rx + dy * ry) <= d * th and abs(dx * ux + dy * uy + dz * uz) <= d * tv:
                seen += 1
        return seen, n

    def shoot(self):
        tier, rig, clay, length, width = JOBS[self.job]
        TIER_NOW[0] = tier
        if E6 and self.rink.are_caps_shown():
            v = views(length, width)[self.view]
            seen, n = self.caps_in_view(v[1], v[2])
            per = 40 if self.rink.are_caps_round() else 24
            log("e6 caps in view %s %dx%d %s: %d of %d caps, %d triangles in view of %d (%s)" % (tier, length, width, v[0], seen, n, seen * per, n * per, "the dome" if self.rink.are_caps_round() else "the far level"))
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
                        if cmd.startswith("a145.hit"):     # lane A145: the arena at rest (0) or a Strength 1 hit's frame held (1)
                            tier, rig, clay, length, width = JOBS[self.job]
                            self.react(("hit", 0, (length * 0.25, width * 0.5 - 0.4), 1.0) if cmd.endswith("1") else None, length, width)
                        else:
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
        TIER_NOW[0] = tier
        self.view += 1
        if self.view >= len(views(length, width)):
            if E5 and GPU and self.job == 0:     # lane A145: ProfileGPU pairs from the 12 m view, the hit's flash off and on
                hw = width * 0.5
                e = unreal.Vector((length * 0.25 - 8.0) * 100.0, (hw - 12.0) * 100.0, 350.0)
                t = unreal.Vector(length * 0.25 * 100.0, (hw - 0.4) * 100.0, 60.0)
                self.rink.advance_reactions(10 ** 9)
                self.cam.set_actor_location_and_rotation(e, unreal.MathLibrary.find_look_at_rotation(e, t), False, False)
                self.les.pilot_level_actor(self.cam)
                self.gpu = []
                for i in range(int(os.environ.get("ARENA_STILLS_GPU_PAIRS", "10"))):
                    self.gpu += [("a145.hit 0", 24), ("ProfileGPU", 10), ("a145.hit 1", 24), ("ProfileGPU", 10)]
                self.wait = 40
                self.state = "gpu"
                return
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
    JOBS = [j for j in JOBS if ("%s-%d" % (j[0], j[3])) in ONLY.split(",")]
RUN = Run()
