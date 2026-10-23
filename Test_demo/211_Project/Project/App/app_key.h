#ifndef APP_KEY
#define APP_KEY 

#include "gd32e23x.h"
#include "stdio.h"
#include "stdint.h"
#include "app_211.h"
#include "led_test.h"

// 按键初始化
void key_init(void);
// 获取按键按下状态
uint8_t get_key_num(void);

// 低功耗
extern volatile uint8_t key_wakeup_flag;

#endif /* APP_KEY */
