#ifndef __RADIO_RX_H
#define __RADIO_RX_H

#include <stdint.h>

#define RADIO_RX_PACKET_SIZE    32

uint8_t radio_rx_init(void);
uint8_t radio_rx_poll_packet(uint8_t *buf, uint8_t len);

#endif
