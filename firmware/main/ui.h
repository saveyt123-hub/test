/* Display layer. ui_stub.c only logs; replace it with the LVGL implementation once the
 * board's display/touch driver (JD9365 over MIPI-DSI) is brought up. */
#pragma once

#include "gift_core.h"

void ui_init(void);
void ui_boot_sequence(void);              /* "Загружаю любовь..." staged loading */
void ui_apply(const gift_state_t *state); /* redraw for the current mode / screen on-off */
