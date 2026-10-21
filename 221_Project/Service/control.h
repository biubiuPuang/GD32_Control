#ifndef __CONTROL_H__
#define __CONTROL_H__

#include "stdbool.h"
#include "stdio.h"

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
    bool UP_ask;   /* 上升请求 */
    bool DOWN_ask; /* 下降请求 */
    bool STOP_ask; /* 急停请求 */
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
Model_t model_choose(Ask wired, Ask wireless);

// 定义动作许可变量
Arbitrament arbiter_resolve(Model_t model, Ask wired, Ask wireless);

#endif /* __CONTROL_H__ */
