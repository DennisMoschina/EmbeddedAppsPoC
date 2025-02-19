#ifndef _MICRO_PYTHON_H_
#define _MICRO_PYTHON_H_

static const char *example =
    "print('Hello, World!')\n\r"
    "for i in range(10):\n\r"
    "    print(i)\n\r"
;

void init_micropython();
void deinit_micropython();

void exec_micropython(const char *src);

#endif