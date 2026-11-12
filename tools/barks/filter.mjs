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
  'fuck*', 'shit*', 'bitch*', 'bastard*', 'asshole*', 'arse*', 'asses', 'goddamn*', 'piss*', 'dick*', 'cock', 'cocks', 'prick*',
  'wanker*', 'bollocks', 'bugger*', 'bloody', 'jerk*', 'suck*', 'screw*', 'douche*', 'twat*', 'turd*', 'pussy', 'pussies', 'balls',
  'frick*', 'freak*', 'friggin*', 'effing', 'wtf', 'omg', 'lmao', 'lol',
  // slurs and identity (any hit is a rejection; the bank is not the place for these words in any sense)
  'retard*', 'spaz*', 'gay', 'fag*', 'homo*', 'tranny', 'dyke*', 'nigg*', 'negro*', 'chink*', 'gook*', 'spic*', 'kike*', 'wetback*', 'cracker*',
  'redneck*', 'gypsy', 'gypsies', 'jew*', 'muslim*', 'christian*', 'jesus', 'christ', 'god', 'allah', 'nazi*', 'hitler', 'terrorist*',
  'cripple*', 'midget*', 'dwarf*', 'fatty', 'fatso', 'psycho*', 'girly', 'sissy', 'mom', 'mommy', 'mama', 'dad', 'daddy',
  // sexual
  'sex*', 'sexy', 'porn*', 'nude*', 'naked', 'boob*', 'tit', 'tits', 'penis', 'vagina', 'horny', 'orgasm*', 'erect*',
  'dating', 'girlfriend*', 'boyfriend*', 'wife', 'husband', 'hooker*', 'whore*', 'slut*', 'pimp*', 'virgin*', 'bedroom',
  // threats and violence beyond the game's own checks
  'kill*', 'murder*', 'butt', 'butts', 'punch you', 'punched you', 'punch him', 'hurt you', 'hurting you', 'death', 'corpse*', 'blood*', 'bleed*', 'stab*', 'shoot you', 
  'gun*', 'knife', 'knives', 'weapon*', 'fist*', 'break your', 'break you', 'snap your', 'smash your',
  'crush your', 'destroy you', 'end you', 'wreck you', 'strangle*', 'hospital*', 'ambulance*', 'injur*', 'cripp*',
  'concuss*', 'skull*', 'spine*', 'bomb*', 'explode*', 'torture*', 'suffer*', 'idiot*', 'moron*', 'dumbass*', 
  'uninstall*', 'noob*', 'scrub*', 'cheat*', 'hack*',
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
  'trump', 'biden', 'putin', 'obama', 'president*', 'election*', 'vote*', 'politic*', 'tax*', 'dollar*', 'euro*', 'cash', 
  'covid*', 'virus*', 'cancer*', 'disease*', 'vomit*', 'puke*', 'fart*', 'poop*', 'pee', 'toilet*', 'diaper*',
];

// Lane A41 (A37's CUTS.md finding): phrases that carry a banned word in a hockey sense. A line passes the word list when every hit of a
// banned word sits inside one of these phrases, and the phrase's words are not read as names by the proper-noun rule ("Sudden Death").
// "death" alone, or in any other phrase, is still banned.
export const ALLOW_PHRASES = ['sudden death'];
const ALLOW_PHRASE_RULES = ALLOW_PHRASES.map((p) => new RegExp(`(^|[^a-z'])${p.replace(/[.*+?^${}()|[\]\\]/g, '\\$&')}(?=$|[^a-z'])`, 'gi'));
// The text with every allowed phrase blanked (same length, so nothing else moves).
function withoutAllowedPhrases(lower) {
  let out = lower;
  for (const r of ALLOW_PHRASE_RULES) out = out.replace(r, (m, lead) => lead + ' '.repeat(m.length - lead.length));
  return out;
}

// R-0015: mild language the Teen bar tolerates, at most one line in ten (bank quota, enforced by filterBank) and never two in one line.
// "ass" only inside a "kick your ass" phrase; "damn" and "hell" as interjections; "crap" as a noun.
const MILD = ['damn', 'hell', 'crap', 'kick your ass', 'kicked your ass', 'kicking your ass', 'kick their ass', 'kick some ass'];
export const MILD_QUOTA = 0.1;
const esc = (w) => w.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
const MILD_RULES = MILD.map((w) => new RegExp(`(^|[^a-z'])${esc(w)}(?=$|[^a-z'])`, 'i'));
const BARE_ASS = /(^|[^a-z'])ass(?=$|[^a-z'])/i;
const ASS_PHRASE = /kick(ed|ing)? (your|their|some|your own) ass/i;

// Things this game does not have: a line about them reads wrong on the ice (the R-0007 reviewer pass, now in the filter).
const OFFWORLD = ['goalie', 'goalies', 'goaltender', 'keeper', 'crowd', 'fans', 'ref', 'refs', 'referee', 'referees', 'penalty', 'penalties', 'offside',
  'coach', 'coaches', 'tooth', 'teeth', 'overtime', 'shootout', 'zamboni', 'helmet cam', 'replay booth'];
const OFFWORLD_RULES = OFFWORLD.map((w) => new RegExp(`(^|[^a-z'])${esc(w)}(?=$|[^a-z'])`, 'i'));

// R-0021 point 2 (lane A20): barks are hockey first. The film words of the Director's Cut theme stay in the menu and the Designer, not on the ice.
// Hard: a line with one of these is rejected (the ones that only ever mean the film). Soft: counted and printed by the CLI as a statistic, never a
// rejection, because "take the puck", "frame the shot", "the lead" and "an extra step" are hockey; the reviewer reads those by eye.
const FILM_HARD = ['director', "director's", 'directors', 'cut', 'cuts', 'scene', 'scenes', 'script', 'scripts', 'reel', 'reels', 'take two', 'take three',
  'blooper', 'bloopers', 'reshoot', 'rehearsal', 'rehearse', 'screen test', 'final cut', 'movie', 'movies', 'camera', 'cameras', 'stunt double', 'cast list', 'actor', 'actors'];
const FILM_SOFT = ['take', 'takes', 'credits', 'frame', 'footnote', 'headline', 'makeup', 'editor', 'edit', 'edits', 'prop', 'props', 'clip', 'sequel', 'draft', 'tape',
  'rewind', 'wrap', 'rewrite', 'plot', 'slate', 'studio', 'extra', 'extras', 'filming', 'film', 'lead', 'picture', 'acting', 'review'];
const FILM_HARD_RULES = FILM_HARD.map((w) => new RegExp(`(^|[^a-z'])${esc(w)}(?=$|[^a-z'])`, 'i'));
const FILM_SOFT_RULES = FILM_SOFT.map((w) => new RegExp(`(^|[^a-z'])${esc(w)}(?=$|[^a-z'])`, 'i'));

// { hard: [...], soft: [...] }: the film words a line leans on. hard is a rejection in checkLine; soft is for the statistics and the review.
export function filmWords(text) {
  const t = String(text ?? '');
  const hard = FILM_HARD.filter((w, i) => FILM_HARD_RULES[i].test(t));
  const soft = FILM_SOFT.filter((w, i) => FILM_SOFT_RULES[i].test(t));
  return { hard, soft };
}

// Number of distinct mild words in a line (0 for a clean line).
export function mildCount(text) {
  let n = 0;
  for (const r of MILD_RULES) if (r.test(text)) n += 1;
  return n;
}

// Capitalised words allowed anywhere (pronoun I, interjections). The film words (TAKE, CUT, WRAP, ACTION, RETAKE, Oscar) left with R-0021 point 2.
const ALLOW_CAPS = new Set([
  'I', "I'm", "I'll", "I'd", "I've", 'OK', 'GG',
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
  const scan = withoutAllowedPhrases(lower);
  for (const r of RULES) if (r.re.test(scan)) { reasons.push(`banned word "${r.word}"`); break; }
  if (mildCount(t) > 1) reasons.push('two mild words in one line');
  if (BARE_ASS.test(lower) && !ASS_PHRASE.test(lower)) reasons.push('"ass" outside a kick-your-ass phrase');
  for (let i = 0; i < OFFWORLD_RULES.length; i += 1) if (OFFWORLD_RULES[i].test(lower)) { reasons.push(`not in this game: "${OFFWORLD[i]}"`); break; }
  const film = filmWords(t);
  if (film.hard.length) reasons.push(`film word "${film.hard[0]}" (R-0021: barks are hockey first)`);
  // Proper nouns: a capitalised word that does not start a sentence and is not allowed.
  const words = t.split(/\s+/);
  const clean = words.map((raw) => raw.replace(/^[("]+|[)",.!?;:]+$/g, ''));
  // Words inside an allowed phrase ("Sudden Death") are vocabulary, not names.
  const inPhrase = new Set();
  for (const p of ALLOW_PHRASES) {
    const pw = p.split(' ');
    for (let i = 0; i + pw.length <= clean.length; i += 1) {
      if (pw.every((w, k) => clean[i + k].toLowerCase() === w)) for (let k = 0; k < pw.length; k += 1) inPhrase.add(i + k);
    }
  }
  let sentenceStart = true;
  for (let i = 0; i < words.length; i += 1) {
    const raw = words[i];
    const w = clean[i];
    if (!w) continue;
    const isCap = /^[A-Z][a-z'’-]*$/.test(w) || /^[A-Z]{2,}$/.test(w);
    if (isCap && !sentenceStart && !ALLOW_CAPS.has(w) && !inPhrase.has(i)) { reasons.push(`capitalised word "${w}" looks like a name, brand or team`); break; }
    sentenceStart = /[.!?]$/.test(raw);
  }
  // A capitalised word with an apostrophe-s at sentence start can still be a name ("Gretzky's"): require a lower-case follower? Kept simple: the
  // direct list above covers the famous ones; the reviewer skims the rest.
  return reasons;
}

export const isClean = (text) => checkLine(text).length === 0;

// maxPerCombo caps every situation x speaker x tone (the bank ships 12..20); the mild quota is MILD_QUOTA of the kept lines.
export function filterBank(lines, { maxPerCombo = 20 } = {}) {
  const kept = [];
  const rejected = [];
  const seen = new Set();
  const perCombo = {};
  const clean = [];
  for (const l of lines) {
    const reasons = checkLine(l.text);
    const key = l.text.trim().toLowerCase().replace(/[^a-z0-9 ]/g, '');
    if (reasons.length === 0 && seen.has(key)) reasons.push('duplicate');
    if (reasons.length) rejected.push({ ...l, reasons });
    else { seen.add(key); clean.push(l); }
  }
  // Clean lines first, then mild ones up to the quota (one in ten of the bank) and the per-combination cap.
  let mildKept = 0;
  const ordered = [...clean.filter((l) => mildCount(l.text) === 0), ...clean.filter((l) => mildCount(l.text) > 0)];
  for (const l of ordered) {
    const combo = `${l.situation}.${l.speaker}.${l.tone}`;
    if ((perCombo[combo] ?? 0) >= maxPerCombo) { rejected.push({ ...l, reasons: ['surplus over the per-combination cap'] }); continue; }
    if (mildCount(l.text) > 0 && mildKept + 1 > Math.floor((kept.length + 1) * MILD_QUOTA)) { rejected.push({ ...l, reasons: ['mild-word quota (one in ten)'] }); continue; }
    if (mildCount(l.text) > 0) mildKept += 1;
    perCombo[combo] = (perCombo[combo] ?? 0) + 1;
    kept.push(l);
  }
  kept.sort((a, b) => (`${a.situation}.${a.speaker}.${a.tone}`).localeCompare(`${b.situation}.${b.speaker}.${b.tone}`) || a.text.localeCompare(b.text));
  return { kept, rejected, mildKept };
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
  // R-0021 soft film words: a statistic, not a rejection (take the puck, the lead, frame the shot are hockey); read them by eye.
  const soft = {};
  let softLines = 0;
  for (const l of bank.lines) { const f = filmWords(l.text).soft; if (f.length) softLines += 1; for (const w of f) soft[w] = (soft[w] ?? 0) + 1; }
  const mild = bank.lines.filter((l) => mildCount(l.text) > 0).length;
  console.log(`INFO theme: ${softLines} lines with a soft film word (hockey sense allowed, read by eye): ${Object.entries(soft).map(([w, n]) => `${w}=${n}`).join(' ') || 'none'}`);
  console.log(`${rejected.length ? 'FAIL' : 'PASS'} filter ${kept.length} clean, ${rejected.length} rejected of ${bank.lines.length}; ${mild} mild`);
  console.log(JSON.stringify({ tool: 'thirdjam.barks.filter/1', ok: rejected.length === 0, clean: kept.length, rejected: rejected.length, mild, soft_film_lines: softLines, soft_film_words: soft, reasons: stats(rejected) }));
  process.exit(rejected.length ? 1 : 0);
}
