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

// #define TEST_PACKET_LEN 32

// /*----------------------------------------------------------*/
// #define GD32_UID_BASE 0x1FFFF7ACU

// void gd32_get_uid(uint32_t uid[3])
// {
//     volatile const uint32_t *p = (volatile const uint32_t *)GD32_UID_BASE;

//     uid[0] = p[0]; // 0x1FFF F7AC
//     uid[1] = p[1]; // 0x1FFF F7B0
//     uid[2] = p[2]; // 0x1FFF F7B4
// }

// /* ----------------------------------------------------------- */
// /*
//  * 你要发送的数据就在这里改。
//  * 注意长度保持 32 字节，因为当前 RFPDK 配置的是固定 32 字节包。
//  */
// static uint8_t tx_buf[TEST_PACKET_LEN] = {
//     0x11,
//     0x22,
//     0x33,
//     0x44,
//     0x55,
//     0x66,
//     0x77,
//     0x88,
//     0x99,
//     0xAA,
//     0xBB,
//     0xCC,
//     0xDD,
//     0xEE,
//     0xFF,
//     0x00,
//     0x01,
//     0x02,
//     0x03,
//     0x04,
//     0x05,
//     0x06,
//     0x07,
//     0x08,
//     0x09,
//     0x0A,
//     0x0B,
//     0x0C,
//     0x0D,
//     0x0E,
//     0x0F,
//     0x10,
// };

// static uint8_t rx_buf[TEST_PACKET_LEN];

// static void print_buf(uint8_t *buf, uint8_t len)
// {
//     uint8_t i;

//     for (i = 0; i < len; i++)
//     {
//         printf("%02X ", buf[i]);
//     }

//     printf("\r\n");
// }

// int main(void)
// {
//     // uint8_t tx_ok;
//     // uint8_t rx_ok;
//     // uint8_t send_ret;
//     // uint32_t tx_count = 0;
//     // uint32_t rx_count = 0;

//     systick_config();

//     usart_gpio_config(115200);
//     printf("\r\nPH2119BBA TX + PH2219BBA RX user data test\r\n");

//     tx_ok = radio_tx_init();
//     if (tx_ok)
//     {
//         printf("TX init OK\r\n");
//     }
//     else
//     {
//         printf("TX init ERROR\r\n");
//     }

//     rx_ok = radio_rx_init();
//     if (rx_ok)
//     {
//         printf("RX init OK\r\n");
//     }
//     else
//     {
//         printf("RX init ERROR\r\n");
//     }

//     printf("User TX data: ");
//     print_buf(tx_buf, TEST_PACKET_LEN);

//     while (1)
//     {
//         /*
//          * 发送你在 tx_buf 里面填写的数据
//          */
//         if (tx_ok)
//         {
//             send_ret = radio_tx_send(tx_buf, TEST_PACKET_LEN);
//             tx_count++;

//             if (send_ret)
//             {
//                 printf("\r\nTX OK, count=%lu\r\n", tx_count);
//             }
//             else
//             {
//                 printf("\r\nTX ERROR, count=%lu\r\n", tx_count);
//             }
//         }

//         /*
//          * 这里判断的是接收芯片内部 INT_FLAG 寄存器。
//          * 0x6D 是 INT_FLAG，bit0 = 1 表示接收芯片收到完整数据包。
//          * 所以下面打印出来的数据，是从 PH2219BBA 接收芯片 FIFO 里面读出来的。
//          */
//         if (rx_ok && (cmt2219b_read_reg(0x6D) & 0x01))
//         {
//             cmt2219b_go_stby();

//             /*
//              * 从接收芯片 FIFO 读取数据，不是直接打印 tx_buf。
//              */
//             cmt2219b_read_fifo(rx_buf, TEST_PACKET_LEN);

//             cmt2219b_clear_rx_fifo();
//             cmt2219b_clear_interrupt_flags();
//             cmt2219b_go_rx();

//             rx_count++;

//             printf("RX chip FIFO data, count=%lu: ", rx_count);
//             print_buf(rx_buf, TEST_PACKET_LEN);
//         }
//         else
//         {
//             printf("RX chip no packet\r\n");
//         }

//         delay_ms(1000);
//     }
// }

int main(void)
{
    while (1)
    {
        // 发送芯片初始化
        app_211_init();
        // 接收芯片初始化
        app_221_init();
        // 211芯片发送数据
        app_211_send_data();
        // 221芯片接收数据
        app_221_receive_data();
    }
}
