#ifndef __RADIO_TX_H
#define __RADIO_TX_H

#include <stdint.h>

uint8_t radio_tx_init(void);
uint8_t radio_tx_send(const uint8_t *buf, uint8_t len);

#endif
