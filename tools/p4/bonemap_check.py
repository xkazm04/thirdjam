"""tools/p4/bonemap_check.py: totality check of a vendor -> Manny bone table (lane A28, registry rule: report the unmapped set, never a boolean).

  python -I tools/p4/bonemap_check.py tools/p4/bonemap-<vendor>.json [--source-bones a,b,c | --source-json inspect.json]

Target chains checked (UE5 Manny, SK_Mannequin 5.9): spine (pelvis, spine_01..05, neck_01, neck_02, head), arms (clavicle, upperarm,
lowerarm, hand, both sides), legs (thigh, calf, foot, ball, both sides), plus root. Twist, IK, finger and corrective bones are listed
separately as "optional" so the report says what the retarget will leave still. With --source-bones (or the JSON that
tools/meshy/inspect.py prints) it also reports table rows whose source bone is not in the delivered file, and delivered bones no row uses.
Last line: {"tool":"thirdjam.bonemap-check/1", ...}. Exit 0 when every core chain bone is mapped, 1 otherwise (the unmapped set is the point).
"""
import json, sys

CORE = {
    "root": ["root"],
    "spine": ["pelvis", "spine_01", "spine_02", "spine_03", "spine_04", "spine_05", "neck_01", "neck_02", "head"],
    "arm_l": ["clavicle_l", "upperarm_l", "lowerarm_l", "hand_l"],
    "arm_r": ["clavicle_r", "upperarm_r", "lowerarm_r", "hand_r"],
    "leg_l": ["thigh_l", "calf_l", "foot_l", "ball_l"],
    "leg_r": ["thigh_r", "calf_r", "foot_r", "ball_r"],
}
OPTIONAL = {
    "twist": [f"{p}_twist_0{i}_{s}" for p in ("upperarm", "lowerarm", "thigh", "calf") for i in (1, 2) for s in ("l", "r")],
    "ik": ["ik_foot_root", "ik_foot_l", "ik_foot_r", "ik_hand_root", "ik_hand_gun", "ik_hand_l", "ik_hand_r"],
    "fingers": [f"{f}_{p}_{s}" for f in ("thumb", "index", "middle", "ring", "pinky") for p in ("metacarpal", "01", "02", "03") for s in ("l", "r") if not (f == "thumb" and p == "metacarpal")],
}

def main():
    args = sys.argv[1:]
    if not args:
        print(__doc__); sys.exit(2)
    table = json.load(open(args[0], encoding="utf8"))
    rows = table["rows"]
    targets = set(rows.values())
    src_bones = None
    if "--source-bones" in args:
        src_bones = set(args[args.index("--source-bones") + 1].split(","))
    if "--source-json" in args:
        for line in open(args[args.index("--source-json") + 1], encoding="utf8"):
            if line.startswith("{") and '"bones"' in line:
                src_bones = set(json.loads(line)["bones"])
    report = {"tool": "thirdjam.bonemap-check/1", "vendor": table.get("vendor"), "rows": len(rows), "core_unmapped": {}, "optional_unmapped": {}}
    for chain, bones in CORE.items():
        miss = [b for b in bones if b not in targets]
        if miss:
            report["core_unmapped"][chain] = miss
    for group, bones in OPTIONAL.items():
        miss = [b for b in bones if b not in targets]
        if miss:
            report["optional_unmapped"][group] = len(miss)
    dup = {}
    for s, t in rows.items():
        dup.setdefault(t, []).append(s)
    report["target_hit_twice"] = {t: s for t, s in dup.items() if len(s) > 1}
    if src_bones is not None:
        report["rows_without_delivered_source"] = sorted(s for s in rows if s not in src_bones)
        report["delivered_bones_unused"] = sorted(b for b in src_bones if b not in rows)
    core_total = sum(len(v) for v in CORE.values())
    core_mapped = core_total - sum(len(v) for v in report["core_unmapped"].values())
    report["core_mapped"] = f"{core_mapped}/{core_total}"
    for chain, miss in report["core_unmapped"].items():
        print(f"bonemap {table.get('vendor')} chain {chain}: unmapped {' '.join(miss)}")
    print(json.dumps(report))
    sys.exit(0 if not report["core_unmapped"] and not report["target_hit_twice"] else 1)

main()
