#include "led_test.h"
#include "gd32e23x.h"

// 引脚PA15初始化
void pa15_output_high_init(void)
{
    /* 开启 GPIOA 时钟 */
    rcu_periph_clock_enable(RCU_GPIOA);

    /* PA15 配置为普通输出 */
    gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_15);

    /* 推挽输出，50MHz */
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_15);

    /* 默认输出高电平 */
    gpio_bit_set(GPIOA, GPIO_PIN_15);
}

