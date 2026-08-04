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

    // ❌
    // 假装串口发送配置参数数据
    if (!rf_uart_config_restore()) 
    {
        rf_test_apply_config();
        delay_ms(100);
    }

    while (1)
    {
        // ❌
        rf_uart_config_process();
        delay_ms(100);

        // 211芯片发送数据
        app_211_send_data();
        delay_ms(100);
        // 221芯片接收数据
        app_221_receive_data();
        delay_ms(100);

        // 串口打印收发芯片的真实寄存器配置
        // rf_print_tx_rx_real_freq();
        // delay_ms(200);

        delay_ms(1000);
        delay_ms(1000);
    }
}
