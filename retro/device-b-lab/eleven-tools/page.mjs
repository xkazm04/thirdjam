// builds audio-lab/listen.html and audio-lab/table.md from ledger.jsonl + measure.json
import fs from "node:fs";
const LAB = "C:/Users/kazda/jam/audio-lab";
const led = {};
for (const l of fs.readFileSync(LAB + "/ledger.jsonl", "utf8").trim().split("\n")) {
  const r = JSON.parse(l);
  if (r.ok && r.out) led[r.out.split("/").pop()] = r;
}
const VN = { pqHfZKP75CvOlQylNhV4: "Bill", TX3LPaxmHKxFdv7VOQHJ: "Liam" };
const M = JSON.parse(fs.readFileSync(LAB + "/measure.json", "utf8"));
const esc = (s) => String(s).replace(/[&<>"]/g, (c) => ({ "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;" }[c]));
const f = (v, d = 1) => (v === null || v === undefined || Number.isNaN(v) ? "n/a" : v.toFixed(d));
const groups = { COM: [], VO: [], SFX: [], MUS: [] };
for (const m of M) groups[m.name.startsWith("VO_Commentator") ? "COM" : m.name.split("_")[0]].push(m);
const CD = LAB + "/commentators", saved = JSON.parse(fs.readFileSync(CD + "/saved.json", "utf8"));
const VNAME = { C1: "Classic play-by-play", C2: "High-energy, excitable", C3: "Dry, witty colour commentator", C4: "1950s newsreel and film-studio announcer", C5: "Warm, sharp female play-by-play" };
const note = (m) => {
  const n = [];
  if (m.lufs === null || m.lufs === undefined) n.push("too short for LUFS");
  if (m.tp !== null && m.tp > -0.1) n.push("peak at full scale");
  if (m.lufs !== null && m.lufs < -35 && !/Room/.test(m.name)) n.push("very quiet");
  if (m.wav_s === null || Number.isNaN(m.wav_s) || m.wav_s < 0.1) n.push("effectively silent");
  if (m.edge_diff_db !== undefined && m.edge_diff_db !== null && Math.abs(m.edge_diff_db) > 3) n.push("loop edges differ " + m.edge_diff_db + " dB");
  return n.join("; ");
};
let html = `<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Hokej audio lab AU1</title>
<style>:root{--bg:#fafafa;--fg:#1a1a1a;--mut:#666;--card:#fff;--bd:#d8d8d8;--ac:#1d4ed8}
@media(prefers-color-scheme:dark){:root{--bg:#141414;--fg:#e8e8e8;--mut:#9a9a9a;--card:#1e1e1e;--bd:#3a3a3a;--ac:#7aa2ff}}
body{background:var(--bg);color:var(--fg);font:16px/1.45 system-ui,sans-serif;margin:0 auto;max-width:860px;padding:0 16px 5rem}
h1{font-size:1.4rem}h2{margin-top:2rem;border-bottom:1px solid var(--bd)}h3{margin:1.2rem 0 .3rem;font-size:1rem;color:var(--mut)}
.take{background:var(--card);border:1px solid var(--bd);border-radius:6px;padding:.5rem .7rem;margin:.4rem 0}
.take label{display:flex;gap:.5rem;align-items:center;font-weight:600}audio{width:100%;margin:.3rem 0}.s{color:var(--mut);font-size:.85rem}
.w{color:#b45309}@media(prefers-color-scheme:dark){.w{color:#fbbf24}}
#bar{position:fixed;left:0;right:0;bottom:0;background:var(--card);border-top:1px solid var(--bd);padding:.6rem 16px}
button{font:inherit;padding:.4rem .9rem;border:1px solid var(--ac);background:var(--ac);color:#fff;border-radius:5px;cursor:pointer}
@media(prefers-color-scheme:dark){button{color:#000}}</style></head><body>
<h1>Hokej audio lab AU1: listen and pick</h1>
<p class="s">Generated 2026-11-12 with ElevenLabs. Loudness is integrated LUFS and true peak of the trimmed 48 kHz WAV (not normalised). Tick the takes you want, then press the button at the bottom.</p>`;
const title = { COM: "Sports commentator (five designed voices)", VO: "Director voice (Bill = old film director, Liam = crisp and energetic)", SFX: "Sound effects", MUS: "Music" };
for (const g of ["COM", "VO", "SFX", "MUS"]) {
  html += `<h2>${title[g]}</h2>`;
  if (g === "COM") {
    html += '<p class="s">Five invented voices made with Voice Design from a text description (no real broadcaster named or imitated). Same ten lines each. Saved to the account as stock-independent custom voices; the other two design previews per variant are spares at the end of each block.</p>';
    for (const c of ["C1", "C2", "C3", "C4", "C5"]) {
      const d = JSON.parse(fs.readFileSync(CD + "/" + c + ".json", "utf8"));
      html += `<h3>${c}: ${VNAME[c]}</h3><div class="take"><label><input type="checkbox" value="VOICE ${c} ${VNAME[c]} (voice_id ${saved[c].voice_id})"> use this voice (${c})</label><div class="s">${esc(d.description)}</div></div>`;
      for (const m of groups.COM.filter((x) => x.name.includes("_" + c + "_"))) {
        const r = led[m.name + ".mp3"] || {}, nt = note(m);
        html += `<div class="take"><label><input type="checkbox" value="${m.name}.mp3"> ${m.name}.mp3</label><audio controls preload="none" src="${m.name}.mp3"></audio><div class="s">${esc(r.text || "")}</div><div class="s">${f(m.wav_s, 2)} s | ${f(m.lufs)} LUFS | ${f(m.tp)} dBTP${nt ? ' | <span class="w">' + esc(nt) + "</span>" : ""}</div></div>`;
      }
      d.previews.forEach((p, i) => { if (i + 1 !== saved[c].preview) html += `<div class="take"><label><input type="checkbox" value="commentators/${c}_${i + 1}.mp3"> spare design preview commentators/${c}_${i + 1}.mp3 (not saved as a voice)</label><audio controls preload="none" src="commentators/${c}_${i + 1}.mp3"></audio><div class="s">${p.duration_secs.toFixed(1)} s</div></div>`; });
    }
    continue;
  }
  let last = "";
  for (const m of groups[g]) {
    const key = g === "SFX" ? m.name.replace(/_\d\d$/, "") : g === "VO" ? m.name.split("_")[2] : "";
    if (g !== "MUS" && key !== last) { html += `<h3>${esc(key)}</h3>`; last = key; }
    const r = led[m.name + ".mp3"] || {};
    const what = r.cmd === "tts" ? `${VN[r.voice] || r.voice}: "${r.text}"` : `${r.prompt || ""}${r.loop ? " [loop]" : ""}`;
    const nt = note(m);
    html += `<div class="take"><label><input type="checkbox" value="${m.name}.mp3"> ${m.name}.mp3</label>
<audio controls preload="none" src="${m.name}.mp3"></audio>
<div class="s">${esc(what)}</div>
<div class="s">${f(m.wav_s, 2)} s trimmed (mp3 ${f(m.mp3_s, 2)} s) | ${f(m.lufs)} LUFS | ${f(m.tp)} dBTP | RMS ${f(m.rms_db)} dB${m.edge_diff_db !== undefined && m.edge_diff_db !== null ? ` | loop edge tail-head ${m.edge_diff_db} dB` : ""}${nt ? ` | <span class="w">${esc(nt)}</span>` : ""}</div></div>`;
  }
}
html += `<div id="bar"><button id="b">Copy my picks</button> <span class="s" id="o"></span></div>
<script>document.getElementById("b").onclick=async()=>{const p=[...document.querySelectorAll("input:checked")].map(i=>i.value).join("\\n");
try{await navigator.clipboard.writeText(p);document.getElementById("o").textContent="copied "+(p?p.split("\\n").length:0)+" names"}catch(e){window.prompt("Copy these names:",p.replace(/\\n/g,", "))}};</script></body></html>`;
fs.writeFileSync(LAB + "/listen.html", html);
let t = "| file | prompt or text | voice | s (trimmed) | LUFS | peak dBTP | note |\n|---|---|---|---|---|---|---|\n";
for (const m of M) {
  const r = led[m.name + ".mp3"] || {};
  t += `| ${m.name} | ${(r.cmd === "tts" ? r.text : r.prompt || "").replace(/\|/g, "/")} | ${r.cmd === "tts" ? VN[r.voice] : ""} | ${f(m.wav_s, 2)} | ${f(m.lufs)} | ${f(m.tp)} | ${note(m)} |\n`;
}
fs.writeFileSync(LAB + "/table.md", t);
console.log(t);
