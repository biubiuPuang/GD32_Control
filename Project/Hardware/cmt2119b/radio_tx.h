#ifndef __RADIO_TX_H
#define __RADIO_TX_H

#include <stdint.h>

uint8_t radio_tx_init(void);
uint8_t radio_tx_send(const uint8_t *buf, uint8_t len);

// 手动快速调频相关 
void radio_tx_set_channel(uint8_t channel);
void radio_tx_set_frequency_step(uint8_t step);

#endif
