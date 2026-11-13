// A stand-in for UnrealEditor-Cmd: prints thirdjam.playtest/1 lines for -seeds=LO-HI. The cartridge name picks the behaviour.
const a = process.argv.slice(2); const get = (k) => (a.find((x) => x.startsWith(`-${k}=`)) || '').split('=')[1];
const cart = get('cartridge'); const [lo, hi] = get('seeds').split('-').map(Number);
const mp = a.includes('-Multiprocess');
if (cart === 'silent') { console.log('LogTurnkeySupport: nothing'); process.exit(1); }
for (let s = lo; s <= hi; s++) {
  if (cart === 'crashy' && s === 7) { console.log('Fatal error: EXCEPTION_ACCESS_VIOLATION at seed 7'); process.exit(3); }
  if (cart === 'ensure' && s === 3) console.log('LogJamAI: Error: Ensure condition failed: x');
  console.log(`[2026.11.13-20.00.00:000][  0]LogJamAI: Display: ${JSON.stringify({ tool: 'thirdjam.playtest/1', cartridge: cart, seed: s, result: s % 2 ? 'win' : 'loss', frames: 1000 + s, hash: `h${s}`, mp, maxframes: get('maxframes') })}`);
}
