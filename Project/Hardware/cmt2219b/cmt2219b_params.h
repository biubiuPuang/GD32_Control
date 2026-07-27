#ifndef __CMT2219B_PARAMS_H
#define __CMT2219B_PARAMS_H

#include <stdint.h>

/*
 * 先使用官方 CMT2300A Demo 的参数做 CMT2219B 接收移植验证：
 *
 * Frequency      : 433.920 MHz
 * Modulation     : GFSK
 * Data Rate      : 9.6 kbps
 * Payload Length : 32 bytes
 * Whitening      : Disable
 * CRC            : Disable
 */

static const uint8_t g_cmt2219b_cmt_bank[] = {
    0x00,
    0x66,
    0xEC,
    0x1D,
    0xF0,
    0x80,
    0x14,
    0x08,
    0x91,
    0x02,
    0x02,
    0xD0,
};

static const uint8_t g_cmt2219b_system_bank[] = {
    0xAE,
    0xE0,
    0x35,
    0x00,
    0x00,
    0xF4,
    0x10,
    0xE2,
    0x42,
    0x20,
    0x00,
    0x81,
};

static const uint8_t g_cmt2219b_frequency_bank[] = {
    0x42,
    0x71,
    0xCE,
    0x1C,
    0x42,
    0x5B,
    0x1C,
    0x1C,
};

static const uint8_t g_cmt2219b_data_rate_bank[] = {
    0xCA,
    0x60,
    0x10,
    0x33,
    0xE1,
    0x36,
    0x19,
    0x0A,
    0x9F,
    0x38,
    0x29,
    0x29,
    0xC0,
    0x94,
    0x0A,
    0x53,
    0x08,
    0x00,
    0xB4,
    0x00,
    0x00,
    0x01,
    0x00,
    0x00,
};

static const uint8_t g_cmt2219b_baseband_bank[] = {
    0x12,
    0x08,
    0x00,
    0xAA,
    0x04,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0xD4,
    0x2D,
    0xAA,
    0x00,
    0x1F,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x60,
    0xFF,
    0x00,
    0x00,
    0x1F,
    0x10,
};

static const uint8_t g_cmt2219b_tx_bank[] = {
    0x70,
    0x9A,
    0x0C,
    0x00,
    0x0F,
    0x90,
    0x00,
    0x8A,
    0x18,
    0x3F,
    0x7F,
};

#define CMT2219B_CMT_BANK_SIZE          ((uint8_t)sizeof(g_cmt2219b_cmt_bank))
#define CMT2219B_SYSTEM_BANK_SIZE       ((uint8_t)sizeof(g_cmt2219b_system_bank))
#define CMT2219B_FREQUENCY_BANK_SIZE    ((uint8_t)sizeof(g_cmt2219b_frequency_bank))
#define CMT2219B_DATA_RATE_BANK_SIZE    ((uint8_t)sizeof(g_cmt2219b_data_rate_bank))
#define CMT2219B_BASEBAND_BANK_SIZE     ((uint8_t)sizeof(g_cmt2219b_baseband_bank))
#define CMT2219B_TX_BANK_SIZE           ((uint8_t)sizeof(g_cmt2219b_tx_bank))

#endif
