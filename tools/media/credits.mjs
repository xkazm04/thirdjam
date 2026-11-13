// Builds submission/credits.md from ASSETS.md (A88). Usage: node tools/media/credits.mjs
import fs from 'node:fs';
const rows = fs.readFileSync('ASSETS.md','utf8').split('\n').filter(l=>/^\| (Game|Content)\//.test(l)).map(l=>{
  const c=l.split('|').slice(1,-1).map(s=>s.trim());
  return {path:c[0],kind:c[1],size:+c[2],origin:c[3],tool:c[4],licence:c[5],url:c[6],date:c[7],device:c[8],shipped:c[9],note:c[10]||''};
});
const by=(f)=>rows.reduce((m,r)=>{const k=f(r);m[k]=(m[k]||0)+1;return m;},{});
const sh=rows.filter(r=>r.shipped==='y');
const src=rows.filter(r=>r.origin==='sourced');
const srcS=src.filter(r=>r.shipped==='y');
const missing=src.filter(r=>!r.licence||!r.url);
const aiTools=by(r=>r.origin==='ai-generated'?r.tool.split(/ \+ | \(/)[0]:null);delete aiTools.null;
let o=`# Credits and provenance (from ASSETS.md, built by tools/media/credits.mjs)\n\nRows: ${rows.length}; shipped: ${sh.length}. Origin counts (all rows): ${JSON.stringify(by(r=>r.origin))}. Shipped: ${JSON.stringify(sh.reduce((m,r)=>{m[r.origin]=(m[r.origin]||0)+1;return m;},{}))}.\n\n`;
o+=`## Sourced (not original, not AI-generated): ${src.length} rows, ${srcS.length} shipped\n\n| path | kind | shipped | tool / origin | licence | url |\n|---|---|---|---|---|---|\n`;
for(const r of src)o+=`| ${r.path} | ${r.kind} | ${r.shipped} | ${r.tool} | ${r.licence||'**EMPTY**'} | ${r.url||'**EMPTY**'} |\n`;
o+=`\n## Findings\n\n- Sourced rows with an empty licence or url: ${missing.length} of ${src.length}.\n`;
o+=`\n## AI-generated content (declared as such, not "sourced"), by tool\n\n`;
for(const [k,v] of Object.entries(aiTools).sort((a,b)=>b[1]-a[1]))o+=`- ${k}: ${v} rows\n`;
fs.writeFileSync('submission/credits.md',o);
console.log(o.slice(0,3000));
