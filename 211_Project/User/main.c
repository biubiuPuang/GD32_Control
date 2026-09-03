#include "gd32e23x.h"
#include "systick.h"
#include "bsp_usart.h"
#include "radio_tx.h"
#include "radio_rx.h"
#include "cmt2219b.h"
#include <stdio.h>
#include <stdint.h>
#include "app_211.h"
#include "app_221.h"
#include "app_tx-rx_config.h"
#include "packet_loss_test.h"
#include "cmt2119b.h"
#include "gd32e23x_timer.h"
#include "gd32e23x_pmu.h"
#include "gd32e23x_exti.h"
#include "low_power_config.h"

// 假装(代码实现)上位机串口发送配置文件参数头文件
#include "rf_apply.h"
// (真实的串口发送)上位机串口发送配置文件参数头文件
#include "rf_uart_set_config.h"
// LED灯测试头文件
#include "led_test.h"
// 按键头文件
#include "app_key.h"

uint8_t key_num = 0;
uint8_t last_key_num = 0;

/* 低功耗相关变量 */
volatile uint32_t inactivity_seconds = 0;
volatile uint8_t sleep_request = 0;
volatile uint8_t low_power_mode = 0;


// 按键
static uint8_t is_double_to_single(uint8_t last, uint8_t cur)
{
    return (last == 11 && cur == 10) ||
           (last == 21 && cur == 20) ||
           (last == 31 && cur == 30) ||
           (last == 41 && cur == 40);
}

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

int main(void)
{
    // LED灯引脚初始化
    pa15_led_init();
    // PB9板载LED灯点亮
    pb9_led_init();
    // 发送芯片初始化
    app_211_init();
    delay_ms(5);
    // 按键初始化
    key_init();

    // 低功耗 1 秒计时器初始化
    low_power_timer_init();

    // 假装串口发送配置参数数据
    // 尝试从 Flash 读取之前保存的 TX/RX 配置
    // debug_printf("=== [DBG] before rf_uart_config_restore ===\r\n");
    if (!rf_uart_config_restore())
    {
        debug_printf("=== [DBG] flash invalid, apply default ===\r\n");
        // flash参数无效 配置默认参数到收发芯片  fh_offset  = 40;  fh_channel = 16;  channel_count = 100;
        rf_test_apply_config();
        debug_printf("=== [DBG] rf_test_apply_config done ===\r\n");
        /* 读取并打印收发芯片当前真实寄存器配置 */
        // 通过 SPI 读取 211 和 221 芯片的实际寄存器
        rf_print_tx_rx_real_freq();
        debug_printf("=== [DBG] rf_print_tx_rx_real_freq done ===\r\n");
        delay_ms(10);
    }
    // debug_printf("=== [DBG] config phase done, entering main loop ===\r\n");

    while (1)
    {
        /*
         * TIMER2 达到 5 分钟后提出休眠请求。
         * 只在主循环中真正进入 Sleep。
         */
        if (sleep_request != 0U)
        {
            enter_low_power_mode();
        }

        /*
         * 正常工作期间处理串口配置。
         * 进入 Sleep 后主循环会暂停，因此不会处理串口。
         */
        rf_uart_config_process();

        /* 获取按键值 */
        key_num = get_key_num();

        /*
         * 只要按键状态发生变化，就认为发生了用户活动。
         * 包括：
         * 1. 无按键 -> 一级
         * 2. 无按键 -> 二级
         * 3. 有按键 -> 无按键
         *
         * get_key_num() 在“有按键 -> 无按键”时已经清空
         * tx_buf_data[8] 到 tx_buf_data[11]。
         */
        if (key_num != last_key_num)
        {
            inactivity_seconds = 0;
        }

        /*
         * 保留现有的二级退回一级逻辑。
         */
        if (is_double_to_single(last_key_num, key_num))
        {
            last_key_num = key_num;
            continue;
        }

        /*
         * 只有按键值变化时发送一次。
         * 无按键时不再在 get_key_num() 内重复发送。
         */
        if (key_num != last_key_num)
        {
            app_211_send_test_data();
        }

        last_key_num = key_num;
    }
}

