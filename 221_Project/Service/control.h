#ifndef __CONTROL_H__
#define __CONTROL_H__

#include "stdbool.h"
#include "stdio.h"
#include "gd32e23x.h"

typedef enum
{
    sleep_model,    /* 睡眠模式 */
    wired_model,    /* 有线模式 */
    wireless_model, /* 无线模式 */
    stop_model      /* 急停模式 */
} Model_t;

/* 请求输出：判断当前的动作请求 */
typedef struct
{
    bool UP_ask;   /* 上升请求或共用上升反馈 */
    bool DOWN_ask; /* 下降请求或共用下降反馈 */
    bool STOP_ask; /* 急停请求 */
    bool FAST_ask; /* 快速请求 */
} Ask;

/* 仲裁输出：给状态机的"干净命令" */
typedef enum
{
    UP_OK = 0, /* 上升许可 */
    DOWN_OK,   /* 下降许可 */
    IDLE_OK,   /* 无许可 */
    ERROR_OK,  /* 许可冲突(有线跟无线的冲突\上升跟下降的冲突)*/
    STOP_OK    /* 急停许可 */
} Arbitrament;

/* 自定义仲裁机制函数 输出干净命令*/
Model_t model_choose(
    Ask wired,
    Ask wireless,
    uint32_t now_ms,       /* 当前毫秒时间 */
    bool output_stopped,  /* 软件停止步骤是否完成 */
    bool *probe_busy      /* 返回：来源检测是否尚未完成 */
);

// 定义动作许可变量
Arbitrament arbiter_resolve(Model_t model, Ask wired, Ask wireless);

/* 输入采样：并把读取到的按键值填进 wired 请求结构体 */
void wired_read_input(Ask *wired);

#endif /* __CONTROL_H__ */
