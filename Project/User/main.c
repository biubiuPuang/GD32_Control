#include "gd32e23x.h"
#include "systick.h"
#include "bsp_usart.h"
#include "radio_tx.h"
#include "radio_rx.h"
#include "cmt2219b.h"
#include <stdio.h>
#include <stdint.h>
#include "app_211.h"
#include "app_221.h"
#include "app_tx-rx_config.h"

// 假设(代码实现)上位机串口发送配置文件参数头文件
#include "rf_apply.h"
// (真正的串口发送)上位机串口发送配置文件参数头文件
#include "rf_uart_set_config.h"

// LED灯测试头文件
#include "led_test.h"

// 按键头文件
#include "app_key.h"

uint8_t key_num = 0;

int main(void)
{
    // LED灯引脚初始化
    pa15_output_high_init();
    // 发送芯片初始化
    app_211_init();
    delay_ms(100);
    // 接收芯片初始化
    app_221_init();
    delay_ms(100);
    // 按键初始化
    key_init();

    // 假装串口发送配置参数数据
    if (!rf_uart_config_restore())
    {
        rf_test_apply_config();
        delay_ms(100);
    }

    while (1)
    {
        // 按键标志位
        uint8_t key_event = key_is_pressed();

        // 这个函数用于处理串口接收到的射频配置命令。
        rf_uart_config_process();

        // 211芯片发送测试数据
        // app_211_send_test_data();

        // 按键按下则发送数据
        if (key_event == 1)
        {
            // 发送数组里面的数据
            app_211_send_data();
            printf("Key1 press\r\n");
            // 串口打印收发芯片的真实寄存器配置
            rf_print_tx_rx_real_freq();
        }
        else if (key_event == 2)
        {
            app_211_send_data();
            printf("Key1 release\r\n");
        }

        // // 测试PA12按钮电
        // if (check_double_key12 == 1)
        // {
        //     if (key_num == 1)
        //     {
        //         led_PA15_toggle();
        //         key_num == 0;
        //     }
        // }
        // else
        // {
        //     key_num == 1;
        // }

        // 测试PA12按钮
        check_double_key12();

        // 221芯片接收数据
        app_221_receive_data();
    }
}
