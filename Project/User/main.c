#include "gd32e23x.h"
#include "systick.h"
#include "radio_tx.h"
#include <stdint.h>

static uint8_t tx_buf[32];

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

    while (1) {
        radio_tx_send(tx_buf, sizeof(tx_buf));
        delay_ms(1000);
    }
}
