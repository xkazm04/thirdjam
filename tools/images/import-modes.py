"""tools/images/import-modes.py: headless import of the MODES illustrations into /Game/UI/Modes (Third Jam, track U6, lane A231).

Run (Git Bash, from the worktree root; no editor window, no MCP port; the editor target must be built):
  "C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "$PWD/Game/ThirdJam.uproject" -run=pythonscript \
      -script="$PWD/tools/images/import-modes.py" -unattended -nopause -nosplash -NullRHI -Multiprocess -abslog="$PWD/Game/logs/import-modes.log"

The pictures: tools/images/modes/<id>.png, 1280 x 720 RGB, picked from the Codex image generation runs of tools/images/modes-jobs.json
(node tools/images/gen.mjs tools/images/modes-jobs.json; prepared by tools/images/modes-prepare.sh). One per row of MODES:
the twelve modes by their cartridge slug with underscores, `rinks` (ALL RINKS) and `make_your_own`.

What it does, for every <id>.png that is there:
  1. imports it as /Game/UI/Modes/T_UI_ModeArt_<id> (legacy TextureFactory, no material, replace the existing asset). A picture whose
     file is the one already imported (sha256 in tools/images/modes/import.json) is checked and KEPT (MODEART_FORCE=1 imports all again);
  2. gives it the settings of A's other UI pictures, read from REFERENCE (/Game/UI/T_UI_MatchCard): texture group, compression,
     no mips, sRGB, never streamed;
  3. checks: 1280 x 720, the group, the compression, no mips; the source path kept in the asset is relative to the repository;
  4. saves it and writes tools/images/modes/import.json (bytes on disk per asset). Prints "MODEART IMPORT PASS|FAIL".
The game loads these in FJamMenuLook::ModePicture (MenuWidget.cpp); device B's /JamAssets/UI/Modes/T_UI_Mode_<slug> stays the fallback.
"""
import hashlib
import json
import os

import unreal

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(HERE, "modes")
REPORT = os.path.join(SRC, "import.json")
DEST = "/Game/UI/Modes"
REFERENCE = "/Game/UI/T_UI_MatchCard"
IDS = ["arsenal", "bruisers", "cage", "fog_of_war", "glass_house", "marathon", "outnumbered", "pond", "power_play", "shotgun", "sniper",
       "sudden_death", "rinks", "make_your_own"]
WIDTH, HEIGHT = 1280, 720
FORCE = os.environ.get("MODEART_FORCE", "") == "1"
COPIED = ("lod_group", "compression_settings", "srgb", "never_stream")


def log(msg):
    unreal.log("MODEART IMPORT " + msg)


def sha256(path):
    with open(path, "rb") as fh:
        return hashlib.sha256(fh.read()).hexdigest()


def disk_bytes(name):
    base = os.path.join(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_content_dir()), "UI", "Modes", name)
    return sum(os.path.getsize(base + ext) for ext in (".uasset", ".uexp", ".ubulk") if os.path.exists(base + ext))


def import_picture(ident, name):
    task = unreal.AssetImportTask()
    task.filename = os.path.join(SRC, ident + ".png").replace("\\", "/")
    task.destination_path = DEST
    task.destination_name = name
    task.automated = True
    task.replace_existing = True
    task.save = False
    factory = unreal.TextureFactory()
    factory.set_editor_property("create_material", False)
    task.factory = factory
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])


def main():
    previous = {}
    if os.path.exists(REPORT):
        with open(REPORT, "r", encoding="utf-8") as fh:
            previous = {row["id"]: row for row in json.load(fh).get("pictures", [])}
    reference = unreal.load_asset(REFERENCE)
    wanted = {}
    if reference:
        for key in COPIED:
            wanted[key] = reference.get_editor_property(key)
    else:
        log("reference %s is absent: the UI group, BC7, sRGB, never streamed" % REFERENCE)
        wanted = {"lod_group": unreal.TextureGroup.TEXTUREGROUP_UI, "compression_settings": unreal.TextureCompressionSettings.TC_BC7,
                  "srgb": True, "never_stream": True}
    wanted["mip_gen_settings"] = unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS
    log("settings %s" % {k: str(v) for k, v in wanted.items()})

    ok = True
    report = {"tool": "thirdjam.modeart-import/1", "destination": DEST, "reference": REFERENCE,
              "settings": {k: str(v) for k, v in wanted.items()}, "pictures": []}
    for ident in IDS:
        png = os.path.join(SRC, ident + ".png")
        name = "T_UI_ModeArt_" + ident
        path = DEST + "/" + name
        row = {"id": ident, "asset": path}
        if not os.path.exists(png):
            row["status"] = "no-source"
            report["pictures"].append(row)
            log("%s: no %s.png (device B's picture stays)" % (ident, ident))
            continue
        digest = sha256(png)
        row["source"] = "tools/images/modes/%s.png" % ident
        row["sha256"] = digest
        same = previous.get(ident, {}).get("sha256") == digest and unreal.EditorAssetLibrary.does_asset_exist(path)
        if FORCE or not same:
            import_picture(ident, name)
            row["status"] = "imported"
        else:
            row["status"] = "kept"
        tex = unreal.load_asset(path)
        if not tex or not isinstance(tex, unreal.Texture2D):
            row["status"] = "failed"
            ok = False
            report["pictures"].append(row)
            log("%s: FAILED, no texture at %s" % (ident, path))
            continue
        if row["status"] == "imported":
            for key, value in wanted.items():
                tex.set_editor_property(key, value)
            # The source path kept in the asset: relative to the repository, never an absolute path of this machine.
            data = tex.get_editor_property("asset_import_data")
            if data:
                data.scripted_add_filename("../../../../tools/images/modes/%s.png" % ident, 0, "")
            unreal.EditorAssetLibrary.save_asset(path, only_if_is_dirty=False)
        row["width"] = tex.blueprint_get_size_x()
        row["height"] = tex.blueprint_get_size_y()
        checks = {"size": row["width"] == WIDTH and row["height"] == HEIGHT}
        for key, value in wanted.items():
            checks[key] = tex.get_editor_property(key) == value
        row["checks"] = checks
        row["bytes"] = disk_bytes(name)
        row["size_mb"] = round(row["bytes"] / 1048576.0, 2)
        if not all(checks.values()):
            ok = False
        report["pictures"].append(row)
        log("%s %s" % (ident, json.dumps(row)))
    done = [r for r in report["pictures"] if r.get("status") in ("imported", "kept")]
    report["count"] = len(done)
    report["total_mb"] = round(sum(r.get("bytes", 0) for r in done) / 1048576.0, 2)
    report["verdict"] = "pass" if ok else "fail"
    with open(REPORT, "w", encoding="utf-8", newline="\n") as fh:
        json.dump(report, fh, indent=2)
        fh.write("\n")
    log("%d of %d pictures in %s, %.2f MB on disk" % (len(done), len(IDS), DEST, report["total_mb"]))
    log("PASS" if ok else "FAIL")


main()
