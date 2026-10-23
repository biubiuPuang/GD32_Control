#ifndef __CMT2219B_H
#define __CMT2219B_H

#include <stdint.h>

#define CMT2219B_OK             1
#define CMT2219B_ERROR          0

#define CMT2219B_MAX_FIFO_SIZE  64

uint8_t cmt2219b_init(void);
uint8_t cmt2219b_is_exist(void);

uint8_t cmt2219b_read_reg(uint8_t addr);
void cmt2219b_write_reg(uint8_t addr, uint8_t dat);

void cmt2219b_set_frequency_channel(uint8_t channel);
void cmt2219b_set_frequency_step(uint8_t step);
void cmt2219b_set_afc_ovf_th(uint8_t afc_ovf_th);

void cmt2219b_read_fifo(uint8_t *buf, uint8_t len);
void cmt2219b_write_fifo(const uint8_t *buf, uint8_t len);

uint8_t cmt2219b_go_sleep(void);
uint8_t cmt2219b_go_stby(void);
uint8_t cmt2219b_go_rx(void);

void cmt2219b_enable_read_fifo(void);
void cmt2219b_clear_rx_fifo(void);
void cmt2219b_clear_interrupt_flags(void);

uint8_t cmt2219b_packet_received(void);

#endif
