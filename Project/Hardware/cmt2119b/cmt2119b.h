#ifndef __CMT2119B_H
#define __CMT2119B_H

#include <stdint.h>

#define CMT2119B_OK          1
#define CMT2119B_ERROR       0

#define CMT2119B_MAX_FIFO_SIZE  64

uint8_t cmt2119b_init(void);
uint8_t cmt2119b_send_packet(const uint8_t *buf, uint8_t len, uint32_t timeout_ms);

uint8_t cmt2119b_read_reg(uint8_t addr);
void cmt2119b_write_reg(uint8_t addr, uint8_t dat);

// // 手动快速调频相关
void cmt2119b_set_frequency_channel(uint8_t channel);
void cmt2119b_set_frequency_step(uint8_t step);

void cmt2119b_write_fifo(const uint8_t *buf, uint8_t len);

void cmt2119b_go_sleep(void);
void cmt2119b_go_stby(void);
void cmt2119b_go_tx(void);

void cmt2119b_clear_interrupt_flags(void);
void cmt2119b_clear_tx_fifo(void);

#endif
