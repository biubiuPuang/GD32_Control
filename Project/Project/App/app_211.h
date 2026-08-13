#ifndef __APP_211_H_
#define __APP_211_H_

#include "gd32e23x.h"
#include "systick.h"
#include "bsp_usart.h"
#include "radio_tx.h"
#include "radio_rx.h"
#include "cmt2219b.h"
#include <stdio.h>
#include <stdint.h>
#include "Debug_printf.h"

// 鍙戦�佹暟鎹?鍥哄畾闀垮害,32瀛楄妭
#define TEST_PACKET_LEN 32

/**
 * @brief 测试数据,串口如果打印11 22 33 则说明是初始化测试数据
 *
 */
extern volatile uint8_t tx_buf_data[TEST_PACKET_LEN];

// GD32E230芯片UID，寄存器地址
#define GD32_UID_BASE 0x1FFFF7ACU

// GD32E230芯片UID，寄存器地址长度，固定3个字节
void gd32_get_uid(uint32_t uid[3]);
// 串口打印16进制数据
void print_buf(uint8_t *buf, uint8_t len);
// 211发送芯片初始化
void app_211_init(void);
// 211发送芯片发送数据
void app_211_send_test_data(void);

#endif /* __APP_211_H_ */
