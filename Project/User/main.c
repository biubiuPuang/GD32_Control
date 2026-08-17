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

// 假�??(代码实现)上位机串口发送配�?文件参数头文�?
#include "rf_apply.h"
// (真�?�的串口发�?)上位机串口发送配�?文件参数头文�?
#include "rf_uart_set_config.h"

// LED�?测试头文�?
#include "led_test.h"

// 按键头文�?
#include "app_key.h"

uint8_t key_num = 0;

int main(void)
{
    // LED�?引脚初�?�化
    pb9_led_init();
    // 发送芯片初始化
    app_211_init();
    delay_ms(10);
    // 接收�?片初始化
    app_221_init();
    delay_ms(10);
    // 按键初�?�化
    key_init();

    // 假�?�串口发送配�?参数数据
    // 尝试�? Flash 读取之前保存�? TX/RX 配置�?
    if (!rf_uart_config_restore())
    {
        // flash参数无效 配置默�?�参数到收发�?�?
        // fh_offset     = 40;
        // fh_channel    = 16;
        // channel_count = 100;
        rf_test_apply_config();
        /* 读取并打印收发芯片当前真实寄存器配置 */
        // 通过 SPI 读取 211 �? 221 �?片的实际寄存�?�?
        rf_print_tx_rx_real_freq();
        delay_ms(10);
    }

    while (1)
    {
        // 这个函数用于处理串口接收到的射�?�配�?命令�?
        rf_uart_config_process();

        // 获取按键�?
        get_key_num();

        // 211 发送数�?
        app_211_send_test_data();
        // 221�?片接收数�?
        app_221_receive_data();

        for (int i = 0; i <= 3; i++)
        {
            delay_ms(1000);
        }

    }
}
