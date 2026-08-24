#ifndef __CMT2219B_SPI_H
#define __CMT2219B_SPI_H

#include <stdint.h>

void cmt2219b_spi_init(void);

void cmt2219b_spi_write_reg(uint8_t addr, uint8_t dat);
uint8_t cmt2219b_spi_read_reg(uint8_t addr);

void cmt2219b_spi_write_fifo(const uint8_t *buf, uint16_t len);
void cmt2219b_spi_read_fifo(uint8_t *buf, uint16_t len);

#endif
