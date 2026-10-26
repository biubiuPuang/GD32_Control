#ifndef CONTROL_CONFIG_H
#define CONTROL_CONFIG_H

/* 建议新增的参数头文件。不是当前工程中的现成文件。
 * 100ms来自已确认方案。其余时间必须依据实际控制回路确定。
 * 下方#error用于阻止把尚未确定的等待时间当成完整固件使用。
 * 在本文件中补齐四个#define，或从工程编译选项传入它们。
 */
/* 无线请求恢复后，保持100ms，再发起下一次例行探测。 */
#define CONTROL_PROBE_PERIOD_MS       100U

/* 撤销无线方向请求后，等待反馈稳定。 */
#define CONTROL_RF_SETTLE_MS           20U

/* 撤销KM1通电许可后，等待主接触器断开。 */
#define CONTROL_KM1_OFF_MS              50U

/* 退出旧方向、旧快速接法后，等待继电器释放。 */
#define CONTROL_RELAY_RELEASE_MS        30U

/* 新方向、快速接法设置后，等待继电器动作完成。 */
#define CONTROL_RELAY_PREPARE_MS        30U

/* 连续300ms没有收到有效无线数据，撤销无线普通操作。 */
#define CONTROL_RF_TIMEOUT_MS          300U

/* CONTROL_RF_SETTLE_MS：撤销无线方向请求后，反馈稳定所需等待。
 * 恢复PB11操作许可后的电路建立时间也必须纳入首次确认。
 * CONTROL_KM1_OFF_MS：PB6高后，到允许退出方向/速度接法的等待。
 * CONTROL_RELAY_RELEASE_MS：PA8/9/10高后，到允许准备新目标的等待。
 * 还须涵盖原系统需要的换向间隔，不能以此声称测到了电机停转。
 * CONTROL_RELAY_PREPARE_MS：新方向/速度接法设好后，到允许PB6低的等待。
 */
#ifndef CONTROL_RF_SETTLE_MS
#error "Define CONTROL_RF_SETTLE_MS from the actual control circuit timing"
#endif
#ifndef CONTROL_KM1_OFF_MS
#error "Define CONTROL_KM1_OFF_MS from the actual drive timing"
#endif
#ifndef CONTROL_RELAY_RELEASE_MS
#error "Define CONTROL_RELAY_RELEASE_MS from the actual drive timing"
#endif
#ifndef CONTROL_RELAY_PREPARE_MS
#error "Define CONTROL_RELAY_PREPARE_MS from the actual drive timing"
#endif

#endif
