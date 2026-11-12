"""tools/p4/ledger_rows.py (lane A28): print C-0004 ledger rows for every file under Game/Content/P4 (and the Manny copies this lane added), sizes from disk.
  python -I tools/p4/ledger_rows.py >> ASSETS.md    (review the rows first: python -I tools/p4/ledger_rows.py | head)
Columns: path | kind | size_mb | origin | tool | licence | url | date | device | shipped | note. Every row shipped n: P4 is a slice under review (R-0025).
"""
import os, re, sys
ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
DATE = "2026-11-12"
VENDOR = {
    "Meshy": ("ai-generated", "Meshy text-to-3d meshy-6-lite + refine 2K + rigging ue5_mannequin (lane A28)", "", "preview 01a1d07d-5f79-75b8-aac0-057e4de60564, refine 01a1d082-cfba-7780-8437-0af0b6b44130, rig 01a1d08d-903b-71a2-8f1c-1093bba6b669"),
    "Rodin": ("ai-generated", "Hyper3D Rodin Gen-2.5-Low (Quad 18k, TAPose) + Meshy rigging ue5_mannequin (lane A28)", "", "rodin ad9c0508-72b2-4a2d-8efc-e4aeba8c7a7b, rig 01a1d095-0270-76cc-887d-cbedf924221d"),
    "Tripo": ("ai-generated", "Tripo3D v2.5 text-to-3d through fal.ai + Meshy rigging ue5_mannequin (lane A28)", "", "fal 01a1d082-11a3-7e70-b431-41bb40729ac1 / tripo 12678a9a-55d1-422b-b16b-2a9bb75a6db5, rig 01a1d095-8b3f-7521-9ca2-cad3c8af3053"),
}
FALLS = {"Meshy": "01a1d098-cbb9-73b3-b683-b3feb62f170d"}
for line in open(os.path.join(ROOT, "tools/p4/ledger/meshy.jsonl"), encoding="utf8"):
    if '"animate"' in line and '"ok":true' in line:
        import json
        d = json.loads(line)
        FALLS.setdefault(d.get("rig_vendor", ""), d["task_id"])
def kind_of(name, folder):
    if name.startswith("SK_"): return "mesh"
    if name.startswith("SKEL_"): return "other"
    if name.startswith("AS_") or name.startswith("MF_") or name.startswith("MM_"): return "anim"
    if name.startswith("T_"): return "texture"
    if name.startswith("M_") or name.startswith("MI_"): return "material"
    if name.startswith("PA_") or name.startswith("IK_") or name.startswith("RTG_"): return "other"
    if name.endswith(".umap") or folder.endswith("Maps"): return "other"
    return "other"
rows = []
MINE = ("Meshy", "Rodin", "Tripo", "Manny", "Maps", "Props")   # lane A28 folders only; /Game/P4/Anim belongs to another lane
for base, _, files in os.walk(os.path.join(ROOT, "Game/Content/P4")):
    if os.path.relpath(base, os.path.join(ROOT, "Game/Content/P4")).replace("\\", "/").split("/")[0] not in MINE: continue
    for f in files:
        if not (f.endswith(".uasset") or f.endswith(".umap")): continue
        p = os.path.join(base, f); rel = os.path.relpath(p, ROOT).replace("\\", "/")
        size = os.path.getsize(p) + sum(os.path.getsize(os.path.join(base, g)) for g in files if g.startswith(f.rsplit(".", 1)[0] + ".") and g.endswith((".uexp", ".ubulk")))
        vendor = rel.split("/")[3] if len(rel.split("/")) > 3 else ""
        name = f.rsplit(".", 1)[0]
        if vendor in VENDOR:
            origin, tool, licence, url = VENDOR[vendor]
            note = "P4 figure slice, %s route (docs/experiments/P4-FIGURE.md)" % vendor
            if name.startswith("MF_") or name.startswith("MM_"):
                origin, tool, licence, url, note = "sourced", "", "Epic content inside a UE product (Templates/TemplateResources 5.9 Manny clip), retargeted by IK Retargeter onto the %s skeleton" % vendor, "", "retargeted Manny clip (tools/p4/ue_setup.py)"
            if "/Falls/" in rel:
                url = "Meshy animation library actions 366, 502-505; animate task " + FALLS.get(vendor, "see tools/p4/ledger/meshy.jsonl")
            if name.startswith("IK_") or name.startswith("RTG_"):
                origin, tool, note = "original", "UE 5.9 IK Rig / IK Retargeter made by tools/p4/ue_setup.py", "retarget assets for the %s skeleton" % vendor
            if name.startswith("PA_"):
                origin, tool, note = "original", "UE 5.9 PhysicsAssetToolset.CreateFromMesh (MCP), tools/p4/run_vendor.sh", "cosmetic ragdoll bodies for knocked_down (%s)" % vendor
            if name.startswith("M_"):
                origin, tool, note = "original", "UE 5.9 MaterialEditingLibrary (tools/p4/ue_setup.py)", "base colour only material for the %s texture" % vendor
        elif vendor == "Manny":
            origin, tool, licence, url, note = "original", "UE 5.9 IK Rig made by tools/p4/ue_setup.py", "", "", "IK rig on the Epic Manny (source side of the P4 retargets)"
        elif vendor == "Maps":
            origin, tool, licence, url, note = "original", "UE 5.9 editor Python (tools/p4/ue_lab.py)", "", "", "P4 lab level: figure, Manny, two SceneCapture2D"
        else:
            origin, tool, licence, url, note = "original", "lane A28", "", "", ""
        rows.append("| %s | %s | %.1f | %s | %s | %s | %s | %s | A | n | %s |" % (rel, kind_of(name, base), size / 1e6, origin, tool, licence, url, DATE, note))
rows.sort()
print("\n".join(rows))
print("ROWS %d" % len(rows), file=sys.stderr)
