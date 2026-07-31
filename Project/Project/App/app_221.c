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
        printf("RX init OK\r\n");
        
        printf("PA15 hight\r\n");
        // PA15 输出高电平
        gpio_bit_set(GPIOA, GPIO_PIN_15);    
    }
    else
    {
        printf("RX init ERROR\r\n");
    }
}

// 221接收数据
void app_221_receive_data(void)
{
    /*
     * 这里判断的是接收芯片内部 INT_FLAG 寄存器。
     * 0x6D 是 INT_FLAG，bit0 = 1 表示接收芯片收到完整数据包。
     * 所以下面打印出来的数据，是从 PH2219BBA 接收芯片 FIFO 里面读出来的。
     */
    if (rx_ok && (cmt2219b_read_reg(0x6D) & 0x01))
    {
        cmt2219b_go_stby();

        /*
         * 从接收芯片 FIFO 读取数据，不是直接打印 tx_buf。
         */
        cmt2219b_read_fifo(rx_buf, TEST_PACKET_LEN);

        // 清空 CMT2219B 的接收 FIFO
        cmt2219b_clear_rx_fifo();
        // 清楚CMT2219B的中断标志位
        cmt2219b_clear_interrupt_flags();
        // 让 CMT2219B 重新进入接收模式 RX。
        cmt2219b_go_rx();

        rx_count++;

        printf("RX chip FIFO data, count=%u: ", rx_count);
        print_buf(rx_buf, TEST_PACKET_LEN);
    }
    else
    {
        printf("RX chip no packet\r\n");
    }
}
