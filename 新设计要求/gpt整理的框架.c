int main(void)
{
    /* 板级库函数：初始化系统滴答计时 */
    systick_config();

    /*
     * 用户自定义GPIO初始化函数。
     * 现有代码已经包含GPIO时钟开启。
     * 后续需要在该函数内部补上已有的PB6初始化调用。
     */
    wired_all_gpio_init();

    /*
     * BSP函数：初始化串口及DMA。
     * 现有函数内部已经开启对应外设时钟。
     */
    usart_gpio_config(115200);

    /* 上电先设置为急停状态 */
    CURRENT_MODE = stop_model;
    END_REG = STOP_OK;

    /*
     * 执行上电停止。
     * 前提：执行函数的STOP_OK分支已经完善。
     */
    wired_execute_up(END_REG);

    last_printf_tick = sys_tick_ms;

    while (1)
    {
        /* 第1步：采集当前有线输入 */
        wired_read_input(&wired);

        /*
         * 无线暂未接入，沿用当前已清零的wireless对象。
         * 以后在这里增加接收、解析，并更新wireless。
         */

        /*
         * 第2步：选择模式。
         * 急停优先、急停保持和解除条件，
         * 都放在model_choose内部处理。
         */
        CURRENT_MODE = model_choose(wired, wireless);

        /* 第3步：根据当前模式，获取最终动作许可 */
        switch (CURRENT_MODE)
        {
        case sleep_model:
        case wired_model:
        case wireless_model:
        case stop_model:
            END_REG = arbiter_resolve(
                CURRENT_MODE, wired, wireless
            );
            break;

        default:
            /* 模式异常时，停止升降 */
            CURRENT_MODE = stop_model;
            END_REG = STOP_OK;
            break;
        }

        /*
         * 第4步：每轮统一更新实际输出。
         * 上升、下降、无操作、冲突、急停，
         * 都由执行函数明确设置对应输出状态。
         */
        wired_execute_up(END_REG);

        /* 第5步：每1秒打印一次运行计数 */
        if ((sys_tick_ms - last_printf_tick) >= 1000U)
        {
            last_printf_tick = sys_tick_ms;

            /* 项目现有调试宏，底层使用串口DMA发送队列 */
            debug_printf(
                "running times: %lu\r\n",
                (unsigned long)count
            );

            count++;
        }

        /* 本轮结束，重新读取输入并判断 */
    }
}


你好,我是GPT,我已经阅读完次工程的配置文件

**这次把“继电器动作”和“电机运行”明确区分：PA8／PA9／PA10 切换方向、速度接法；KM1 接通电机主电源。继电器吸合，不代表电机已经通电运行。**

以下按元件正常、按原图接线说明：

| 引脚 | 方向 | 适用范围 | 直接作用 | 低电平含义 | 高电平含义 |
|---|---|---|---|---|---|
| **PB3** | 输入 | **无线和有线共用** | 检测经过限位后的上升信号 | 检测到上升信号 | 未检测到上升信号 |
| **PB4** | 输入 | **无线和有线共用** | 检测经过限位后的下降信号 | 检测到下降信号 | 未检测到下降信号 |
| **PB5** | 输入 | **有线专用** | 检测有线快速请求 | 检测到快速信号 | 未检测到快速信号 |
| **PB12** | 输出 | **无线专用** | 提出无线上升请求 | 撤销请求 | 提出请求，仍需经过硬件选择、互锁和限位 |
| **PB13** | 输出 | **无线专用** | 提出无线下降请求 | 撤销请求 | 提出请求，仍需经过硬件选择、互锁和限位 |
| **PA9** | 输出 | **无线和有线共用** | 驱动上升继电器 KA2 | **KA2 吸合，切换上升接法** | KA2 释放 |
| **PA8** | 输出 | **无线和有线共用** | 驱动下降继电器 KA1 | **KA1 吸合，切换下降接法** | KA1 释放 |
| **PA10** | 输出 | **无线和有线共用** | 驱动快速继电器 KA3 | **KA3 吸合，选择快速档** | KA3 释放，退出快速档 |
| **PB6** | 输出 | **无线和有线共用** | 控制主接触器 KM1 的线圈驱动 | **允许 KM1 吸合，但还需要方向回路供电** | **关闭 KM1 线圈驱动** |
| **PB11** | 输出 | **无线和有线共用** | 通过 RLY3 控制操作回路供电 `CTRL_SAFE` | **取消软件禁止；实体急停正常时允许操作回路供电** | **发出软件禁止，要求切断操作回路供电** |

使用这张表时，记住下面三点：

- **电机实际运行：**需要 PB11 低、PB6 低，同时存在有效方向信号、正确的方向继电器状态，且供电和限位条件正常。
- **PB11 或 PB6 任意一个高：**正常电路下 KM1 无法吸合接通电机主电源；但 **PA8／PA9／PA10 控制的继电器仍可能吸合，只是切换了线路，电机没有主电源。**
- **检测信号的区别：**PB11 高会切断操作回路供电，正常情况下 PB3／PB4／PB5 的有效信号随之消失；单独 PB6 高、PB11 低时，这些输入仍可检测操作信号。两者都不切断 MCU 和无线芯片的供电。

**无线快速没有独立请求 GPIO，最终共用 PA10；它的有线优先处理还需落实。**

:chatgpt-content-reference{index="0"}