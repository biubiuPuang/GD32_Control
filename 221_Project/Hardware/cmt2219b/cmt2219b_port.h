#ifndef __CMT2219B_PORT_H
#define __CMT2219B_PORT_H

#include "gd32e23x.h"
#include <stdint.h>

/*
 * 本板CMT2219B接口：
 *
 * FCSB -> PB0
 * CSB  -> PA4
 * SCLK -> PA5
 * SDIO -> PA7，双向数据
 *
 * 使用SPI0，PA5/PA7复用AF0。
 * 芯片GPIO1/2/3未连接到MCU。
 * 当前接收状态通过寄存器轮询。
 */
#define CMT2219B_FCSB_PORT      GPIOB
#define CMT2219B_FCSB_PIN       GPIO_PIN_0
#define CMT2219B_FCSB_RCU       RCU_GPIOB

#define CMT2219B_CSB_PORT       GPIOA
#define CMT2219B_CSB_PIN        GPIO_PIN_4
#define CMT2219B_CSB_RCU        RCU_GPIOA

#define CMT2219B_CLK_PORT       GPIOA
#define CMT2219B_CLK_PIN        GPIO_PIN_5
#define CMT2219B_CLK_RCU        RCU_GPIOA

#define CMT2219B_SDIO_PORT      GPIOA
#define CMT2219B_SDIO_PIN       GPIO_PIN_7
#define CMT2219B_SDIO_RCU       RCU_GPIOA

#define CMT2219B_SPI_PERIPH     SPI0
#define CMT2219B_SPI_RCU        RCU_SPI0
#define CMT2219B_SPI_AF         GPIO_AF_0

void cmt2219b_port_init(void);

/*
 * SPI0 硬件功能和 GPIO 模拟 SPI 之间切换。
 */
void cmt2219b_spi_pins_to_gpio(void);
void cmt2219b_spi_pins_to_spi0(void);

void cmt2219b_csb_high(void);
void cmt2219b_csb_low(void);

void cmt2219b_fcsb_high(void);
void cmt2219b_fcsb_low(void);

void cmt2219b_clk_high(void);
void cmt2219b_clk_low(void);

void cmt2219b_sdio_high(void);
void cmt2219b_sdio_low(void);
uint8_t cmt2219b_sdio_read(void);

void cmt2219b_sdio_output(void);
void cmt2219b_sdio_input(void);

void cmt2219b_delay_us(uint32_t us);
void cmt2219b_delay_ms(uint32_t ms);

#endif
