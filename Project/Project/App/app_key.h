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
uint8_t key_is_pressed(void);
// PA12按钮测试代码
uint8_t check_double_key12(void);


#endif /* APP_KEY */
