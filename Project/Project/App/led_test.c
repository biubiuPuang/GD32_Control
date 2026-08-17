#include "led_test.h"
#include "gd32e23x.h"

// 配置 PB9 为 LED 输出（默认熄灭：高电平）
void pb9_led_init(void)
{
    /* 使能 GPIOB 时钟 */
    rcu_periph_clock_enable(RCU_GPIOB);

    /* PB9 配置为推挽输出 */
    gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_9);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);

    /* 默认输出高电平 = 熄灭（LED 阳极经 R8 1kΩ 到 VCC，低电平才点亮） */
    gpio_bit_set(GPIOB, GPIO_PIN_9);
}

// 点亮 LED（PB9 拉低）
void pb9_led_on(void)
{
    gpio_bit_reset(GPIOB, GPIO_PIN_9);
}

// 熄灭 LED（PB9 拉高）
void pb9_led_off(void)
{
    gpio_bit_set(GPIOB, GPIO_PIN_9);
}

// 翻转 LED（PB9）
void pb9_led_toggle(void)
{
    gpio_bit_toggle(GPIOB, GPIO_PIN_9);
}
