"""tools/blender/arena/rig_clearance.py: which actors of device B's rig maps cross the arena pieces (Third Jam, track F3, lane A103). Read only.

Run (Git Bash, from the worktree root; a commandlet, no window, about 80 s):
  "C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "$PWD/Game/ThirdJam.uproject" -run=pythonscript
      -script="$PWD/tools/blender/arena/rig_clearance.py" -unattended -nopause -nosplash -NullRHI -abslog="$PWD/Game/logs/a103-rig.log"
(one line)

Loads each rig map as a package (nothing is saved, nothing is edited), reads every actor's bounds in the rig's own frame (built for a 90 m rink and scaled by
ULookTierSwitcher with the rink's length), and lists the actors whose bounds cross the volume of a bench or a penalty box on a 90 x 45 rink, and the nearest
actor that stands low beside each piece (within 3 m of it in plan, under 3 m high). Bounds are the static meshes' boxes through the serialized transforms;
an actor without a static mesh (lights, volumes, Blueprint-built dressing) cannot be judged: read the "sized" figure before trusting a zero.
Writes tools/blender/arena/out/rig_clearance.json and prints "A103 RIG ..." lines.
"""
import json
import os

import unreal

HERE = os.path.dirname(os.path.abspath(__file__))
RIGS = ["LR_L2_Night", "LR_P3_Clay", "LR_P4_Broadcast", "LR_L1_Day", "LR_L3_Neon"]
L, W = 90.0, 45.0     # the rig's own rink (ULookTierSwitcher::RigRinkLengthM)
Y0 = (W * 0.5 + 0.2 + 0.05) * 100.0
BENCH_HALF = 0.1 * L * 0.5 * 100.0 + 6.0
# piece volumes in the rig's frame (cm): (name, xmin, xmax, ymin, ymax, zmin, zmax)
VOLUMES = [
    ("bench_neg", -0.0833 * L * 100.0 - BENCH_HALF, -0.0833 * L * 100.0 + BENCH_HALF, Y0 - 4.0, Y0 + 166.0, 0.0, 190.0),
    ("bench_pos", 0.0833 * L * 100.0 - BENCH_HALF, 0.0833 * L * 100.0 + BENCH_HALF, Y0 - 4.0, Y0 + 166.0, 0.0, 190.0),
    ("box_neg", -300.0 - 226.0, -300.0 + 226.0, -Y0 - 166.0, -Y0 + 4.0, 0.0, 266.0),
    ("box_pos", 300.0 - 226.0, 300.0 + 226.0, -Y0 - 166.0, -Y0 + 4.0, 0.0, 266.0),
    # lane A119: the scorer's table between the boxes, and the default goals (4 x 0.61 m) on the goal lines at x = +-(L/2 - 4)
    ("table", -70.0, 70.0, -Y0 - 166.0, -Y0 + 4.0, 0.0, 278.0),
    ("goal_neg", -(L * 50.0 - 400.0) - 158.0, -(L * 50.0 - 400.0) + 8.0, -217.0, 217.0, 0.0, 80.0),
    ("goal_pos", (L * 50.0 - 400.0) - 8.0, (L * 50.0 - 400.0) + 158.0, -217.0, 217.0, 0.0, 80.0),
]
NEAR_CM = 300.0


def log(msg):
    unreal.log("A103 RIG " + msg)


def gap(lo, hi, a0, a1):
    """Distance between the intervals [lo, hi] and [a0, a1]; 0 when they overlap."""
    return max(0.0, a0 - hi, lo - a1)


def mesh_bounds(actor):
    """World box of the actor's static meshes, from the serialized relative transforms: a map loaded as a package has unregistered components, so
    get_actor_bounds answers zero for every actor (the first version of this script reported a false "nothing crosses")."""
    lo = [1e12, 1e12, 1e12]
    hi = [-1e12, -1e12, -1e12]
    found = False
    for c in actor.get_components_by_class(unreal.StaticMeshComponent):
        mesh = c.get_editor_property("static_mesh")
        if not mesh:
            continue
        t = unreal.Transform(c.get_editor_property("relative_location"), c.get_editor_property("relative_rotation"), c.get_editor_property("relative_scale3d"))
        parent = c.get_attach_parent()
        guard = 0
        while parent and guard < 8:
            pt = unreal.Transform(parent.get_editor_property("relative_location"), parent.get_editor_property("relative_rotation"), parent.get_editor_property("relative_scale3d"))
            t = unreal.MathLibrary.compose_transforms(t, pt)
            parent = parent.get_attach_parent()
            guard += 1
        b = mesh.get_bounding_box()
        for x in (b.min.x, b.max.x):
            for y in (b.min.y, b.max.y):
                for z in (b.min.z, b.max.z):
                    w = unreal.MathLibrary.transform_location(t, unreal.Vector(x, y, z))
                    lo = [min(lo[0], w.x), min(lo[1], w.y), min(lo[2], w.z)]
                    hi = [max(hi[0], w.x), max(hi[1], w.y), max(hi[2], w.z)]
        found = True
    return (lo, hi) if found else None


def main():
    out = {"rink_m": [L, W], "volumes_cm": VOLUMES, "rigs": {}}
    for rig in RIGS:
        world = unreal.load_asset("/JamAssets/Rigs/%s" % rig)
        if not world:
            out["rigs"][rig] = {"loaded": False}
            log("%s not loaded" % rig)
            continue
        actors = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.Actor)
        hits, near, sized = [], [], 0
        for a in actors:
            box = mesh_bounds(a)
            if not box:
                continue
            sized += 1
            lo, hi = box
            row = {"actor": a.get_actor_label(), "class": a.get_class().get_name(), "hidden_in_game": bool(a.get_editor_property("hidden")),
                   "min_cm": [round(v) for v in lo], "max_cm": [round(v) for v in hi]}
            for (name, x0, x1, y0, y1, z0, z1) in VOLUMES:
                gx, gy, gz = gap(lo[0], hi[0], x0, x1), gap(lo[1], hi[1], y0, y1), gap(lo[2], hi[2], z0, z1)
                if gx == 0.0 and gy == 0.0 and gz == 0.0:
                    hits.append(dict(row, volume=name))
                elif gz == 0.0 and max(gx, gy) <= NEAR_CM and (hi[2] - lo[2]) < 300.0:
                    near.append(dict(row, volume=name, gap_cm=round(max(gx, gy))))
        out["rigs"][rig] = {"loaded": True, "actors": len(actors), "sized": sized, "crossing": hits, "near": near}
        log("%s: %d actors, %d with sized bounds, %d crossing a piece, %d low actors within 3 m" % (rig, len(actors), sized, len(hits), len(near)))
        for h in hits[:30]:
            log("  CROSS %s %s (%s) hidden_in_game=%s min %s max %s" % (h["volume"], h["actor"], h["class"], h["hidden_in_game"], h["min_cm"], h["max_cm"]))
        for h in sorted(near, key=lambda r: r["gap_cm"])[:8]:
            log("  NEAR %s %s (%s) gap %d cm" % (h["volume"], h["actor"], h["class"], h["gap_cm"]))
    with open(os.path.join(HERE, "out", "rig_clearance.json"), "w") as fh:
        json.dump(out, fh, indent=2)
    log("DONE")


main()
