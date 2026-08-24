#include "radio_tx.h"
#include "cmt2119b.h"

#define RADIO_TX_TIMEOUT_MS 1000

// 手动快速调频相关
#define RADIO_FH_STEP_100KHZ 40
#define RADIO_FH_DEFAULT_CHANNEL 0

// 有添加跟手动快速调频相关代码
uint8_t radio_tx_init(void)
{
    if (cmt2119b_init() != CMT2119B_OK)
    {
        return CMT2119B_ERROR;
    }

    cmt2119b_go_stby();
    cmt2119b_set_frequency_step(RADIO_FH_STEP_100KHZ);
    cmt2119b_set_frequency_channel(RADIO_FH_DEFAULT_CHANNEL);
    cmt2119b_go_sleep();

    return CMT2119B_OK;
}

uint8_t radio_tx_send(const uint8_t *buf, uint8_t len)
{
    return cmt2119b_send_packet(buf, len, RADIO_TX_TIMEOUT_MS);
}


// 以下函数都是跟手动快速调频相关 
void radio_tx_set_channel(uint8_t channel)
{
    cmt2119b_go_stby();
    cmt2119b_set_frequency_channel(channel);
    cmt2119b_go_sleep();
}

void radio_tx_set_frequency_step(uint8_t step)
{
    cmt2119b_go_stby();
    cmt2119b_set_frequency_step(step);
    cmt2119b_go_sleep();
}