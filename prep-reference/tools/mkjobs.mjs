// builds hockey-jobs.json (prompt text for the hockey ladder studies; reference images only)
import fs from 'node:fs';
const subject = "An ice hockey rink for a toy-like 3 versus 3 game, a 60 by 30 metre rink with rounded corners, boards all around and a four metre wide goal at each end, seen from a high three-quarter camera that keeps BOTH goals in frame. Six stylized chunky toy figures (capsule bodies, round heads, simple sticks, three in deep blue jerseys and three in warm orange jerseys) and one small near-black puck on the ice.";
const tiers = {
  l1: "Tier look, daylight toy rink: clean stylized toy diorama, saturated flat-ish materials, pale blue-white ice with simple painted markings, bright boards, soft global illumination, strong rim light, glossy plastic accents, cheerful.",
  l2: "Tier look, night arena, the same rink after dark: dark ambient light, emissive trim lines along the boards and goal frames, a few strong spotlights, mild bloom, cool shadows, ice still readable and dry (not wet, not reflective), a bit grittier with scuffed ice, one or two figures tipped over.",
  l3: "Tier look, full neon: the same rink at full neon, wet mirror-like reflective ice, cyan and magenta neon strips along the boards and goal frames, low ground fog, strong bloom and glow, small glowing boost pickups (bright capsules and rings) lying on the ice, path-traced look, high contrast."
};
const variants = {
  l1: { a: "Variant: an open-air rink under a clear sunny sky with simple stands around it.", b: "Variant: an indoor community rink with big windows and warm light.", c: "Variant: a tabletop-sized rink standing on a wooden table, toy-box feel, soft window light." },
  l2: { a: "Variant: the open-air rink at night under floodlights with a dark sky.", b: "Variant: the indoor rink with the house lights down and spotlights on.", c: "Variant: the tabletop rink on a table in a dark room, lit by the toy's own glowing trim." },
  l3: { a: "Variant: open-air rink, neon strips along boards and goals, their reflections visible in the ice, dark city skyline or hills behind.", b: "Variant: indoor rink with neon trim, haze in the air and reflections in the ice.", c: "Variant: tabletop rink in a dark room, the whole toy glowing, reflections on a glossy table." }
};
const jobs = [];
for (const t of ['l1','l2','l3']) for (const v of ['a','b','c'])
  jobs.push({ name: `hockey-${t}-${v}`, tier: t.toUpperCase(), prompt: `${subject} ${tiers[t]} ${variants[t][v]}` });
const style = "Reference study for art direction only. Landscape 16:9, same high three-quarter camera angle in every study. No text, no letters, no logos, no watermark, no readable numbers or letters on jerseys or boards, no real people, no real teams or leagues.";
fs.writeFileSync(new URL('./hockey-jobs.json', import.meta.url), JSON.stringify({ style, jobs }, null, 2));
console.log(jobs.length);
