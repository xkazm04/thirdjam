// node old_new.mjs : old (skh/export, S4 clips) against new (skh/export2, SKF clips) per candidate, from the re-imported files
import fs from "node:fs";
const S = "C:/Users/kazda/jam/skater-lab/skh/";
const J = (p) => JSON.parse(fs.readFileSync(S + p));
const rows = [
  ["P4B", "old: S4 clips", "out/P4B/verify_v2.json", "out/P4B/retarget_report.json"],
  ["P4B", "new: p2_cel (control)", "out2/P4B_ctl_p2/verify.json", "out2/P4B_ctl_p2/retarget_report.json"],
  ["P4B", "new: p4_broadcast", "out2/P4B/verify.json", "out2/P4B/retarget_report.json"],
  ["P3A", "old: S4 clips", "out/P3A/verify_v2.json", "out/P3A/retarget_report.json"],
  ["P3A", "new: p2_cel (control)", "out2/P3A_ctl_p2/verify.json", "out2/P3A_ctl_p2/retarget_report.json"],
  ["P3A", "new: p3_clay", "out2/P3A/verify.json", "out2/P3A/retarget_report.json"],
  ["R", "old: S4 clips (= S5)", "out/R/verify_v2.json", "out/R/retarget_report.json"],
  ["R", "new: p2_cel", "out2/R/verify.json", "out2/R/retarget_report.json"],
];
const TWO = ["Idle_Faceoff", "Glide", "Stop", "Crossover_Right", "Lean_Right", "Check", "Celebrate", "GetUp", "GetUp_Front", "Fall", "Fall_Forward",
  "Poke", "Stumble", "Lift", "Pass", "Foul_Reaction", "Faceoff_Ready", "Faceoff_Draw", "Shot_Charge", "Aim_Front", "Aim_Left", "Aim_Right"];
const LIE = ["Fall", "Fall_Forward", "GetUp", "GetUp_Front", "Goalie_Block_Left", "Goalie_Block_Right"];
const STAND = ["Idle_Faceoff", "Stride", "Glide", "Stop", "Crossover_Left", "Crossover_Right", "Check", "Shot_Slap", "Shot_Wrist", "Poke", "Pass", "Lift", "Stumble", "Foul_Reaction", "Faceoff_Ready", "Faceoff_Draw", "Celebrate"];
const REACH = { Aim_Front: 1.0, Aim_Left: 1.0, Aim_Right: 1.0, Aim_FarReach: 1.6, Poke: 1.6 };
const f = (x, d = 1) => (x === null || x === undefined ? "n/a" : Number(x).toFixed(d));
console.log("| Candidate | Clip source | Left mitt to shaft, two-handed takes: mean / max (take) | Slap + wrist: max | Stick or mitt in head (frames) | Skate lift lying/kneeling: max (take) | Fall / Fall_Forward / GetUp / GetUp_Front / Block L / Block R | Standing skate err max | Goalie_Hold stick min (S4 16-19) | Blade err max (take) | Reach error cm: Front / Left / Right / FarReach / Poke (before rule -> after) |");
console.log("|---|---|---|---|---|---|---|---|---|---|---|");
for (const [c, src, vp, rp] of rows) {
  const v = J(vp).takes, r = J(rp).clips;
  const g = TWO.filter(t => v[t] && v[t].left_mitt_to_shaft_cm);
  const gmean = g.reduce((a, t) => a + v[t].left_mitt_to_shaft_cm[1], 0) / g.length;
  let gmax = ["", -1]; for (const t of g) if (v[t].left_mitt_to_shaft_cm[2] > gmax[1]) gmax = [t, v[t].left_mitt_to_shaft_cm[2]];
  const sh = Math.max(...["Shot_Slap", "Shot_Wrist"].map(t => v[t].left_mitt_to_shaft_cm ? v[t].left_mitt_to_shaft_cm[2] : 0));
  const inh = Object.values(v).reduce((a, x) => a + x.mitt_inside_head_frames + x.stick_inside_head_frames, 0);
  let lmax = ["", -1]; for (const t of LIE) if (v[t].skate_err_on_ice_cm > lmax[1]) lmax = [t, v[t].skate_err_on_ice_cm];
  const lie = LIE.map(t => f(v[t].skate_err_on_ice_cm)).join(" / ");
  const st = Math.max(...STAND.map(t => v[t].skate_err_on_ice_cm));
  let be = ["", -1]; for (const [t, x] of Object.entries(v)) if (x.blade_contact_err_cm > be[1]) be = [t, x.blade_contact_err_cm];
  const reach = Object.entries(REACH).map(([t, tg]) => {
    const after = v[t].reach_m; const rr = r[t] && r[t].reach;
    const before = rr ? rr.before_m : null;
    const a = after === null || after === undefined ? "n/a" : f((after - tg) * 100);
    return rr && rr.rule ? `${f((before - tg) * 100)}->${a}` : a;
  }).join(" / ");
  console.log(`| ${c} | ${src} | ${f(gmean)} / ${f(gmax[1])} (${gmax[0]}) | ${f(sh)} | ${inh} | ${f(lmax[1])} (${lmax[0]}) | ${lie} | ${f(st, 2)} | ${f(v["Goalie_Hold"].lowest_stick_cm)} | ${f(be[1], 2)} (${be[0]}) | ${reach} |`);
}
