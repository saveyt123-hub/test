/* UNTESTED on hardware: written before the board arrived. Expect to adjust console/USB setup. */
#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "gift_core.h"
#include "ui.h"

static gift_state_t     s_state;
static SemaphoreHandle_t s_lock;
static volatile bool    s_dirty;

/* Reads command lines from the console (USB) and updates the shared state. */
static void serial_task(void *arg)
{
    (void)arg;
    char line[GIFT_PHRASE_LEN + 32];
    char reply[96];
    while (1) {
        if (!fgets(line, sizeof(line), stdin)) {
            vTaskDelay(pdMS_TO_TICKS(20));
            continue;
        }
        xSemaphoreTake(s_lock, portMAX_DELAY);
        bool changed = gift_handle_line(&s_state, line, reply, sizeof(reply));
        if (changed) {
            s_dirty = true;
        }
        xSemaphoreGive(s_lock);
        if (reply[0]) {
            printf("%s\n", reply); /* replies start with OK/ERR; ESP log lines never do */
        }
    }
}

void app_main(void)
{
    s_lock = xSemaphoreCreateMutex();
    gift_state_init(&s_state);

    ui_init();
    ui_boot_sequence();
    ui_apply(&s_state);

    xTaskCreate(serial_task, "serial", 4096, NULL, 5, NULL);

    while (1) {
        if (s_dirty) {
            gift_state_t snapshot;
            xSemaphoreTake(s_lock, portMAX_DELAY);
            snapshot = s_state;
            s_dirty  = false;
            xSemaphoreGive(s_lock);
            ui_apply(&snapshot);
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
