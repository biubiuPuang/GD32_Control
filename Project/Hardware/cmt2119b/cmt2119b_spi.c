#include "cmt2119b_spi.h"
#include "cmt2119b_port.h"



void cmt2119b_spi_init(void)
{
    // 参数含义：
    // SPI_MASTER                         GD32 是主机
    // SPI_TRANSMODE_BDTRANSMIT           单线半双工，初始为发送方向
    // SPI_FRAMESIZE_8BIT                 每次传输 8 位
    // SPI_NSS_SOFT                       不使用硬件 NSS
    // SPI_ENDIAN_MSB                     高位先发送
    // SPI_CK_PL_LOW_PH_1EDGE             Mode 0：低电平空闲，上升沿采样
    // SPI_PSC_16                         72MHz / 16 = 4.5MHz
    spi_parameter_struct spi_init_struct;

    cmt2119b_port_init();

    spi_i2s_deinit(CMT2119B_SPI_PERIPH);
    spi_struct_para_init(&spi_init_struct);

    spi_init_struct.device_mode = SPI_MASTER;
    spi_init_struct.trans_mode = SPI_TRANSMODE_BDTRANSMIT;
    spi_init_struct.frame_size = SPI_FRAMESIZE_8BIT;
    spi_init_struct.nss = SPI_NSS_SOFT;
    spi_init_struct.endian = SPI_ENDIAN_MSB;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;
    spi_init_struct.prescale = SPI_PSC_16;

    spi_init(CMT2119B_SPI_PERIPH, &spi_init_struct);
    spi_enable(CMT2119B_SPI_PERIPH);

    cmt2119b_csb_high();
    cmt2119b_fcsb_high();
}



static void cmt2119b_spi_send_byte(uint8_t data)
{
    while (spi_i2s_flag_get(CMT2119B_SPI_PERIPH, SPI_FLAG_TBE) == RESET) {
    }

    spi_i2s_data_transmit(CMT2119B_SPI_PERIPH, data);

    while (spi_i2s_flag_get(CMT2119B_SPI_PERIPH, SPI_FLAG_TRANS) != RESET) {
    }
}

/*
 * GPIO 发送一个字节。
 * 仅用于 CMT2119B 读寄存器时发送读地址。
 * 函数结束时，SCLK 停在高电平。
 */
static void cmt2119b_gpio_send_byte(uint8_t data)
{
    uint8_t i;

    for (i = 0U; i < 8U; i++) {
        cmt2119b_sclk_low();

        if ((data & 0x80U) != 0U) {
            cmt2119b_sdio_high();
        } else {
            cmt2119b_sdio_low();
        }

        cmt2119b_delay_us(CMT2119B_READ_SCLK_HALF_PERIOD_US);

        /* CMT2119B 在上升沿采样 */
        cmt2119b_sclk_high();

        cmt2119b_delay_us(CMT2119B_READ_SCLK_HALF_PERIOD_US);

        data <<= 1U;
    }
}

/*
 * GPIO 接收一个字节。
 * 进入函数前，SDIO 必须已变成输入，且 SCLK 已处于低电平。
 */
static uint8_t cmt2119b_gpio_recv_byte(void)
{
    uint8_t i;
    uint8_t data = 0U;

    for (i = 0U; i < 8U; i++) {
        cmt2119b_sclk_low();

        /*
         * SCLK 下降沿后，CMT2119B 准备下一位数据。
         */
        cmt2119b_delay_us(CMT2119B_READ_SCLK_HALF_PERIOD_US);

        cmt2119b_sclk_high();

        cmt2119b_delay_us(CMT2119B_READ_SCLK_HALF_PERIOD_US);

        data <<= 1U;

        if (cmt2119b_sdio_read() != 0U) {
            data |= 0x01U;
        }
    }

    /*
     * 最后一位上升沿采样完成后，
     * 生成最终下降沿，使 SCLK 回到 Mode 0 的空闲低电平。
     */
    cmt2119b_sclk_low();

    return data;
}

void cmt2119b_spi_write_reg(uint8_t addr, uint8_t dat)
{
    cmt2119b_fcsb_high();
    cmt2119b_csb_low();

    cmt2119b_delay_us(1);

    cmt2119b_spi_send_byte(addr & 0x7FU);
    cmt2119b_spi_send_byte(dat);

    cmt2119b_delay_us(1);

    cmt2119b_csb_high();
}

uint8_t cmt2119b_spi_read_reg(uint8_t addr)
{
    uint8_t data;
    uint32_t primask;

    /*
     * PA5/PA7 即将从 SPI0 复用切成 GPIO。
     * 短暂关闭普通中断，避免中断拉长时序或其他代码抢占该总线。
     */
    primask = __get_PRIMASK();
    __disable_irq();

    /*
     * 正常情况下，前一次硬件 SPI 发送已等待 TRANS 清零。
     * 这里再次确认 SPI0 不在传输。
     */
    while (spi_i2s_flag_get(CMT2119B_SPI_PERIPH, SPI_FLAG_TRANS) != RESET) {
    }

    /*
     * 停止 SPI0，避免 SPI0 与 GPIO 同时驱动 PA5、PA7。
     */
    spi_disable(CMT2119B_SPI_PERIPH);

    /*
     * PA5/PA7 临时由 GPIO 接管。
     */
    cmt2119b_spi_pins_to_gpio();

    cmt2119b_fcsb_high();
    cmt2119b_csb_low();

    /*
     * 满足 CSB 拉低到第一个 SCLK 上升沿的建立时间。
     */
    cmt2119b_delay_us(CMT2119B_READ_CSB_SETUP_US);

    cmt2119b_sdio_output();
    cmt2119b_sclk_low();

    /*
     * bit7=1：读寄存器命令。
     * 此函数结束时 SCLK 为高，最后一个地址位已在上升沿被芯片采样。
     */
    cmt2119b_gpio_send_byte(addr | 0x80U);

    /*
     * 关键顺序，绝对不能调换：
     *
     * 1. PA7 先切输入，释放给 CMT2119B；
     * 2. SCLK 保持高电平等待 1us；
     * 3. 再产生地址字节的最后一个下降沿。
     *
     * 这样满足 CMT2119B 的 SDIO 转向要求。
     */
    cmt2119b_sdio_input();
    cmt2119b_delay_us(CMT2119B_READ_SCLK_HALF_PERIOD_US);
    cmt2119b_sclk_low();

    /*
     * GPIO 生成严格 8 个读时钟，读取芯片返回的数据。
     */
    data = cmt2119b_gpio_recv_byte();

    /*
     * 最后一个下降沿之后保持 CSB 低电平。
     */
    cmt2119b_delay_us(CMT2119B_READ_CSB_HOLD_US);

    cmt2119b_csb_high();
    cmt2119b_fcsb_high();

    /*
     * 恢复 PA5/PA7 到 SPI0 复用功能，继续给硬件 SPI 写操作使用。
     */
    cmt2119b_spi_pins_to_spi0();
    spi_enable(CMT2119B_SPI_PERIPH);

    /*
     * 恢复进入本函数前的中断屏蔽状态。
     * 不可用 __enable_irq() 直接替换，否则可能误打开原本已关闭的中断。
     */
    __set_PRIMASK(primask);

    return data;
}

void cmt2119b_spi_write_fifo(const uint8_t *buf, uint16_t len)
{
     uint16_t i;

    cmt2119b_csb_high();

    for (i = 0; i < len; i++) {
        cmt2119b_fcsb_low();

        /* FCSB low to first SCLK rising edge: >= 1 SCLK cycle */
        cmt2119b_delay_us(1);

        cmt2119b_spi_send_byte(buf[i]);

        /* Last SCLK falling edge to FCSB high: >= 2us */
        cmt2119b_delay_us(2);

        cmt2119b_fcsb_high();

        /* FCSB high before next FIFO byte: >= 4us */
        cmt2119b_delay_us(4);
    }
}


