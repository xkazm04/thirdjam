// Second pass: compares work2/ (twist continuity + foot pitch) with work/ (first pass) and before2/ (first-pass exports re-checked
// with the new rotation measurement). node compare2.mjs  -> prints per style and writes work2/<style>/compare.md
import fs from 'node:fs';
const J = p => JSON.parse(fs.readFileSync(p, 'utf8'));
for (const s of ['p2_cel', 'p3_clay', 'p4_broadcast']) {
  const o = J(`work/${s}/check_fbx.json`), n = J(`work2/${s}/check_fbx.json`), ol = J(`work/${s}/check_fbx_loopcut.json`), nl = J(`work2/${s}/check_fbx_loopcut.json`);
  const ro = J(`work/${s}/clips_report.json`), rn = J(`work2/${s}/clips_report.json`), b = J(`before2/check_${s}.json`), bl = J(`before2/check_${s}_loopcut.json`);
  const cnt = (x, t) => Object.values(x.rot_over_45).flat().filter(r => r[3] > t).length;
  const tw = (x, t) => Object.values(x.rot_over_45).flat().filter(r => Math.abs(r[4]) > t).length;
  let pel = 0, root = 0, seam = 0;
  for (const t of Object.values(n.takes).concat(Object.values(nl.takes))) { pel = Math.max(pel, t.pelvis_horizontal_max_cm); root = Math.max(root, t.root_moves_cm); }
  for (const [k, t] of Object.entries(n.takes)) if (rn[k].loop) seam = Math.max(seam, t.seam_first_last_max_joint_cm);
  console.log(`== ${s}: steps>60 ${cnt(b, 60)} -> ${cnt(n, 60)} (loopcut ${cnt(bl, 60)} -> ${cnt(nl, 60)}); steps>45 ${cnt(b, 45)} -> ${cnt(n, 45)}; twist>60 ${tw(b, 60)} -> ${tw(n, 60)}; twist>45 ${tw(b, 45)} -> ${tw(n, 45)} (loopcut ${tw(bl, 45)} -> ${tw(nl, 45)})`);
  console.log(`   pelvis x,y max ${pel} cm, root ${root} cm, loop seam main ${seam} cm, tris ${JSON.stringify(n.triangles)}, bones ${n.bone_count}, takes ${Object.keys(n.takes).length}/${Object.keys(nl.takes).length}`);
  console.log('   reach ' + ['Aim_Front', 'Aim_Left', 'Aim_Right', 'Aim_FarReach', 'Poke'].map(k => `${k} ${ro[k].reach.reach_m}->${rn[k].reach.reach_m} (h ${ro[k].reach.blade_centre_height_cm}->${rn[k].reach.blade_centre_height_cm})`).join('; '));
  const agg = {};
  for (const [t, rows] of Object.entries(n.rot_over_45)) for (const r of rows) { const k = t + ' ' + r[0]; agg[k] = agg[k] || { max: 0, tw: 0, fr: [] }; agg[k].max = Math.max(agg[k].max, r[3]); agg[k].tw = Math.max(agg[k].tw, Math.abs(r[4])); agg[k].fr.push(r[1]); }
  console.log('   over 45 after: ' + Object.entries(agg).map(([k, v]) => `${k} ${v.max}/tw ${v.tw} f${v.fr.join(',')}`).join('; '));
  const rows = ['| Take | Steps >60 before -> after | Foot pitch max before: dorsi / plantar (deg) | after | Frames limited | Other numbers that moved (first pass -> second) |', '|---|---|---|---|---|---|'];
  for (const k of Object.keys(n.takes)) {
    const a = o.takes[k], c = n.takes[k], al = ol.takes[k], cl = nl.takes[k], d = [];
    const cmp = (lab, x, y, tol) => { if (Math.abs((x ?? 0) - (y ?? 0)) > tol) d.push(`${lab} ${x}->${y}`); };
    cmp('skate low', a.lowest_skate_cm, c.lowest_skate_cm, 0.1); cmp('stick low', a.lowest_stick_cm, c.lowest_stick_cm, 0.1); cmp('any low', a.lowest_point_cm, c.lowest_point_cm, 0.1);
    cmp('hand gap', ro[k].lh_gap_max_cm, rn[k].lh_gap_max_cm, 0.1); cmp('arm ext', ro[k].arm_r_ext_max, rn[k].arm_r_ext_max, 0.005);
    if (!rn[k].loop) cmp('first-last', a.seam_first_last_max_joint_cm, c.seam_first_last_max_joint_cm, 0.1); else cmp('loopcut wrap', al.seam_first_last_max_joint_cm, cl.seam_first_last_max_joint_cm, 0.05);
    cmp('pelvis z range', ro[k].pelvis_z_range_cm, rn[k].pelvis_z_range_cm, 0.1);
    const fb = b.foot_pitch[k], fa = n.foot_pitch[k];
    rows.push(`| ${k} | ${b.rot_over_60_count[k]} -> ${n.rot_over_60_count[k]} | ${fb.max_dorsiflexion_deg} / ${fb.max_plantar_flexion_deg} | ${fa.max_dorsiflexion_deg} / ${fa.max_plantar_flexion_deg} | ${rn[k].foot_limited_frames} | ${d.join('; ') || '-'} |`);
  }
  fs.writeFileSync(`work2/${s}/compare.md`, rows.join('\n') + '\n');
  console.log(rows.filter(r => !r.endsWith('| - |')).slice(2).join('\n'));
}
