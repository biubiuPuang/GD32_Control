#include "rf_config.h"

/*
这个函数负责防止错误参数：
fh_offset = 0
channel_count = 0
channel = 120
最大频率超范围
*/
uint8_t rf_config_check(const rf_factory_config_t *cfg)
{
    uint32_t max_freq_khz;

    if (cfg == 0)
    {
        return RF_ERROR;
    }

    if (cfg->magic != RF_CONFIG_MAGIC)
    {
        return RF_ERROR;
    }

    if (cfg->version != RF_CONFIG_VERSION)
    {
        return RF_ERROR;
    }

    if ((cfg->role != RF_ROLE_TX) && (cfg->role != RF_ROLE_RX))
    {
        return RF_ERROR;
    }

    if (cfg->fh_offset == 0)
    {
        return RF_ERROR;
    }

    if (cfg->channel_count == 0)
    {
        return RF_ERROR;
    }

    if (cfg->channel_count > RF_CHANNEL_MAX)
    {
        return RF_ERROR;
    }

    if (cfg->fh_channel >= cfg->channel_count)
    {
        return RF_ERROR;
    }

    /*
     * 计算最大频率：
     * base + 2.5kHz * offset * (channel_count - 1)
     *
     * 为了避免小数，这里用 Hz 计算更清楚：
     * 433920000Hz + 2500Hz * offset * (count - 1)
     */
    max_freq_khz = RF_BASE_FREQ_KHZ +
                   ((uint32_t)cfg->fh_offset * 25UL * (cfg->channel_count - 1)) / 10UL;

    /*
     * 这里先按你的默认方案限制：
     * 433.920MHz ~ 443.820MHz
     */
    if (max_freq_khz > 443820UL)
    {
        return RF_ERROR;
    }

    return RF_OK;
}

/**
 * @brief CRC16校验位函数
 * 
 * @param data 
 * @param len 
 * @return uint16_t 
 */
uint16_t rf_crc16_calc(const uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    uint16_t i;
    uint8_t j;

    if (data == 0)
    {
        return 0;
    }

    for (i = 0; i < len; i++)
    {
        crc ^= data[i];

        for (j = 0; j < 8; j++)
        {
            if (crc & 0x0001)
            {
                crc = (crc >> 1) ^ 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }

    return crc;
}

