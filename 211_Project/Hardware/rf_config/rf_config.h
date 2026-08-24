#ifndef __RF_CONFIG_H
#define __RF_CONFIG_H

#include <stdint.h>

#define RF_CONFIG_MAGIC      0x52464347UL  /* 'RFCG' */
#define RF_CONFIG_VERSION    1

#define RF_ROLE_TX           1
#define RF_ROLE_RX           2

#define RF_BASE_FREQ_KHZ     433920UL
#define RF_CHANNEL_MAX       100

#define RF_OK       1
#define RF_ERROR    0

typedef struct
{
    uint32_t magic;
    uint16_t version;

    uint16_t pair_id;
    uint8_t role;

    uint8_t fh_offset;
    uint8_t fh_channel;
    uint8_t channel_count;

    uint16_t crc16;
} rf_factory_config_t;

uint8_t rf_config_check(const rf_factory_config_t *cfg);
uint16_t rf_crc16_calc(const uint8_t *data, uint16_t len);

#endif
