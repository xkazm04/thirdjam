// Summarises one style's measurements: node summary.mjs <style> [all]
// Prints build numbers, reaches, and per take: frames main/loopcut, pelvis x,y max, root, seams, lowest skate / stick / any point,
// right-arm extension max, bottom-hand gap max (two-handed frames), knob moved by the reach guard.
import fs from 'node:fs';
const st = process.argv[2], all = process.argv[3] === 'all';
const W = `work${process.env.SKF_SUFFIX || ""}/${st}`;
const j = f => JSON.parse(fs.readFileSync(`${W}/${f}`, 'utf8'));
const b = j('build.json'), c = j('check_fbx.json'), l = j('check_fbx_loopcut.json'), r = j('clips_report.json');
console.log(`${st}: tris ${b.tris_total} (body ${b.tris_body} + stick ${b.tris_stick}), height ${b.height_cm} cm, bones ${c.bone_count}, takes ${Object.keys(c.takes).length}/${Object.keys(l.takes).length}, root rest identity ${JSON.stringify(c.root_rest_matrix_local) === JSON.stringify([[1,0,0,0],[0,1,0,0],[0,0,1,0],[0,0,0,1]])}`);
for (const n of ['Aim_Front', 'Aim_Left', 'Aim_Right', 'Aim_FarReach', 'Poke']) { const x = r[n].reach; console.log(`  ${n} f${x.frame}: reach ${x.reach_m} m (target ${x.target_m}), angle ${x.angle_deg}, blade centre ${x.blade_centre_height_cm} cm up, arm ext ${x.right_arm_extension}`); }
console.log('| Take | Frames main/loopcut | Loop | Pelvis x,y max cm | Root cm | Seam main / loopcut cm | Lowest skate / stick / any cm | Arm ext max | Bottom hand gap max cm | Knob moved max cm |');
for (const [k, t] of Object.entries(c.takes)) {
  const m = r[k], lc = l.takes[k];
  const flag = t.pelvis_horizontal_max_cm > 0.01 || t.root_moves_cm > 0.01 || (m.loop && t.seam_first_last_max_joint_cm > 0.01) || t.lowest_skate_cm < -0.5 || t.lowest_stick_cm < -0.5 || m.arm_r_ext_max > 1 || (m.lh_gap_max_cm || 0) > 2;
  if (!all && !flag) continue;
  console.log(`| ${k} | ${t.frames}/${lc.frames} | ${m.loop ? 'yes' : 'no'} | ${t.pelvis_horizontal_max_cm} | ${t.root_moves_cm} | ${m.loop ? t.seam_first_last_max_joint_cm + ' / ' + lc.seam_first_last_max_joint_cm : 'one-shot'} | ${t.lowest_skate_cm} / ${t.lowest_stick_cm} / ${t.lowest_point_cm} | ${m.arm_r_ext_max} | ${m.lh_gap_max_cm ?? 'one hand'} | ${m.knob_moved_max_cm} |`);
}
