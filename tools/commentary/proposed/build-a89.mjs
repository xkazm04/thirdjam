// Lane A89: builds a89-lines.json (proposed lines, NOT merged into the bank) and checks every text with the bank's own filter.
//   node tools/commentary/proposed/build-a89.mjs
import { writeFileSync } from 'node:fs';
import { checkLine, mildCount } from '../../barks/filter.mjs';
const S = [
 // [event, situation, kind, priority, cooldown_s, when, [texts]]
 ['check','light','',2,8,'a body check with Strength under 0.4 (NEW situation: needs a picker rule like shot.soft)',[
  'A tap on the shoulder. Rude, but a tap.','Just a nudge. The real one is coming.','Polite little bump.','He felt that about as much as a breeze.','Shoulder to shoulder. Warming up.','That one was a hello.']],
 ['check','solid','',3,10,'a body check with Strength 0.4 to under 0.8 (NEW situation)',[
  'That one rattled the glass.','Now he knows where the boards are.','Square hit. Clean and loud.','Lays the body and walks away.','Hell of a hit.','Shoulders, hips, and a lot of gravity.','That is how you carry the check.']],
 ['knockdown','big','check',4,12,'a knockdown whose check had Strength 0.8 or more (NEW situation; knockdown carries the check Strength)',[
  'Flattened! Somebody check the ice for dents.','Sent flying, and the puck stayed behind.','Folded in half and set down gently. Not gently.','Planted him like a flag.','He is wearing the boards now.','Gravity wins again.','The ice has a new best friend.']],
 ['check','cross','cross',4,12,'a cross-check lands (NEW Kind: the sim emits Kind "cross" on the check and its knockdown; today Kind is only "check" or "boost")',[
  'Stick across the chest! That is a cross-check.','Cross-check, and he sits down hard.','Bowled over with the shaft.','Cross-check! Nobody saw that coming. He did not.','Wood across the middle. Down he goes.','The cross-check does the talking.','Sent sprawling by a cross-check.','Damn, that cross-check had a lot of mustard.','He ran into the stick and the ice caught him.','Cross-check! The glass felt that too.']],
 ['knockdown','human','',3,15,'the human skater is knocked down (existing situation, ids 04 to 13 would extend knockdown.human.01..03)',[
  'You are down. Count the lights, then get up.','The ice caught you. Shake it off.','Good hit. Now go give one back.','Up you get. They are not done.','That one had your name on it. Go answer.','Flat on the ice. It happens to the best of us.','Sit a second. Then go find him.','That one is going on the wall. Now answer it.','Folded. Back up, there is a puck to chase.','Rolled right over you. Time to return the favour.']],
 ['knockdown','massacre','check',5,25,'the third knockdown by one team within 1200 frames (20 s) (NEW; the bank cannot express a count in a window: the picker needs a per-team ring of knockdown frames)',[
  'Three down in a flash. This is a massacre shift.','They are bowling out there. Strike after strike.','Another one down. The ice is getting crowded.','Nobody is standing. Well, nearly nobody.','Hit after hit after hit. What a shift.','They are going through the other team like a lawn mower.','Down, down, and down again.','This is not hockey, this is a wrecking yard.','Pile them up! Somebody sweep the ice.','The shift of the night, and the night is young.']],
 ['goal','after_hit','',5,20,'a goal within 300 frames after the scoring team landed a check (NEW situation, like board.bank evaluated at the goal)',[
  'Hit him, take the puck, score. Simple.','The check made the goal.','Flatten, grab, fire. That is a goal.','He went down and the net went in.','The hit paid off in full.','Lays the body, lights the lamp.','Rough work, pretty finish.','One hit, one goal. Efficient.','The ice shook and the net followed.','That goal started with a shoulder.']],
];
const lines = [];
const bad = [];
let mild = 0;
for (const [event, sit, kind, priority, cooldown_s, when, texts] of S) {
  texts.forEach((text, i) => {
    const nn = String(i + 1).padStart(2, '0');
    // knockdown.human continues the live ids
    const num = (event === 'knockdown' && sit === 'human') ? String(i + 4).padStart(2, '0') : nn;
    const id = `${event}.${sit}.${num}`;
    const r = checkLine(text);
    if (r.length) bad.push(`${id}: ${text} -> ${r.join('; ')}`);
    if (mildCount(text)) mild += 1;
    const file = `VO_Commentator_${id.replace(/\./g, '_')}`;
    lines.push({ id, event, kind, team: 'any', priority, cooldown_s, when, text,
      sound: `/JamAssets/Audio/VO/${file}.${file}`, seconds: Math.round((0.4 + text.length * 0.055) * 100) / 100 });
  });
}
const out = { schema: 'hokej.commentary/1', note: 'PROPOSED by lane A89, not merged: the Director merges after A84 lands; sound files do not exist yet (B records them); situations marked NEW need a picker rule in CommentarySystem.cpp', count: lines.length, lines };
writeFileSync(new URL('./a89-lines.json', import.meta.url), JSON.stringify(out, null, 1) + '\n');
const per = {};
lines.forEach((l) => { const k = `${l.event}.${l.id.split('.')[1]}`; per[k] = (per[k] || 0) + 1; });
console.log(JSON.stringify(per), 'total', lines.length, 'mild', mild, 'mild max len', Math.max(...lines.map((l) => l.text.length)));
if (bad.length) { console.log(bad.join('\n')); process.exit(1); }
console.log('PASS all lines pass the bark filter');
