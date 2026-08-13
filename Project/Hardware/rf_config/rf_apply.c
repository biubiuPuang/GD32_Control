#include "rf_apply.h"
#include "Debug_printf.h"

/**
 * @brief 应用配置到无线芯片
 * 
 * @param cfg 
 * @return uint8_t 
 */
uint8_t rf_config_apply(const rf_factory_config_t *cfg)
{
    if (rf_config_check(cfg) != RF_OK)
    {
        debug_printf("Check and Config ERROR\r\n");
        return RF_ERROR;
    }

    if (cfg->role == RF_ROLE_TX)
    {
        radio_tx_set_frequency_step(cfg->fh_offset);
        radio_tx_set_channel(cfg->fh_channel);
        debug_printf("Check and Config TX APPLY OK\r\n");
        return RF_OK;
    }

    if (cfg->role == RF_ROLE_RX)
    {
        radio_rx_set_frequency_step(cfg->fh_offset);
        radio_rx_set_channel(cfg->fh_channel);
        debug_printf("Check and Config RX APPLY OK\r\n");
        return RF_OK;
    }

    return RF_ERROR;
}


/**
 * @brief 测试函数 通过此函数假装串口发送配置参数
 * 
 */
void rf_test_apply_config(void)
{
    rf_factory_config_t cfg;

    cfg.magic = RF_CONFIG_MAGIC;
    cfg.version = RF_CONFIG_VERSION;
    cfg.pair_id = 17;

    cfg.fh_offset = 40;
    cfg.fh_channel = 16;
    cfg.channel_count = 100;

    cfg.role = RF_ROLE_TX;
    cfg.crc16 = 0;
    cfg.crc16 = rf_crc16_calc((uint8_t *)&cfg, sizeof(cfg));

    if (rf_config_apply(&cfg) != RF_OK)
    {
        debug_printf("RF TX APPLY ERROR\r\n");
        return;
    }

    cfg.role = RF_ROLE_RX;
    cfg.crc16 = 0;
    cfg.crc16 = rf_crc16_calc((uint8_t *)&cfg, sizeof(cfg));

    if (rf_config_apply(&cfg) != RF_OK)
    {
        debug_printf("USART RF tx and rx APPLY ERROR\r\n");
        return;
    }

    debug_printf("USART RF TX/RX APPLY OK\r\n");
}

