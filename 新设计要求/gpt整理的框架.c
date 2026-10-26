#include "main.h"
#include "control_time.h" /* 建议新增的计时封装头文件 */

/* 沿用原来的结构体 */
Ask wired = {0};
Ask wireless = {0};

Arbitrament END_REG = STOP_OK;
static Model_t CURRENT_MODE = stop_model;

/* 当前希望使用的快速状态 */
static bool FAST_TARGET = false;

/* 是否正在等待来源检测完成 */
static bool PROBE_BUSY = false;

/* 软件停止步骤是否完成，不代表实际测到了电机停转 */
static bool OUTPUT_STOPPED = false;

int main(void)
{
    uint32_t now_ms;

    /* 现有BSP函数：保留原驱动需要的延时支持 */
    systick_config();

    /* 现有用户BSP函数：内部开启GPIO时钟并初始化
     * 需要补充PB12、PB13初始化，默认撤销无线方向请求
     */
    wired_all_gpio_init();

    /* 现有用户BSP函数：内部完成串口时钟和初始化 */
    usart_gpio_config(115200);

    /* 新增用户函数：开启定时器时钟，建立连续毫秒计时 */
    control_time_init();

    /* 修改后的用户执行函数：上电先停止 */
    OUTPUT_STOPPED = wired_order(
        STOP_OK, false, false, control_now_ms()
    );

    /* 现有用户函数：初始化无线接收
     * 必须先修正接收驱动的软件引脚配置，见下文
     */
    app_221_init();

    while (1)
    {
        /* 第1步：接收无线数据
         * 修改现有函数，把收到的请求填入wireless
         * 没有新数据就返回，不能停在这里等待
         */
        app_221_receive_data(&wireless, control_now_ms());

        now_ms = control_now_ms();

        /* 第2步：读取PB3、PB4、PB5
         * UP/DOWN是共用方向反馈，FAST是有线快速输入
         */
        wired_read_input(&wired);

        /* 第3步：判断模式，同时推进来源检测
         * 急停最优先
         * 已确认有线时，不周期打断有线输出
         * 无线控制时，到检测时间便撤销无线方向请求，
         * 等信号稳定后再判断有线是否正在操作
         */
        CURRENT_MODE = model_choose(
            wired,
            wireless,
            now_ms,
            OUTPUT_STOPPED,
            &PROBE_BUSY
        );

        /* 第4步：模式函数可能改变了无线请求输出，
         * 所以重新读取共用反馈
         * 硬件等待是否完成，仍由PROBE_BUSY约束
         */
        wired_read_input(&wired);

        /* 第5步：裁决方向
         * 急停返回STOP_OK
         * 没方向返回IDLE_OK，方向冲突返回ERROR_OK
         * 升降许可必须结合PB3/PB4有效反馈
         */
        END_REG = arbiter_resolve(
            CURRENT_MODE, wired, wireless
        );

        /* 第6步：选择同一控制来源的快速请求 */
        if ((END_REG != UP_OK) && (END_REG != DOWN_OK))
        {
            FAST_TARGET = false;
        }
        else if (!PROBE_BUSY)
        {
            if (CURRENT_MODE == wired_model)
                FAST_TARGET = wired.FAST_ask;
            else if (CURRENT_MODE == wireless_model)
                FAST_TARGET = wireless.FAST_ask;
            else
                FAST_TARGET = false;
        }

        /* 检测未结束时，暂不更新快速目标。
         * 能否保持原输出，由执行函数结合方向反馈决定。
         */

        /* 第7步：推进继电器的停止、切换或保持
         * 每圈都执行，检测等待期间也不能跳过
         * 急停、无方向和冲突必须优先处理
         */
        OUTPUT_STOPPED = wired_order(
            END_REG,
            FAST_TARGET,
            PROBE_BUSY,
            control_now_ms()
        );

        /* 不加delay_ms(100)
         * 100ms是来源检测的启动间隔，不是主循环延时
         */
    }
}