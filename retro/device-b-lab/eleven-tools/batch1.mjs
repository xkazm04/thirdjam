// batch 1 runner: voices + sfx. Calls eleven.mjs one at a time; each POST once.
import { spawnSync } from "node:child_process";
import fs from "node:fs";
const LAB = "C:/Users/kazda/jam/audio-lab";
const E = "C:/Users/kazda/jam/eleven-tools/eleven.mjs";
const LEDGER = LAB + "/ledger.jsonl";
const only = process.argv[2] || "all";
function run(args) {
  const r = spawnSync("node", [E, ...args, "--ledger", LEDGER], { encoding: "utf8" });
  const last = (r.stdout || "").trim().split("\n").pop();
  console.log(args[0], args.find((a, i) => args[i - 1] === "--out")?.split("/").pop(), r.status, last.slice(0, 160));
  if (r.status !== 0) console.log(r.stderr.slice(0, 300));
  return r.status === 0;
}
const voices = { Bill: "pqHfZKP75CvOlQylNhV4", Liam: "TX3LPaxmHKxFdv7VOQHJ" };
const lines = { Action: "Action!", Cut: "Cut!", CutPrintIt: "Cut! Print it.", ThatsAWrap: "That's a wrap!", TakeTwo: "Take two.",
  TakeThree: "Take three.", PlacesEveryone: "Places, everyone.", QuietOnTheSet: "Quiet on the set.", Rolling: "Rolling.",
  FromTheTop: "From the top.", DirectorsCall: "Director's call!", Retake: "Retake?" };
const sfx = [
  ["SkateCarve", 1, false, "A single ice skate blade carving a sharp turn on a hockey rink, crisp scrape and spray of ice, close, no music"],
  ["SkateStrideLoop", 4, true, "Steady rhythmic ice skating strides, alternating blade glides on a hockey rink, seamless loop, no music"],
  ["SlapShot", 1, false, "A hockey slap shot: stick whacks a puck with a sharp crack and a quick whoosh, close, dry"],
  ["PuckBoards", 1, false, "A hard rubber hockey puck slamming into wooden rink boards, solid thud with a short rattle"],
  ["PuckPost", 1.5, false, "A hockey puck hitting a metal goal post with a bright ringing ping that decays"],
  ["GoalHorn", 2.5, false, "A short loud generic arena goal horn, one blast, brassy air horn, no crowd, no music"],
  ["BodyCheck", 1.5, false, "A hockey body check slamming a player into the boards, heavy thump and plexiglass rattling"],
  ["Clapperboard", 0.6, false, "A wooden film clapperboard slate clap, sharp crisp wood on wood snap, dry"],
  ["ProjectorLoop", 5, true, "An old film projector whirring, steady mechanical sprocket clatter and soft motor hum, seamless loop"],
  ["SoundStageRoom", 8, true, "Room tone of a very large empty film sound stage: quiet airy reverberant space with a faint electrical lamp hum, seamless loop"],
  ["RuleChangeSting", 1.5, false, "A quick cinematic whoosh rising into a deep impact hit, short trailer sting, no music"],
  ["PickupChime", 0.8, false, "A soft pleasant pickup chime, two gentle bell notes rising, bright and short"],
  ["UiClick", 0.3, false, "A tiny dry click like a slate marker tap, short wooden tick, UI button sound"],
];
if (only === "all" || only === "vo") {
  for (const [vt, vid] of Object.entries(voices)) for (const [k, text] of Object.entries(lines)) {
    const out = `${LAB}/VO_Director_${vt}_${k}.mp3`;
    if (fs.existsSync(out)) continue;
    run(["tts", "--voice", vid, "--text", text, "--out", out, "--stability", vt === "Bill" ? "0.4" : "0.3", "--style", "0.5"]);
  }
}
if (only === "all" || only === "sfx") {
  for (const [name, sec, loop, prompt] of sfx) for (const n of ["01", "02"]) {
    const out = `${LAB}/SFX_${name}_${n}.mp3`;
    if (fs.existsSync(out)) continue;
    run(["sfx", "--prompt", prompt, "--seconds", String(sec), "--influence", "0.5", ...(loop ? ["--loop"] : []), "--out", out]);
  }
}
