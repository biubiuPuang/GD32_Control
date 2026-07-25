#include "radio_tx.h"
#include "cmt2119b.h"

#define RADIO_TX_TIMEOUT_MS     1000

uint8_t radio_tx_init(void)
{
    return cmt2119b_init();
}

uint8_t radio_tx_send(const uint8_t *buf, uint8_t len)
{
    return cmt2119b_send_packet(buf, len, RADIO_TX_TIMEOUT_MS);
}
