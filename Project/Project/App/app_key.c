#include "app_key.h"
#include "systick.h"

// 按键GPIO初始化
void key_init(void)
{
    /* 开启GPIOA时钟：GD32官方库函数 */
    rcu_periph_clock_enable(RCU_GPIOA);

    /* PA0配置为下拉输入：GD32官方库函数 */
    gpio_mode_set(GPIOA,
                  GPIO_MODE_INPUT,
                  GPIO_PUPD_PULLUP,
                  GPIO_PIN_0);
}

// 检测按键是否按下
uint8_t key_is_pressed(void)
{
    // 是否按下标志位
    static uint8_t key_down = 0;

    if (gpio_input_bit_get(GPIOA, GPIO_PIN_0) == RESET)
    {
        delay_ms(15);

        if (gpio_input_bit_get(GPIOA, GPIO_PIN_0) == RESET)
        {
            if (key_down == 0)
            {
                // 确认按下返回1
                key_down = 1;
                return 1;
            }
        }
    }
    else
    {
        // 松手返回0
        key_down = 0;
    }

    return 0;
}



