#include "app_221.h"

// LED�?头文�?
#include "gd32e23x.h"


extern void print_buf(uint8_t *buf, uint8_t len);

// 数据接收长度 固定32字节
#define TEST_PACKET_LEN 32
// 数据接收数组
static uint8_t rx_buf[TEST_PACKET_LEN];
// 接收数据�?片初始化标志�?
static uint8_t rx_ok;
static uint32_t rx_count = 0;

// 翻转LED�?的标志位
static uint8_t cmd_44_handled = 0;

// 221�?片初始化
void app_221_init(void)
{
    // 时钟配置
    systick_config();
    // 串口配置
    usart_gpio_config(115200);

    // 判断221初�?�化�?否成�?
    rx_ok = radio_rx_init();
    if (rx_ok)
    {
        debug_printf("RX init OK\r\n");

        debug_printf("PA15 hight\r\n");
        // PA15 输出高电�?
        gpio_bit_set(GPIOA, GPIO_PIN_15);
    }
    else
    {
        debug_printf("RX init ERROR\r\n");
    }
}

// 根据按键内�?�进行判�?
void key_press_handle(void)
{
    switch (rx_buf[3])
    {
    case 0x10:
        // 单按�?按下
        gpio_bit_set(GPIOA, GPIO_PIN_15);
        break;
    case 0x11:
        // 双按�?按下
        gpio_bit_reset(GPIOA, GPIO_PIN_15);
        break;
    }
}

// 221接收数据
void app_221_receive_data(void)
{
    /*
     * 这里判断的是接收�?片内�? INT_FLAG 寄存器�?
     * 0x6D �? INT_FLAG，bit0 = 1 表示接收�?片收到完整数�?包�?
     * 所以下面打印出来的数据，是�? PH2219BBA 接收�?�? FIFO 里面读出来的�?
     */
    if (rx_ok && cmt2219b_packet_received())
    {
        cmt2219b_go_stby();

        /*
         * 从接收芯�? FIFO 读取数据，不�?直接打印 tx_buf�?
         */
        cmt2219b_read_fifo(rx_buf, TEST_PACKET_LEN);

        // 处理按键值的内�??
        key_press_handle();

        // 清空 CMT2219B 的接�? FIFO
        cmt2219b_clear_rx_fifo();
        // 清�?�CMT2219B的中�?标志�?
        cmt2219b_clear_interrupt_flags();
        // �? CMT2219B 重新进入接收模式 RX�?
        cmt2219b_go_rx();

        rx_count++;

        // 串口打印接收�?�? FIFO 数据
        debug_printf("RX chip FIFO data: ");
        print_buf(rx_buf, TEST_PACKET_LEN);

        // debug_printf("RX chip FIFO data, count=%u: ", rx_count);
        // print_buf(rx_buf, TEST_PACKET_LEN);
    }
    else
    {
        debug_printf("RX chip no packet\r\n");
    }
}
