#ifndef APP_TX_RX_CONFIG_H
#define APP_TX_RX_CONFIG_H

#include <stdint.h>
#include <stdio.h>
#include "rf_config.h"
#include "cmt2119b.h"
#include "cmt2219b.h"
#include "Debug_printf.h"

/*
 * 这两个寄存器就是芯片里面真实保存的值
 * 0x63 = 当前通道 fh_channel
 * 0x64 = 当前频隔 fh_offset
 */
#define RF_REG_FH_CHANNEL    0x63U
#define RF_REG_FH_OFFSET     0x64U

typedef struct
{
    uint8_t fh_offset;
    uint8_t fh_channel;
    uint32_t freq_hz;
} rf_chip_freq_t;

typedef struct
{
    rf_chip_freq_t tx;
    rf_chip_freq_t rx;
} rf_tx_rx_freq_t;

static uint32_t rf_calc_freq_hz(uint8_t fh_offset, uint8_t fh_channel);
void rf_get_tx_rx_real_freq(rf_tx_rx_freq_t *info);
void rf_print_tx_rx_real_freq(void);


#endif /* APP_TX_RX_CONFIG_H */
