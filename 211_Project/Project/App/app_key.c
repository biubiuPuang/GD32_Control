#include "app_key.h"
#include "systick.h"
#include "gd32e23x_exti.h"
#include "gd32e23x_syscfg.h"
#include "gd32e23x_misc.h"
#include <string.h>

// 低功耗
volatile uint8_t key_wakeup_flag = 0;

// 按键GPIO初始化

void key_init(void)
{
    /* 开启GPIOA\B时钟：GD32官方库函数 */
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);

    // 低功耗
    rcu_periph_clock_enable(RCU_CFGCMP);

    /* PA0配置为下拉输入：GD32官方库函数 */
    gpio_mode_set(GPIOA,
                  GPIO_MODE_INPUT,
                  GPIO_PUPD_PULLUP,
                  GPIO_PIN_0);

    // 右按键
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

    // 下按键
    // PA9 配置为上拉输入
    gpio_mode_set(GPIOA,
                  GPIO_MODE_INPUT,
                  GPIO_PUPD_PULLUP,
                  GPIO_PIN_9);
    // PA10 配置为上拉输入
    gpio_mode_set(GPIOA,
                  GPIO_MODE_INPUT,
                  GPIO_PUPD_PULLUP,
                  GPIO_PIN_10);

    // 左按键
    // PB5 配置为上拉输入
    gpio_mode_set(GPIOB,
                  GPIO_MODE_INPUT,
                  GPIO_PUPD_PULLUP,
                  GPIO_PIN_5);
    // PB6 配置为上拉输入
    gpio_mode_set(GPIOB,
                  GPIO_MODE_INPUT,
                  GPIO_PUPD_PULLUP,
                  GPIO_PIN_6);

    // 上按键
    // PB3 配置为上拉输入
    gpio_mode_set(GPIOB,
                  GPIO_MODE_INPUT,
                  GPIO_PUPD_PULLUP,
                  GPIO_PIN_3);
    // PB4 配置为上拉输入
    gpio_mode_set(GPIOB,
                  GPIO_MODE_INPUT,
                  GPIO_PUPD_PULLUP,
                  GPIO_PIN_4);

    /*
     * 只配置每组按键的主检测脚作为唤醒源。
     * 这是为了保持现有按键识别逻辑不变：
     * PA11、PA9、PB5、PB3 是当前 get_key_num() 的主检测脚。
     */

    /* PA11 -> EXTI11 */
    syscfg_exti_line_config(EXTI_SOURCE_GPIOA,
                            EXTI_SOURCE_PIN11);
    exti_init(EXTI_11,
              EXTI_INTERRUPT,
              EXTI_TRIG_FALLING);
    exti_interrupt_enable(EXTI_11);

    /* PA9 -> EXTI9 */
    syscfg_exti_line_config(EXTI_SOURCE_GPIOA,
                            EXTI_SOURCE_PIN9);
    exti_init(EXTI_9,
              EXTI_INTERRUPT,
              EXTI_TRIG_FALLING);
    exti_interrupt_enable(EXTI_9);

    /* PB5 -> EXTI5 */
    syscfg_exti_line_config(EXTI_SOURCE_GPIOB,
                            EXTI_SOURCE_PIN5);
    exti_init(EXTI_5,
              EXTI_INTERRUPT,
              EXTI_TRIG_FALLING);
    exti_interrupt_enable(EXTI_5);

    /* PB3 -> EXTI3 */
    syscfg_exti_line_config(EXTI_SOURCE_GPIOB,
                            EXTI_SOURCE_PIN3);
    exti_init(EXTI_3,
              EXTI_INTERRUPT,
              EXTI_TRIG_FALLING);
    exti_interrupt_enable(EXTI_3);

    /* 清除可能已经存在的 EXTI 挂起标志 */
    exti_interrupt_flag_clear(EXTI_11);
    exti_interrupt_flag_clear(EXTI_9);
    exti_interrupt_flag_clear(EXTI_5);
    exti_interrupt_flag_clear(EXTI_3);

    /* 开启 EXTI 中断分组 */
    nvic_irq_enable(EXTI4_15_IRQn, 1);
    nvic_irq_enable(EXTI2_3_IRQn, 1);
}

// 获取按键值
uint8_t get_key_num(void)
{
    // 右按键按下,如果PA11按下
    if (gpio_input_bit_get(GPIOA, GPIO_PIN_11) == RESET)
    {
        // 消抖
        delay_ms(15);
        // 消抖完按键确认按下
        if (gpio_input_bit_get(GPIOA, GPIO_PIN_11) == RESET)
        {
            debug_printf("right\r\n");
            // PA11也按下
            if (gpio_input_bit_get(GPIOA, GPIO_PIN_12) == RESET)
            {
                // 双按键按下返回11
                // 同时将发送数据的数组第三位内容进行更改
                tx_buf_data[8] = 0x11;
                return 11;
            }
            // 只有PA12单按键确认按下返回10表示单键
            // 同时将发送数据的数组第三位内容进行更改
            tx_buf_data[8] = 0x10;
            return 10;
        }
    }

    // 下按键按下,如果PA9按下
    else if (gpio_input_bit_get(GPIOA, GPIO_PIN_9) == RESET)
    {
        // 消抖
        delay_ms(15);
        // 消抖完按键确认按下
        if (gpio_input_bit_get(GPIOA, GPIO_PIN_9) == RESET)
        {
            debug_printf("dowm\r\n");
            // PA10也按下
            if (gpio_input_bit_get(GPIOA, GPIO_PIN_10) == RESET)
            {
                // 双按键按下返回21
                // 同时将发送数据的数组第四位内容进行更改
                tx_buf_data[9] = 0x21;
                return 21;
            }
            // 只有PA9单按键确认按下返回20表示单键
            // 同时将发送数据的数组第四位内容进行更改
            tx_buf_data[9] = 0x20;
            return 20;
        }
    }

    // 左按键按下,如果PB5按下
    else if (gpio_input_bit_get(GPIOB, GPIO_PIN_5) == RESET)
    {
        // 消抖
        delay_ms(15);
        // 消抖完按键确认按下
        if (gpio_input_bit_get(GPIOB, GPIO_PIN_5) == RESET)
        {
            debug_printf("left\r\n");
            debug_printf("PA5\r\n");
            // PB6也按下
            if (gpio_input_bit_get(GPIOB, GPIO_PIN_6) == RESET)
            {
                // 双按键按下返回32
                // 同时将发送数据的数组第五位内容进行更改
                debug_printf("PA6\r\n");
                tx_buf_data[10] = 0x31;
                return 31;
            }
            // 只有PB5单按键确认按下返回30表示单键
            // 同时将发送数据的数组第五位内容进行更改
            tx_buf_data[10] = 0x30;
            return 30;
        }
    }

    // 上按键按下,如果PB3按下
    else if (gpio_input_bit_get(GPIOB, GPIO_PIN_3) == RESET)
    {
        // 消抖
        delay_ms(15);
        // 消抖完按键确认按下
        if (gpio_input_bit_get(GPIOB, GPIO_PIN_3) == RESET)
        {
            debug_printf("up\r\n");
            // PB4也按下
            if (gpio_input_bit_get(GPIOB, GPIO_PIN_4) == RESET)
            {
                // 双按键按下返回41
                // 同时将发送数据的数组第六位内容进行更改
                tx_buf_data[11] = 0x41;
                return 41;
            }
            // 只有PB3单按键确认按下返回40表示单键
            // 同时将发送数据的数组第六位内容进行更改
            tx_buf_data[11] = 0x40;
            return 40;
        }
    }

    // 无按键按下,对数组数据内容进行清零
    memset(&tx_buf_data[8], 0, (4 * sizeof(tx_buf_data[0])));
   
    return 0;
}
