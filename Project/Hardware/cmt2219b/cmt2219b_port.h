#ifndef __CMT2219B_PORT_H
#define __CMT2219B_PORT_H

#include "gd32e23x.h"
#include <stdint.h>

/*
 * PH2219BBA / CMT2219B 接收模块引脚连接：
 *
 * FCSB  -> PB12
 * CSB   -> PB13
 * SDIO  -> PB15
 * CLK   -> PB14
 * GPIO3 -> PB10
 * VCC   -> 3V3
 * GND   -> GND
 * ANT   -> 433MHz 接收天线
 */

#define CMT2219B_FCSB_PORT      GPIOB
#define CMT2219B_FCSB_PIN       GPIO_PIN_12
#define CMT2219B_FCSB_RCU       RCU_GPIOB

#define CMT2219B_CSB_PORT       GPIOB
#define CMT2219B_CSB_PIN        GPIO_PIN_13
#define CMT2219B_CSB_RCU        RCU_GPIOB

#define CMT2219B_CLK_PORT       GPIOB
#define CMT2219B_CLK_PIN        GPIO_PIN_14
#define CMT2219B_CLK_RCU        RCU_GPIOB

#define CMT2219B_SDIO_PORT      GPIOB
#define CMT2219B_SDIO_PIN       GPIO_PIN_15
#define CMT2219B_SDIO_RCU       RCU_GPIOB

#define CMT2219B_GPIO3_PORT     GPIOB
#define CMT2219B_GPIO3_PIN      GPIO_PIN_10
#define CMT2219B_GPIO3_RCU      RCU_GPIOB

void cmt2219b_port_init(void);

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

uint8_t cmt2219b_gpio3_read(void);

void cmt2219b_delay_us(uint32_t us);
void cmt2219b_delay_ms(uint32_t ms);

#endif
