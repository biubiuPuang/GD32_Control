#include "main.h"

// 有线请求定义
Ask wired = {0};
// 无线请求定义
Ask wireless = {0};
// 最终获得的动作许可定义
Arbitrament END_REG;

// 定义一个表示当前模式状态的标志位 且默认为有线模式
static Model_t CURRENT_MODE = wired_model;

int main(void)
{
    // 初始化时钟配置
    systick_config();
    // 初始化有线部分的所有GPIO引脚
    wired_all_gpio_init();
    // 初始化串口且配置默认波特率为115200
    usart_gpio_config(115200);

    // 默认就是急急停模式,且停止升降操作
    CURRENT_MODE = stop_model;
    END_REG = STOP_OK;
    wired_order(END_REG);

    while (1)
    {
        // 读取当前按键值的状态
        wired_read_input(&wired);

        // 根据按键值返回判断更新当前最新模式
        CURRENT_MODE = model_choose(wired, wireless);

        switch (CURRENT_MODE)
        {
        case sleep_model:
            // 获取睡眠模式的动作许可
            END_REG = arbiter_resolve(CURRENT_MODE, wired, wireless);
            break;
        case wired_model:
            // 获取有线模式的动作许可
            END_REG = arbiter_resolve(CURRENT_MODE, wired, wireless);
            break;
        case wireless_model:
            // 获取无线模式的动作许可
            END_REG = arbiter_resolve(CURRENT_MODE, wired, wireless);
            break;
        case stop_model:
            // 获取急停模式的动作许可
            END_REG = arbiter_resolve(CURRENT_MODE, wired, wireless);
            break;
        default:
            printf("ERROR:cant input any model\r\n");
            break;
        }

        // 根据动作许可执行对应的动作
        wired_order(END_REG);

    }
}

// /* =======================原项目的无线main函数代码============================= */

// // 假装(代码实现)上位机串口发送配置文件参数头文件
// #include "rf_apply.h"
// // (真实的串口发送)上位机串口发送配置文件参数头文件
// #include "rf_uart_set_config.h"
// // LED灯测试头文件
// #include "led_test.h"
// // 按键头文件
// #include "app_key.h"

// uint8_t key_num = 0;

// int main(void)
// {
//     // LED灯引脚初始化
//     pa15_led_init();
//     // PB9板载LED灯点亮
//     pb9_led_init();
//     // 发送芯片初始化
//     // app_211_init();
//     // delay_ms(10);
//     // 接收芯片初始化
//     app_221_init();
//     delay_ms(10);
//     // 按键初始化
//     key_init();

//     // 假装串口发送配置参数数据
//     // 尝试从 Flash 读取之前保存的 TX/RX 配置
//     // debug_printf("=== [DBG] before rf_uart_config_restore ===\r\n");
//     if (!rf_uart_config_restore())
//     {
//         debug_printf("=== [DBG] flash invalid, apply default ===\r\n");
//         // flash参数无效 配置默认参数到收发芯片  fh_offset  = 40;  fh_channel = 16;  channel_count = 100;
//         rf_test_apply_config();
//         debug_printf("=== [DBG] rf_test_apply_config done ===\r\n");
//         /* 读取并打印收发芯片当前真实寄存器配置 */
//         // 通过 SPI 读取 211 和 221 芯片的实际寄存器
//         rf_print_tx_rx_real_freq();
//         debug_printf("=== [DBG] rf_print_tx_rx_real_freq done ===\r\n");
//         delay_ms(10);
//     }
//     // debug_printf("=== [DBG] config phase done, entering main loop ===\r\n");

//     while (1)
//     {
//         // 这个函数用于处理串口接收到的射频配置命令
//         rf_uart_config_process();

//         // 获取按键值
//         get_key_num();

//         // 221芯片接收数据
//         app_221_receive_data();
//     }
// }
// /* =======================原项目的无线main函数代码============================= */

// /* =======================WorkBuddy给出的状态机思路============================= */

// /* ===== control/arbiter/arbiter.h ===== */
// #include <stdint.h>
// #include <stdbool.h>

// /* 无线端解码出来的原始意图（协议层给的） */
// typedef struct
// {
//     bool valid; /* 帧是否合法（CRC/超时由协议层判） */
//     bool up;
//     bool down;
//     bool fast;  /* V1 阶段无线 FAST 禁用 */
//     bool estop; /* 无线急停 */
// } RemoteIntent;

// /* 板子上能看到的输入（PB3/4/5 采样结果） */
// typedef struct
// {
//     bool lift_active;  /* PB3：上限位回路有电 */
//     bool lower_active; /* PB4：下限位回路有电 */
//     bool wired_fast;   /* PB5：有线快速键 */
// } ObservedInputs;

// /* 监督层给的许可结论 */
// typedef struct
// {
//     bool permit_ok; /* false = 失联/故障/急停，全部清零 */
// } PermitDecision;

// /* 仲裁输出：给状态机的"干净命令" */
// typedef enum
// {
//     ARB_OK = 0,
//     ARB_CONFLICT,    /* UP+DN 同帧 */
//     ARB_NO_PERMIT,   /* 无许可 */
//     ARB_FAST_BLOCKED /* 无线 FAST 被禁用 */
// } ArbResult;

// typedef struct
// {
//     bool radio_up;
//     bool radio_down;
//     bool radio_fast; /* V1 恒为 0 */
//     bool permit_req; /* 是否需要拉低 PB11 申请运行 */
// } EffectiveRequest;

// /* ===== control/arbiter/arbiter.c ===== */

// /* 用户自定义函数：仲裁核心，纯函数，不读硬件不写寄存器 */
// ArbResult arbiter_resolve(const RemoteIntent *it,
//                           const ObservedInputs *in,
//                           const PermitDecision *pm,
//                           EffectiveRequest *out)
// {
//     /* 默认全清零 = 什么都不动（安全默认） */
//     out->radio_up = false;
//     out->radio_down = false;
//     out->radio_fast = false;
//     out->permit_req = false;
//     (void)in;

//     /* 规则1：无线急停或无许可 → 全清零 */
//     if (it->estop || !pm->permit_ok || !it->valid)
//         return ARB_NO_PERMIT;

//     /* 规则2：UP+DN 同帧 → 双向清零，报冲突 */
//     if (it->up && it->down)
//         return ARB_CONFLICT;

//     /* 规则3：FAST 单独有效 → 不许动（快速不能独立产生运动） */
//     if (it->fast && !it->up && !it->down)
//         return ARB_OK; /* 无命令，但不算故障 */

//     /* 规则4：V1 无线 FAST 一律禁用 */
//     if (it->fast)
//         return ARB_FAST_BLOCKED;

//     /* 规则5：合法单方向 → 放行 */
//     out->radio_up = it->up;
//     out->radio_down = it->down;
//     out->permit_req = true; /* 通知输出层：这次要拉低 PB11 */
//     return ARB_OK;
// }

// /* =======================WorkBuddy给出的状态机思路============================= */
