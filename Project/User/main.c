#include "gd32e23x.h"
#include "systick.h"
#include "bsp_usart.h"
#include "radio_tx.h"
#include "radio_rx.h"
#include "cmt2219b.h"
#include "cmt2219b_port.h"
#include <stdio.h>
#include <stdint.h>

#include "cmt2119b_port.h"


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

int main(void)
{
    uint8_t tx_ok;
    uint8_t rx_ok;
    uint8_t send_ret;
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

    while (1)
    {
        uint8_t tx_gpio_before;
        uint8_t tx_gpio_after;
        uint8_t rx_gpio_after;
        uint16_t i;
        uint8_t rx_found = 0;

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

        /*
         * 发完后连续观察 200ms，避免 RX GPIO3 低电平脉冲太短漏掉。
         */
        for (i = 0; i < 200; i++)
        {
            if (rx_ok && (cmt2219b_gpio3_read() == 0))
            {
                cmt2219b_go_stby();

                cmt2219b_read_fifo(rx_buf, sizeof(rx_buf));

                cmt2219b_clear_rx_fifo();
                cmt2219b_clear_interrupt_flags();
                cmt2219b_go_rx();

                rx_count++;
                rx_found = 1;

                printf("RX OK, count=%lu, data: ", rx_count);
                print_buf(rx_buf, sizeof(rx_buf));

                break;
            }

            delay_ms(1);
        }

        if (!rx_found)
        {
            rx_gpio_after = cmt2219b_gpio3_read();
            printf("RX no packet, RX_GPIO3=%d\r\n", rx_gpio_after);
        }

        delay_ms(800);
    }
}
