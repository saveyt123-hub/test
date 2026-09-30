/* Platform-independent core of the gift device: state + serial command parser.
 * No ESP-IDF or LVGL dependencies, so it is unit-tested on the host (see test_host/). */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define GIFT_MAX_PHRASES 32
#define GIFT_PHRASE_LEN  128 /* bytes incl. terminating NUL */

typedef enum {
    GIFT_MODE_LOVE,
    GIFT_MODE_CLOCK,
    GIFT_MODE_ANIM,
    GIFT_MODE_COUNT,
} gift_mode_t;

typedef struct {
    gift_mode_t mode;
    bool        screen_on;
    int64_t     epoch;  /* UTC seconds as last reported by the PC, 0 = unknown */
    int         tz_min; /* local offset from UTC in minutes */
    size_t      phrase_count;
    char        phrases[GIFT_MAX_PHRASES][GIFT_PHRASE_LEN];
} gift_state_t;

/* Resets to defaults: love mode, screen on, built-in phrases. */
void gift_state_init(gift_state_t *s);

const char *gift_mode_name(gift_mode_t m);

/* Handles one command line (CR/LF already stripped or not, both fine) and writes a
 * one-line reply ("OK ..." / "ERR ...") into out. Returns true if the state changed
 * and the UI should be refreshed. See docs/protocol.md for the command list. */
bool gift_handle_line(gift_state_t *s, const char *line, char *out, size_t out_len);
