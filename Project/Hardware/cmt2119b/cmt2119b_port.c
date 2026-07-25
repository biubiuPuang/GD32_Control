#include "cmt2119b_port.h"
#include "systick.h"

void cmt2119b_port_init(void)
{
    rcu_periph_clock_enable(CMT2119B_FCSB_RCU);
    rcu_periph_clock_enable(CMT2119B_CSB_RCU);
    rcu_periph_clock_enable(CMT2119B_SDIO_RCU);
    rcu_periph_clock_enable(CMT2119B_SCLK_RCU);
    rcu_periph_clock_enable(CMT2119B_GPIO3_RCU);

    /* CSB, FCSB, SCLK output */
    gpio_mode_set(CMT2119B_CSB_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, CMT2119B_CSB_PIN);
    gpio_output_options_set(CMT2119B_CSB_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, CMT2119B_CSB_PIN);

    gpio_mode_set(CMT2119B_FCSB_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, CMT2119B_FCSB_PIN);
    gpio_output_options_set(CMT2119B_FCSB_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, CMT2119B_FCSB_PIN);

    gpio_mode_set(CMT2119B_SCLK_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, CMT2119B_SCLK_PIN);
    gpio_output_options_set(CMT2119B_SCLK_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, CMT2119B_SCLK_PIN);

    /* SDIO defaults to output high */
    cmt2119b_sdio_output();

    /* GPIO3 input: first use pull-down or floating.
     * If TX_DONE cannot be read later, change to GPIO_PUPD_PULLUP or GPIO_PUPD_NONE according to actual waveform.
     */
    gpio_mode_set(CMT2119B_GPIO3_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, CMT2119B_GPIO3_PIN);

    cmt2119b_csb_high();
    cmt2119b_fcsb_high();
    cmt2119b_sclk_low();
    cmt2119b_sdio_high();
}

void cmt2119b_csb_high(void)
{
    gpio_bit_set(CMT2119B_CSB_PORT, CMT2119B_CSB_PIN);
}

void cmt2119b_csb_low(void)
{
    gpio_bit_reset(CMT2119B_CSB_PORT, CMT2119B_CSB_PIN);
}

void cmt2119b_fcsb_high(void)
{
    gpio_bit_set(CMT2119B_FCSB_PORT, CMT2119B_FCSB_PIN);
}

void cmt2119b_fcsb_low(void)
{
    gpio_bit_reset(CMT2119B_FCSB_PORT, CMT2119B_FCSB_PIN);
}

void cmt2119b_sclk_high(void)
{
    gpio_bit_set(CMT2119B_SCLK_PORT, CMT2119B_SCLK_PIN);
}

void cmt2119b_sclk_low(void)
{
    gpio_bit_reset(CMT2119B_SCLK_PORT, CMT2119B_SCLK_PIN);
}

void cmt2119b_sdio_high(void)
{
    gpio_bit_set(CMT2119B_SDIO_PORT, CMT2119B_SDIO_PIN);
}

void cmt2119b_sdio_low(void)
{
    gpio_bit_reset(CMT2119B_SDIO_PORT, CMT2119B_SDIO_PIN);
}

uint8_t cmt2119b_sdio_read(void)
{
    return gpio_input_bit_get(CMT2119B_SDIO_PORT, CMT2119B_SDIO_PIN) ? 1 : 0;
}

void cmt2119b_sdio_output(void)
{
    gpio_mode_set(CMT2119B_SDIO_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, CMT2119B_SDIO_PIN);
    gpio_output_options_set(CMT2119B_SDIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, CMT2119B_SDIO_PIN);
}

void cmt2119b_sdio_input(void)
{
    gpio_mode_set(CMT2119B_SDIO_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, CMT2119B_SDIO_PIN);
}

uint8_t cmt2119b_gpio3_read(void)
{
    return gpio_input_bit_get(CMT2119B_GPIO3_PORT, CMT2119B_GPIO3_PIN) ? 1 : 0;
}

void cmt2119b_delay_us(uint32_t us)
{
    delay_us(us);
}

void cmt2119b_delay_ms(uint32_t ms)
{
    delay_ms(ms);
}
