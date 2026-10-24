#include "bsp_wired_execute.h"

// 执行对应动作
void wired_order(Arbitrament END_REG)
{
    /* 有关即debug_printf()打印的部分代码 */
    // 定义一个初始标记为为-1,-1 != 任何状态 除非状态改变否则debug_printf()不会执行
    // 第一次进来时 last_print 是 -1，跟真实状态（0~4）都不一样，等于也算"状态改变"，所以会打一次。之后状态不变就不打，变了才再打。
    static Arbitrament last_print = (Arbitrament)-1;
    uint8_t state_changed = (END_REG != last_print);
    last_print = END_REG;
    /* 有关即debug_printf()打印的部分代码 */

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
        if (state_changed) debug_printf("up model");
        // 判断快速按键是否按下
        if (gpio_input_bit_get(GPIOB, GPIO_PIN_5) == 0)
        {
            // 进入快速模式
            gpio_bit_reset(GPIOA, GPIO_PIN_10);
            if (state_changed) debug_printf("fast up model");
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
        if (state_changed) debug_printf("dowm model");
        // 判断快速按键是否按下
        if (gpio_input_bit_get(GPIOB, GPIO_PIN_5) == 0)
        {
            // 进入快速模式
            gpio_bit_reset(GPIOA, GPIO_PIN_10);
            if (state_changed) debug_printf("fast dowm model");
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
        if (state_changed) debug_printf("error model");
        break;
    case STOP_OK:
        /* 急停(不执行任何动作) */
        // 初始化有线部分的所有函数
        wired_all_gpio_init();
        if (state_changed) debug_printf("stop model");
        /* 退出急停状态许可的代码:待编写 */
        /*
         *
         *
         */
        /* 退出急停状态许可的代码:待编写 */

        break;
    }
}
