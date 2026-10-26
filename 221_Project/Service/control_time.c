/* 建议替换工程Service/control_time.c的内容。
 * 用户自定义计时封装；底层rcu_/timer_/nvic_函数来自GD32官方库。
 * TIMER13在本次读取的业务代码中未被占用，启动文件已有其弱中断入口。
 * 中断函数定义放在本文件，不要再在gd32e23x_it.c重复定义。
 * 未改工程；这是独立建议文件。仅做语法检查，未做硬件验证。
 */
#include "control_time.h"
#include "gd32e23x.h"

static volatile uint32_t control_ms = 0U;

void control_time_init(void)
{
    timer_parameter_struct timer_config;
    uint32_t timer_clock;

    /* APB1不分频时定时器时钟等于APB1；分频时定时器时钟为其两倍。 */
    timer_clock = rcu_clock_freq_get(CK_APB1);
    if ((RCU_CFG0 & RCU_CFG0_APB1PSC) != RCU_APB1_CKAHB_DIV1)
    {
        timer_clock *= 2U;
    }

    /* 先开外设时钟，再配置定时器。 */
    rcu_periph_clock_enable(RCU_TIMER13);
    timer_deinit(TIMER13);
    timer_struct_para_init(&timer_config);

    /* 本工程为72MHz配置；先分频到1MHz，再每1000次计数产生1ms更新。 */
    timer_config.prescaler = timer_clock / 1000000U - 1U;
    timer_config.period = 1000U - 1U;
    timer_config.clockdivision = TIMER_CKDIV_DIV1;
    timer_init(TIMER13, &timer_config);

    control_ms = 0U;
    timer_interrupt_flag_clear(TIMER13, TIMER_INT_FLAG_UP);
    timer_interrupt_enable(TIMER13, TIMER_INT_UP);
    nvic_irq_enable(TIMER13_IRQn, 2U);
    timer_enable(TIMER13);
}

uint32_t control_now_ms(void)
{
    return control_ms;
}

void TIMER13_IRQHandler(void)
{
    if (timer_interrupt_flag_get(TIMER13, TIMER_INT_FLAG_UP) == SET)
    {
        timer_interrupt_flag_clear(TIMER13, TIMER_INT_FLAG_UP);
        ++control_ms;
    }
}
