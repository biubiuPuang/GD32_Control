#include "low_power_config.h"

#include "gd32e23x.h"
#include "gd32e23x_timer.h"
#include "gd32e23x_pmu.h"
#include "gd32e23x_exti.h"

#include "systick.h"
#include "cmt2119b.h"
#include "app_key.h"


/* 低功耗相关变量 */
static volatile uint32_t inactivity_seconds = 0;
static volatile uint8_t sleep_request = 0;
static volatile uint8_t low_power_mode = 0;


static void low_power_timer_init(void)
{
    timer_parameter_struct timer_initpara;

    /*
     * 当前工程按 72 MHz 主频计算：
     * 72 MHz / (7199 + 1) = 10 kHz
     * 10 kHz / (9999 + 1) = 1 Hz
     */
    rcu_periph_clock_enable(RCU_TIMER2);

    timer_deinit(TIMER2);
    timer_struct_para_init(&timer_initpara);

    timer_initpara.prescaler = 7199U;
    timer_initpara.alignedmode = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection = TIMER_COUNTER_UP;
    timer_initpara.clockdivision = TIMER_CKDIV_DIV1;
    timer_initpara.period = 9999U;
    timer_initpara.repetitioncounter = 0U;

    timer_init(TIMER2, &timer_initpara);

    timer_interrupt_flag_clear(TIMER2, TIMER_INT_FLAG_UP);
    timer_interrupt_enable(TIMER2, TIMER_INT_UP);

    nvic_irq_enable(TIMER2_IRQn, 1);

    timer_enable(TIMER2);
}

void low_power_init(void)
{
    inactivity_seconds = 0U;
    sleep_request = 0U;
    low_power_mode = 0U;

    low_power_timer_init();
}


static void enter_low_power_mode(void)
{
    /*
     * 如果进入休眠前主按键仍然处于低电平，
     * 下降沿已经发生，可能导致无法再次产生唤醒边沿。
     */
    if ((gpio_input_bit_get(GPIOA, GPIO_PIN_11) == RESET) ||
        (gpio_input_bit_get(GPIOA, GPIO_PIN_9) == RESET) ||
        (gpio_input_bit_get(GPIOB, GPIO_PIN_5) == RESET) ||
        (gpio_input_bit_get(GPIOB, GPIO_PIN_3) == RESET))
    {
        inactivity_seconds = 0;
        sleep_request = 0;
        return;
    }
    
    /*
     * 标记当前已经准备进入低功耗。
     * EXTI 中断只负责唤醒，不在中断中发送无线数据。
     */
    low_power_mode = 1;
    sleep_request = 0;

    /* 让 PH2119BBA 进入自身 Sleep */
    cmt2119b_go_sleep();

    /* 停止 TIMER2，避免定时器不断唤醒 MCU */
    timer_disable(TIMER2);
    timer_interrupt_flag_clear(TIMER2, TIMER_INT_FLAG_UP);

    /* 清除可能已经存在的按键 EXTI 挂起标志 */
    exti_interrupt_flag_clear(EXTI_11);
    exti_interrupt_flag_clear(EXTI_9);
    exti_interrupt_flag_clear(EXTI_5);
    exti_interrupt_flag_clear(EXTI_3);

    key_wakeup_flag = 0;

    /*
     * 普通 Sleep：
     * pmu_to_sleepmode() 内部会执行 __WFI()
     */
    pmu_to_sleepmode(WFI_CMD);

    /*
     * 程序从这里继续执行，说明已经被按键 EXTI 唤醒。
     */
    low_power_mode = 0;
    inactivity_seconds = 0;

    /* 清除唤醒后的标志 */
    sleep_request = 0;
    key_wakeup_flag = 0;

    /* 重置 TIMER2 计数器并恢复计时 */
    timer_counter_value_config(TIMER2, 0U);
    timer_interrupt_flag_clear(TIMER2, TIMER_INT_FLAG_UP);
    timer_enable(TIMER2);

    /*
     * 等待按键电平稳定。
     * get_key_num() 内部本身也有按键消抖。
     */
    delay_ms(15);
}


void low_power_process(void)
{
    if (sleep_request != 0U)
    {
        enter_low_power_mode();
    }
}


void low_power_activity_notify(void)
{
    inactivity_seconds = 0U;
}

void low_power_timer_irq_handler(void)
{
    if (timer_interrupt_flag_get(TIMER2,
                                  TIMER_INT_FLAG_UP) != RESET)
    {
        timer_interrupt_flag_clear(TIMER2,
                                    TIMER_INT_FLAG_UP);

        /*
         * 只有正常工作时才累计无按键时间。
         * 进入低功耗后 TIMER2 已经关闭，这个判断是额外保护。
         */
        if (low_power_mode == 0U)
        {
            if (inactivity_seconds < LOW_POWER_TIMEOUT_SECONDS)
            {
                inactivity_seconds++;
            }

            if (inactivity_seconds >= LOW_POWER_TIMEOUT_SECONDS)
            {
                sleep_request = 1;
            }
        }
    }
}


