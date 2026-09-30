const test = require('node:test');
const assert = require('node:assert/strict');
const { sanitizePhrase, commands, parseReply, phraseSyncCommands, MAX_PHRASE_BYTES, MAX_PHRASES } = require('../src/protocol');

test('time command sends UTC seconds and tz offset in minutes east of UTC', () => {
  const d = new Date('2026-09-30T12:00:00Z');
  const [cmd, epoch, tz] = commands.time(d).split(' ');
  assert.equal(cmd, 'TIME');
  assert.equal(epoch, '1790769600');
  assert.equal(Number(tz), 0 - d.getTimezoneOffset());
});

test('mode validates names', () => {
  assert.equal(commands.mode('clock'), 'MODE clock');
  assert.throws(() => commands.mode('disco'));
});

test('sanitizePhrase strips newlines and control characters', () => {
  assert.equal(sanitizePhrase('a\r\nb\tc'), 'a b c');
});

test('sanitizePhrase truncates on UTF-8 code point boundary', () => {
  const s = sanitizePhrase('Ира'.repeat(100));
  assert.ok(Buffer.byteLength(s) <= MAX_PHRASE_BYTES);
  assert.equal(Buffer.from(s, 'utf8').toString('utf8'), s); // no broken characters
  assert.ok(!s.includes('�'));
});

test('phrase rejects empty text', () => {
  assert.throws(() => commands.phrase('  \n '));
});

test('parseReply recognises OK/ERR and ignores log lines', () => {
  assert.deepEqual(parseReply('OK'), { ok: true, text: '' });
  assert.deepEqual(parseReply('OK PONG\r'), { ok: true, text: 'PONG' });
  assert.deepEqual(parseReply('ERR bad_mode'), { ok: false, error: 'bad_mode' });
  assert.equal(parseReply('I (123) wifi: connected'), null);
  assert.equal(parseReply('OKAY'), null);
});

test('phraseSyncCommands clears first, drops empties, caps the list', () => {
  const cmds = phraseSyncCommands(['a', '', '  ', 'b']);
  assert.deepEqual(cmds, ['PHRASES_CLEAR', 'PHRASE a', 'PHRASE b']);
  const many = phraseSyncCommands(Array.from({ length: 50 }, (_, i) => `p${i}`));
  assert.equal(many.length, 1 + MAX_PHRASES);
});
