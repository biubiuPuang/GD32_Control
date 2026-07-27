#include "gd32e23x.h"
#include "systick.h"
#include "bsp_usart.h"
#include "radio_tx.h"
#include "radio_rx.h"
#include "cmt2119b_port.h"
#include "cmt2219b.h"
#include "cmt2219b_port.h"
#include <stdio.h>
#include <stdint.h>

static uint8_t tx_buf[32];
static uint8_t rx_buf[32];

static void test_packet_init(void)
{
    uint8_t i;

    for (i = 0; i < sizeof(tx_buf); i++)
    {
        tx_buf[i] = i + 1;
    }
}

static void print_buf(uint8_t *buf, uint8_t len)
{
    uint8_t i;

    for (i = 0; i < len; i++)
    {
        printf("%02X ", buf[i]);
    }

    printf("\r\n");
}

static void print_rx_regs(void)
{
    printf("RX REG: MODE_STA=0x%02X, INT1_CTL=0x%02X, INT2_CTL=0x%02X, INT_EN=0x%02X, FIFO_CTL=0x%02X, INT_CLR1=0x%02X, INT_CLR2=0x%02X, INT_FLAG=0x%02X, FIFO_FLAG=0x%02X\r\n",
           cmt2219b_read_reg(0x61),
           cmt2219b_read_reg(0x66),
           cmt2219b_read_reg(0x67),
           cmt2219b_read_reg(0x68),
           cmt2219b_read_reg(0x69),
           cmt2219b_read_reg(0x6A),
           cmt2219b_read_reg(0x6B),
           cmt2219b_read_reg(0x6D),
           cmt2219b_read_reg(0x6E));
}

int main(void)
{
    uint8_t tx_ok;
    uint8_t rx_ok;
    uint8_t send_ret;
    uint8_t tx_gpio_before;
    uint8_t tx_gpio_after;
    uint8_t rx_gpio_after;
    uint8_t rx_found;
    uint16_t i;
    uint32_t tx_count = 0;
    uint32_t rx_count = 0;

    systick_config();

    usart_gpio_config(115200);
    printf("\r\nPH2119BBA TX + PH2219BBA RX test start\r\n");

    test_packet_init();

    tx_ok = radio_tx_init();
    if (tx_ok)
    {
        printf("TX init OK\r\n");
    }
    else
    {
        printf("TX init ERROR\r\n");
    }

    rx_ok = radio_rx_init();
    if (rx_ok)
    {
        printf("RX init OK\r\n");
    }
    else
    {
        printf("RX init ERROR\r\n");
    }

    printf("TX packet: ");
    print_buf(tx_buf, sizeof(tx_buf));

    print_rx_regs();

    while (1)
    {
        tx_gpio_before = cmt2119b_gpio3_read();

        if (tx_ok)
        {
            send_ret = radio_tx_send(tx_buf, sizeof(tx_buf));
            tx_count++;

            tx_gpio_after = cmt2119b_gpio3_read();

            if (send_ret)
            {
                printf("TX OK, count=%lu, TX_GPIO3 before=%d after=%d\r\n",
                       tx_count, tx_gpio_before, tx_gpio_after);
            }
            else
            {
                printf("TX ERROR, count=%lu, TX_GPIO3 before=%d after=%d\r\n",
                       tx_count, tx_gpio_before, tx_gpio_after);
            }
        }

        rx_found = 0;

        /*
         * 发完后连续观察 200ms，避免 RX GPIO3 脉冲太短漏掉。
         * 当前实测 PH2219BBA GPIO3/PB10 空闲为高电平，所以临时按低电平有效判断。
         */
        for (i = 0; i < 200; i++)
        {
            if (rx_ok && (cmt2219b_read_reg(0x6D) & 0x01))
            {
                cmt2219b_go_stby();

                cmt2219b_read_fifo(rx_buf, sizeof(rx_buf));

                cmt2219b_clear_rx_fifo();
                cmt2219b_clear_interrupt_flags();
                cmt2219b_go_rx();

                rx_count++;

                printf("RX OK, count=%lu, data: ", rx_count);
                print_buf(rx_buf, sizeof(rx_buf));
            }

            delay_ms(1);
        }

        if (!rx_found)
        {
            rx_gpio_after = cmt2219b_gpio3_read();
            printf("RX no packet, RX_GPIO3=%d\r\n", rx_gpio_after);
            print_rx_regs();
        }

        delay_ms(800);
    }
}
