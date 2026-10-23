#include "packet_loss_test.h"
#include "radio_tx.h"      /* 发送底层接口 radio_tx_send() */
#include "radio_rx.h"      /* 接收底层接口 radio_rx_poll_packet() */
#include "systick.h"       /* delay_us() */
#include "Debug_printf.h"  /* debug_printf() 串口打印 */

/* 包长：必须和 RFPDK 的 Payload Length 一致（当前 32 字节） */
#define TEST_PACKET_LEN   32
/* 测试包数：1500 包 ≈ 60 秒（每包约 40ms：37.5ms 空中 + 处理） */
#define TEST_PACKET_TOTAL 1500

/* ===== 模块内部状态 ===== */
static uint8_t  s_tx_buf[TEST_PACKET_LEN];  /* 发送缓冲 */
static uint8_t  s_rx_buf[TEST_PACKET_LEN];  /* 接收缓冲 */
static uint16_t s_tx_seq;                   /* 发送序号 0~65535，本次不回绕 */
static uint32_t s_tx_count;                 /* 发送计数 */
static uint32_t s_rx_count;                 /* 接收计数 */
static uint32_t s_lost_by_gap;              /* 按序号跳变统计的丢包数（参考） */
static uint16_t s_last_seq;                 /* 上一个收到的序号 */

/* 复位所有统计状态，跑测试前先调用 */
static void packet_loss_test_reset(void)
{
    uint8_t i;

    for (i = 0; i < TEST_PACKET_LEN; i++)
    {
        s_tx_buf[i] = (uint8_t)i;   /* 填充固定数据，前 2 字节会被序号覆盖 */
    }

    s_tx_seq      = 0;
    s_tx_count    = 0;
    s_rx_count    = 0;
    s_lost_by_gap = 0;
    s_last_seq    = 0xFFFF;         /* 0xFFFF 表示"还没收到过" */
}

/* 发一包：序号放前 2 字节，阻塞到 TX_DONE（约 37.5ms） */
static void packet_loss_test_send(void)
{
    s_tx_buf[0] = (uint8_t)(s_tx_seq & 0xFF);   /* 序号低字节 */
    s_tx_buf[1] = (uint8_t)(s_tx_seq >> 8);     /* 序号高字节 */

    radio_tx_send(s_tx_buf, TEST_PACKET_LEN);

    s_tx_seq++;
    s_tx_count++;
}

/* 收一次包（有包才处理），并累计丢包数 */
static void packet_loss_test_poll_rx(void)
{
    uint16_t seq;

    if (radio_rx_poll_packet(s_rx_buf, TEST_PACKET_LEN))
    {
        /* 还原序号：低字节在前，和发送端一致 */
        seq = (uint16_t)(((uint16_t)s_rx_buf[1] << 8) | s_rx_buf[0]);

        /* 序号跳变 => 中间丢了 seq - s_last_seq - 1 个包 */
        if (s_last_seq != 0xFFFF && seq > s_last_seq + 1)
        {
            s_lost_by_gap += (uint32_t)(seq - s_last_seq - 1);
        }
        s_last_seq = seq;
        s_rx_count++;
    }
}

/**
 * @brief 丢包率测试主流程（对上层暴露的唯一函数）
 */
void packet_loss_test_run(void)
{
    uint32_t lost;

    packet_loss_test_reset();

    debug_printf("==== 丢包率测试开始 ====\r\n");

    /* 背靠背：发一包、收一包，跑满 TEST_PACKET_TOTAL 包 */
    while (s_tx_count < TEST_PACKET_TOTAL)
    {
        packet_loss_test_send();
        delay_us(200);            /* 给接收芯片留 CRC 校验 + 置 PKT_DONE 的时间 */
        packet_loss_test_poll_rx();
    }

    /* 循环是"先发后收"，最后一包发出后还没收，这里补收一次 */
    packet_loss_test_poll_rx();

    /* 丢包数：主结果用"发送 - 接收"，序号法作参考 */
    lost = s_tx_count - s_rx_count;

    debug_printf("==== 丢包率测试结果 ====\r\n");
    debug_printf("发送包数: %u\r\n", (unsigned)s_tx_count);
    debug_printf("接收包数: %u\r\n", (unsigned)s_rx_count);
    debug_printf("丢包数:   %u\r\n", (unsigned)lost);
    debug_printf("丢包率:   %u.%02u%%\r\n",
                 (unsigned)(lost * 100 / s_tx_count),
                 (unsigned)(lost * 10000 / s_tx_count % 100));
    debug_printf("序号法丢包数: %u\r\n", (unsigned)s_lost_by_gap);
    debug_printf("理论发包速度: ~25 包/秒 (9.6kbps)\r\n");
}
