#ifndef __BSP_WIRED_EXECUTE_H__
#define __BSP_WIRED_EXECUTE_H__

// 官方库函数
#include "gd32e23x.h"
// 仲裁函数头文件
#include "control.h"

// 有线模式引脚初始化头文件
#include "bsp_wired.h"

extern Arbitrament END_REG;

void wired_execute_up(Arbitrament END_REG);

#endif /* __BSP_WIRED_EXECUTE_H__ */
