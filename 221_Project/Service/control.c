#include "control.h"

/* -----------------------自定义模式选择函数----------------------------- */
/* 自定义仲裁机制函数 输出干净命令*/
/**
 * @brief
 *
 * @param wired 传入有线请求结构体
 * @param wireless 传入有线请求结构体
 * @return Model_t 返回当前模式
 */
Model_t model_choose(Ask wired, Ask wireless)
{
    // 定义一个表示模式的局部变量
    Model_t flag_model;
    /*判断当前的模式 */
    if ((wired.UP_ask || wired.DOWN_ask || wired.STOP_ask) == 1)
    {
        // 有线模式
        flag_model = wired_model;
    }
    else if ((wireless.UP_ask || wireless.DOWN_ask || wireless.STOP_ask) == 1)
    {
        // 无线模式
        flag_model = wireless_model;
    }
    // 保留暂时不写 判定低功耗模式的条件
    // else if ()
    // {

    // }
    else if (wired.STOP_ask || wireless.STOP_ask)
    {
        // 急停模式
        flag_model = stop_model;
    }
    return flag_model;
}
/* -----------------------自定义模式选择函数----------------------------- */


/* -----------------------自定义动作裁决许可函数----------------------------- */
// 定义动作许可变量
/**
 * @brief 
 * 
 * @param model 当前模式
 * @param wired 有线请求枚举
 * @param wireless 无线请求枚举
 * @return Arbitrament 动作许可
 */
Arbitrament arbiter_resolve(Model_t model, Ask wired, Ask wireless)
{
    // 定义一个保存当前模式的变量
    Model_t arbiter_model = model;
    // 判断当前是什么模式
    switch (arbiter_model)
    {
    // 有线模式
    case wired_model:
        if ((wired.UP_ask == 1) && (wired.DOWN_ask == 0) && (wired.STOP_ask == 0) == 1)
        {
            // 返回上升许可
            return UP_OK;
        }
        else if ((wired.UP_ask == 0) && (wired.DOWN_ask == 1) && (wired.STOP_ask == 0) == 1)
        {
            // 返回上升许可
            return DOWN_OK;
        }
        else if ((wired.UP_ask == 1) && (wired.DOWN_ask == 1) && (wired.STOP_ask == 0) == 1)
        {
            // 返回冲突许可
            return ERROR_OK;
        }
        else if (wired.STOP_ask == 1)
        {
            // 返回急停许可
            return STOP_OK;
        }
        break;
    // 无线模式
    case wireless_model:
        if ((wireless.UP_ask == 1) && (wireless.DOWN_ask == 0) && (wireless.STOP_ask == 0) == 1)
        {
            // 返回上升许可
            return UP_OK;
        }
        else if ((wireless.UP_ask == 0) && (wireless.DOWN_ask == 1) && (wireless.STOP_ask == 0) == 1)
        {
            // 返回上升许可
            return DOWN_OK;
        }
        else if ((wireless.UP_ask == 1) && (wireless.DOWN_ask == 1) && (wireless.STOP_ask == 0) == 1)
        {
            // 返回冲突许可
            return ERROR_OK;
        }
        else if (wireless.STOP_ask == 1)
        {
            // 返回急停许可
            return STOP_OK;
        }
        break;
    // 急停模式
    case stop_model:
        /* code */
        printf("current is stop mode ");
        break;
    }
}
/* -----------------------自定义动作裁决许可函数----------------------------- */


