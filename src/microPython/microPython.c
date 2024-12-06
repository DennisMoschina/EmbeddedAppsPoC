#include "microPython.h"

#define MP_HEAP_SIZE 4096

#include "micropython_embed/port/micropython_embed.h"
#include <zephyr/logging/log.h>

static char heap[MP_HEAP_SIZE];
int stack_top;

LOG_MODULE_REGISTER(microPython, CONFIG_MAIN_LOG_LEVEL);

void init_micropython() {
    mp_embed_init(&heap[0], sizeof(heap), &stack_top);
    LOG_INF("MicroPython initialized");
}

void deinit_micropython() {
    mp_embed_deinit();
    LOG_INF("MicroPython deinitialized");
}

void exec_micropython(const char *src) {
    LOG_INF("Executing MicroPython script\n\r%s", src);
    mp_embed_exec_str(src);
}