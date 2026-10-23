#include "bsp_wired_execute.h"

// 执行上升许可命令
void wired_execute_up(Arbitrament END_REG)
{
    switch (END_REG)
    {
    case UP_OK:
        /* 执行上升动作 */
        // 总控制开关(关闭则不会获得信号输入)
        gpio_bit_reset(GPIOB, GPIO_PIN_11); // 开启
        // 电机上\下\快速开关(关闭下 即使PA8\9\10继电器吸合也无法驱动电机)
        gpio_bit_reset(GPIOB, GPIO_PIN_6); // 开启
        // 上升继电器吸合
        gpio_bit_reset(GPIOA, GPIO_PIN_9);
        debug_printf("up model");
        // 判断快速按键是否按下
        if (gpio_input_bit_get(GPIOB, GPIO_PIN_5) == 0)
        {
            // 进入快速模式
            gpio_bit_reset(GPIOA, GPIO_PIN_10);
            debug_printf("fast up model");
        }
        break;
    case DOWN_OK:
        /* 执行下降动作 */
        // 总控制开关(关闭则不会获得信号输入)
        gpio_bit_reset(GPIOB, GPIO_PIN_11); // 开启
        // 电机上\下\快速开关(关闭下 即使PA8\9\10继电器吸合也无法驱动电机)
        gpio_bit_reset(GPIOB, GPIO_PIN_6); // 开启
        // 下降继电器吸合
        gpio_bit_reset(GPIOA, GPIO_PIN_8);
        debug_printf("dowm model");
        // 判断快速按键是否按下
        if (gpio_input_bit_get(GPIOB, GPIO_PIN_5) == 0)
        {
            // 进入快速模式
            gpio_bit_reset(GPIOA, GPIO_PIN_10);
            debug_printf("fast dowm model");
        }
        break;
    case IDLE_OK:
        /* 无动作 */
        break;
    case ERROR_OK:
        /* 动作冲突-不执行任何动作 */
        // 总控制开关(关闭则不会获得信号输入)
        gpio_bit_reset(GPIOB, GPIO_PIN_11); // 开启
        // 电机上\下\快速开关(关闭下 即使PA8\9\10继电器吸合也无法驱动电机)
        gpio_bit_set(GPIOB, GPIO_PIN_6); // 关闭
        debug_printf("error model");
        break;
    case STOP_OK:
        /* 急停(不执行任何动作) */
        // 初始化有线部分的所有函数
        wired_all_gpio_init();
        debug_printf("stop model");
        /* 退出急停状态许可的代码:待编写 */
        /*
         *
         *
         */
        /* 退出急停状态许可的代码:待编写 */

        break;
    }
}
