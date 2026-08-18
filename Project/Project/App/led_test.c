#include "led_test.h"
#include "gd32e23x.h"

void pa15_led_init(void)
{
    /* GD32 官方库函数：使能 GPIOA 时钟 */
    rcu_periph_clock_enable(RCU_GPIOA);

    /* GD32 官方库函数：PA15 配置为输出模式 */
    gpio_mode_set(GPIOA,
                  GPIO_MODE_OUTPUT,
                  GPIO_PUPD_NONE,
                  GPIO_PIN_15);

    /* GD32 官方库函数：配置为推挽输出、50 MHz */
    gpio_output_options_set(GPIOA,
                            GPIO_OTYPE_PP,
                            GPIO_OSPEED_50MHZ,
                            GPIO_PIN_15);

    /* 默认输出高电平 */
    gpio_bit_set(GPIOA, GPIO_PIN_15);
}

