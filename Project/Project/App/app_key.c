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
                // 确认按下返回1 ,
                // 同时将发送数据的数组第三位内容进行更改
                tx_buf_data[3] = 0x44;
                key_down = 1;
                return 1;
            }
        }
    }
    else if (key_down == 1)
    {
        // 松手返回0
        tx_buf_data[3] = 0x40;
        key_down = 0;

        return 2;
    }

    return 0;
}

// PA11按钮测试代码
uint8_t check_double_key12(void)
{
    // 是否按下标志位
    static uint8_t key_down = 0;
    if (gpio_input_bit_get(GPIOA, GPIO_PIN_12) == RESET)
    {
        delay_ms(20);
        if (gpio_input_bit_get(GPIOA, GPIO_PIN_12) == RESET)
        {
            if (key_down == 0)
            {
                // 返回1防止一直按下
                key_down = 1;
                led_PA15_toggle();
                printf("Key12 Down\r\n");
                return 1;
            }
        }
        else
        {
            // 返回0 表示按下以后已经取消掉了
            key_down = 0;
            return 0;
        }
    }
}
