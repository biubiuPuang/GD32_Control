#ifndef RF_UART_SET_CONFIG_H
#define RF_UART_SET_CONFIG_H

#include <stdint.h>

/*
 * 启动时读取并应用Flash配置。
 * 返回1：Flash配置有效并应用成功。
 * 返回0：Flash中没有有效配置，应使用原来的默认配置。
 */
uint8_t rf_uart_config_restore(void);

/*
 * 在主循环中调用。
 * 没收到完整命令时直接返回；
 * 收到命令后完成解析、保存和应用。
 */
void rf_uart_config_process(void);

#endif /* RF_UART_SET_CONFIG_H */
