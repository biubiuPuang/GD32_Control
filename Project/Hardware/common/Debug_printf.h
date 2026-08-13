#ifndef DEBUG_PRINTF__H
#define DEBUG_PRINTF__H

#include <stdio.h>

/* 1：允许调试输出；0：关闭调试输出 */
#define DEBUG_PRINTF_ENABLE  0

#if (DEBUG_PRINTF_ENABLE == 1)
#define debug_printf(...)    printf(__VA_ARGS__)
#else
#define debug_printf(...)
#endif

#endif /* DEBUG_PRINTF__H */
