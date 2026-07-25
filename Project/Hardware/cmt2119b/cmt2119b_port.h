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

void cmt2119b_port_init(void);

void cmt2119b_csb_high(void);
void cmt2119b_csb_low(void);

void cmt2119b_fcsb_high(void);
void cmt2119b_fcsb_low(void);

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
