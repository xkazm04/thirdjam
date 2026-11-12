// Lane A12 (ruling R-0007 point 1): the content filter every bark line must pass before it is committed. Pure, dependency-free,
// shared by generate.mjs (rejects at generation time), barks.test.mjs (the committed bank is clean) and the review file.
// Rules: a word list (profanity, slurs, sexual terms, threats, drugs), a regex ban on proper nouns that look like names, brands or
// teams (any capitalised word that is not the first word of a sentence and not in the allow list), ASCII only, 6..60 characters,
// no digits-as-score-brags with real teams, no URLs, no newlines, no quotes around the whole line, no emoji.
//   node tools/barks/filter.mjs Game/Content/Barks/barks.json      # prints statistics and the first rejected lines, exit 1 on any rejection

export const MAX_CHARS = 60;
export const MIN_CHARS = 6;

// Lower-case stems; a match is a whole word (or the stem followed by letters for the ones marked with *).
const BANNED = [
  // profanity
  'fuck*', 'shit*', 'bitch*', 'bastard*', 'asshole*', 'arse*', 'ass', 'asses', 'damn*', 'goddamn*', 'crap*', 'piss*', 'dick*', 'cock', 'cocks', 'prick*',
  'wanker*', 'bollocks', 'bugger*', 'bloody', 'hell', 'jerk*', 'suck*', 'screw*', 'douche*', 'twat*', 'turd*', 'pussy', 'pussies', 'balls',
  'frick*', 'freak*', 'friggin*', 'effing', 'wtf', 'omg', 'lmao', 'lol',
  // slurs and identity (any hit is a rejection; the bank is not the place for these words in any sense)
  'retard*', 'spaz*', 'gay', 'fag*', 'homo*', 'tranny', 'dyke*', 'nigg*', 'negro*', 'chink*', 'gook*', 'spic*', 'kike*', 'wetback*', 'cracker*',
  'redneck*', 'gypsy', 'gypsies', 'jew*', 'muslim*', 'christian*', 'jesus', 'christ', 'god', 'allah', 'nazi*', 'hitler', 'terrorist*',
  'cripple*', 'midget*', 'dwarf*', 'fatty', 'fatso', 'dumb', 'lame', 'psycho*', 'girly', 'sissy', 'mom', 'mommy', 'mama', 'dad', 'daddy',
  // sexual
  'sex*', 'sexy', 'porn*', 'nude*', 'naked', 'boob*', 'tit', 'tits', 'butt*', 'penis', 'vagina', 'horny', 'orgasm*', 'erect*',
  'dating', 'girlfriend*', 'boyfriend*', 'wife', 'husband', 'hooker*', 'whore*', 'slut*', 'pimp*', 'virgin*', 'bedroom',
  // threats and violence beyond the game's own checks
  'kill*', 'murder*', 'die', 'dies', 'died', 'dying', 'dead', 'death', 'corpse*', 'blood*', 'bleed*', 'stab*', 'shoot you', 'shot you',
  'gun*', 'knife', 'knives', 'weapon*', 'punch*', 'fist*', 'hurt*', 'break your', 'break you', 'snap your', 'smash your',
  'crush your', 'destroy you', 'end you', 'wreck you', 'strangle*', 'hospital*', 'ambulance*', 'injur*', 'cripp*',
  'concuss*', 'skull*', 'spine*', 'bomb*', 'explode*', 'torture*', 'suffer*', 'idiot*', 'moron*', 'stupid*', 'dumbass*', 'pathetic', 'worthless', 'useless',
  'shut up', 'go home', 'uninstall*', 'noob*', 'scrub*', 'cheat*', 'hack*',
  // drugs, alcohol, gambling
  'drunk*', 'beer*', 'booze*', 'vodka', 'whisk*', 'wine', 'weed', 'drug*', 'stoned', 'dope', 'smoke*', 'cigar*', 'vape*', 'betting',
  'casino*', 'gamble*',
  // real people, brands, teams, places (a short direct list; the proper-noun regex catches the rest)
  'nhl', 'khl', 'iihf', 'olympic*', 'stanley', 'gretzky', 'crosby', 'ovechkin', 'mcdavid', 'jagr', 'hasek', 'lemieux', 'orr', 'howe', 'messi', 'ronaldo',
  'lebron', 'jordan', 'brady', 'bauer', 'ccm', 'easton', 'reebok', 'nike', 'adidas', 'gatorade', 'redbull', 'red bull', 'pepsi',
  'coke', 'cola', 'mcdonald*', 'starbucks', 'google', 'microsoft', 'sony', 'nintendo', 'xbox', 'playstation', 'twitch', 'youtube',
  'tiktok', 'instagram', 'facebook', 'twitter', 'reddit', 'discord', 'netflix', 'disney', 'marvel', 'canada', 'canadian*', 'america*', 'usa', 'russia*',
  'sweden', 'swedish', 'finland', 'finnish', 'czech*', 'slovak*', 'german*', 'france', 'french', 'china', 'chinese', 'japan*', 'toronto', 'montreal',
  'boston', 'chicago', 'detroit', 'vegas', 'moscow', 'prague', 'stockholm', 'helsinki', 'leafs', 'habs', 'bruins', 'penguins', 'oilers',
  'canucks', 'avalanche', 'canadiens', 'blackhawks', 'kraken', 'coyotes', 'sabres', 'islanders', 'flyers', 'predators',
  // politics, money, religion-adjacent, sickness
  'trump', 'biden', 'putin', 'obama', 'president*', 'election*', 'vote*', 'politic*', 'tax*', 'dollar*', 'euro*', 'cash', 'money',
  'covid*', 'virus*', 'cancer*', 'disease*', 'vomit*', 'puke*', 'fart*', 'poop*', 'pee', 'toilet*', 'diaper*',
];

// Capitalised words allowed anywhere (hockey and film terms the theme uses, pronoun I, interjections).
const ALLOW_CAPS = new Set([
  'I', "I'm", "I'll", "I'd", "I've", 'OK', 'TAKE', 'CUT', 'WRAP', 'ACTION', 'RETAKE', 'GG', 'Zamboni', 'Oscar',
]);

const RULES = BANNED.map((w) => {
  const star = w.endsWith('*');
  const stem = (star ? w.slice(0, -1) : w).replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
  return { word: w, re: new RegExp(`(^|[^a-z'])${stem}${star ? "[a-z']*" : ''}(?=$|[^a-z'])`, 'i') };
});

// Returns [] for a clean line, else the list of reasons (each a short string; the first one is enough to reject).
export function checkLine(text) {
  const reasons = [];
  if (typeof text !== 'string') return ['not a string'];
  const t = text.trim();
  if (t !== text) reasons.push('leading or trailing whitespace');
  if (t.length < MIN_CHARS) reasons.push(`shorter than ${MIN_CHARS} characters`);
  if (t.length > MAX_CHARS) reasons.push(`longer than ${MAX_CHARS} characters (${t.length})`);
  if (/[^\x20-\x7E]/.test(t)) reasons.push('non-ASCII character (emoji, smart quote, accent)');
  if (/[\r\n\t]/.test(t)) reasons.push('line break or tab');
  if (/https?:|www\.|\.com\b|@/.test(t)) reasons.push('url or handle');
  if (/^["'].*["']$/.test(t)) reasons.push('wrapped in quotes');
  if (/[#*_`<>{}\[\]\\|~^]/.test(t)) reasons.push('markup character');
  if (/\d{3,}/.test(t)) reasons.push('long number');
  const lower = t.toLowerCase();
  for (const r of RULES) if (r.re.test(lower)) { reasons.push(`banned word "${r.word}"`); break; }
  // Proper nouns: a capitalised word that does not start a sentence and is not allowed.
  const words = t.split(/\s+/);
  let sentenceStart = true;
  for (const raw of words) {
    const w = raw.replace(/^[("]+|[)",.!?;:]+$/g, '');
    if (!w) continue;
    const isCap = /^[A-Z][a-z'’-]*$/.test(w) || /^[A-Z]{2,}$/.test(w);
    if (isCap && !sentenceStart && !ALLOW_CAPS.has(w)) { reasons.push(`capitalised word "${w}" looks like a name, brand or team`); break; }
    sentenceStart = /[.!?]$/.test(raw);
  }
  // A capitalised word with an apostrophe-s at sentence start can still be a name ("Gretzky's"): require a lower-case follower? Kept simple: the
  // direct list above covers the famous ones; the reviewer skims the rest.
  return reasons;
}

export const isClean = (text) => checkLine(text).length === 0;

export function filterBank(lines) {
  const kept = [];
  const rejected = [];
  const seen = new Set();
  for (const l of lines) {
    const reasons = checkLine(l.text);
    const key = l.text.trim().toLowerCase().replace(/[^a-z0-9 ]/g, '');
    if (reasons.length === 0 && seen.has(key)) reasons.push('duplicate');
    if (reasons.length) rejected.push({ ...l, reasons });
    else { seen.add(key); kept.push(l); }
  }
  return { kept, rejected };
}

// Reason histogram for the review file and the report.
export function stats(rejected) {
  const h = {};
  for (const r of rejected) { const k = r.reasons[0].replace(/".*"/, '"..."'); h[k] = (h[k] ?? 0) + 1; }
  return h;
}

if (process.argv[1] && /filter\.mjs$/.test(process.argv[1].replace(/\\/g, '/'))) {
  const { readFileSync } = await import('node:fs');
  const file = process.argv[2] ?? 'Game/Content/Barks/barks.json';
  const bank = JSON.parse(readFileSync(file, 'utf8'));
  const { kept, rejected } = filterBank(bank.lines);
  for (const r of rejected.slice(0, 40)) console.log(`REJECT ${r.id ?? ''} ${JSON.stringify(r.text)}: ${r.reasons.join('; ')}`);
  console.log(`${rejected.length ? 'FAIL' : 'PASS'} filter ${kept.length} clean, ${rejected.length} rejected of ${bank.lines.length}`);
  console.log(JSON.stringify({ tool: 'thirdjam.barks.filter/1', ok: rejected.length === 0, clean: kept.length, rejected: rejected.length, reasons: stats(rejected) }));
  process.exit(rejected.length ? 1 : 0);
}
