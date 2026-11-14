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


def rinks_shipped():
    """Lane A172: every rink size the shipped cuts play on (Game/Content/Cartridges, the parked modes and the test cuts left out), with the tiers and
    the cuts that use it: {(length, width): {"tiers": set, "cuts": [ids]}}."""
    import glob
    root = os.path.normpath(os.path.join(HERE, "..", "..", "..", "Game", "Content", "Cartridges"))
    found = {}
    for path in sorted(glob.glob(os.path.join(root, "**", "*.cartridge.json"), recursive=True)):
        rel = os.path.relpath(path, root).replace("\\", "/")
        if "parked" in rel or ".test." in rel or "fallback" in rel:
            continue
        with open(path, encoding="utf-8") as fh:
            game = json.load(fh).get("game", {})
        rink = game.get("rink", {})
        key = (float(rink.get("length_m", 90)), float(rink.get("width_m", 45)))
        row = found.setdefault(key, {"tiers": set(), "cuts": []})
        row["tiers"].add(game.get("look", {}).get("tier", "l2_night"))
        row["cuts"].append(os.path.basename(path).replace(".cartridge.json", ""))
    return found


def volumes_for(length, width, sizes):
    """Lane A172: the A side's pieces on a rink of this size, in centimetres (ARinkActor::Build's places): the benches grow with the length, the boxes,
    the table and the goal lights keep their size, the boards with their caps run round the ring (four straight runs; the corners are not judged)."""
    y0 = (width * 0.5 + 0.2 + 0.05) * 100.0
    bench_half = 0.1 * length * 0.5 * 100.0 + 6.0
    bx = 0.0833 * length * 100.0
    ex = (length * 0.5 + 0.2 + 0.05) * 100.0
    lw, ld, lh = [v * 100.0 for v in sizes.get("SM_Arena_GoalLight", [0.9, 0.6, 3.0])]
    hl, hw = length * 50.0, width * 50.0
    return [
        ("bench_neg", -bx - bench_half, -bx + bench_half, y0 - 4.0, y0 + 166.0, 0.0, 190.0),
        ("bench_pos", bx - bench_half, bx + bench_half, y0 - 4.0, y0 + 166.0, 0.0, 190.0),
        ("box_neg", -300.0 - 226.0, -300.0 + 226.0, -y0 - 166.0, -y0 + 4.0, 0.0, 266.0),
        ("box_pos", 300.0 - 226.0, 300.0 + 226.0, -y0 - 166.0, -y0 + 4.0, 0.0, 266.0),
        ("table", -70.0, 70.0, -y0 - 173.0, -y0 + 4.0, 0.0, 290.0),
        ("goal_light_neg", -ex - max(ld, lw), -ex + 4.0, -max(lw, ld) * 0.5, max(lw, ld) * 0.5, 0.0, lh),
        ("goal_light_pos", ex - 4.0, ex + max(ld, lw), -max(lw, ld) * 0.5, max(lw, ld) * 0.5, 0.0, lh),
        ("boards_and_caps_neg_y", -hl + 600.0, hl - 600.0, -hw - 30.0, -hw + 10.0, 0.0, 165.0),
        ("boards_and_caps_pos_y", -hl + 600.0, hl - 600.0, hw - 10.0, hw + 30.0, 0.0, 165.0),
        ("boards_and_caps_neg_x", -hl - 30.0, -hl + 10.0, -hw + 600.0, hw - 600.0, 0.0, 165.0),
        ("boards_and_caps_pos_x", hl - 10.0, hl + 30.0, -hw + 600.0, hw - 600.0, 0.0, 165.0),
    ]


def chain_transform(c):
    """A component's transform in the rig's frame, from the serialized relative transforms up its attach chain (the root's is the actor's)."""
    t = unreal.Transform(c.get_editor_property("relative_location"), c.get_editor_property("relative_rotation"), c.get_editor_property("relative_scale3d"))
    parent = c.get_attach_parent()
    guard = 0
    while parent and guard < 8:
        pt = unreal.Transform(parent.get_editor_property("relative_location"), parent.get_editor_property("relative_rotation"), parent.get_editor_property("relative_scale3d"))
        t = unreal.MathLibrary.compose_transforms(t, pt)
        parent = parent.get_attach_parent()
        guard += 1
    return t


def box_through(mesh, t):
    b = mesh.get_bounding_box()
    lo = [1e12, 1e12, 1e12]
    hi = [-1e12, -1e12, -1e12]
    for x in (b.min.x, b.max.x):
        for y in (b.min.y, b.max.y):
            for z in (b.min.z, b.max.z):
                w = unreal.MathLibrary.transform_location(t, unreal.Vector(x, y, z))
                lo = [min(lo[0], w.x), min(lo[1], w.y), min(lo[2], w.z)]
                hi = [max(hi[0], w.x), max(hi[1], w.y), max(hi[2], w.z)]
    return lo, hi


def instance_boxes(actor):
    """Every instance of every instanced component of a crowd actor (device B's BP_ArenaCrowd: stands, fans, barn props) as (mesh name, lo, hi) in the
    rig's frame; a plain static mesh component counts as one instance."""
    out = []
    for c in actor.get_components_by_class(unreal.StaticMeshComponent):
        mesh = c.get_editor_property("static_mesh")
        if not mesh:
            continue
        ct = chain_transform(c)
        if isinstance(c, unreal.InstancedStaticMeshComponent):
            for i in range(c.get_instance_count()):
                r = c.get_instance_transform(i, False)
                it = r[1] if isinstance(r, tuple) and len(r) == 2 and isinstance(r[0], bool) else (r[0] if isinstance(r, tuple) else r)
                if it is None or isinstance(it, bool):
                    continue
                lo, hi = box_through(mesh, unreal.MathLibrary.compose_transforms(it, ct))
                out.append((mesh.get_name(), lo, hi))
        else:
            lo, hi = box_through(mesh, ct)
            out.append((mesh.get_name(), lo, hi))
    return out


def all_rinks():
    """Lane A172 (evolution 7): A's pieces against device B's dressed rigs on every shipped rink size. The tier switcher scales a rig with the rink's
    length (every place and size x length / 90 about the origin), while A's boxes, table and goal lights keep their size. B's stands and crowd are one
    BP_ArenaCrowd actor a rink scale (`Crowd_P3_Clay_k0667` ... `k1556`: k = length / 90), each a set of instanced meshes: every INSTANCE is judged, for the
    actor whose k is the rink's scale (the nearest within 0.03). An actor whose box holds the whole rink (sky caps, the hall) is not a crossing.
    Writes out/rig_clearance_rinks.json and prints "A172 CLEARANCE ..." lines; the verdict is the count of crossings on the rinks the shipped cuts use."""
    import re
    with open(os.path.join(HERE, "arena.stats.json")) as fh:
        stats = json.load(fh).get("pieces", {})
    sizes = {k: v.get("dimensions_m") for k, v in stats.items() if isinstance(v, dict) and v.get("dimensions_m")}
    rinks = rinks_shipped()
    out = {"rinks": {}, "crowd_actors": {}}
    total = 0
    judged_any = False
    for rig in ("LR_P3_Clay", "LR_L2_Night"):
        world = unreal.load_asset("/JamAssets/Rigs/%s" % rig)
        if not world:
            log("A172 CLEARANCE %s not loaded" % rig)
            continue
        plain = []
        crowds = {}
        for a in unreal.GameplayStatics.get_all_actors_of_class(world, unreal.Actor):
            label = a.get_actor_label()
            m = re.search(r"_k(\d{4})$", label)
            if "Crowd" in a.get_class().get_name() and m:
                crowds[int(m.group(1)) / 1000.0] = (label, instance_boxes(a))
                continue
            if any(isinstance(c, unreal.InstancedStaticMeshComponent) for c in a.get_components_by_class(unreal.StaticMeshComponent)):
                # an actor built of instances (the hall's bays, bunting, trusses): one union box would span the rink; every instance is judged
                for (mesh, lo, hi) in instance_boxes(a):
                    plain.append((label, mesh, lo, hi))
                continue
            box = mesh_bounds(a)
            if box:
                plain.append((label, a.get_class().get_name(), box[0], box[1]))
        out["crowd_actors"][rig] = {("%.3f" % k): {"actor": v[0], "instances": len(v[1])} for k, v in sorted(crowds.items())}
        log("A172 CLEARANCE %s: %d plain actors with sized bounds; crowd actors by scale: %s" % (rig, len(plain), ", ".join("%.3f %s (%d instances)" % (k, v[0], len(v[1])) for k, v in sorted(crowds.items())) or "none"))
        tier = "p3_clay" if "Clay" in rig else "l2_night"
        for (length, width), info in sorted(rinks.items()):
            s = length / L
            hl, hw = length * 50.0, width * 50.0
            vols = [(n, x0, x1, y0, y1, max(z0, 5.0), z1) for (n, x0, x1, y0, y1, z0, z1) in volumes_for(length, width, sizes)]
            hits = []
            def judge(label, kind, lo, hi):
                slo, shi = [v * s for v in lo], [v * s for v in hi]
                if slo[0] < -hl and shi[0] > hl and slo[1] < -hw and shi[1] > hw:
                    return      # holds the whole rink: a sky cap, the hall's shell or its floor
                for (name, x0, x1, y0, y1, z0, z1) in vols:
                    if gap(slo[0], shi[0], x0, x1) == 0.0 and gap(slo[1], shi[1], y0, y1) == 0.0 and gap(slo[2], shi[2], z0, z1) == 0.0:
                        hits.append({"actor": label, "what": kind, "piece": name, "min_cm": [round(v) for v in slo], "max_cm": [round(v) for v in shi]})
            for (label, cls, lo, hi) in plain:
                judge(label, cls, lo, hi)
            crowd = None
            if crowds:
                k = min(crowds, key=lambda v: abs(v - s))
                if abs(k - s) <= 0.03:
                    crowd = (k,) + crowds[k]
                    for (mesh, lo, hi) in crowd[2]:
                        judge(crowd[1], mesh, lo, hi)
            used = tier in info["tiers"]
            judged_any = judged_any or (used and crowd is not None)
            key = "%s %gx%g" % (rig, length, width)
            by = {}
            for h in hits:
                by[(h["what"], h["piece"])] = by.get((h["what"], h["piece"]), 0) + 1
            out["rinks"][key] = {"scale": round(s, 4), "tier_shipped_on_this_rink": used, "cuts": info["cuts"], "crowd_actor": crowd[1] if crowd else None,
                                 "crowd_instances_judged": len(crowd[2]) if crowd else 0, "crossings": hits}
            total += len(hits) if used else 0
            log("A172 CLEARANCE %s %g x %g (scale %.3f; %s; crowd %s): %d crossings%s" % (rig, length, width, s, ("shipped: " + ", ".join(info["cuts"]))[:70] if used else "no shipped cut on this tier",
                ("%s, %d instances" % (crowd[1], len(crowd[2]))) if crowd else "NONE for this scale", len(hits), "".join("; %d x %s on %s" % (n, w, p) for (w, p), n in sorted(by.items(), key=lambda kv: -kv[1])[:6])))
    out["crossings_on_shipped_rinks"] = total
    out["verdict"] = "pass" if total == 0 and judged_any else ("no crowd actor judged" if not judged_any else "fail")
    with open(os.path.join(HERE, "out", "rig_clearance_rinks.json"), "w") as fh:
        json.dump(out, fh, indent=2)
    log("A172 CLEARANCE %s: %d crossings on the rinks and tiers the shipped cuts use" % (out["verdict"].upper(), total))


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
try:
    all_rinks()
except Exception:
    import traceback
    unreal.log_error("A103 RIG A172 CLEARANCE exception: %s" % traceback.format_exc())
