#include <zephyr/llext/symbol.h>
#include <zephyr/sys/printk.h>

void hello_world(void) {
    printk("Hello from dynamically loaded extension!\n");
}

LL_EXTENSION_SYMBOL(hello_world);