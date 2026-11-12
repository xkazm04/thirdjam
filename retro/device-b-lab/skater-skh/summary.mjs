// node summary.mjs <name...> : per-candidate facts and per-clip-group errors from out/<name>/verify.json, verify_loopcut.json, retarget_report.json
import fs from "node:fs";
const S = "C:/Users/kazda/jam/skater-lab/skh/out/";
const G = {
  Locomotion: ["Idle_Faceoff", "Stride", "Glide", "Stop", "Crossover_Left", "Crossover_Right", "Lean_Left", "Lean_Right"],
  Shots: ["Shot_Slap", "Shot_Wrist", "Shot_Charge"],
  Contact: ["Check", "Fall", "Fall_Forward", "GetUp", "GetUp_Front", "Celebrate"],
  StickVerbs: ["Poke", "Stumble", "Lift", "Pass", "Foul_Reaction"],
  Faceoff: ["Faceoff_Ready", "Faceoff_Draw"],
  Goalie: ["Goalie_Hold", "Goalie_Block_Left", "Goalie_Block_Right"],
  Aims: ["Aim_Front", "Aim_Left", "Aim_Right", "Aim_FarReach"],
};
const LOOPS = new Set(["Idle_Faceoff", "Stride", "Glide", "Crossover_Left", "Crossover_Right", "Lean_Left", "Lean_Right", "Faceoff_Ready", "Goalie_Hold", "Shot_Charge", "Aim_Front", "Aim_Left", "Aim_Right", "Aim_FarReach"]);
const mx = (a) => a.length ? Math.max(...a) : null, mn = (a) => a.length ? Math.min(...a) : null;
const f = (x, d = 1) => x === null || x === undefined ? "n/a" : Number(x).toFixed(d);
for (const name of process.argv.slice(2)) {
  const v = JSON.parse(fs.readFileSync(S + name + "/verify.json"));
  const lc = fs.existsSync(S + name + "/verify_loopcut.json") ? JSON.parse(fs.readFileSync(S + name + "/verify_loopcut.json")) : null;
  const r = JSON.parse(fs.readFileSync(S + name + "/retarget_report.json"));
  const m = Object.values(v.meshes);
  console.log(`\n### ${name}`);
  console.log(`- File: ${v.bytes} bytes, ${v.bone_count} bones, top ${v.top_bones}, root rest identity ${v.root_rest_is_identity}, armature object scale ${v.armature_object_scale} (cm file), rest height ${v.rest_height_cm} cm, lowest rest ${v.rest_lowest_cm} cm, ${Object.keys(v.takes).length} takes at ${v.fps} fps, tris ${Object.entries(v.meshes).map(([k, x]) => k + " " + x.tris).join(", ")}; images ${JSON.stringify(v.images.map(i => [i.name, i.size, i.packed]))}`);
  console.log(`- Measured: ${JSON.stringify(r.measured)}; K ${r.K_leg_ratio}`);
  console.log("| Group | Pelvis horiz. max | Root max | Loop seam main / loopcut wrap max | Skate err on ice max (take) | Lowest stick min | Blade contact err max (take) | Left mitt to shaft, two-hand frames: mean / max (take) | Mitt or stick inside head (frames) | Nearest mitt / stick to head centre |");
  console.log("|---|---|---|---|---|---|---|---|---|---|");
  for (const [g, takes] of Object.entries(G)) {
    const T = takes.filter(t => v.takes[t]).map(t => [t, v.takes[t]]);
    const arg = (key) => { let b = null; for (const [t, x] of T) if (b === null || x[key] > b[1]) b = [t, x[key]]; return b; };
    const se = arg("skate_err_on_ice_cm"), be = arg("blade_contact_err_cm");
    const grips = T.filter(([t, x]) => x.left_mitt_to_shaft_cm);
    const gm = grips.length ? grips.map(([t, x]) => x.left_mitt_to_shaft_cm[1]) : [];
    let gmax = null; for (const [t, x] of grips) if (!gmax || x.left_mitt_to_shaft_cm[2] > gmax[1]) gmax = [t, x.left_mitt_to_shaft_cm[2]];
    const loops = T.filter(([t]) => LOOPS.has(t));
    const seam = mx(loops.map(([t, x]) => x.seam_max_joint_cm));
    const lcs = lc ? mx(loops.map(([t]) => lc.takes[t] ? lc.takes[t].seam_max_joint_cm : 0)) : null;
    const inside = T.reduce((a, [t, x]) => a + x.mitt_inside_head_frames + x.stick_inside_head_frames, 0);
    console.log(`| ${g} | ${f(mx(T.map(([t, x]) => x.pelvis_horizontal_max_cm)), 3)} | ${f(mx(T.map(([t, x]) => x.root_max_cm)), 3)} | ${loops.length ? f(seam, 3) + " / " + f(lcs, 2) : "one-shots"} | ${f(se[1], 2)} (${se[0]}) | ${f(mn(T.map(([t, x]) => x.lowest_stick_cm)), 2)} | ${f(be[1], 2)} (${be[0]}) | ${gm.length ? f(gm.reduce((a, b) => a + b, 0) / gm.length) + " / " + f(gmax[1]) + " (" + gmax[0] + ")" : "no two-hand frames"} | ${inside} | ${f(mn(T.map(([t, x]) => x.mitt_to_head_min_cm)))} / ${f(mn(T.map(([t, x]) => x.stick_to_head_min_cm)))} |`);
  }
}
