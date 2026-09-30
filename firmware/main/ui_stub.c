#include "ui.h"

#include <stdio.h>

void ui_init(void)
{
    printf("[ui] init (stub)\n");
}

void ui_boot_sequence(void)
{
    static const char *const stages[] = {"Загружаю любовь...", "Инициализация милоты...", "Готово!"};
    for (size_t i = 0; i < sizeof(stages) / sizeof(stages[0]); i++) {
        printf("[ui] boot: %s\n", stages[i]);
    }
}

void ui_apply(const gift_state_t *state)
{
    printf("[ui] mode=%s screen=%s\n", gift_mode_name(state->mode), state->screen_on ? "on" : "off");
}
