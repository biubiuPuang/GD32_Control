#include "bsp_wired.h"

/* ---------------有线功能相关的所有引脚初始化-------------- */
// 初始化有线相关的所有引脚
void wired_all_gpio_init(void)
{

    /* ---------------急停按钮控制相关的引脚初始化-------------- */
    // PB11 电源控制引脚的初始化 高电平有效(有效=急停)
    stop_gpio_init();
    /* ---------------上升反馈电路引脚的初始化-------------- */
    // PB3 电机上升引脚的相关初始化操作 低＝有上升信号
    read_up_gpio_init();
    /* ---------------下降反馈电路引脚的初始化-------------- */
    // PB4 电机下降引脚的相关初始化操作 低＝有下降信号
    read_dowm_gpio_init();
    /* ---------------有线快速按键反馈引脚初始化-------------- */
    // PB5 有线快速按键引脚的相关初始化操作 低＝按下；高＝未按下
    read_quicker_gpio_init();
    /* ---------------下降继电器控制引脚初始化-------------- */
    // PA8下降引脚控制初始化 低电平吸合，执行下降
    dowm_gpio_init();
    /* ---------------上升继电器控制引脚初始化-------------- */
    // PA9上升引脚控制初始化 低电平有效
    up_gpio_init();
    /* ---------------快速控制引脚初始化-------------- */
    // PA10快速控制引脚初始化 低电平有效 (有效-吸合 = 快速模式 \ 无效-断开 = 慢速模式)
    quicker_gpio_init();
    /* ---------------上升\下降许可开关引脚初始化-------------- */
    // PB6 上升\下降许可开关引脚的初始化 高电平有效
    up_or_down_permit_init();
}

/* ---------------急停按钮控制相关的引脚初始化-------------- */
// PB11 电源控制引脚的初始化 高电平有效(有效=急停)
void stop_gpio_init(void)
{
    // 开启GPIOB的时钟
    rcu_periph_clock_enable(RCU_GPIOB);

    // 设置PB11默认为高电平
    GPIO_BOP(GPIOB) = GPIO_PIN_11;

    // 相关配置为 GPIO端口号 输出模式 无上下拉 指定的端口引脚
    gpio_mode_set(GPIOB,
                  GPIO_MODE_OUTPUT,
                  GPIO_PUPD_NONE,
                  GPIO_PIN_11);

    // 相关配置为 GPIO端口号 推挽输出模式 2MHz速度 指定的端口引脚
    gpio_output_options_set(GPIOB,
                            GPIO_OTYPE_PP,
                            GPIO_OSPEED_2MHZ,
                            GPIO_PIN_11);
}

/* ---------------上升\下降许可开关引脚初始化-------------- */
// PB6 上升\下降许可开关引脚的初始化 高电平有效
void up_or_down_permit_init(void)
{
    // 开启GPIOB的时钟
    rcu_periph_clock_enable(RCU_GPIOB);

    // 设置PB6默认为高电平
    GPIO_BOP(GPIOB) = GPIO_PIN_6;

    // 相关配置为 GPIO端口号 输出模式 无上下拉 指定的端口引脚
    gpio_mode_set(GPIOB,
                  GPIO_MODE_OUTPUT,
                  GPIO_PUPD_NONE,
                  GPIO_PIN_6);

    // 相关配置为 GPIO端口号 推挽输出模式 2MHz速度 指定的端口引脚
    gpio_output_options_set(GPIOB,
                            GPIO_OTYPE_PP,
                            GPIO_OSPEED_2MHZ,
                            GPIO_PIN_6);
}

/* ---------------上升反馈电路引脚的初始化-------------- */
// PB3 电机上升引脚的相关初始化操作 低＝有上升信号
void read_up_gpio_init(void)
{
    // 开启GPIOB的时钟
    rcu_periph_clock_enable(RCU_GPIOB);

    // 相关配置为 GPIO端口号 输入模式 无上下拉 指定的端口引脚
    gpio_mode_set(GPIOB,
                  GPIO_MODE_INPUT,
                  GPIO_PUPD_NONE,
                  GPIO_PIN_3);
}

/* ---------------下降反馈电路引脚的初始化-------------- */
// PB4 电机下降引脚的相关初始化操作 低＝有下降信号
void read_dowm_gpio_init(void)
{
    // 开启GPIOB的时钟
    rcu_periph_clock_enable(RCU_GPIOB);

    // 相关配置为 GPIO端口号 输入模式 无上下拉 指定的端口引脚
    gpio_mode_set(GPIOB,
                  GPIO_MODE_INPUT,
                  GPIO_PUPD_NONE,
                  GPIO_PIN_4);
}

/* ---------------有线快速按键反馈引脚初始化-------------- */
// PB5 有线快速按键引脚的相关初始化操作 低＝按下；高＝未按下
void read_quicker_gpio_init(void)
{
    // 开启GPIOB的时钟
    rcu_periph_clock_enable(RCU_GPIOB);

    // 相关配置为 GPIO端口号 输入模式 无上下拉 指定的端口引脚
    gpio_mode_set(GPIOB,
                  GPIO_MODE_INPUT,
                  GPIO_PUPD_NONE,
                  GPIO_PIN_5);
}

/* ---------------下降继电器控制引脚初始化-------------- */
// PA8下降引脚控制初始化 低电平吸合，执行下降
void dowm_gpio_init(void)
{
    // 开启GPIOA的时钟
    rcu_periph_clock_enable(RCU_GPIOA);

    // 设置PA8默认为高电平
    GPIO_BOP(GPIOA) = GPIO_PIN_8;

    // 相关配置为 GPIO端口号 输出模式 无上下拉 指定的端口引脚
    gpio_mode_set(GPIOA,
                  GPIO_MODE_OUTPUT,
                  GPIO_PUPD_NONE,
                  GPIO_PIN_8);

    // 相关配置为 GPIO端口号 推挽输出模式 2MHz速度 指定的端口引脚
    gpio_output_options_set(GPIOA,
                            GPIO_OTYPE_PP,
                            GPIO_OSPEED_2MHZ,
                            GPIO_PIN_8);
}

/* ---------------上升继电器控制引脚初始化-------------- */
// PA9上升引脚控制初始化 低电平有效
void up_gpio_init(void)
{
    // 开启GPIOA的时钟
    rcu_periph_clock_enable(RCU_GPIOA);

    // 设置PA8默认为高电平
    GPIO_BOP(GPIOA) = GPIO_PIN_9;

    // 相关配置为 GPIO端口号 输出模式 无上下拉 指定的端口引脚
    gpio_mode_set(GPIOA,
                  GPIO_MODE_OUTPUT,
                  GPIO_PUPD_NONE,
                  GPIO_PIN_9);

    // 相关配置为 GPIO端口号 推挽输出模式 2MHz速度 指定的端口引脚
    gpio_output_options_set(GPIOA,
                            GPIO_OTYPE_PP,
                            GPIO_OSPEED_2MHZ,
                            GPIO_PIN_9);
}

/* ---------------快速控制引脚初始化-------------- */
// PA10快速控制引脚初始化 低电平有效 (有效-吸合 = 快速模式 \ 无效-断开 = 慢速模式)
void quicker_gpio_init(void)
{
    // 开启GPIOA的时钟
    rcu_periph_clock_enable(RCU_GPIOA);

    // 设置PA10默认为高电平
    GPIO_BOP(GPIOA) = GPIO_PIN_10;

    // 相关配置为 GPIO端口号 输出模式 无上下拉 指定的端口引脚
    gpio_mode_set(GPIOA,
                  GPIO_MODE_OUTPUT,
                  GPIO_PUPD_NONE,
                  GPIO_PIN_10);

    // 相关配置为 GPIO端口号 推挽输出模式 2MHz速度 指定的端口引脚
    gpio_output_options_set(GPIOA,
                            GPIO_OTYPE_PP,
                            GPIO_OSPEED_2MHZ,
                            GPIO_PIN_10);
}
