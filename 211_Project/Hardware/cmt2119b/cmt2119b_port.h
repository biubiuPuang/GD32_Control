#ifndef __CMT2119B_PORT_H
#define __CMT2119B_PORT_H

#include "gd32e23x.h"
#include <stdint.h>

/*
 * PH2119BBA / CMT2119B module connection:
 *
 * FCSB  -> PB0
 * CSB   -> PA4
 * SDIO  -> PA7
 * SCLK  -> PA5
 * GPIO3 -> PB1
 * VCC   -> 3V3
 * GND   -> GND
 * ANT   -> 433MHz antenna
 */

#define CMT2119B_FCSB_PORT       GPIOB
#define CMT2119B_FCSB_PIN        GPIO_PIN_0
#define CMT2119B_FCSB_RCU        RCU_GPIOB

#define CMT2119B_CSB_PORT        GPIOA
#define CMT2119B_CSB_PIN         GPIO_PIN_4
#define CMT2119B_CSB_RCU         RCU_GPIOA

#define CMT2119B_SDIO_PORT       GPIOA
#define CMT2119B_SDIO_PIN        GPIO_PIN_7
#define CMT2119B_SDIO_RCU        RCU_GPIOA

#define CMT2119B_SCLK_PORT       GPIOA
#define CMT2119B_SCLK_PIN        GPIO_PIN_5
#define CMT2119B_SCLK_RCU        RCU_GPIOA

#define CMT2119B_GPIO3_PORT      GPIOB
#define CMT2119B_GPIO3_PIN       GPIO_PIN_1
#define CMT2119B_GPIO3_RCU       RCU_GPIOB

/* CMT2119B uses GD32E230 SPI0:
 * PA5 -> SPI0_SCK
 * PA7 -> SPI0_MOSI / 1-line bidirectional SDIO
 */
#define CMT2119B_SPI_PERIPH       SPI0
#define CMT2119B_SPI_RCU          RCU_SPI0
#define CMT2119B_SPI_AF           GPIO_AF_0

/*
 * 读寄存器时临时使用 GPIO 模拟 SPI。
 * 1us 高电平 + 1us 低电平，读时钟约 500kHz。
 */
#define CMT2119B_READ_SCLK_HALF_PERIOD_US  1U
#define CMT2119B_READ_CSB_SETUP_US         1U
#define CMT2119B_READ_CSB_HOLD_US          1U

void cmt2119b_port_init(void);

void cmt2119b_csb_high(void);
void cmt2119b_csb_low(void);

void cmt2119b_fcsb_high(void);
void cmt2119b_fcsb_low(void);

/* 寄存器读期间，PA5/PA7 在 GPIO 与 SPI0 之间切换 */
void cmt2119b_spi_pins_to_gpio(void);
void cmt2119b_spi_pins_to_spi0(void);

/* GPIO 模拟读 SPI 时使用 */
void cmt2119b_sclk_high(void);
void cmt2119b_sclk_low(void);

void cmt2119b_sdio_high(void);
void cmt2119b_sdio_low(void);
uint8_t cmt2119b_sdio_read(void);

void cmt2119b_sdio_output(void);
void cmt2119b_sdio_input(void);

uint8_t cmt2119b_gpio3_read(void);

void cmt2119b_delay_us(uint32_t us);
void cmt2119b_delay_ms(uint32_t ms);

#endif
