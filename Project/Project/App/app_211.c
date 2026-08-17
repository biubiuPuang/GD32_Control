#include "app_211.h"

// 串口�?�?打印输出开�?
#define RF_LOOP_LOG_ENABLE 0

static uint8_t tx_ok;
static uint8_t send_ret;
static uint32_t tx_count = 0;

/**
 * @brief 测试数据,串口如果打印11 22 33 则�?�明�?初�?�化测试数据
 *
 */
volatile uint8_t tx_buf_data[TEST_PACKET_LEN] = {
    0x11,
    0x22,
    0x33,
    0x44,
    0x55,
    0x66,
    0x77,
    0x88,
    0x99,
    0xAA,
    0xBB,
    0xCC,
    0xDD,
    0xEE,
    0xFF,
    0x00,
    0x01,
    0x02,
    0x03,
    0x04,
    0x05,
    0x06,
    0x07,
    0x08,
    0x09,
    0x0A,
    0x0B,
    0x0C,
    0x0D,
    0x0E,
    0x0F,
    0x10,
};

/**
 * @brief 获取GD32E230的UID
 *
 * @param uid 固定长度3�?字节
 */
void gd32_get_uid(uint32_t uid[3])
{
    volatile const uint32_t *p = (volatile const uint32_t *)GD32_UID_BASE;

    uid[0] = p[0]; // 0x1FFF F7AC
    uid[1] = p[1]; // 0x1FFF F7B0
    uid[2] = p[2]; // 0x1FFF F7B4
}

/**
 * @brief 串口打印16进制数据
 *
 * @param buf 打印输出数组
 * @param len 数组长度
 */
void print_buf(uint8_t *buf, uint8_t len)
{
    uint8_t i;

    for (i = 0; i < len; i++)
    {
        debug_printf("%02X ", buf[i]);
    }

    debug_printf("\r\n");
}

/**
 * @brief 211发送芯片初始化
 *
 */
void app_211_init(void)
{
    // 时钟配置
    systick_config();
    // 串口配置
    usart_gpio_config(115200);

    // 判断211发送芯片是否成�?
    tx_ok = radio_tx_init();
    if (tx_ok)
    {
        debug_printf("TX init OK\r\n");
    }
    else
    {
        debug_printf("TX init ERROR\r\n");
    }
}

/**
 * @brief 211串口发送循�?打印数据
 *
 */

void app_211_send_test_data(void)
{
    debug_printf("TX User data: ");
    print_buf(tx_buf_data, TEST_PACKET_LEN);

    if (tx_ok)
    {
        send_ret = radio_tx_send(tx_buf_data, TEST_PACKET_LEN);
        tx_count++;

        if (send_ret)
        {
            debug_printf("\r\nTX OK, count=%u\r\n", tx_count);
        }
        else
        {
            /*
             * 发送失败仍然打印，方便发现真�?�的发送异常�?
             */
            debug_printf("\r\nTX ERROR, count=%u\r\n", tx_count);
        }
    }
}
