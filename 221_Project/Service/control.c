/* 完整control.c参考文件，供手动整体替换，原工程未改。
 * 已合并周期探测model_choose，以及工程现有的arbiter_resolve和wired_read_input。
 * 依赖control_config.h中实际确定的四项等待时间。 */
#include "control.h"
#include "control_config.h"

static void source_rf_cancel(void)
{
    gpio_bit_reset(GPIOB, GPIO_PIN_12 | GPIO_PIN_13);
}

static void source_rf_apply(Ask request)
{
    /* 调用前必须已经撤请求并完成稳定等待，只允许一个方向。 */
    source_rf_cancel();
    if (request.UP_ask && !request.DOWN_ask)
    {
        gpio_bit_set(GPIOB, GPIO_PIN_12);
    }
    else if (request.DOWN_ask && !request.UP_ask)
    {
        gpio_bit_set(GPIOB, GPIO_PIN_13);
    }
}

Model_t model_choose(Ask wired, Ask wireless,
                     uint32_t now_ms, bool output_stopped,
                     bool *probe_busy)
{
    /* static保存上一圈做到哪一步，不是在main外再增加控制模式。 */
    static Model_t owner = stop_model;
    static bool probing = false;
    /* 本次探测从什么时候开始，用于反馈稳定等待。 */
    static uint32_t probe_start_ms = 0U;

    /* 无线方向请求从什么时候恢复，用于下一次例行探测。 */
    static uint32_t rf_resume_ms = 0U;
    static Ask applied_request = {0};
    bool wired_active;
    bool rf_single_direction;
    bool rf_unchanged;
    bool feedback_opposite;

    *probe_busy = false; /* main总是传入有效地址；所有返回路径均写该标志。 */

    /* 第一优先：Stop随时取消探测，立即禁止操作回路。 */
    if (wired.STOP_ask || wireless.STOP_ask)
    {
        gpio_bit_set(GPIOB, GPIO_PIN_11);
        source_rf_cancel();
        probing = false;
        owner = stop_model;
        return stop_model;
    }

    /* 非Stop取消软件禁止；实体Stop仍由原硬件独立控制。
     * 不要求松开全部按键，也不能一直返回上一圈的stop_model。
     */
    gpio_bit_reset(GPIOB, GPIO_PIN_11);
    if (owner == stop_model)
    {
        owner = sleep_model;
    }

    wired_active = wired.UP_ask || wired.DOWN_ask || wired.FAST_ask;
    rf_single_direction = (wireless.UP_ask != wireless.DOWN_ask);
    rf_unchanged = (wireless.UP_ask == applied_request.UP_ask) &&
                   (wireless.DOWN_ask == applied_request.DOWN_ask) &&
                   (wireless.FAST_ask == applied_request.FAST_ask);
    feedback_opposite = (wireless.UP_ask && wired.DOWN_ask) ||
                        (wireless.DOWN_ask && wired.UP_ask) ||
                        (wired.UP_ask && wired.DOWN_ask);

    if (!probing)
    {
        /* 只有“之前已确认有线”才允许直接走这个保持分支。 */
        if ((owner == wired_model) && wired_active)
        {
            source_rf_cancel();
            return wired_model;
        }

        /* 无线请求未变且未到周期时保持；有效方向仍由后续共用执行判断。 */
        if ((owner == wireless_model) && rf_single_direction && rf_unchanged &&
        !feedback_opposite && !wired.FAST_ask &&
        ((uint32_t)(now_ms - rf_resume_ms) < CONTROL_PROBE_PERIOD_MS))
        {
            return wireless_model;
        }

        /* 仅进入探测时记录一次时间，不能每轮重新计时。
         * 不因探测主动改PB6或PA8/PA9/PA10，也不关闭接收器。
         */
        source_rf_cancel();
        probe_start_ms = now_ms;
        probing = true;
    }

    *probe_busy = true;
    source_rf_cancel();
    if ((uint32_t)(now_ms - probe_start_ms) < CONTROL_RF_SETTLE_MS)
    {
        return owner; /* 等待期间回main，继续收包、检查Stop和共用执行。 */
    }

    /* 当前wired是main本圈刚采集的输入，撤请求已跨过必要的稳定等待。 */
    if (wired_active)
    {
        owner = wired_model;
        probing = false;
        *probe_busy = false;
        return owner;
    }

    /* 无方向时先让wired_order完成正常停止，不能提前重新提出无线请求。 */
    if (!output_stopped)
    {
        return owner;
    }

    if (!rf_single_direction)
    {
        owner = sleep_model;
        probing = false;
        *probe_busy = false;
        return owner; /* 无线FAST不能独立启动。 */
    }

    /* 使用本圈最新请求，不恢复探测前缓存的旧命令。
     * 三路高不证明原始按键全松开；候选请求继续受原硬件门控。
     */
    source_rf_apply(wireless);
    /* 重新提出无线方向请求后，再开始计算保持时间。 */
    rf_resume_ms = now_ms;
    applied_request = wireless;
    owner = wireless_model;
    probing = false;
    *probe_busy = false;
    return owner; /* main后续重新采样，收到有效方向反馈才允许共用驱动启动。 */
}


Arbitrament arbiter_resolve(
    Model_t model,
    Ask wired,
    Ask wireless)
{
    /* 无论原来是什么模式，已知急停请求都优先 */
    if (wired.STOP_ask ||
        wireless.STOP_ask ||
        model == stop_model)
    {
        return STOP_OK;
    }

    /* 待机不执行升降 */
    if (model == sleep_model)
    {
        return IDLE_OK;
    }

    /* 非法模式按急停处理 */
    if ((model != wired_model) &&
        (model != wireless_model))
    {
        return STOP_OK;
    }

    /* 有线、无线最终都需要经过硬件和限位后的反馈 */
    if (wired.UP_ask && wired.DOWN_ask)
    {
        return ERROR_OK;
    }

    if (wired.UP_ask)
    {
        return UP_OK;
    }

    if (wired.DOWN_ask)
    {
        return DOWN_OK;
    }

    return IDLE_OK;
}
/* -----------------------自定义动作裁决许可函数----------------------------- */
/* -----------------------自定义读取按键值判断是否按下函数----------------------------- */
/* 输入采样：并把读取到的按键值填进 wired 请求结构体 */
void wired_read_input(Ask *wired)
{
    /* gpio_input_bit_get()是官方库函数。
     * GPIO时钟和引脚初始化由wired_all_gpio_init()提前完成。
     */

    /* PB3、PB4：有线和无线共用的有效方向反馈 */
    wired->UP_ask =
        (gpio_input_bit_get(GPIOB, GPIO_PIN_3) == RESET);

    wired->DOWN_ask =
        (gpio_input_bit_get(GPIOB, GPIO_PIN_4) == RESET);

    /* PB5：有线专用快速输入 */
    wired->FAST_ask =
        (gpio_input_bit_get(GPIOB, GPIO_PIN_5) == RESET);

    /* 实体急停没有独立输入接到MCU，不能伪造检测值。
     * 实体急停由原硬件直接切断操作回路。
     */
    wired->STOP_ask = false;
}
/* -----------------------自定义读取按键值判断是否按下函数----------------------------- */
