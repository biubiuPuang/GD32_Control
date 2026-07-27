#include "cmt2219b_port.h"
#include "systick.h"

void cmt2219b_port_init(void)
{
    rcu_periph_clock_enable(CMT2219B_FCSB_RCU);
    rcu_periph_clock_enable(CMT2219B_CSB_RCU);
    rcu_periph_clock_enable(CMT2219B_CLK_RCU);
    rcu_periph_clock_enable(CMT2219B_SDIO_RCU);
    rcu_periph_clock_enable(CMT2219B_GPIO3_RCU);

    /* FCSB -> PB12 输出 */
    gpio_mode_set(CMT2219B_FCSB_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, CMT2219B_FCSB_PIN);
    gpio_output_options_set(CMT2219B_FCSB_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, CMT2219B_FCSB_PIN);

    /* CSB -> PB13 输出 */
    gpio_mode_set(CMT2219B_CSB_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, CMT2219B_CSB_PIN);
    gpio_output_options_set(CMT2219B_CSB_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, CMT2219B_CSB_PIN);

    /* CLK -> PB14 输出 */
    gpio_mode_set(CMT2219B_CLK_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, CMT2219B_CLK_PIN);
    gpio_output_options_set(CMT2219B_CLK_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, CMT2219B_CLK_PIN);

    /* SDIO -> PB15 默认先配置成输出 */
    cmt2219b_sdio_output();

    /* GPIO3 -> PB10 输入 */
    gpio_mode_set(CMT2219B_GPIO3_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, CMT2219B_GPIO3_PIN);

    /* 默认空闲电平 */
    cmt2219b_csb_high();
    cmt2219b_fcsb_high();
    cmt2219b_clk_low();
    cmt2219b_sdio_high();
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
    if (gpio_input_bit_get(CMT2219B_SDIO_PORT, CMT2219B_SDIO_PIN)) {
        return 1;
    } else {
        return 0;
    }
}

void cmt2219b_sdio_output(void)
{
    gpio_mode_set(CMT2219B_SDIO_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, CMT2219B_SDIO_PIN);
    gpio_output_options_set(CMT2219B_SDIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, CMT2219B_SDIO_PIN);
}

void cmt2219b_sdio_input(void)
{
    gpio_mode_set(CMT2219B_SDIO_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, CMT2219B_SDIO_PIN);
}

uint8_t cmt2219b_gpio3_read(void)
{
    if (gpio_input_bit_get(CMT2219B_GPIO3_PORT, CMT2219B_GPIO3_PIN)) {
        return 1;
    } else {
        return 0;
    }
}

void cmt2219b_delay_us(uint32_t us)
{
    delay_us(us);
}

void cmt2219b_delay_ms(uint32_t ms)
{
    delay_ms(ms);
}
