#include "app_221.h"

// LED灯头文件
#include "gd32e23x.h"
#include "cmt2219b_spi.h"
#include "control_config.h"

/* 保留当前接收代码的字段位置 */
#define CONTROL_FRAME_SIZE TEST_PACKET_LEN

#define CONTROL_STOP_INDEX 8U
#define CONTROL_DOWN_INDEX 9U
#define CONTROL_FAST_INDEX 10U
#define CONTROL_UP_INDEX 11U

#define CONTROL_STOP_CODE 0x10U
#define CONTROL_DOWN_CODE 0x20U
#define CONTROL_FAST_CODE 0x30U
#define CONTROL_UP_CODE 0x40U

#ifndef CONTROL_RF_TIMEOUT_MS
#error "Define CONTROL_RF_TIMEOUT_MS from the actual link timing"
#endif

static bool have_valid_frame = false;
static uint32_t last_valid_ms = 0U;

extern void print_buf(uint8_t *buf, uint8_t len);

// 数据接收长度 固定32字节
#define TEST_PACKET_LEN 32
// 数据接收数组
static uint8_t rx_buf[TEST_PACKET_LEN];
// 接收数据芯片初始化标志位
static uint8_t rx_ok;
static uint32_t rx_count = 0;

// 翻转LED灯的标志位
static uint8_t cmd_44_handled = 0;

// 221芯片初始化
void app_221_init(void)
{
    have_valid_frame = false;
    last_valid_ms = 0U;

    cmt2219b_spi_clear_error();
    rx_ok = radio_rx_init();

    if (cmt2219b_spi_has_error())
    {
        rx_ok = CMT2219B_ERROR;
    }

    if (rx_ok == CMT2219B_OK)
        debug_printf("RX init OK\r\n");
    else
        debug_printf("RX init ERROR\r\n");
}

/* 撤销无线普通运动，不自行清除已收到的Stop */
static void wireless_motion_cancel(Ask *request)
{
    request->UP_ask = false;
    request->DOWN_ask = false;
    request->FAST_ask = false;
}

/* 按现有单键/双键编码判断有效；0表示松开 */
static bool decode_key(
    uint8_t value,
    uint8_t code,
    bool *pressed)
{
    if (value == 0U)
    {
        *pressed = false;
        return true;
    }

    if ((value == code) ||
        (value == (uint8_t)(code + 1U)))
    {
        *pressed = true;
        return true;
    }

    return false;
}

// 根据按键内容进行判断
static bool key_press_handle(Ask *next)
{
    Ask parsed = {0};

    if (!decode_key(
            rx_buf[CONTROL_STOP_INDEX],
            CONTROL_STOP_CODE,
            &parsed.STOP_ask))
    {
        return false;
    }

    /* 合法Stop不被普通操作挡住 */
    if (parsed.STOP_ask)
    {
        *next = parsed;
        return true;
    }

    if (!decode_key(
            rx_buf[CONTROL_UP_INDEX],
            CONTROL_UP_CODE,
            &parsed.UP_ask) ||
        !decode_key(
            rx_buf[CONTROL_DOWN_INDEX],
            CONTROL_DOWN_CODE,
            &parsed.DOWN_ask) ||
        !decode_key(
            rx_buf[CONTROL_FAST_INDEX],
            CONTROL_FAST_CODE,
            &parsed.FAST_ask))
    {
        return false;
    }

    *next = parsed;
    return true;
}

// 221接收数据
void app_221_receive_data(Ask *wireless, uint32_t now_ms)
{
    Ask next = {0};
    bool accepted = false;
    uint8_t packet_ready;
    uint8_t resume_ok;

    if (rx_ok != CMT2219B_OK)
    {
        wireless_motion_cancel(wireless);
        return;
    }

    /* 一轮操作开始时清错误，操作过程中错误保持 */
    cmt2219b_spi_clear_error();

    /* 底层现有函数已经检查收包和CRC标志 */
    packet_ready = cmt2219b_packet_received();

    if (cmt2219b_spi_has_error())
    {
        goto receive_failed;
    }

    if (packet_ready)
    {
        if (cmt2219b_go_stby() != CMT2219B_OK)
        {
            goto receive_failed;
        }

        cmt2219b_read_fifo(rx_buf, CONTROL_FRAME_SIZE);

        if (cmt2219b_spi_has_error())
        {
            goto receive_failed;
        }

        accepted = key_press_handle(&next);

        if (accepted && next.STOP_ask)
        {
            /* 后面恢复RX即使失败，也不能丢掉已收到的Stop */
            wireless->STOP_ask = true;
        }

        cmt2219b_clear_rx_fifo();
        cmt2219b_clear_interrupt_flags();

        resume_ok = cmt2219b_go_rx();

        if ((resume_ok != CMT2219B_OK) ||
            cmt2219b_spi_has_error())
        {
            goto receive_failed;
        }

        if (accepted)
        {
            *wireless = next;
            last_valid_ms = now_ms;
            have_valid_frame = true;
        }
    }

    /* 没有新帧时，保留尚未超时的最近有效状态 */
    if (!have_valid_frame ||
        ((uint32_t)(now_ms - last_valid_ms) >=
         CONTROL_RF_TIMEOUT_MS))
    {
        wireless_motion_cancel(wireless);
    }

    return;

receive_failed:
    wireless_motion_cancel(wireless);
    rx_ok = CMT2219B_ERROR;
    debug_printf("RX disabled after IO/state error\r\n");
}
