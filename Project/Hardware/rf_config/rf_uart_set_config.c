#include "rf_uart_set_config.h"


// 芯片Flash内部最后一页起始页地址(用于存储收发chip配置参数)
#define RF_FLASH_PAGE_ADDR       0x0800FC00UL
// Flash配置的识别标志
#define RF_FLASH_MAGIC           0x52464643UL
// Flash配置的识别标志
#define RF_FLASH_VERSION         1U
// 串口配置命令的最大长度为64字节，防止接收缓冲区越界
#define RF_UART_COMMAND_MAX      64U

// 收发命令标志位
/*
    RF_TARGET_TX：只配置发射芯片。
    RF_TARGET_RX：只配置接收芯片。
    RF_TARGET_BOTH：同时配置发射和接收芯片。
*/
#define RF_TARGET_TX             1U
#define RF_TARGET_RX             2U
#define RF_TARGET_BOTH           3U

typedef struct
{
    uint32_t magic; // 配置识别标志。读取Flash时，用它判断数据是不是本程序保存的射频配置。
    uint16_t version; // 配置格式版本号。用于避免以后结构体格式变化后错误读取旧数据。
    uint16_t data_size; // 记录整个 rf_flash_config_t 的大小。读取时用于检查Flash数据长度是否正确。

    rf_factory_config_t tx; // 发射芯片的配置。
    rf_factory_config_t rx; // 接收芯片的配置。

    uint16_t crc16; // 整个结构体的CRC16校验值。读取Flash后重新计算CRC，与它比较，判断配置是否损坏。
    uint16_t reserved; // 保留字段，目前没有实际功能，预留给以后扩展，同时有助于保持结构体按4字节对齐。
} rf_flash_config_t;

static rf_flash_config_t g_rf_saved_config;


/**
 * @brief 这个函数用于生成一份TX或RX的射频配置数据。
 * 
 * @param cfg 存放生成后的配置。
 * @param role 配置对象，TX或RX。
 * @param offset 跳频间隔参数 fh_offset
 * @param channel 当前频道 fh_channel
 * @param count 总频道数 channel_count
 */
static void rf_build_one_config(rf_factory_config_t *cfg,
                                uint8_t role,
                                uint8_t offset,
                                uint8_t channel,
                                uint8_t count)
{
    // 先把整个配置结构体清零，防止残留数据影响CRC校验。
    memset(cfg, 0, sizeof(*cfg));

    cfg->magic = RF_CONFIG_MAGIC;
    cfg->version = RF_CONFIG_VERSION;
    cfg->pair_id = 17U;
    cfg->role = role;
    cfg->fh_offset = offset;
    cfg->fh_channel = channel;
    cfg->channel_count = count;

    // 计算CRC前，先把CRC字段清零，避免旧CRC参与本次计算。
    cfg->crc16 = 0U;
    // 对整个配置结构体计算CRC16，并将结果保存到 crc16 中，之后可以用它检查配置数据是否损坏。
    cfg->crc16 = rf_crc16_calc((const uint8_t *)cfg, sizeof(*cfg));
}

/**
 * @brief 这个函数用于生成一份默认的TX和RX配置，但它本身不会把配置写入Flash。
 * 
 * @param record 用于存放生成的完整默认配置
 */
static void rf_build_default_flash_config(rf_flash_config_t *record)
{
    // 将整个配置结构体清零，避免残留数据影响配置和CRC校验。
    memset(record, 0, sizeof(*record));

    record->magic = RF_FLASH_MAGIC;
    record->version = RF_FLASH_VERSION;
    record->data_size = sizeof(*record);

    rf_build_one_config(&record->tx, RF_ROLE_TX, 40U, 16U, 100U);
    rf_build_one_config(&record->rx, RF_ROLE_RX, 40U, 16U, 100U);

    record->crc16 = 0U;
    record->crc16 =
        rf_crc16_calc((const uint8_t *)record, sizeof(*record));
}
/**
 * @brief 这个函数用于检查从Flash读取的整份TX/RX配置是否有效。
 * 
 * @param record 要检查的Flash配置。
 * @return uint8_t 
 */
static uint8_t rf_flash_record_check(const rf_flash_config_t *record)
{
    rf_flash_config_t temp;
    uint16_t saved_crc;
    uint16_t calculated_crc;

    if (record == 0)
    {
        return 0U;
    }

    if (record->magic != RF_FLASH_MAGIC)
    {
        return 0U;
    }

    if (record->version != RF_FLASH_VERSION)
    {
        return 0U;
    }

    if (record->data_size != sizeof(rf_flash_config_t))
    {
        return 0U;
    }

    memcpy(&temp, record, sizeof(temp));

    saved_crc = temp.crc16;
    temp.crc16 = 0U;
    calculated_crc =
        rf_crc16_calc((const uint8_t *)&temp, sizeof(temp));

    if (saved_crc != calculated_crc)
    {
        return 0U;
    }

    if (rf_config_check(&temp.tx) != RF_OK)
    {
        return 0U;
    }

    if (rf_config_check(&temp.rx) != RF_OK)
    {
        return 0U;
    }

    return 1U;
}

/**
 * @brief 这个函数用于从指定的Flash地址读取射频配置，并检查配置是否有效。
 * 
 * @param record 用于接收读取到的配置。
 * @return uint8_t 
 */
static uint8_t rf_flash_config_read(rf_flash_config_t *record)
{
    const rf_flash_config_t *flash_record;

    if (record == 0)
    {
        return 0U;
    }

    flash_record =
        (const rf_flash_config_t *)RF_FLASH_PAGE_ADDR;

    memcpy(record, flash_record, sizeof(*record));

    return rf_flash_record_check(record);
}

/**
 * @brief 这个函数用于擦除配置Flash页、写入新的TX/RX配置，并读回校验。返回 1U 表示成功，返回 0U 表示失败
 * 
 * @param record 
 * @return uint8_t 
 */
static uint8_t rf_flash_config_write(rf_flash_config_t *record)
{
    uint32_t address;
    uint32_t offset;
    uint32_t word_data;
    fmc_state_enum result;
    rf_flash_config_t verify_record;

    if (record == 0)
    {
        return 0U;
    }

    record->magic = RF_FLASH_MAGIC;
    record->version = RF_FLASH_VERSION;
    record->data_size = sizeof(*record);
    record->reserved = 0U;

    record->crc16 = 0U;
    record->crc16 =
        rf_crc16_calc((const uint8_t *)record, sizeof(*record));

    fmc_unlock();

    fmc_flag_clear(FMC_FLAG_END |
                   FMC_FLAG_PGERR |
                   FMC_FLAG_PGAERR |
                   FMC_FLAG_WPERR);

    result = fmc_page_erase(RF_FLASH_PAGE_ADDR);
    if (result != FMC_READY)
    {
        fmc_lock();
        return 0U;
    }

    address = RF_FLASH_PAGE_ADDR;

    for (offset = 0U;
         offset < sizeof(*record);
         offset += sizeof(uint32_t))
    {
        memcpy(&word_data,
               ((const uint8_t *)record) + offset,
               sizeof(word_data));

        result = fmc_word_program(address + offset, word_data);
        if (result != FMC_READY)
        {
            fmc_lock();
            return 0U;
        }
    }

    fmc_lock();

    memcpy(&verify_record,
           (const void *)RF_FLASH_PAGE_ADDR,
           sizeof(verify_record));

    if (memcmp(&verify_record, record, sizeof(*record)) != 0)
    {
        return 0U;
    }

    return rf_flash_record_check(&verify_record);
}

/**
 * @brief 从字符串当前位置读取一个十进制整数，将结果保存为 uint8_t，并把字符串指针移动到数字结束的位置。
 * 
 * @param cursor 
 * @param value 
 * @return uint8_t 
 */
static uint8_t rf_parse_uint8(const char **cursor, uint8_t *value)
{
    const char *p;
    uint16_t result = 0U;
    uint8_t digit_count = 0U;

    if ((cursor == 0) || (*cursor == 0) || (value == 0))
    {
        return 0U;
    }

    p = *cursor;

    while ((*p >= '0') && (*p <= '9'))
    {
        result = (uint16_t)(result * 10U + (uint16_t)(*p - '0'));
        digit_count++;

        if (result > 255U)
        {
            return 0U;
        }

        p++;
    }

    if (digit_count == 0U)
    {
        return 0U;
    }

    *value = (uint8_t)result;
    *cursor = p;

    return 1U;
}

/**
 * @brief 这个函数用于检查字符串当前位置是否为指定字符。
 * 
 * @param cursor 
 * @param expected 
 * @return uint8_t 
 */
static uint8_t rf_expect_char(const char **cursor, char expected)
{
    if ((cursor == 0) || (*cursor == 0))
    {
        return 0U;
    }

    if (**cursor != expected)
    {
        return 0U;
    }

    (*cursor)++;
    return 1U;
}

/**
 * @brief 这个函数用于解析串口发送的射频配置命令。
 * 
 * @param command 
 * @param target 
 * @param offset 
 * @param channel 
 * @param count 
 * @return uint8_t 
 */
static uint8_t rf_parse_command(const char *command,
                                uint8_t *target,
                                uint8_t *offset,
                                uint8_t *channel,
                                uint8_t *count)
{
    const char *p;

    if ((command == 0) ||
        (target == 0) ||
        (offset == 0) ||
        (channel == 0) ||
        (count == 0))
    {
        return 0U;
    }

    p = command;

    if ((p[0] != 'S') ||
        (p[1] != 'E') ||
        (p[2] != 'T') ||
        (p[3] != ','))
    {
        return 0U;
    }

    p += 4;

    if ((p[0] == 'T') && (p[1] == 'X') && (p[2] == ','))
    {
        *target = RF_TARGET_TX;
        p += 3;
    }
    else if ((p[0] == 'R') && (p[1] == 'X') && (p[2] == ','))
    {
        *target = RF_TARGET_RX;
        p += 3;
    }
    else if ((p[0] == 'B') &&
             (p[1] == 'O') &&
             (p[2] == 'T') &&
             (p[3] == 'H') &&
             (p[4] == ','))
    {
        *target = RF_TARGET_BOTH;
        p += 5;
    }
    else
    {
        return 0U;
    }

    if (!rf_parse_uint8(&p, offset))
    {
        return 0U;
    }

    if (!rf_expect_char(&p, ','))
    {
        return 0U;
    }

    if (!rf_parse_uint8(&p, channel))
    {
        return 0U;
    }

    if (!rf_expect_char(&p, ','))
    {
        return 0U;
    }

    if (!rf_parse_uint8(&p, count))
    {
        return 0U;
    }

    return (*p == '\0') ? 1U : 0U;
}

/**
 * @brief 这个函数用于设备启动时从Flash恢复TX和RX的射频配置。
 * 
 * @return uint8_t 
 */
uint8_t rf_uart_config_restore(void)
{
    if (!rf_flash_config_read(&g_rf_saved_config))
    {
        rf_build_default_flash_config(&g_rf_saved_config);
        return 0U;
    }

    if (rf_config_apply(&g_rf_saved_config.tx) != RF_OK)
    {
        return 0U;
    }

    if (rf_config_apply(&g_rf_saved_config.rx) != RF_OK)
    {
        return 0U;
    }

    return 1U;
}

/**
 * @brief 这个函数用于执行解析完成的射频配置命令。
 * 
 * @param target 
 * @param offset 
 * @param channel 
 * @param count 
 * @return uint8_t 
 */
static uint8_t rf_execute_command(uint8_t target,
                                  uint8_t offset,
                                  uint8_t channel,
                                  uint8_t count)
{
    rf_flash_config_t new_record;

    memcpy(&new_record, &g_rf_saved_config, sizeof(new_record));

    if ((target == RF_TARGET_TX) ||
        (target == RF_TARGET_BOTH))
    {
        rf_build_one_config(&new_record.tx,
                            RF_ROLE_TX,
                            offset,
                            channel,
                            count);

        if (rf_config_check(&new_record.tx) != RF_OK)
        {
            return 0U;
        }
    }

    if ((target == RF_TARGET_RX) ||
        (target == RF_TARGET_BOTH))
    {
        rf_build_one_config(&new_record.rx,
                            RF_ROLE_RX,
                            offset,
                            channel,
                            count);

        if (rf_config_check(&new_record.rx) != RF_OK)
        {
            return 0U;
        }
    }

    if (!rf_flash_config_write(&new_record))
    {
        return 0U;
    }

    if ((target == RF_TARGET_TX) ||
        (target == RF_TARGET_BOTH))
    {
        if (rf_config_apply(&new_record.tx) != RF_OK)
        {
            return 0U;
        }
    }

    if ((target == RF_TARGET_RX) ||
        (target == RF_TARGET_BOTH))
    {
        if (rf_config_apply(&new_record.rx) != RF_OK)
        {
            return 0U;
        }
    }

    memcpy(&g_rf_saved_config, &new_record, sizeof(g_rf_saved_config));

    return 1U;
}

/**
 * @brief 这个函数用于处理串口接收到的射频配置命令。
 * 
 */
void rf_uart_config_process(void)
{
    char command[RF_UART_COMMAND_MAX];
    uint16_t length;
    uint16_t i;
    uint8_t target;
    uint8_t offset;
    uint8_t channel;
    uint8_t count;
    uint8_t result;

    if (g_recv_complete_flag == 0U)
    {
        return;
    }

    nvic_irq_disable(BSP_USART_IRQ);

    length = g_recv_length;

    if (length < RF_UART_COMMAND_MAX)
    {
        memcpy(command, g_recv_buff, length);
    }

    g_recv_length = 0U;
    g_recv_complete_flag = 0U;
    g_recv_buff[0] = '\0';

    nvic_irq_enable(BSP_USART_IRQ, 2U);

    if ((length == 0U) || (length >= RF_UART_COMMAND_MAX))
    {
        usart_send_string((uint8_t *)"ERROR\r\n");
        return;
    }

    command[length] = '\0';

    i = length;
    while ((i > 0U) &&
           ((command[i - 1U] == '\r') ||
            (command[i - 1U] == '\n')))
    {
        i--;
    }
    command[i] = '\0';

    result = rf_parse_command(command,
                              &target,
                              &offset,
                              &channel,
                              &count);

    if (result)
    {
        result = rf_execute_command(target,
                                    offset,
                                    channel,
                                    count);
    }

    if (result)
    {
        usart_send_string((uint8_t *)"OK\r\n");
        /*
        * 配置已经写入 Flash 并应用到芯片后，
        * 读取收发芯片寄存器中的真实配置并返回。
        */
        rf_print_tx_rx_real_freq();
    }
    else
    {
        usart_send_string((uint8_t *)"ERROR\r\n");
    }
}

