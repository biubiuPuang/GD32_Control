#include "app_221.h"

// LED灯头文件
#include "gd32e23x.h"

extern void print_buf(uint8_t *buf, uint8_t len);

// 数据接收长度 固定32字节
#define TEST_PACKET_LEN 32
// 数据接收数组
static uint8_t rx_buf[TEST_PACKET_LEN];
// 接收数据芯片初始化标志位
static uint8_t rx_ok;
static uint32_t rx_count = 0;

// 翻转LED灯的标志位
static uint8_t cmd_44_handled = 0;

// 221芯片初始化
void app_221_init(void)
{
    // 时钟配置
    systick_config();
    // 串口配置
    usart_gpio_config(115200);

    // 判断221初始化是否成功
    rx_ok = radio_rx_init();
    if (rx_ok)
    {
        debug_printf("RX init OK\r\n");

        debug_printf("PA15 hight\r\n");
        // PA15 输出高电平
        gpio_bit_set(GPIOA, GPIO_PIN_15);
    }
    else
    {
        debug_printf("RX init ERROR\r\n");
    }
}

// 根据按键内容进行判断
void key_press_handle(void)
{
    // 对右按键逻辑进行判断
    switch (rx_buf[3])
    {
    case 0x10:
        // 右单按键按下
        gpio_bit_set(GPIOA, GPIO_PIN_15);
        break;
    case 0x11:
        // 右双按键按下
        gpio_bit_reset(GPIOA, GPIO_PIN_15);
        break;
    }

    // 对下按键逻辑进行判断
    switch (rx_buf[4])
    {
    case 0x20:
        // 下单按键按下
        gpio_bit_set(GPIOA, GPIO_PIN_15);
        break;
    case 0x21:
        // 下双按键按下
        gpio_bit_reset(GPIOA, GPIO_PIN_15);
        break;
    }

    // 对左按键逻辑进行判断
    switch (rx_buf[5])
    {
    case 0x30:
        // 左单按键按下
        gpio_bit_set(GPIOA, GPIO_PIN_15);
        break;
    case 0x31:
        // 左双按键按下
        gpio_bit_reset(GPIOA, GPIO_PIN_15);
        break;
    }

     // 对上按键逻辑进行判断
    switch (rx_buf[6])
    {
    case 0x40:
        // 左单按键按下
        gpio_bit_set(GPIOA, GPIO_PIN_15);
        break;
    case 0x41:
        // 左双按键按下
        gpio_bit_reset(GPIOA, GPIO_PIN_15);
        break;
    }
}

// 221接收数据
void app_221_receive_data(void)
{
    /*
     * 这里判断的是接收芯片内部 INT_FLAG 寄存器
     * 0x6D 是 INT_FLAG，bit0 = 1 表示接收芯片收到完整数据包
     * 所以下面打印出来的数据，是从 PH2219BBA 接收芯片 FIFO 里面读出来的
     */
    if (rx_ok && cmt2219b_packet_received())
    {
        cmt2219b_go_stby();

        /*
         * 从接收芯片 FIFO 读取数据，不是直接打印 tx_buf
         */
        cmt2219b_read_fifo(rx_buf, TEST_PACKET_LEN);

        // 处理按键值的内容
        key_press_handle();

        // 清空 CMT2219B 的接收 FIFO
        cmt2219b_clear_rx_fifo();
        // 清除 CMT2219B 的中断标志位
        cmt2219b_clear_interrupt_flags();
        // 让 CMT2219B 重新进入接收模式 RX
        if (cmt2219b_go_rx() != CMT2219B_OK)
        {
            debug_printf("go_rx FAIL\r\n");
        }

        rx_count++;

        // 串口打印接收芯片 FIFO 数据
        debug_printf("RX chip FIFO data: ");
        print_buf(rx_buf, TEST_PACKET_LEN);

        // debug_printf("RX chip FIFO data, count=%u: ", rx_count);
        // print_buf(rx_buf, TEST_PACKET_LEN);
    }
    else
    {
        debug_printf("RX no pkt, FLAG=0x%02X, STA=0x%02X\r\n",
                     cmt2219b_read_reg(0x6D),  // 中断标志位: bit0=接收完成, bit1=CRC通过
                     cmt2219b_read_reg(0x61)); // 芯片状态: 0x05=接收模式, 0x02=待机模式
    }
}
