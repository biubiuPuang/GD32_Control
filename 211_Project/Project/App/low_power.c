#include "gd32e23x.h"
#include "low_power.h"
#include "cmt2119b.h"
#include "systick.h"
#include "system_gd32e23x.h"

/*
 * Normal value: 300000 ms = 5 minutes.
 *
 * For initial testing, change this value to 10000U.
 */
#define LOW_POWER_TIMEOUT_MS       (10000U)
#define LOW_POWER_TICK_MS          (10U)

#define LOW_POWER_TIMER_PSC        (7199U)
#define LOW_POWER_TIMER_PERIOD     (99U)

static volatile uint32_t s_tick_ms = 0U;
static volatile uint8_t s_key_wakeup_flag = 0U;
static uint32_t s_last_activity_ms = 0U;

static void low_power_timer13_init(void);
static void low_power_exti_init(void);
static void low_power_clear_exti_flags(void);
static void low_power_enter_deepsleep(void);


/*
 * TIMER13 clock calculation:
 *
 * 72 MHz / (7199 + 1) = 10 kHz
 * 10 kHz / (99 + 1) = 100 Hz
 * Period = 10 ms
 */
static void low_power_timer13_init(void)
{
    timer_parameter_struct timer_initpara;

    rcu_periph_clock_enable(RCU_TIMER13);

    timer_deinit(TIMER13);

    timer_struct_para_init(&timer_initpara);

    timer_initpara.prescaler = LOW_POWER_TIMER_PSC;
    timer_initpara.alignedmode = TIMER_COUNTER_EDGE;
    timer_initpara.counterdirection = TIMER_COUNTER_UP;
    timer_initpara.period = LOW_POWER_TIMER_PERIOD;
    timer_initpara.clockdivision = TIMER_CKDIV_DIV1;
    timer_initpara.repetitioncounter = 0U;

    timer_init(TIMER13, &timer_initpara);

    timer_interrupt_flag_clear(TIMER13, TIMER_INT_FLAG_UP);
    timer_interrupt_enable(TIMER13, TIMER_INT_UP);

    nvic_irq_enable(TIMER13_IRQn, 1U);

    timer_enable(TIMER13);
}


/*
 * Configure EXTI for the key pins that are actually scanned by
 * get_key_num().
 *
 * The key inputs use pull-up resistors and become low when pressed,
 * therefore the falling edge is used as the wake-up trigger.
 */
static void low_power_exti_init(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_CFGCMP);

    syscfg_exti_line_config(EXTI_SOURCE_GPIOB, EXTI_SOURCE_PIN3);
    syscfg_exti_line_config(EXTI_SOURCE_GPIOB, EXTI_SOURCE_PIN4);
    syscfg_exti_line_config(EXTI_SOURCE_GPIOB, EXTI_SOURCE_PIN5);
    syscfg_exti_line_config(EXTI_SOURCE_GPIOB, EXTI_SOURCE_PIN6);

    syscfg_exti_line_config(EXTI_SOURCE_GPIOA, EXTI_SOURCE_PIN9);
    syscfg_exti_line_config(EXTI_SOURCE_GPIOA, EXTI_SOURCE_PIN10);
    syscfg_exti_line_config(EXTI_SOURCE_GPIOA, EXTI_SOURCE_PIN11);
    syscfg_exti_line_config(EXTI_SOURCE_GPIOA, EXTI_SOURCE_PIN12);

    exti_init(EXTI_3, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    exti_init(EXTI_4, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    exti_init(EXTI_5, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    exti_init(EXTI_6, EXTI_INTERRUPT, EXTI_TRIG_FALLING);

    exti_init(EXTI_9, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    exti_init(EXTI_10, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    exti_init(EXTI_11, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    exti_init(EXTI_12, EXTI_INTERRUPT, EXTI_TRIG_FALLING);

    exti_interrupt_enable(EXTI_3);
    exti_interrupt_enable(EXTI_4);
    exti_interrupt_enable(EXTI_5);
    exti_interrupt_enable(EXTI_6);

    exti_interrupt_enable(EXTI_9);
    exti_interrupt_enable(EXTI_10);
    exti_interrupt_enable(EXTI_11);
    exti_interrupt_enable(EXTI_12);

    low_power_clear_exti_flags();

    nvic_irq_enable(EXTI2_3_IRQn, 0U);
    nvic_irq_enable(EXTI4_15_IRQn, 0U);
}


static void low_power_clear_exti_flags(void)
{
    exti_interrupt_flag_clear(EXTI_3);
    exti_interrupt_flag_clear(EXTI_4);
    exti_interrupt_flag_clear(EXTI_5);
    exti_interrupt_flag_clear(EXTI_6);

    exti_interrupt_flag_clear(EXTI_9);
    exti_interrupt_flag_clear(EXTI_10);
    exti_interrupt_flag_clear(EXTI_11);
    exti_interrupt_flag_clear(EXTI_12);
}


void low_power_init(void)
{
    rcu_periph_clock_enable(RCU_PMU);

    low_power_exti_init();
    low_power_timer13_init();

    s_tick_ms = 0U;
    s_last_activity_ms = 0U;
    s_key_wakeup_flag = 0U;
}


void low_power_note_activity(void)
{
    s_last_activity_ms = s_tick_ms;
}


void low_power_timer13_irq_handler(void)
{
    if (SET == timer_interrupt_flag_get(TIMER13, TIMER_INT_FLAG_UP)) {
        timer_interrupt_flag_clear(TIMER13, TIMER_INT_FLAG_UP);

        s_tick_ms += LOW_POWER_TICK_MS;
    }
}


void low_power_key_exti_handler(void)
{
    s_key_wakeup_flag = 1U;
}


void low_power_process(void)
{
    /*
     * An EXTI event is also treated as activity. This is useful when
     * the key edge occurs before the polling code reads the key state.
     */
    if (s_key_wakeup_flag != 0U) {
        s_key_wakeup_flag = 0U;
        low_power_note_activity();
    }

    /*
     * Unsigned subtraction is intentional. It remains correct when
     * the 32-bit tick counter wraps around.
     */
    if ((uint32_t)(s_tick_ms - s_last_activity_ms)
        >= LOW_POWER_TIMEOUT_MS) {
        debug_printf("Enter Low Power Mode\r\n");
        delay_ms(10);
        low_power_enter_deepsleep();
    }
}


static void low_power_enter_deepsleep(void)
{
    /*
     * Stop the low-power timer before entering Deep-sleep.
     */
    timer_disable(TIMER13);
    timer_interrupt_flag_clear(TIMER13, TIMER_INT_FLAG_UP);

    /*
     * The radio has its own sleep command. The radio should be put
     * into sleep before the MCU enters Deep-sleep.
     */
    cmt2119b_go_sleep();

    /*
     * The existing project uses SysTick for delay timing. Stop it
     * explicitly so it cannot wake the MCU periodically.
     */
    SysTick->CTRL = 0U;

    /*
     * Do not clear EXTI flags here. If a new key edge happens between
     * this point and WFI, the pending EXTI flag must remain available
     * as the wake-up source.
     */
    pmu_to_deepsleepmode(PMU_LDO_LOWPOWER, WFI_CMD);

    /*
     * Execution continues here after a wake-up interrupt.
     * Deep-sleep wake-up may leave the system clock in a different
     * state, so recover the 72 MHz clock before using delay_ms().
     */
    system_clock_recover();
    systick_config();

    low_power_clear_exti_flags();
    low_power_timer13_init();

    s_last_activity_ms = s_tick_ms;
    s_key_wakeup_flag = 0U;

    /*
     * get_key_num() already uses software debounce. This extra delay
     * gives the key level time to stabilize after wake-up.
     */
    delay_ms(20U);
}


