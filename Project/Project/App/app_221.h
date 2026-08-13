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


// 221接收�?片初始化
void app_221_init(void);
// 221接收�?片接收数�?
void app_221_receive_data(void);

#endif /* __APP_221_H_ */
