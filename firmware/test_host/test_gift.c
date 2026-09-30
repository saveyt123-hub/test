#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "gift_core.h"

static bool run(gift_state_t *s, const char *line, char *out, size_t n)
{
    return gift_handle_line(s, line, out, n);
}

int main(void)
{
    gift_state_t s;
    char out[128];

    gift_state_init(&s);
    assert(s.mode == GIFT_MODE_LOVE && s.screen_on && s.phrase_count == 3);

    assert(!run(&s, "PING\r\n", out, sizeof out) && strcmp(out, "OK PONG") == 0);
    assert(!run(&s, "", out, sizeof out) && out[0] == '\0');
    assert(!run(&s, "PINGX", out, sizeof out) && strcmp(out, "ERR unknown_command") == 0);

    assert(run(&s, "MODE clock", out, sizeof out) && s.mode == GIFT_MODE_CLOCK && strcmp(out, "OK") == 0);
    assert(!run(&s, "MODE disco", out, sizeof out) && strcmp(out, "ERR bad_mode") == 0 && s.mode == GIFT_MODE_CLOCK);

    assert(run(&s, "SCREEN off", out, sizeof out) && !s.screen_on);
    assert(run(&s, "SCREEN on", out, sizeof out) && s.screen_on);
    assert(!run(&s, "SCREEN dim", out, sizeof out) && strcmp(out, "ERR bad_screen") == 0);

    assert(run(&s, "TIME 1790000000 180", out, sizeof out) && s.epoch == 1790000000LL && s.tz_min == 180);
    assert(run(&s, "TIME 1790000001", out, sizeof out) && s.epoch == 1790000001LL && s.tz_min == 0);
    assert(!run(&s, "TIME abc", out, sizeof out) && strcmp(out, "ERR bad_time") == 0);
    assert(!run(&s, "TIME 1790000000 99999", out, sizeof out) && strcmp(out, "ERR bad_time") == 0);
    assert(!run(&s, "TIME 1790000000 12x", out, sizeof out) && strcmp(out, "ERR bad_time") == 0);
    assert(s.epoch == 1790000001LL);

    assert(run(&s, "PHRASES_CLEAR", out, sizeof out) && s.phrase_count == 0);
    assert(run(&s, "PHRASE Привет, Ира!", out, sizeof out) && s.phrase_count == 1);
    assert(strcmp(s.phrases[0], "Привет, Ира!") == 0);
    assert(!run(&s, "PHRASE", out, sizeof out) && strcmp(out, "ERR empty_phrase") == 0);

    char longline[GIFT_PHRASE_LEN + 32] = "PHRASE ";
    memset(longline + 7, 'a', GIFT_PHRASE_LEN);
    longline[7 + GIFT_PHRASE_LEN] = '\0';
    assert(!run(&s, longline, out, sizeof out) && strcmp(out, "ERR too_long") == 0);

    for (size_t i = s.phrase_count; i < GIFT_MAX_PHRASES; i++) {
        assert(run(&s, "PHRASE x", out, sizeof out));
    }
    assert(s.phrase_count == GIFT_MAX_PHRASES);
    assert(!run(&s, "PHRASE y", out, sizeof out) && strcmp(out, "ERR full") == 0);

    assert(!run(&s, "GET", out, sizeof out) && strncmp(out, "OK mode=clock screen=on phrases=32", 34) == 0);

    /* tiny output buffer must not overflow or crash */
    char tiny[4];
    run(&s, "GET", tiny, sizeof tiny);
    assert(strlen(tiny) < sizeof tiny);

    puts("gift_core: all tests passed");
    return 0;
}
