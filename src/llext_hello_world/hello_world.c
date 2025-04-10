#include <zephyr/llext/symbol.h>
#include <zephyr/sys/printk.h>
#include <zephyr/kernel.h>

void hello_world(void) {
    printk("Hello from dynamically loaded extension!\n");
    printk("allocating 300 bytes\n");

    // allocate 300 bytes
    char *buf = alloca(300);
    if (buf == NULL) {
        printk("Failed to allocate memory\n");
        return;
    }
    // fill the buffer with 'A's
    for (int i = 0; i < 300; i++) {
        buf[i] = 'A';
    }
    buf[299] = '\0'; // null-terminate the string
    printk("Allocated buffer: %s\n", buf);
}

LL_EXTENSION_SYMBOL(hello_world);