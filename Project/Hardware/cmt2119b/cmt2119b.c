#include "cmt2119b.h"
#include "cmt2119b_spi.h"
#include "cmt2119b_port.h"
#include "cmt2119b_params.h"

/*
 * These addresses/commands must be checked against CMT2119B datasheet
 * or AN167 Quick Start Guide before real RF validation.
 *
 * The purpose of this first porting step is to build a clean GD32E230
 * driver framework and verify GPIO/SPI waveforms.
 */

#define CMT2119B_ADDR_CMT_BANK          0x00
#define CMT2119B_ADDR_SYSTEM_BANK       0x0C
#define CMT2119B_ADDR_FREQUENCY_BANK    0x18
#define CMT2119B_ADDR_DATA_RATE_BANK    0x20
#define CMT2119B_ADDR_BASEBAND_BANK     0x38
#define CMT2119B_ADDR_TX_BANK           0x55

/* The following control registers must be confirmed with CMT2119B register table. */
#define CMT2119B_REG_CTL                0x60
#define CMT2119B_REG_FIFO_CTL           0x63
#define CMT2119B_REG_INT_CLR1           0x66
#define CMT2119B_REG_INT_CLR2           0x67

#define CMT2119B_CMD_GO_SLEEP           0x01
#define CMT2119B_CMD_GO_STBY            0x02
#define CMT2119B_CMD_GO_TX              0x04

static void cmt2119b_config_reg_bank(uint8_t start_addr, const uint8_t *buf, uint8_t len)
{
    uint8_t i;

    for (i = 0; i < len; i++) {
        cmt2119b_write_reg((uint8_t)(start_addr + i), buf[i]);
    }
}

uint8_t cmt2119b_init(void)
{
    cmt2119b_spi_init();

    /*
     * Load RFPDK-generated parameter banks.
     * Empty arrays are allowed for compile-only stage.
     */
    cmt2119b_config_reg_bank(CMT2119B_ADDR_CMT_BANK,
                             g_cmt2119b_cmt_bank,
                             CMT2119B_CMT_BANK_SIZE);

    cmt2119b_config_reg_bank(CMT2119B_ADDR_SYSTEM_BANK,
                             g_cmt2119b_system_bank,
                             CMT2119B_SYSTEM_BANK_SIZE);

    cmt2119b_config_reg_bank(CMT2119B_ADDR_FREQUENCY_BANK,
                             g_cmt2119b_frequency_bank,
                             CMT2119B_FREQUENCY_BANK_SIZE);

    cmt2119b_config_reg_bank(CMT2119B_ADDR_DATA_RATE_BANK,
                             g_cmt2119b_data_rate_bank,
                             CMT2119B_DATA_RATE_BANK_SIZE);

    cmt2119b_config_reg_bank(CMT2119B_ADDR_BASEBAND_BANK,
                             g_cmt2119b_baseband_bank,
                             CMT2119B_BASEBAND_BANK_SIZE);

    cmt2119b_config_reg_bank(CMT2119B_ADDR_TX_BANK,
                             g_cmt2119b_tx_bank,
                             CMT2119B_TX_BANK_SIZE);

    cmt2119b_go_sleep();

    return CMT2119B_OK;
}

uint8_t cmt2119b_read_reg(uint8_t addr)
{
    return cmt2119b_spi_read_reg(addr);
}

void cmt2119b_write_reg(uint8_t addr, uint8_t dat)
{
    cmt2119b_spi_write_reg(addr, dat);
}

void cmt2119b_write_fifo(const uint8_t *buf, uint8_t len)
{
    if (len > CMT2119B_MAX_FIFO_SIZE) {
        len = CMT2119B_MAX_FIFO_SIZE;
    }

    cmt2119b_spi_write_fifo(buf, len);
}

void cmt2119b_go_sleep(void)
{
    cmt2119b_write_reg(CMT2119B_REG_CTL, CMT2119B_CMD_GO_SLEEP);
}

void cmt2119b_go_stby(void)
{
    cmt2119b_write_reg(CMT2119B_REG_CTL, CMT2119B_CMD_GO_STBY);
}

void cmt2119b_go_tx(void)
{
    cmt2119b_write_reg(CMT2119B_REG_CTL, CMT2119B_CMD_GO_TX);
}

void cmt2119b_clear_interrupt_flags(void)
{
    cmt2119b_write_reg(CMT2119B_REG_INT_CLR1, 0xFF);
    cmt2119b_write_reg(CMT2119B_REG_INT_CLR2, 0xFF);
}

void cmt2119b_clear_tx_fifo(void)
{
    cmt2119b_write_reg(CMT2119B_REG_FIFO_CTL, 0x01);
}

uint8_t cmt2119b_send_packet(const uint8_t *buf, uint8_t len, uint32_t timeout_ms)
{
    if ((buf == 0) || (len == 0) || (len > CMT2119B_MAX_FIFO_SIZE)) {
        return CMT2119B_ERROR;
    }

    cmt2119b_go_stby();
    cmt2119b_clear_interrupt_flags();
    cmt2119b_clear_tx_fifo();
    cmt2119b_write_fifo(buf, len);
    cmt2119b_go_tx();

    while (timeout_ms--) {
        if (cmt2119b_gpio3_read()) {
            cmt2119b_clear_interrupt_flags();
            cmt2119b_go_sleep();
            return CMT2119B_OK;
        }

        cmt2119b_delay_ms(1);
    }

    cmt2119b_go_sleep();
    return CMT2119B_ERROR;
}
