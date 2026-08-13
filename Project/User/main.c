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
    delay_ms(10);
    // 接收芯片初始化
    app_221_init();
    delay_ms(10);
    // 按键初始化
    key_init();

    // 假装串口发送配置参数数据
    // 尝试从 Flash 读取之前保存的 TX/RX 配置：
    if (!rf_uart_config_restore())
    {
        // flash参数无效 配置默认参数到收发芯片
        // fh_offset     = 40;
        // fh_channel    = 16;
        // channel_count = 100;
        rf_test_apply_config();
        /* 读取并打印收发芯片当前真实寄存器配置 */
        // 通过 SPI 读取 211 和 221 芯片的实际寄存器：
        rf_print_tx_rx_real_freq();
        delay_ms(10);
    }

    while (1)
    {
        // 这个函数用于处理串口接收到的射频配置命令。
        rf_uart_config_process();

        // 获取按键值
        get_key_num();

        // 211 发送数据
        app_211_send_test_data();
        // 221芯片接收数据
        app_221_receive_data();

        for (int i = 0; i <= 3; i++)
        {
            delay_ms(1000);
        }

    }
}
