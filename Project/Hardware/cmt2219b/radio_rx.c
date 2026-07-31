#include "radio_rx.h"
#include "cmt2219b.h"

// 手动快速调频相关 
#define RADIO_FH_STEP_100KHZ        40
#define RADIO_FH_DEFAULT_CHANNEL    0

uint8_t radio_rx_init(void)
{
    if (cmt2219b_init() != CMT2219B_OK)
    {
        return CMT2219B_ERROR;
    }

    if (cmt2219b_is_exist() != CMT2219B_OK)
    {
        return CMT2219B_ERROR;
    }

    // 此函数下面的这些部分都是手动快速调频相关 
    cmt2219b_go_stby();

    cmt2219b_set_frequency_step(RADIO_FH_STEP_100KHZ);
    cmt2219b_set_frequency_channel(RADIO_FH_DEFAULT_CHANNEL);

    /*
     * AFC 阈值先移植函数，不默认启用。
     * 后面如果接收端快速跳频不稳定，再打开这一句。
     */
    /* cmt2219b_set_afc_ovf_th(0x27); */

    cmt2219b_enable_read_fifo();
    cmt2219b_clear_interrupt_flags();
    cmt2219b_clear_rx_fifo();

    if (cmt2219b_go_rx() != CMT2219B_OK)
    {
        return CMT2219B_ERROR;
    }

    return CMT2219B_OK;
}

uint8_t radio_rx_poll_packet(uint8_t *buf, uint8_t len)
{
    if (buf == 0)
    {
        return CMT2219B_ERROR;
    }

    if (len == 0)
    {
        return CMT2219B_ERROR;
    }

    if (len > CMT2219B_MAX_FIFO_SIZE)
    {
        return CMT2219B_ERROR;
    }

    /*
     * 没收到包，直接返回 0。
     */
    if (!cmt2219b_packet_received())
    {
        return 0;
    }

    /*
     * 收到包后：
     * 1. 进入待机
     * 2. 读 FIFO
     * 3. 清 FIFO
     * 4. 清中断
     * 5. 重新进入 RX
     */
    cmt2219b_go_stby();

    cmt2219b_read_fifo(buf, len);

    cmt2219b_clear_rx_fifo();
    cmt2219b_clear_interrupt_flags();

    cmt2219b_go_rx();

    return CMT2219B_OK;
}

void radio_rx_set_channel(uint8_t channel)
{
    cmt2219b_go_stby();

    cmt2219b_set_frequency_channel(channel);

    cmt2219b_enable_read_fifo();
    cmt2219b_clear_interrupt_flags();
    cmt2219b_clear_rx_fifo();

    cmt2219b_go_rx();
}

void radio_rx_set_frequency_step(uint8_t step)
{
    cmt2219b_go_stby();

    cmt2219b_set_frequency_step(step);

    cmt2219b_enable_read_fifo();
    cmt2219b_clear_interrupt_flags();
    cmt2219b_clear_rx_fifo();

    cmt2219b_go_rx();
}

void radio_rx_set_afc_ovf_th(uint8_t afc_ovf_th)
{
    cmt2219b_go_stby();

    cmt2219b_set_afc_ovf_th(afc_ovf_th);

    cmt2219b_enable_read_fifo();
    cmt2219b_clear_interrupt_flags();
    cmt2219b_clear_rx_fifo();

    cmt2219b_go_rx();
}
