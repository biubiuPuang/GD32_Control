#include "app_tx-rx_config.h"

/*
 * 当前频率 = 433.920MHz + 2.5kHz * fh_offset * fh_channel
 */
static uint32_t rf_calc_freq_hz(uint8_t fh_offset, uint8_t fh_channel)
{
    return 433920000UL + ((uint32_t)fh_offset * 2500UL * (uint32_t)fh_channel);
}

/*
 * 获取当前 211发送芯片 和 221接收芯片 的真实寄存器配置
 */
void rf_get_tx_rx_real_freq(rf_tx_rx_freq_t *info)
{
    uint8_t tx_channel;
    uint8_t tx_offset;
    uint8_t rx_channel;
    uint8_t rx_offset;

    if (info == 0)
    {
        return;
    }

    /*
     * 直接从 211 发送芯片寄存器读取
     */
    tx_channel = cmt2119b_read_reg(RF_REG_FH_CHANNEL);
    tx_offset  = cmt2119b_read_reg(RF_REG_FH_OFFSET);

    /*
     * 直接从 221 接收芯片寄存器读取
     */
    rx_channel = cmt2219b_read_reg(RF_REG_FH_CHANNEL);
    rx_offset  = cmt2219b_read_reg(RF_REG_FH_OFFSET);

    info->tx.fh_channel = tx_channel;
    info->tx.fh_offset = tx_offset;
    info->tx.freq_hz = rf_calc_freq_hz(tx_offset, tx_channel);

    info->rx.fh_channel = rx_channel;
    info->rx.fh_offset = rx_offset;
    info->rx.freq_hz = rf_calc_freq_hz(rx_offset, rx_channel);
}

/*
 * 如果你想直接串口打印确认，就调用这个函数
 */
void rf_print_tx_rx_real_freq(void)
{
    
    rf_tx_rx_freq_t info;

    rf_get_tx_rx_real_freq(&info);

    printf("TX real reg data: fh_channel=%u(0x%02X), fh_offset=%u(0x%02X), freq=%lu Hz\r\n",
           info.tx.fh_channel,
           info.tx.fh_channel,
           info.tx.fh_offset,
           info.tx.fh_offset,
           (unsigned long)info.tx.freq_hz);

    printf("RX real reg data: fh_channel=%u(0x%02X), fh_offset=%u(0x%02X), freq=%lu Hz\r\n",
           info.rx.fh_channel,
           info.rx.fh_channel,
           info.rx.fh_offset,
           info.rx.fh_offset,
           (unsigned long)info.rx.freq_hz);
}


