// builds path-jobs.json (prompt text for the three art-style paths; reference images only)
import fs from 'node:fs';
const subject = "Ice hockey 3 vs 3 in a huge rink 120 by 60 metres with rounded corners and boards all round, seen from a third-person camera behind and slightly above one skater's shoulder, looking down the length of the ice toward the far net; the player's skater is large in the lower foreground with a stick and a small near-black puck ahead; the other five skaters are spread down the ice; two jersey colours, deep blue and warm orange.";
const styles = {
  p2: "Style, painted cel: hand-painted textures, visible ink-like outlines, flat cel lighting steps, a limited palette of five or six colours plus one accent, painted backdrop cards. Figures are chunky toy-proportioned skaters, inked and flat-shaded.",
  p3: "Style, clay miniature: matte clay and felt materials with fingerprint detail, soft rounded forms, warm bounce light, a hint of tilt-shift depth of field at the far end. Figures are clay toys with a stitched jersey look.",
  p4: "Style, stylized broadcast: semi-realistic physically based materials, glossy boards, real ice with scrape marks, glass panels, arena rig lighting, lens flare and bloom, a sports-broadcast feel, stylized proportions. Figures are stylized athletes with helmets and visors, proportions between toy and real."
};
const tiers = {
  p2: {
    t1: "Tier, regular: bright, full house lights or daylight, cheerful.",
    t2: "Tier, raw night: lights down, spot cones, emissive trim, scuffed ice, one tipped-over figure.",
    t3: "Tier, boost climax: a saturated poster-like glow with painted light rays and glowing boost pickups (bright capsules and rings) on the ice."
  },
  p3: {
    t1: "Tier, regular: bright, full house lights or daylight, cheerful.",
    t2: "Tier, raw night: lights down, spot cones, emissive trim, scuffed ice, one tipped-over figure.",
    t3: "Tier, boost climax: strings of fairy lights and glowing clay boost pickups (capsules and rings) in a dark room."
  },
  p4: {
    t1: "Tier, regular: bright, full house lights or daylight, cheerful.",
    t2: "Tier, raw night: lights down, spot cones, emissive trim, scuffed ice, one tipped-over figure.",
    t3: "Tier, boost climax: full neon strips, wet reflective ice, haze, glowing boost pickups (capsules and rings) on the ice."
  }
};
const restr = "No text, no letters, no logos, no readable numbers.";
const jobs = [];
for (const p of ['p2','p3','p4']) for (const t of ['t1','t2','t3'])
  jobs.push({ name: `path-${p}-${t}`, path: p.toUpperCase(), tier: t.toUpperCase(), prompt: `${subject} ${styles[p]} ${tiers[p][t]} ${restr}` });
const style = "Reference study for art direction only. Landscape 16:9, same third-person camera in every study. No text, no letters, no logos, no watermark, no readable numbers or letters on jerseys or boards, no real people, no real teams or leagues.";
fs.writeFileSync(new URL('./path-jobs.json', import.meta.url), JSON.stringify({ style, jobs }, null, 2));
console.log(jobs.length);
