// Line protocol shared with firmware/components/gift_core (see docs/protocol.md).
// Kept dependency-free so it can be unit-tested with plain node.

const MAX_PHRASE_BYTES = 127; // firmware GIFT_PHRASE_LEN (128) minus the NUL
const MAX_PHRASES = 32; // firmware GIFT_MAX_PHRASES
const MODES = ['love', 'clock', 'anim'];

// Single line, no control characters, at most MAX_PHRASE_BYTES of UTF-8 (cut on a code point boundary).
function sanitizePhrase(text) {
  let s = String(text).replace(/[\u0000-\u001f\u007f]+/g, ' ').trim();
  while (Buffer.byteLength(s, 'utf8') > MAX_PHRASE_BYTES) {
    s = Array.from(s).slice(0, -1).join('');
  }
  return s;
}

const commands = {
  ping: () => 'PING',
  get: () => 'GET',
  time: (date = new Date()) =>
    `TIME ${Math.floor(date.getTime() / 1000)} ${-date.getTimezoneOffset()}`,
  mode: (name) => {
    if (!MODES.includes(name)) throw new Error(`unknown mode: ${name}`);
    return `MODE ${name}`;
  },
  screen: (on) => `SCREEN ${on ? 'on' : 'off'}`,
  clearPhrases: () => 'PHRASES_CLEAR',
  phrase: (text) => {
    const s = sanitizePhrase(text);
    if (!s) throw new Error('empty phrase');
    return `PHRASE ${s}`;
  },
};

// Firmware replies start with OK/ERR; everything else on the console is ESP log noise.
function parseReply(line) {
  const l = String(line).trim();
  if (l === 'OK' || l.startsWith('OK ')) return { ok: true, text: l.slice(2).trim() };
  if (l === 'ERR' || l.startsWith('ERR ')) return { ok: false, error: l.slice(3).trim() };
  return null;
}

// Full phrase list as a command sequence (clear first, then one PHRASE per line).
function phraseSyncCommands(phrases) {
  const clean = phrases.map(sanitizePhrase).filter(Boolean).slice(0, MAX_PHRASES);
  return [commands.clearPhrases(), ...clean.map((p) => `PHRASE ${p}`)];
}

module.exports = { MAX_PHRASE_BYTES, MAX_PHRASES, MODES, sanitizePhrase, commands, parseReply, phraseSyncCommands };
