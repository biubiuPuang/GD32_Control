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
#include "packet_loss_test.h"
#include "cmt2119b.h"

// 假装(代码实现)上位机串口发送配置文件参数头文件
#include "rf_apply.h"
// (真实的串口发送)上位机串口发送配置文件参数头文件
#include "rf_uart_set_config.h"
// LED灯测试头文件
#include "led_test.h"
// 按键头文件
#include "app_key.h"

uint8_t key_num = 0;

int main(void)
{
    // LED灯引脚初始化
    pa15_led_init();
    // PB9板载LED灯点亮
    pb9_led_init(); 
    // 发送芯片初始化
    app_211_init();
    delay_ms(10);
    // 接收芯片初始化
    app_221_init();
    delay_ms(10);

    // ===== 裸机测试：直接往串口寄存器写字符，绕过DMA =====
    USART_TDATA(USART0) = 'A';
    while (usart_flag_get(USART0, USART_FLAG_TC) == RESET) {}
    USART_TDATA(USART0) = 'B';
    while (usart_flag_get(USART0, USART_FLAG_TC) == RESET) {}
    USART_TDATA(USART0) = '\r';
    while (usart_flag_get(USART0, USART_FLAG_TC) == RESET) {}
    USART_TDATA(USART0) = '\n';
    while (usart_flag_get(USART0, USART_FLAG_TC) == RESET) {}
    // ===== 裸机测试结束 =====

    // 按键初始化
    key_init();

    // 假装串口发送配置参数数据
    // 尝试从 Flash 读取之前保存的 TX/RX 配置
    debug_printf("=== [DBG] before rf_uart_config_restore ===\r\n");
    if (!rf_uart_config_restore())
    {
        debug_printf("=== [DBG] flash invalid, apply default ===\r\n");
        // flash参数无效 配置默认参数到收发芯片  fh_offset  = 40;  fh_channel = 16;  channel_count = 100;
        rf_test_apply_config();
        debug_printf("=== [DBG] rf_test_apply_config done ===\r\n");
        /* 读取并打印收发芯片当前真实寄存器配置 */
        // 通过 SPI 读取 211 和 221 芯片的实际寄存器
        rf_print_tx_rx_real_freq();
        debug_printf("=== [DBG] rf_print_tx_rx_real_freq done ===\r\n");
        delay_ms(10);
    }
    debug_printf("=== [DBG] config phase done, entering main loop ===\r\n");

    // /* ===== 诊断：测试开始前打印 RX 状态 + 频点寄存器 ===== */
    // debug_printf("== RX 状态 STA=0x%02X (0x05=RX 0x02=STBY 0x01=SLEEP) ==\r\n",
    //              cmt2219b_read_reg(0x61));
    // debug_printf("== TX chnl=0x%02X ofs=0x%02X | RX chnl=0x%02X ofs=0x%02X ==\r\n",
    //              cmt2119b_read_reg(0x63), cmt2119b_read_reg(0x64),
    //              cmt2219b_read_reg(0x63), cmt2219b_read_reg(0x64));

    while (1)
    {

        // /* ------------------------------------- */
        // // 丢包率测试
        // /* ===== 丢包率测试 ===== */
        // packet_loss_test_run();

        // while (1)
        // {
        //     /* 测试跑完后停在这里，方便看串口结果 */
        // }
        // /* ------------------------------------- */


        // 这个函数用于处理串口接收到的射频配置命令
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

        // delay_ms(200);
    }


}
