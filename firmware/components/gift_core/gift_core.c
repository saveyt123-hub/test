#include "gift_core.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *const MODE_NAMES[GIFT_MODE_COUNT] = {"love", "clock", "anim"};

/* Placeholders; the real phrases are sent from the PC app. */
static const char *const DEFAULT_PHRASES[] = {
    "Ирочка, ты лучшая!",
    "Загружаю любовь...",
    "Инициализация милоты...",
};

const char *gift_mode_name(gift_mode_t m)
{
    return (m >= 0 && m < GIFT_MODE_COUNT) ? MODE_NAMES[m] : "?";
}

void gift_state_init(gift_state_t *s)
{
    memset(s, 0, sizeof(*s));
    s->mode      = GIFT_MODE_LOVE;
    s->screen_on = true;
    for (size_t i = 0; i < sizeof(DEFAULT_PHRASES) / sizeof(DEFAULT_PHRASES[0]); i++) {
        strncpy(s->phrases[s->phrase_count++], DEFAULT_PHRASES[i], GIFT_PHRASE_LEN - 1);
    }
}

static void reply(char *out, size_t out_len, const char *fmt, ...) __attribute__((format(printf, 3, 4)));

static void reply(char *out, size_t out_len, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(out, out_len, fmt, ap);
    va_end(ap);
}

/* Copies line into buf without trailing whitespace. Returns false if it does not fit. */
static bool trim_copy(const char *line, char *buf, size_t buf_len)
{
    size_t n = strlen(line);
    while (n > 0 && (line[n - 1] == '\r' || line[n - 1] == '\n' || line[n - 1] == ' ' || line[n - 1] == '\t')) {
        n--;
    }
    if (n >= buf_len) {
        return false;
    }
    memcpy(buf, line, n);
    buf[n] = '\0';
    return true;
}

/* If buf is "<name>" or "<name> <rest>", returns rest ("" when absent), else NULL. */
static const char *match_cmd(const char *buf, const char *name)
{
    size_t n = strlen(name);
    if (strncmp(buf, name, n) != 0) {
        return NULL;
    }
    if (buf[n] == '\0') {
        return buf + n;
    }
    if (buf[n] == ' ') {
        return buf + n + 1;
    }
    return NULL;
}

bool gift_handle_line(gift_state_t *s, const char *line, char *out, size_t out_len)
{
    char buf[GIFT_PHRASE_LEN + 16];
    const char *arg;

    if (!trim_copy(line, buf, sizeof(buf))) {
        reply(out, out_len, "ERR too_long");
        return false;
    }
    if (buf[0] == '\0') {
        out[0] = '\0'; /* empty line: no reply */
        return false;
    }

    if (match_cmd(buf, "PING")) {
        reply(out, out_len, "OK PONG");
        return false;
    }

    if ((arg = match_cmd(buf, "TIME"))) {
        char *end;
        long long epoch = strtoll(arg, &end, 10);
        if (end == arg || epoch <= 0) {
            reply(out, out_len, "ERR bad_time");
            return false;
        }
        long tz = 0;
        if (*end == ' ') {
            char *end2;
            tz = strtol(end + 1, &end2, 10);
            if (end2 == end + 1 || *end2 != '\0' || tz < -14 * 60 || tz > 14 * 60) {
                reply(out, out_len, "ERR bad_time");
                return false;
            }
        } else if (*end != '\0') {
            reply(out, out_len, "ERR bad_time");
            return false;
        }
        s->epoch  = epoch;
        s->tz_min = (int)tz;
        reply(out, out_len, "OK");
        return true;
    }

    if ((arg = match_cmd(buf, "MODE"))) {
        for (int m = 0; m < GIFT_MODE_COUNT; m++) {
            if (strcmp(arg, MODE_NAMES[m]) == 0) {
                s->mode = (gift_mode_t)m;
                reply(out, out_len, "OK");
                return true;
            }
        }
        reply(out, out_len, "ERR bad_mode");
        return false;
    }

    if ((arg = match_cmd(buf, "SCREEN"))) {
        if (strcmp(arg, "on") == 0 || strcmp(arg, "off") == 0) {
            s->screen_on = (arg[1] == 'n');
            reply(out, out_len, "OK");
            return true;
        }
        reply(out, out_len, "ERR bad_screen");
        return false;
    }

    if (match_cmd(buf, "PHRASES_CLEAR")) {
        s->phrase_count = 0;
        reply(out, out_len, "OK");
        return true;
    }

    if ((arg = match_cmd(buf, "PHRASE"))) {
        size_t n = strlen(arg);
        if (n == 0) {
            reply(out, out_len, "ERR empty_phrase");
            return false;
        }
        if (n >= GIFT_PHRASE_LEN) {
            reply(out, out_len, "ERR too_long");
            return false;
        }
        if (s->phrase_count >= GIFT_MAX_PHRASES) {
            reply(out, out_len, "ERR full");
            return false;
        }
        memcpy(s->phrases[s->phrase_count], arg, n + 1);
        s->phrase_count++;
        reply(out, out_len, "OK");
        return true;
    }

    if (match_cmd(buf, "GET")) {
        reply(out, out_len, "OK mode=%s screen=%s phrases=%zu", gift_mode_name(s->mode),
              s->screen_on ? "on" : "off", s->phrase_count);
        return false;
    }

    reply(out, out_len, "ERR unknown_command");
    return false;
}
