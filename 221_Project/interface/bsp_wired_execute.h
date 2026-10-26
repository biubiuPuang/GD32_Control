#ifndef __BSP_WIRED_EXECUTE_H__
#define __BSP_WIRED_EXECUTE_H__

// 官方库函数
#include "gd32e23x.h"
// 仲裁函数头文件
#include "control.h"
// 打印输出头文件
#include "Debug_printf.h"

// 有线模式引脚初始化头文件
#include "bsp_wired.h"

extern Arbitrament END_REG;

/* 现有用户自定义执行函数，需同步重写函数体 */
bool wired_order(
    Arbitrament action,
    bool fast_target,
    bool probe_busy,
    uint32_t now_ms
);

#endif /* __BSP_WIRED_EXECUTE_H__ */
