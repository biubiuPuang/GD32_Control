#include "cmt2219b_spi.h"
#include "cmt2219b_port.h"

static void cmt2219b_spi_delay(void)
{
    volatile uint32_t n = 20U;

    while (n--) {
    }
}

static void cmt2219b_spi_delay_us(void)
{
    cmt2219b_delay_us(1U);
}

/*
 * 初始化 CMT2219B 使用的 SPI1。
 *
 * 参数完全按照 CMT2119B 的 SPI0 配置：
 * SPI 主机
 * 单线双向，初始为发送
 * 8 bit
 * 软件 NSS
 * MSB first
 * SPI Mode 0
 * 72 MHz / 16 = 4.5 MHz
 */
void cmt2219b_spi_init(void)
{
    spi_parameter_struct spi_init_struct;

    cmt2219b_port_init();

    spi_i2s_deinit(CMT2219B_SPI_PERIPH);
    spi_struct_para_init(&spi_init_struct);

    spi_init_struct.device_mode = SPI_MASTER;
    spi_init_struct.trans_mode = SPI_TRANSMODE_BDTRANSMIT;
    spi_init_struct.frame_size = SPI_FRAMESIZE_8BIT;
    spi_init_struct.nss = SPI_NSS_SOFT;
    spi_init_struct.endian = SPI_ENDIAN_MSB;
    spi_init_struct.clock_polarity_phase =
        SPI_CK_PL_LOW_PH_1EDGE;
    spi_init_struct.prescale = SPI_PSC_16;

    spi_init(CMT2219B_SPI_PERIPH, &spi_init_struct);

    /*
    * SPI1 带独立 FIFO。
    * CMT2219B 的寄存器读写单位为 8 bit，
    * 所以 SPI1 的 SPI_DATA / FIFO 必须采用字节访问。
    */
    spi_fifo_access_size_config(CMT2219B_SPI_PERIPH,
                                SPI_BYTE_ACCESS);

    spi_enable(CMT2219B_SPI_PERIPH);                       

    cmt2219b_csb_high();
    cmt2219b_fcsb_high();
}

/*
 * 使用 SPI1 硬件发送一个字节。
 */
static void cmt2219b_spi_send_byte(uint8_t data)
{
    /*
     * 等待发送缓冲区为空。
     */
    while (spi_i2s_flag_get(CMT2219B_SPI_PERIPH,
                             SPI_FLAG_TBE) == RESET) {
    }

    spi_i2s_data_transmit(CMT2219B_SPI_PERIPH, data);

    /*
     * 等待当前字节真正发送完成。
     */
    while (spi_i2s_flag_get(CMT2219B_SPI_PERIPH,
                             SPI_FLAG_TRANS) != RESET) {
    }
}

/*
 * GPIO 模拟发送一个字节。
 *
 * 该函数只用于读取事务中的“读地址”发送。
 */
static void cmt2219b_gpio_send_byte(uint8_t data)
{
    uint8_t i;

    for (i = 0U; i < 8U; i++) {
        cmt2219b_clk_low();

        if ((data & 0x80U) != 0U) {
            cmt2219b_sdio_high();
        } else {
            cmt2219b_sdio_low();
        }

        cmt2219b_spi_delay();

        cmt2219b_clk_high();

        cmt2219b_spi_delay();

        data <<= 1U;
    }
}

/*
 * GPIO 模拟接收一个字节。
 *
 * 函数进入时：
 * CLK 为低电平；
 * SDIO 已经切换为输入。
 */
static uint8_t cmt2219b_gpio_recv_byte(void)
{
    uint8_t i;
    uint8_t data = 0U;

    for (i = 0U; i < 8U; i++) {
        cmt2219b_clk_low();
        cmt2219b_spi_delay();

        cmt2219b_clk_high();

        data <<= 1U;

        if (cmt2219b_sdio_read() != 0U) {
            data |= 0x01U;
        }

        cmt2219b_spi_delay();
    }

    /*
     * 最后一个数据位采样完成后，
     * 将 CLK 拉回低电平。
     */
    cmt2219b_clk_low();

    return data;
}

/*
 * 写 CMT2219B 寄存器。
 *
 * 采用 SPI1 硬件发送：
 * CSB 低
 * 发送写地址
 * 发送数据
 * CSB 高
 */
void cmt2219b_spi_write_reg(uint8_t addr, uint8_t dat)
{
    cmt2219b_fcsb_high();
    cmt2219b_csb_low();

    /*
     * CSB 拉低后等待一段建立时间。
     */
    cmt2219b_delay_us(1U);

    /*
     * bit7 = 0 表示写寄存器。
     */
    cmt2219b_spi_send_byte(addr & 0x7FU);
    cmt2219b_spi_send_byte(dat);

    /*
     * 最后一个 SCLK 结束后再释放 CSB。
     */
    cmt2219b_delay_us(1U);
    cmt2219b_csb_high();
}

/*
 * 读 CMT2219B 寄存器。
 *
 * 由于 CMT2219B 要求 SDIO 在地址和返回数据之间进行精确换向，
 * 这里保留 GPIO 模拟读操作。
 */
uint8_t cmt2219b_spi_read_reg(uint8_t addr)
{
    uint8_t dat;
    uint32_t primask;

    /*
     * 保存中断状态，并暂时关闭中断。
     *
     * 读取期间必须保证 GPIO 模拟时序不被中断打断。
     */
    primask = __get_PRIMASK();
    __disable_irq();

    /*
     * 确保前一笔 SPI1 硬件发送已经结束。
     */
    while (spi_i2s_flag_get(CMT2219B_SPI_PERIPH,
                             SPI_FLAG_TRANS) != RESET) {
    }

    /*
     * 停止 SPI1，准备由 GPIO 接管 PB13/PB15。
     */
    spi_disable(CMT2219B_SPI_PERIPH);
    cmt2219b_spi_pins_to_gpio();

    cmt2219b_fcsb_high();
    cmt2219b_csb_low();

    cmt2219b_delay_us(1U);

    /*
     * 发送读地址。
     * bit7 = 1 表示读寄存器。
     */
    cmt2219b_sdio_output();
    cmt2219b_clk_low();
    cmt2219b_gpio_send_byte(addr | 0x80U);

    /*
     * 地址发送完毕后，必须在下一个下降沿之前
     * 将 SDIO 切换为输入，释放 SDIO 总线。
     */
    cmt2219b_sdio_input();

    /*
     * 读取 CMT2219B 返回的 8 bit 数据。
     */
    dat = cmt2219b_gpio_recv_byte();

    /*
     * 保持 CSB 低电平一段时间，然后释放。
     */
    cmt2219b_delay_us(1U);

    cmt2219b_csb_high();
    cmt2219b_fcsb_high();

    /*
     * 恢复 PB13/PB15 的 SPI1 复用功能。
     */
    cmt2219b_spi_pins_to_spi1();
    spi_enable(CMT2219B_SPI_PERIPH);

    /*
     * 恢复进入本函数之前的中断状态。
     */
    __set_PRIMASK(primask);

    return dat;
}

/*
 * 使用 SPI1 硬件发送 FIFO。
 *
 * CMT2219B FIFO 写入要求：
 * CSB 保持高电平；
 * 每个字节单独控制一次 FCSB。
 */
void cmt2219b_spi_write_fifo(const uint8_t *buf, uint16_t len)
{
    uint16_t i;

    cmt2219b_csb_high();
    cmt2219b_fcsb_high();

    for (i = 0U; i < len; i++) {
        /*
         * FCSB 拉低到第一个 SCLK 上升沿前，
         * 至少保持一个 SCLK 周期。
         */
        cmt2219b_fcsb_low();
        cmt2219b_delay_us(1U);

        cmt2219b_spi_send_byte(buf[i]);

        /*
         * 最后一个 SCLK 下降沿后，
         * FCSB 至少保持低电平 2 us。
         */
        cmt2219b_delay_us(2U);
        cmt2219b_fcsb_high();

        /*
         * 两个连续 FIFO 字节之间，
         * FCSB 至少保持高电平 4 us。
         */
        cmt2219b_delay_us(4U);
    }
}

/*
 * GPIO 模拟读取 FIFO。
 *
 * 读取 FIFO 时不能使用 SPI1 的连续硬件接收，
 * 因为 CMT2219B 要求每个 FIFO 字节单独控制 FCSB，
 * 并且对字节之间的时间有明确要求。
 */
void cmt2219b_spi_read_fifo(uint8_t *buf, uint16_t len)
{
    uint16_t i;
    uint32_t primask;

    if ((buf == 0) || (len == 0U)) {
        return;
    }

    primask = __get_PRIMASK();
    __disable_irq();

    /*
     * 确保前一笔 SPI1 硬件发送已经完成。
     */
    while (spi_i2s_flag_get(CMT2219B_SPI_PERIPH,
                             SPI_FLAG_TRANS) != RESET) {
    }

    /*
     * SPI1 暂停，由 GPIO 接管 PB13/PB15。
     */
    spi_disable(CMT2219B_SPI_PERIPH);
    cmt2219b_spi_pins_to_gpio();

    cmt2219b_csb_high();
    cmt2219b_fcsb_high();
    cmt2219b_clk_low();

    /*
     * FIFO 读取时，SDIO 直接作为输入。
     */
    cmt2219b_sdio_input();

    for (i = 0U; i < len; i++) {
        /*
         * 每个 FIFO 字节单独拉低 FCSB。
         */
        cmt2219b_fcsb_low();

        /*
         * FCSB 低到第一个 SCLK 上升沿前，
         * 至少保持一个 SCLK 周期。
         */
        cmt2219b_delay_us(1U);

        buf[i] = cmt2219b_gpio_recv_byte();

        /*
         * 最后一个 SCLK 下降沿后，
         * FCSB 继续保持低电平至少 2 us。
         */
        cmt2219b_delay_us(2U);

        cmt2219b_fcsb_high();

        /*
         * 两次 FIFO 读取之间，
         * FCSB 至少保持高电平 4 us。
         */
        cmt2219b_delay_us(4U);
    }

    cmt2219b_fcsb_high();
    cmt2219b_csb_high();

    /*
     * 恢复 SPI1 硬件功能。
     */
    cmt2219b_spi_pins_to_spi1();
    spi_enable(CMT2219B_SPI_PERIPH);

    __set_PRIMASK(primask);
}
