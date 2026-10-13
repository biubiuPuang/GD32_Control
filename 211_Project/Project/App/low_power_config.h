#ifndef LOW_POWER_CONFIG_H
#define LOW_POWER_CONFIG_H

#include <stdint.h>

/* 超过多少秒没有按键动作后进入休眠 */
#define LOW_POWER_TIMEOUT_SECONDS 10U

/* 初始化低功耗使用的 TIMER2，1 秒进入一次中断 */
void low_power_init(void);

/* 用户发生按键动作时调用，重新开始计算空闲时间 */
void low_power_activity_notify(void);

/* 主循环周期调用：收到休眠请求时，真正进入 Sleep */
void low_power_process(void);

/* TIMER2 中断中调用：每秒累计一次空闲时间 */
void low_power_timer_irq_handler(void);

#endif
