// commentator voices: design -> save -> TTS the same ten lines. Usage: node commentators.mjs design|save|tts
import { spawnSync } from "node:child_process";
import fs from "node:fs";
const LAB = "C:/Users/kazda/jam/audio-lab", E = "C:/Users/kazda/jam/eleven-tools/eleven.mjs", LED = LAB + "/ledger.jsonl";
const D = LAB + "/commentators";
fs.mkdirSync(D, { recursive: true });
const V = {
  C1: ["Classic play-by-play", "Native American English, North American broadcast. Male, 45 to 55. Studio quality. Persona: classic sports play-by-play announcer. Emotion: steady, authoritative, building excitement. A deep, steady baritone with a warm resonant timbre and measured pacing that rises quickly in pitch and intensity as the action speeds up."],
  C2: ["High-energy excitable", "Native American English. Male, 25 to 35. Studio quality. Persona: hyper-energetic sports announcer. Emotion: excited, breathless, thrilled. A bright, fast-talking voice with a higher pitch, rapid pacing and punchy delivery, always on the edge of shouting."],
  C3: ["Dry witty colour", "Native English with a mid-Atlantic accent. Male, 40 to 50. Studio quality. Persona: dry witty colour commentator. Emotion: relaxed, amused, wry. A smooth, slightly raspy mid-range voice with a laid-back pace, deadpan timing and a hint of a smile in every line."],
  C4: ["1950s newsreel announcer", "Native English with a mid-Atlantic accent. Male, 40 to 50. Studio quality. Persona: 1950s newsreel and film-studio announcer. Emotion: theatrical, proud, urgent. A clipped, crisp, formal delivery with a projecting chest voice, precise consonants and dramatic pauses."],
  C5: ["Warm sharp female play-by-play", "Native American English. Female, 30 to 40. Studio quality. Persona: confident sports play-by-play announcer. Emotion: warm, sharp, enthusiastic. A clear, quick, confident voice with a warm mid-range timbre, crisp diction and fast pacing that lifts with the action."],
};
const LINES = { Welcome: "Welcome to the sound stage: blue against orange, take one.", Faceoff: "Blue wins the faceoff.", Break: "Orange on the break, nobody back!",
  Scores: "He shoots... he scores!", Boards: "Off the boards and out.", BigHit: "Big hit along the boards!", DirectorsCall: "Director's call! The ice is now glass.",
  TenSeconds: "Ten seconds left in the take.", Wrap: "And that's a wrap! Blue takes it.", Retake: "We'll need a retake on that one." };
const PREVIEW = "Blue wins the faceoff and Orange is on the break, nobody back! He shoots... he scores! Big hit along the boards, and that's a wrap on the second take.";
function run(args) {
  const r = spawnSync("node", [E, ...args, "--ledger", LED], { encoding: "utf8" });
  const last = (r.stdout || "").trim().split("\n").pop();
  let j = {}; try { j = JSON.parse(last); } catch {}
  if (!j.ok) console.log("FAIL", args[0], (r.stderr || last).slice(0, 300));
  return j;
}
const mode = process.argv[2];
if (mode === "design") for (const [k, [, desc]] of Object.entries(V)) {
  if (fs.existsSync(`${D}/${k}.json`)) continue;
  const j = run(["voice-design", "--description", desc, "--text", PREVIEW, "--guidance", "5", "--loudness", "0.5", "--out", `${D}/${k}`]);
  console.log(k, j.ok, j.credits_header, (j.previews || []).map((p) => p.duration_secs).join(","));
}
if (mode === "save") {
  const ids = fs.existsSync(`${D}/saved.json`) ? JSON.parse(fs.readFileSync(`${D}/saved.json`, "utf8")) : {};
  for (const [k, [name, desc]] of Object.entries(V)) {
    if (ids[k]) continue;
    const pick = Number(process.argv[3 + Object.keys(V).indexOf(k)] || 1);
    const prev = JSON.parse(fs.readFileSync(`${D}/${k}.json`, "utf8")).previews[pick - 1];
    const j = run(["voice-save", "--generated", prev.generated_voice_id, "--name", `Hokej ${k} ${name}`, "--description", desc]);
    if (j.ok) { ids[k] = { voice_id: j.voice_id, preview: pick }; fs.writeFileSync(`${D}/saved.json`, JSON.stringify(ids, null, 1)); }
    console.log(k, j.ok, j.voice_id);
  }
}
if (mode === "tts") {
  const ids = JSON.parse(fs.readFileSync(`${D}/saved.json`, "utf8"));
  for (const [k, { voice_id }] of Object.entries(ids)) for (const [ln, text] of Object.entries(LINES)) {
    const out = `${LAB}/VO_Commentator_${k}_${ln}.mp3`;
    if (fs.existsSync(out)) continue;
    const j = run(["tts", "--voice", voice_id, "--text", text, "--out", out, "--stability", "0.35", "--style", "0.4"]);
    console.log(k, ln, j.ok, j.credits_header);
  }
}
