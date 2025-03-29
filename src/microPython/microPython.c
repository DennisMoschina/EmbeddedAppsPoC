#include "microPython.h"

#define MP_HEAP_SIZE 4096

#include <zephyr/logging/log.h>
#include "py/mperrno.h"
#include "py/builtin.h"
#include "py/compile.h"
#include "py/runtime.h"
#include "py/repl.h"
#include "py/gc.h"
#include "py/mphal.h"
#include "py/stackctrl.h"
#include "shared/runtime/gchelper.h"
#include "shared/runtime/pyexec.h"
#include "shared/readline/readline.h"
#include "extmod/modbluetooth.h"

#include "src/zephyr_getchar.h"

#include "modzephyr.h"

#if MICROPY_PY_MACHINE
#include "modmachine.h"
#endif

#if MICROPY_VFS
#include "extmod/vfs.h"
#endif

LOG_MODULE_REGISTER(microPython, CONFIG_MAIN_LOG_LEVEL);

static char heap[MICROPY_HEAP_SIZE];

#if MICROPY_VFS
static void vfs_init(void) {
    mp_obj_t bdev = NULL;
    mp_obj_t mount_point;
    const char *mount_point_str = NULL;
    int ret = 0;

    #ifdef CONFIG_DISK_DRIVER_SDMMC
    mp_obj_t args[] = { mp_obj_new_str_from_cstr(CONFIG_SDMMC_VOLUME_NAME) };
    bdev = MP_OBJ_TYPE_GET_SLOT(&zephyr_disk_access_type, make_new)(&zephyr_disk_access_type, ARRAY_SIZE(args), 0, args);
    mount_point_str = "/sd";
    #elif defined(CONFIG_FLASH_MAP) && FIXED_PARTITION_EXISTS(storage_partition)
    mp_obj_t args[] = { MP_OBJ_NEW_SMALL_INT(FIXED_PARTITION_ID(storage_partition)), MP_OBJ_NEW_SMALL_INT(4096) };
    bdev = MP_OBJ_TYPE_GET_SLOT(&zephyr_flash_area_type, make_new)(&zephyr_flash_area_type, ARRAY_SIZE(args), 0, args);
    mount_point_str = "/flash";
    #endif

    if ((bdev != NULL)) {
        mount_point = mp_obj_new_str_from_cstr(mount_point_str);
        ret = mp_vfs_mount_and_chdir_protected(bdev, mount_point);
        // TODO: if this failed, make a new file system and try to mount again
    }
}
#endif // MICROPY_VFS

int real_main(void);
int mp_console_init(void);

void init_micropython() {
    #ifdef CONFIG_CONSOLE_SUBSYS
    mp_console_init();
    #else
    zephyr_getchar_init();
    #endif

    real_main();
    LOG_INF("MicroPython initialized");
}

int real_main(void) {
    volatile int stack_dummy = 0;

    #if MICROPY_PY_THREAD
    struct k_thread *z_thread = (struct k_thread *)k_current_get();
    mp_thread_init((void *)z_thread->stack_info.start, z_thread->stack_info.size / sizeof(uintptr_t));
    #endif

    mp_hal_init();

soft_reset:
    mp_stack_set_top((void *)&stack_dummy);
    // Make MicroPython's stack limit somewhat smaller than full stack available
    mp_stack_set_limit(CONFIG_MAIN_STACK_SIZE - 512);
    #if MICROPY_ENABLE_GC
    gc_init(heap, heap + sizeof(heap));
    #endif
    mp_init();

    #ifdef CONFIG_USB_DEVICE_STACK
    usb_enable(NULL);
    #endif

    #if MICROPY_VFS
    vfs_init();
    #endif

    #if MICROPY_MODULE_FROZEN || MICROPY_VFS
    pyexec_file_if_exists("main.py");
    #endif

    for (;;) {
        if (pyexec_mode_kind == PYEXEC_MODE_RAW_REPL) {
            if (pyexec_raw_repl() != 0) {
                break;
            }
        } else {
            if (pyexec_friendly_repl() != 0) {
                break;
            }
        }
    }

    LOG_INF("soft reboot\n");

    #if MICROPY_PY_BLUETOOTH
    mp_bluetooth_deinit();
    #endif
    #if MICROPY_PY_MACHINE
    machine_pin_deinit();
    #endif

    #if MICROPY_PY_THREAD
    mp_thread_deinit();
    gc_collect();
    #endif

    gc_sweep_all();
    mp_deinit();

    goto soft_reset;

    return 0;
}

void gc_collect(void) {
    gc_collect_start();
    gc_helper_collect_regs_and_stack();
    #if MICROPY_PY_THREAD
    mp_thread_gc_others();
    #endif
    gc_collect_end();
}

// void deinit_micropython() {
//     mp_embed_deinit();
//     LOG_INF("MicroPython deinitialized");
// }

// void exec_micropython(const char *src) {
//     LOG_INF("Executing MicroPython script\n\r%s", src);
//     mp_embed_exec_str(src);
// }

#if !MICROPY_READER_VFS
mp_lexer_t *mp_lexer_new_from_file(qstr filename) {
    mp_raise_OSError(ENOENT);
}
#endif

#if !MICROPY_VFS
mp_import_stat_t mp_import_stat(const char *path) {
    return MP_IMPORT_STAT_NO_EXIST;
}

mp_obj_t mp_builtin_open(size_t n_args, const mp_obj_t *args, mp_map_t *kwargs) {
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(mp_builtin_open_obj, 1, mp_builtin_open);
#endif

NORETURN void nlr_jump_fail(void *val) {
    while (1) {
        ;
    }
}

#ifndef NDEBUG
void MP_WEAK __assert_func(const char *file, int line, const char *func, const char *expr) {
    printf("Assertion '%s' failed, at file %s:%d\n", expr, file, line);
    __fatal_error("Assertion failed");
}
#endif