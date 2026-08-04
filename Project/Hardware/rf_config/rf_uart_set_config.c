#include "rf_uart_set_config.h"
#include "rf_apply.h"
#include "rf_config.h"
#include "bsp_usart.h"
#include "gd32e23x.h"
#include "gd32e23x_fmc.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define RF_FLASH_PAGE_ADDR       0x0800FC00UL
#define RF_FLASH_MAGIC           0x52464643UL
#define RF_FLASH_VERSION         1U
#define RF_UART_COMMAND_MAX      64U

#define RF_TARGET_TX             1U
#define RF_TARGET_RX             2U
#define RF_TARGET_BOTH           3U

typedef struct
{
    uint32_t magic;
    uint16_t version;
    uint16_t data_size;

    rf_factory_config_t tx;
    rf_factory_config_t rx;

    uint16_t crc16;
    uint16_t reserved;
} rf_flash_config_t;

static rf_flash_config_t g_rf_saved_config;



static void rf_build_one_config(rf_factory_config_t *cfg,
                                uint8_t role,
                                uint8_t offset,
                                uint8_t channel,
                                uint8_t count)
{
    memset(cfg, 0, sizeof(*cfg));

    cfg->magic = RF_CONFIG_MAGIC;
    cfg->version = RF_CONFIG_VERSION;
    cfg->pair_id = 17U;
    cfg->role = role;
    cfg->fh_offset = offset;
    cfg->fh_channel = channel;
    cfg->channel_count = count;

    cfg->crc16 = 0U;
    cfg->crc16 = rf_crc16_calc((const uint8_t *)cfg, sizeof(*cfg));
}

static void rf_build_default_flash_config(rf_flash_config_t *record)
{
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
    }
    else
    {
        usart_send_string((uint8_t *)"ERROR\r\n");
    }
}

