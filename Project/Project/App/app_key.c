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

    // PA12 配置为上拉输入
    gpio_mode_set(GPIOA,
                  GPIO_MODE_INPUT,
                  GPIO_PUPD_PULLUP,
                  GPIO_PIN_12);

    // PA11 配置为上拉输入
    gpio_mode_set(GPIOA,
                  GPIO_MODE_INPUT,
                  GPIO_PUPD_PULLUP,
                  GPIO_PIN_11);
}

// 获取按键值
uint8_t get_key_num(void)
{

    // 如果PA12按下
    if (gpio_input_bit_get(GPIOA, GPIO_PIN_12) == RESET)
    {

        // 消抖
        delay_ms(15);
        // 消抖完按键确认按下
        if (gpio_input_bit_get(GPIOA, GPIO_PIN_12) == RESET)
        {
            // PA11也按下
            if (gpio_input_bit_get(GPIOA, GPIO_PIN_11) == RESET)
            {
                // 双按键按下返回11
                // 同时将发送数据的数组第三位内容进行更改
                tx_buf_data[3] = 0x11;
                return 11;
            }
            // 只有PA12单按键确认按下返回10表示单键
            // 同时将发送数据的数组第三位内容进行更改
            tx_buf_data[3] = 0x10;
            return 10;
        }
    }

    // 没有按键按下
    return 0;
}
