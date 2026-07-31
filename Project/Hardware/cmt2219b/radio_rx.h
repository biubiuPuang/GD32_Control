#ifndef __RADIO_RX_H
#define __RADIO_RX_H

#include <stdint.h>

#define RADIO_RX_PACKET_SIZE    32

uint8_t radio_rx_init(void);
uint8_t radio_rx_poll_packet(uint8_t *buf, uint8_t len);

// 手动快速调频相关 
void radio_rx_set_channel(uint8_t channel);
void radio_rx_set_frequency_step(uint8_t step);
void radio_rx_set_afc_ovf_th(uint8_t afc_ovf_th);

#endif
