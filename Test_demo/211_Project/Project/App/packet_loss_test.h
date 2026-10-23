#ifndef __PACKET_LOSS_TEST_H
#define __PACKET_LOSS_TEST_H

#include <stdint.h>

/**
 * @brief 丢包率测试
 *        固定发送 TEST_PACKET_TOTAL 包（约 60 秒），统计收发/丢包并串口打印。
 *        调用前需先完成 radio_tx_init() 和 radio_rx_init()
 *        （也就是 main 里先执行 app_211_init() / app_221_init()）。
 */
void packet_loss_test_run(void);

#endif
