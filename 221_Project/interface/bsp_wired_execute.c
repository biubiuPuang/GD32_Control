/* 建议替换原bsp_wired_execute.c的内容，不把两个同名实现同时加入工程。
 * 用户自定义执行函数；gpio_bit_set/reset来自GD32官方库。
 * 正常PB11许可由model_choose管理；这里在Stop时立即发出禁止。
 * 等待时间来自control_config.h，尚需实际系统确认，未做硬件验证。
 */
#include "bsp_wired_execute.h"
#include "control_config.h"

/* 私有执行步骤，记录继电器操作进行到哪里，不增加Model_t工作模式。 */
typedef enum
{
    EXEC_STOP_KM1,
    EXEC_STOP_RELAYS,
    EXEC_IDLE,
    EXEC_PREPARE,
    EXEC_RUN
} ExecuteStep;

static void execute_relays_release(void)
{
    gpio_bit_set(GPIOA, GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10);
}

static void execute_relays_prepare(Arbitrament direction, bool fast)
{
    /* 调用时KM1许可已撤销且旧接法已退出。 */
    execute_relays_release();
    if (direction == UP_OK)
    {
        gpio_bit_reset(GPIOA, GPIO_PIN_9);
    }
    else
    {
        gpio_bit_reset(GPIOA, GPIO_PIN_8);
    }
    if (fast)
    {
        gpio_bit_reset(GPIOA, GPIO_PIN_10);
    }
}

bool wired_order(Arbitrament action, bool fast_target,
                 bool probe_busy, uint32_t now_ms)
{
    static bool initialized = false;
    static ExecuteStep step = EXEC_STOP_KM1;
    static uint32_t step_start_ms = 0U;
    static Arbitrament saved_direction = IDLE_OK;
    static bool saved_fast = false;
    bool motion;
    bool force_stop;

    motion = (action == UP_OK) || (action == DOWN_OK);
    force_stop = (action == STOP_OK) ||
                 (!motion && (action != IDLE_OK) && (action != ERROR_OK));

    /* Stop/非法命令先执行禁止，不等待原来的准备/保持步骤。 */
    if (force_stop)
    {
        gpio_bit_set(GPIOB, GPIO_PIN_11 | GPIO_PIN_6);
        gpio_bit_reset(GPIOB, GPIO_PIN_12 | GPIO_PIN_13);
    }

    /* 上电第一次同样需要记录一次停止开始时间。 */
    if (!initialized)
    {
        initialized = true;
        gpio_bit_set(GPIOB, GPIO_PIN_6);
        step = EXEC_STOP_KM1;
        step_start_ms = now_ms;
    }

    /* 停止已经开始时继续做完，不因下一圈又收到同一命令而重置时间。 */
    if (step == EXEC_STOP_KM1)
    {
        gpio_bit_set(GPIOB, GPIO_PIN_6);
        if ((uint32_t)(now_ms - step_start_ms) >= CONTROL_KM1_OFF_MS)
        {
            execute_relays_release();
            step = EXEC_STOP_RELAYS;
            step_start_ms = now_ms;
        }
        return false;
    }
    if (step == EXEC_STOP_RELAYS)
    {
        gpio_bit_set(GPIOB, GPIO_PIN_6);
        if ((uint32_t)(now_ms - step_start_ms) >= CONTROL_RELAY_RELEASE_MS)
        {
            step = EXEC_IDLE;
            saved_direction = IDLE_OK;
            saved_fast = false;
            return true;
        }
        return false;
    }

    /* 无方向、冲突、Stop，或探测时方向改变：按正常步骤停止。
     * 探测期间只允许保留RUN状态下已经建立的同方向输出。
     */
    if (force_stop || !motion ||
        (probe_busy && !((step == EXEC_RUN) && (action == saved_direction))))
    {
        gpio_bit_set(GPIOB, GPIO_PIN_6);
        if (step == EXEC_IDLE)
        {
            execute_relays_release();
            return true;
        }
        step = EXEC_STOP_KM1;
        step_start_ms = now_ms;
        return false;
    }

    if (probe_busy)
    {
        /* 不重写已有接法，不根据未确认来源新开FAST。 */
        return false;
    }

    if (step == EXEC_RUN)
    {
        if ((action == saved_direction) && (fast_target == saved_fast))
        {
            return false; /* 目标没变就保持，已确认有线不会周期停机。 */
        }
        /* 真正换向/换速时先撤KM1许可，下一圈开始继续停止步骤。 */
        gpio_bit_set(GPIOB, GPIO_PIN_6);
        step = EXEC_STOP_KM1;
        step_start_ms = now_ms;
        return false;
    }

    if (step == EXEC_IDLE)
    {
        gpio_bit_set(GPIOB, GPIO_PIN_6);
        execute_relays_prepare(action, fast_target);
        saved_direction = action;
        saved_fast = fast_target;
        step = EXEC_PREPARE;
        step_start_ms = now_ms;
        return false;
    }

    /* EXEC_PREPARE：准备途中目标变化也先正常退出，不重新带电切换。 */
    if ((action != saved_direction) || (fast_target != saved_fast))
    {
        gpio_bit_set(GPIOB, GPIO_PIN_6);
        step = EXEC_STOP_KM1;
        step_start_ms = now_ms;
        return false;
    }
    if ((uint32_t)(now_ms - step_start_ms) >= CONTROL_RELAY_PREPARE_MS)
    {
        gpio_bit_reset(GPIOB, GPIO_PIN_6);
        step = EXEC_RUN;
    }
    return false;
}
