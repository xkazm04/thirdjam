// post-process: mp3 -> trimmed 48 kHz 16-bit wav, measure duration / LUFS / true peak / RMS, loop edge check. Writes audio-lab/measure.json
import { spawnSync } from "node:child_process";
import fs from "node:fs";
const LAB = "C:/Users/kazda/jam/audio-lab";
const ff = (args) => spawnSync("ffmpeg", ["-hide_banner", "-nostdin", ...args], { encoding: "utf8", maxBuffer: 1 << 26 });
const LOOPS = /SkateStrideLoop|ProjectorLoop|SoundStageRoom|MatchLoop|MenuTheme/;
function probeDur(f) {
  const r = spawnSync("ffprobe", ["-v", "error", "-show_entries", "format=duration", "-of", "csv=p=0", f], { encoding: "utf8" });
  return parseFloat(r.stdout);
}
function loud(f) {
  const r = ff(["-i", f, "-af", "loudnorm=I=-23:TP=-1:LRA=11:print_format=json", "-f", "null", "-"]);
  const m = r.stderr.match(/\{[\s\S]*?"target_offset"[\s\S]*?\}/);
  if (!m) return {};
  const j = JSON.parse(m[0]);
  const I = parseFloat(j.input_i), TP = parseFloat(j.input_tp);
  return { lufs: isFinite(I) ? I : null, tp: isFinite(TP) ? TP : null };
}
function rmsOf(f, ss, t) {
  const a = ["-i", f];
  if (ss !== undefined) a.unshift("-ss", String(ss));
  const r = ff([...a, ...(t ? ["-t", String(t)] : []), "-af", "astats=metadata=0:measure_overall=RMS_level", "-f", "null", "-"]);
  const m = [...r.stderr.matchAll(/RMS level dB:\s*(-?[\d.inf]+)/g)].pop();
  return m ? parseFloat(m[1]) : null;
}
const out = [];
for (const name of fs.readdirSync(LAB).filter((n) => n.endsWith(".mp3")).sort()) {
  const mp3 = `${LAB}/${name}`, base = name.replace(/\.mp3$/, ""), wav = `${LAB}/wav/${base}.wav`;
  const stereo = name.startsWith("MUS_");
  const th = "-55dB";
  const one = `silenceremove=start_periods=1:start_threshold=${th}:start_silence=0.01:detection=peak`;
  const sr = `${one},areverse,${one},areverse`;
  const filt = LOOPS.test(name) && !stereo ? "anull" : sr; // do not touch the loops (SFX loops, music keep their full length)
  const r = ff(["-y", "-i", mp3, "-af", filt, "-ac", stereo ? "2" : "1", "-ar", "48000", "-c:a", "pcm_s16le", wav]);
  if (r.status !== 0) { out.push({ name, error: r.stderr.slice(-200) }); continue; }
  const rec = { name, wav: `wav/${base}.wav`, mp3_s: +probeDur(mp3).toFixed(3), wav_s: +probeDur(wav).toFixed(3), channels: stereo ? 2 : 1, ...loud(wav) };
  rec.rms_db = rmsOf(wav);
  if (LOOPS.test(name)) {
    const d = rec.wav_s;
    rec.edge_head_rms = rmsOf(wav, 0, 0.05);
    rec.edge_tail_rms = rmsOf(wav, Math.max(0, d - 0.05), 0.05);
    rec.edge_diff_db = rec.edge_head_rms != null && rec.edge_tail_rms != null ? +(rec.edge_tail_rms - rec.edge_head_rms).toFixed(1) : null;
  }
  out.push(rec);
  console.log(base, rec.mp3_s, rec.wav_s, rec.lufs, rec.tp, rec.rms_db, rec.edge_diff_db ?? "");
}
fs.writeFileSync(`${LAB}/measure.json`, JSON.stringify(out, null, 1));
