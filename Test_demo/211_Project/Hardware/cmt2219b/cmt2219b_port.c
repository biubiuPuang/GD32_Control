#include "cmt2219b_port.h"
#include "systick.h"

void cmt2219b_port_init(void)
{
    rcu_periph_clock_enable(CMT2219B_FCSB_RCU);
    rcu_periph_clock_enable(CMT2219B_CSB_RCU);
    rcu_periph_clock_enable(CMT2219B_CLK_RCU);
    rcu_periph_clock_enable(CMT2219B_SDIO_RCU);
    rcu_periph_clock_enable(CMT2219B_GPIO3_RCU);
    rcu_periph_clock_enable(CMT2219B_SPI_RCU);

    /*
     * FCSB -> PB12，普通 GPIO 输出
     */
    gpio_mode_set(CMT2219B_FCSB_PORT,
                  GPIO_MODE_OUTPUT,
                  GPIO_PUPD_NONE,
                  CMT2219B_FCSB_PIN);

    gpio_output_options_set(CMT2219B_FCSB_PORT,
                            GPIO_OTYPE_PP,
                            GPIO_OSPEED_50MHZ,
                            CMT2219B_FCSB_PIN);

    /*
     * CSB -> PB14，普通 GPIO 输出
     */
    gpio_mode_set(CMT2219B_CSB_PORT,
                  GPIO_MODE_OUTPUT,
                  GPIO_PUPD_NONE,
                  CMT2219B_CSB_PIN);

    gpio_output_options_set(CMT2219B_CSB_PORT,
                            GPIO_OTYPE_PP,
                            GPIO_OSPEED_50MHZ,
                            CMT2219B_CSB_PIN);

    /*
     * CLK -> PB13 -> SPI1_SCK
     */
    gpio_af_set(CMT2219B_CLK_PORT,
                CMT2219B_SPI_AF,
                CMT2219B_CLK_PIN);

    gpio_mode_set(CMT2219B_CLK_PORT,
                  GPIO_MODE_AF,
                  GPIO_PUPD_NONE,
                  CMT2219B_CLK_PIN);

    gpio_output_options_set(CMT2219B_CLK_PORT,
                            GPIO_OTYPE_PP,
                            GPIO_OSPEED_50MHZ,
                            CMT2219B_CLK_PIN);

    /*
     * SDIO -> PB15 -> SPI1_MOSI / 单线双向数据
     */
    gpio_af_set(CMT2219B_SDIO_PORT,
                CMT2219B_SPI_AF,
                CMT2219B_SDIO_PIN);

    gpio_mode_set(CMT2219B_SDIO_PORT,
                  GPIO_MODE_AF,
                  GPIO_PUPD_NONE,
                  CMT2219B_SDIO_PIN);

    gpio_output_options_set(CMT2219B_SDIO_PORT,
                            GPIO_OTYPE_PP,
                            GPIO_OSPEED_50MHZ,
                            CMT2219B_SDIO_PIN);

    /*
     * GPIO3 -> PB10，接收状态输入
     */
    gpio_mode_set(CMT2219B_GPIO3_PORT,
                  GPIO_MODE_INPUT,
                  GPIO_PUPD_NONE,
                  CMT2219B_GPIO3_PIN);

    /*
     * 初始化默认空闲电平
     */
    cmt2219b_csb_high();
    cmt2219b_fcsb_high();

    /*
     * 设置 SPI 引脚对应的默认电平。
     */
    gpio_bit_reset(CMT2219B_CLK_PORT, CMT2219B_CLK_PIN);
    gpio_bit_set(CMT2219B_SDIO_PORT, CMT2219B_SDIO_PIN);
}

/*
 * 将 PB13、PB15 从 SPI1 复用功能切换为普通 GPIO。
 *
 * 221 读取寄存器和读取 FIFO 时使用。
 */
void cmt2219b_spi_pins_to_gpio(void)
{
    /*
     * 切换前保证 CLK 为低电平，
     * 保证 SDIO 输出锁存值为高电平。
     */
    gpio_bit_reset(CMT2219B_CLK_PORT, CMT2219B_CLK_PIN);
    gpio_bit_set(CMT2219B_SDIO_PORT, CMT2219B_SDIO_PIN);

    /*
     * PB13 -> 普通 GPIO 输出，模拟 CLK。
     */
    gpio_mode_set(CMT2219B_CLK_PORT,
                  GPIO_MODE_OUTPUT,
                  GPIO_PUPD_NONE,
                  CMT2219B_CLK_PIN);

    gpio_output_options_set(CMT2219B_CLK_PORT,
                            GPIO_OTYPE_PP,
                            GPIO_OSPEED_50MHZ,
                            CMT2219B_CLK_PIN);

    /*
     * PB15 -> 普通 GPIO 输出。
     * 读取数据之前再切换为输入。
     */
    gpio_mode_set(CMT2219B_SDIO_PORT,
                  GPIO_MODE_OUTPUT,
                  GPIO_PUPD_NONE,
                  CMT2219B_SDIO_PIN);

    gpio_output_options_set(CMT2219B_SDIO_PORT,
                            GPIO_OTYPE_PP,
                            GPIO_OSPEED_50MHZ,
                            CMT2219B_SDIO_PIN);
}

/*
 * 将 PB13、PB15 恢复为 SPI1 复用功能。
 *
 * 硬件 SPI 写寄存器和写 FIFO 时使用。
 */
void cmt2219b_spi_pins_to_spi1(void)
{
    /*
     * 切换前保证 CLK 为低电平。
     */
    gpio_bit_reset(CMT2219B_CLK_PORT, CMT2219B_CLK_PIN);
    gpio_bit_set(CMT2219B_SDIO_PORT, CMT2219B_SDIO_PIN);

    /*
     * 恢复 PB15 -> SPI1_MOSI / SDIO。
     */
    gpio_af_set(CMT2219B_SDIO_PORT,
                CMT2219B_SPI_AF,
                CMT2219B_SDIO_PIN);

    gpio_mode_set(CMT2219B_SDIO_PORT,
                  GPIO_MODE_AF,
                  GPIO_PUPD_NONE,
                  CMT2219B_SDIO_PIN);

    gpio_output_options_set(CMT2219B_SDIO_PORT,
                            GPIO_OTYPE_PP,
                            GPIO_OSPEED_50MHZ,
                            CMT2219B_SDIO_PIN);

    /*
     * 恢复 PB13 -> SPI1_SCK。
     *
     * 先恢复 SDIO，再恢复 CLK，避免切换时产生错误时钟。
     */
    gpio_af_set(CMT2219B_CLK_PORT,
                CMT2219B_SPI_AF,
                CMT2219B_CLK_PIN);

    gpio_mode_set(CMT2219B_CLK_PORT,
                  GPIO_MODE_AF,
                  GPIO_PUPD_NONE,
                  CMT2219B_CLK_PIN);

    gpio_output_options_set(CMT2219B_CLK_PORT,
                            GPIO_OTYPE_PP,
                            GPIO_OSPEED_50MHZ,
                            CMT2219B_CLK_PIN);
}

void cmt2219b_csb_high(void)
{
    gpio_bit_set(CMT2219B_CSB_PORT, CMT2219B_CSB_PIN);
}

void cmt2219b_csb_low(void)
{
    gpio_bit_reset(CMT2219B_CSB_PORT, CMT2219B_CSB_PIN);
}

void cmt2219b_fcsb_high(void)
{
    gpio_bit_set(CMT2219B_FCSB_PORT, CMT2219B_FCSB_PIN);
}

void cmt2219b_fcsb_low(void)
{
    gpio_bit_reset(CMT2219B_FCSB_PORT, CMT2219B_FCSB_PIN);
}

void cmt2219b_clk_high(void)
{
    gpio_bit_set(CMT2219B_CLK_PORT, CMT2219B_CLK_PIN);
}

void cmt2219b_clk_low(void)
{
    gpio_bit_reset(CMT2219B_CLK_PORT, CMT2219B_CLK_PIN);
}

void cmt2219b_sdio_high(void)
{
    gpio_bit_set(CMT2219B_SDIO_PORT, CMT2219B_SDIO_PIN);
}

void cmt2219b_sdio_low(void)
{
    gpio_bit_reset(CMT2219B_SDIO_PORT, CMT2219B_SDIO_PIN);
}

uint8_t cmt2219b_sdio_read(void)
{
    return gpio_input_bit_get(CMT2219B_SDIO_PORT,
                              CMT2219B_SDIO_PIN) ? 1U : 0U;
}

void cmt2219b_sdio_output(void)
{
    gpio_mode_set(CMT2219B_SDIO_PORT,
                  GPIO_MODE_OUTPUT,
                  GPIO_PUPD_NONE,
                  CMT2219B_SDIO_PIN);

    gpio_output_options_set(CMT2219B_SDIO_PORT,
                            GPIO_OTYPE_PP,
                            GPIO_OSPEED_50MHZ,
                            CMT2219B_SDIO_PIN);
}

void cmt2219b_sdio_input(void)
{
    gpio_mode_set(CMT2219B_SDIO_PORT,
                  GPIO_MODE_INPUT,
                  GPIO_PUPD_PULLUP,
                  CMT2219B_SDIO_PIN);
}

uint8_t cmt2219b_gpio3_read(void)
{
    return gpio_input_bit_get(CMT2219B_GPIO3_PORT,
                              CMT2219B_GPIO3_PIN) ? 1U : 0U;
}

void cmt2219b_delay_us(uint32_t us)
{
    delay_us(us);
}

void cmt2219b_delay_ms(uint32_t ms)
{
    delay_ms(ms);
}
