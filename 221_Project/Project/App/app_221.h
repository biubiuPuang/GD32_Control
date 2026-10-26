#ifndef __APP_221_H_
#define __APP_221_H_

#include "gd32e23x.h"
#include "systick.h"
#include "bsp_usart.h"
#include "radio_tx.h"
#include "radio_rx.h"
#include "cmt2219b.h"
#include <stdio.h>
#include <stdint.h>
#include "led_test.h"
#include "Debug_printf.h"
#include "control.h"

// 221鎺ユ敹鑺?鐗囧垵濮嬪寲
void app_221_init(void);
/* 现有用户自定义接收函数，需同步修改函数体 */
void app_221_receive_data(
    Ask *wireless,
    uint32_t now_ms
);

#endif /* __APP_221_H_ */
