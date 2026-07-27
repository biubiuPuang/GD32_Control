#include "gd32e23x.h"
#include "systick.h"
#include "radio_tx.h"
#include "radio_rx.h"
#include <stdint.h>

static uint8_t tx_buf[32];
static uint8_t rx_buf[32];

static volatile uint32_t rx_count;
static volatile uint8_t radio_rx_ready;

static void test_packet_init(void)
{
    uint8_t i;

    for (i = 0; i < sizeof(tx_buf); i++) {
        tx_buf[i] = i + 1;
    }
}

int main(void)
{
    systick_config();

    test_packet_init();

    radio_tx_init();
    radio_rx_ready = radio_rx_init();

    while (1) {
        if (radio_rx_ready) {
            if (radio_rx_poll_packet(rx_buf, sizeof(rx_buf))) {
                rx_count++;
            }
        }

        radio_tx_send(tx_buf, sizeof(tx_buf));
        delay_ms(1000);
    }
}
