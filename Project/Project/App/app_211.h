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

// GD32E230оƬID�żĴ��ַ
#define GD32_UID_BASE 0x1FFFF7ACU

// GD32оƬUID��ȡ
void gd32_get_uid(uint32_t uid[3]);
// ��ӡ32λ����
void print_buf(uint8_t *buf, uint8_t len);
// 211оƬ��ʼ��
void app_211_init(void);
// 211оƬ��������
void app_211_send_data(void);

#endif /* __APP_211_H_ */
