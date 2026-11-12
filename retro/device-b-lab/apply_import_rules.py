# apply_import_rules.py  (lane B8, device A's rules from A-to-B-0004)
# Applies the texture import rules to every Texture2D under /JamAssets/ and reports what it changed.
#
# Run it inside the editor (Python through MCP, or Tools > Execute Python Script). Re-runnable: a second run changes nothing.
#   dry run (no change, no save):  DRY_RUN = True ; exec(open(PATH).read()); REPORT
#   apply and save:                exec(open(PATH).read()); REPORT   (the report text is also in the variable REPORT)
# PATH = C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/b8/apply_import_rules.py
# Optional globals set before exec: ROOT (default "/JamAssets"), DRY_RUN (default False), SKIP (list of path fragments to leave alone).
# The last expression is the report text (one line per texture, then a summary line).
#
# Rules (colour textures):
#   no alpha in the source  -> TC_Default + CompressionNoAlpha True (BC1, 4 bits per pixel)
#   real alpha in the source -> TC_BC7 (8 bits per pixel), CompressionNoAlpha False
#   LossyCompressionAmount Medium (Oodle RDO 20; has an effect only if bFinalUsesRDO is on in config: see the report)
#   VirtualTextureStreaming False
#   MaxTextureSize: 2048 under /Skaters/, 1024 everywhere else (a smaller value already set is kept; 0 = unlimited gets the cap)
# Textures with another compression setting (normal map, masks, HDR, grayscale, alpha-only) keep their format; the other rules still apply.
import os
import unreal

ROOT = globals().get("ROOT", "/JamAssets")
DRY_RUN = bool(globals().get("DRY_RUN", False))
SKIP = list(globals().get("SKIP", []))

_TC = unreal.TextureCompressionSettings
_COLOUR_FORMATS = (_TC.TC_DEFAULT, _TC.TC_BC7)
_PROPS = ["compression_settings", "compression_no_alpha", "lossy_compression_amount",
          "virtual_texture_streaming", "max_texture_size"]


def _short(v):
    s = str(v)
    return s.split(".")[-1].rstrip(">").split(":")[0] if "<" in s else s


def _cap_for(path):
    return 2048 if "/Skaters/" in path else 1024


def _disk_bytes(pkg):
    rel = pkg.split("/", 2)[2] if pkg.startswith("/JamAssets/") else None
    if rel is None:
        return -1
    base = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir()) + "Plugins/JamAssets/Content/" + rel + ".uasset"
    try:
        return os.path.getsize(base)
    except Exception:
        return -1


def _has_alpha(tex):
    try:
        lo, hi = tex.compute_texture_source_channel_min_max()
        return lo.a < 0.999
    except Exception:
        return True  # unknown: keep alpha-capable format


def _snapshot(tex):
    return {p: tex.get_editor_property(p) for p in _PROPS}


def _est_cooked_bytes(tex, fmt_bc7):
    # analytic: BC1 0.5 byte/pixel, BC7 1 byte/pixel, full mip chain x 4/3, after the MaxTextureSize cap
    try:
        w = int(tex.blueprint_get_size_x()); h = int(tex.blueprint_get_size_y())
    except Exception:
        return -1
    m = int(tex.get_editor_property("max_texture_size"))
    if m > 0:
        while max(w, h) > m:
            w //= 2; h //= 2
    return int(w * h * (1.0 if fmt_bc7 else 0.5) * 4 / 3)


def run():
    ar = unreal.AssetRegistryHelpers.get_asset_registry()
    rows, changed, saved = [], 0, 0
    for a in ar.get_assets_by_path(ROOT, recursive=True):
        if str(a.asset_class_path.asset_name) != "Texture2D":
            continue
        pkg = str(a.package_name)
        if any(s in pkg for s in SKIP):
            continue
        tex = a.get_asset()
        before = _snapshot(tex)
        alpha = _has_alpha(tex)
        want = {}
        fmt = before["compression_settings"]
        if fmt in _COLOUR_FORMATS:
            want["compression_settings"] = _TC.TC_BC7 if alpha else _TC.TC_DEFAULT
            want["compression_no_alpha"] = (not alpha)
        want["lossy_compression_amount"] = unreal.TextureLossyCompressionAmount.TLCA_MEDIUM
        want["virtual_texture_streaming"] = False
        cap = _cap_for(pkg)
        cur = int(before["max_texture_size"])
        want["max_texture_size"] = cap if (cur == 0 or cur > cap) else cur
        diffs = {k: (_short(before[k]), _short(v)) for k, v in want.items() if before[k] != v}
        if diffs:
            changed += 1
            if not DRY_RUN:
                for k, v in want.items():
                    if before[k] != v:
                        tex.set_editor_property(k, v)
                tex.modify()
                if unreal.EditorAssetLibrary.save_loaded_asset(tex, False):
                    saved += 1
        bc7 = (want.get("compression_settings", fmt) == _TC.TC_BC7)
        rows.append("%s | alpha=%s | %s | disk=%s | est cooked=%s B | %s" % (
            pkg, alpha, "no change" if not diffs else ("; ".join("%s %s->%s" % (k, o, n) for k, (o, n) in diffs.items())),
            _disk_bytes(pkg), _est_cooked_bytes(tex, bc7), "DRY" if DRY_RUN else ("saved" if diffs else "-")))
    rows.append("SUMMARY: %d textures, %d changed, %d saved%s. Shipped size is only known after a cook." % (
        len(rows), changed, saved, " (dry run)" if DRY_RUN else ""))
    return "\n".join(rows)


REPORT = run()
REPORT
