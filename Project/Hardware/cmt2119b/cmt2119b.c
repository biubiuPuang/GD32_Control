#include "cmt2119b.h"
#include "cmt2119b_spi.h"
#include "cmt2119b_port.h"
#include "cmt2119b_params.h"

#define CMT2119B_ADDR_CMT_BANK          0x00
#define CMT2119B_ADDR_SYSTEM_BANK       0x0C
#define CMT2119B_ADDR_FREQUENCY_BANK    0x18
#define CMT2119B_ADDR_DATA_RATE_BANK    0x20
#define CMT2119B_ADDR_BASEBAND_BANK     0x38
#define CMT2119B_ADDR_TX_BANK           0x55

#define CMT2119B_CUS_CMT10              0x09
#define CMT2119B_CUS_SYS2               0x0D
#define CMT2119B_CUS_MODE_CTL           0x60
#define CMT2119B_CUS_MODE_STA           0x61
#define CMT2119B_CUS_EN_CTL             0x62
// 手动快速调频相关
#define CMT2119B_CUS_FREQ_CHNL          0x63
// 手动快速调频相关
#define CMT2119B_CUS_FREQ_OFS           0x64
#define CMT2119B_CUS_IO_SEL             0x65
#define CMT2119B_CUS_INT1_CTL           0x66
#define CMT2119B_CUS_INT2_CTL           0x67
#define CMT2119B_CUS_INT_EN             0x68
#define CMT2119B_CUS_FIFO_CTL           0x69
#define CMT2119B_CUS_INT_CLR1           0x6A
#define CMT2119B_CUS_INT_CLR2           0x6B
#define CMT2119B_CUS_FIFO_CLR           0x6C
#define CMT2119B_CUS_FIFO_FLAG          0x6E

#define CMT2119B_GO_STBY                0x02
#define CMT2119B_GO_TX                  0x40
#define CMT2119B_GO_SLEEP               0x10

#define CMT2119B_STA_SLEEP              0x01
#define CMT2119B_STA_STBY               0x02
#define CMT2119B_STA_TX                 0x06

#define CMT2119B_MASK_CHIP_MODE_STA     0x0F
#define CMT2119B_MASK_CFG_RETAIN        0x10
#define CMT2119B_MASK_RSTN_IN_EN        0x20
#define CMT2119B_MASK_LOCKING_EN        0x20

#define CMT2119B_MASK_LFOSC_RECAL_EN    0x80
#define CMT2119B_MASK_LFOSC_CAL1_EN     0x40
#define CMT2119B_MASK_LFOSC_CAL2_EN     0x20

#define CMT2119B_GPIO3_SEL_INT2         0x20
#define CMT2119B_MASK_INT2_SEL          0x1F
#define CMT2119B_INT_SEL_TX_DONE        0x0A

#define CMT2119B_MASK_TX_DONE_EN        0x20

#define CMT2119B_MASK_FIFO_RX_TX_SEL    0x04
#define CMT2119B_MASK_SPI_FIFO_RD_WR_SEL 0x01
#define CMT2119B_MASK_FIFO_CLR_TX       0x01
#define CMT2119B_MASK_TX_FIFO_NMTY_FLG  0x02

#define CMT2119B_MASK_TX_DONE_FLG       0x08
#define CMT2119B_MASK_TX_DONE_CLR       0x04
#define CMT2119B_MASK_SL_TMO_CLR        0x02
#define CMT2119B_MASK_RX_TMO_CLR        0x01

#define CMT2119B_MASK_LBD_CLR           0x20
#define CMT2119B_MASK_PREAM_OK_CLR      0x10
#define CMT2119B_MASK_SYNC_OK_CLR       0x08
#define CMT2119B_MASK_NODE_OK_CLR       0x04
#define CMT2119B_MASK_CRC_OK_CLR        0x02
#define CMT2119B_MASK_PKT_DONE_CLR      0x01

static void cmt2119b_config_reg_bank(uint8_t start_addr, const uint8_t *buf, uint8_t len)
{
    uint8_t i;

    for (i = 0; i < len; i++) {
        cmt2119b_write_reg((uint8_t)(start_addr + i), buf[i]);
    }
}

static void cmt2119b_soft_reset(void)
{
    cmt2119b_write_reg(0x7F, 0xFF);
}

static uint8_t cmt2119b_get_chip_status(void)
{
    return cmt2119b_read_reg(CMT2119B_CUS_MODE_STA) & CMT2119B_MASK_CHIP_MODE_STA;
}

static uint8_t cmt2119b_auto_switch_status(uint8_t cmd, uint8_t wait_status)
{
    uint8_t i;

    cmt2119b_write_reg(CMT2119B_CUS_MODE_CTL, cmd);

    for (i = 0; i < 100; i++) {
        cmt2119b_delay_us(100);

        if (cmt2119b_get_chip_status() == wait_status) {
            return CMT2119B_OK;
        }

        if (cmd == CMT2119B_GO_TX) {
            if (cmt2119b_read_reg(CMT2119B_CUS_INT_CLR1) & CMT2119B_MASK_TX_DONE_FLG) {
                return CMT2119B_OK;
            }
        }
    }

    return CMT2119B_ERROR;
}

static void cmt2119b_enable_lfosc(uint8_t enable)
{
    uint8_t tmp;

    tmp = cmt2119b_read_reg(CMT2119B_CUS_SYS2);

    if (enable) {
        tmp |= CMT2119B_MASK_LFOSC_RECAL_EN;
        tmp |= CMT2119B_MASK_LFOSC_CAL1_EN;
        tmp |= CMT2119B_MASK_LFOSC_CAL2_EN;
    } else {
        tmp &= (uint8_t)(~CMT2119B_MASK_LFOSC_RECAL_EN);
        tmp &= (uint8_t)(~CMT2119B_MASK_LFOSC_CAL1_EN);
        tmp &= (uint8_t)(~CMT2119B_MASK_LFOSC_CAL2_EN);
    }

    cmt2119b_write_reg(CMT2119B_CUS_SYS2, tmp);
}

static void cmt2119b_config_gpio_interrupt(void)
{
    uint8_t tmp;

    cmt2119b_write_reg(CMT2119B_CUS_IO_SEL, CMT2119B_GPIO3_SEL_INT2);

    tmp = cmt2119b_read_reg(CMT2119B_CUS_INT2_CTL);
    tmp &= (uint8_t)(~CMT2119B_MASK_INT2_SEL);
    tmp |= CMT2119B_INT_SEL_TX_DONE;
    cmt2119b_write_reg(CMT2119B_CUS_INT2_CTL, tmp);

    cmt2119b_write_reg(CMT2119B_CUS_INT_EN, CMT2119B_MASK_TX_DONE_EN);
}

static void cmt2119b_enable_write_fifo(void)
{
    uint8_t tmp;

    tmp = cmt2119b_read_reg(CMT2119B_CUS_FIFO_CTL);

    tmp |= CMT2119B_MASK_FIFO_RX_TX_SEL;
    tmp |= CMT2119B_MASK_SPI_FIFO_RD_WR_SEL;

    cmt2119b_write_reg(CMT2119B_CUS_FIFO_CTL, tmp);
}

uint8_t cmt2119b_init(void)
{
    uint8_t tmp;

    cmt2119b_spi_init();

    cmt2119b_soft_reset();
    cmt2119b_delay_ms(20);

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

    tmp = cmt2119b_read_reg(CMT2119B_CUS_MODE_STA);
    tmp |= CMT2119B_MASK_CFG_RETAIN;
    tmp &= (uint8_t)(~CMT2119B_MASK_RSTN_IN_EN);
    cmt2119b_write_reg(CMT2119B_CUS_MODE_STA, tmp);

    tmp = cmt2119b_read_reg(CMT2119B_CUS_EN_CTL);
    tmp |= CMT2119B_MASK_LOCKING_EN;
    cmt2119b_write_reg(CMT2119B_CUS_EN_CTL, tmp);

    tmp = cmt2119b_read_reg(CMT2119B_CUS_CMT10);
    tmp &= (uint8_t)(~0x07);
    tmp |= 0x02;
    cmt2119b_write_reg(CMT2119B_CUS_CMT10, tmp);

    cmt2119b_enable_lfosc(0);
    cmt2119b_config_gpio_interrupt();
    cmt2119b_clear_interrupt_flags();
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

// 手动快速调频相关
void cmt2119b_set_frequency_channel(uint8_t channel)
{
    cmt2119b_write_reg(CMT2119B_CUS_FREQ_CHNL, channel);
}
// 手动快速调频相关
void cmt2119b_set_frequency_step(uint8_t step)
{
    cmt2119b_write_reg(CMT2119B_CUS_FREQ_OFS, step);
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
    cmt2119b_auto_switch_status(CMT2119B_GO_SLEEP, CMT2119B_STA_SLEEP);
}

void cmt2119b_go_stby(void)
{
    cmt2119b_auto_switch_status(CMT2119B_GO_STBY, CMT2119B_STA_STBY);
}

void cmt2119b_go_tx(void)
{
    cmt2119b_auto_switch_status(CMT2119B_GO_TX, CMT2119B_STA_TX);
}

void cmt2119b_clear_interrupt_flags(void)
{
    cmt2119b_write_reg(CMT2119B_CUS_INT_CLR1,
                       CMT2119B_MASK_TX_DONE_CLR |
                       CMT2119B_MASK_SL_TMO_CLR |
                       CMT2119B_MASK_RX_TMO_CLR);

    cmt2119b_write_reg(CMT2119B_CUS_INT_CLR2,
                       CMT2119B_MASK_LBD_CLR |
                       CMT2119B_MASK_PREAM_OK_CLR |
                       CMT2119B_MASK_SYNC_OK_CLR |
                       CMT2119B_MASK_NODE_OK_CLR |
                       CMT2119B_MASK_CRC_OK_CLR |
                       CMT2119B_MASK_PKT_DONE_CLR);
}

void cmt2119b_clear_tx_fifo(void)
{
    cmt2119b_write_reg(CMT2119B_CUS_FIFO_CLR, CMT2119B_MASK_FIFO_CLR_TX);
}

uint8_t cmt2119b_send_packet(const uint8_t *buf, uint8_t len, uint32_t timeout_ms)
{
    uint8_t fifo_flag;

    if ((buf == 0) || (len == 0) || (len > CMT2119B_MAX_FIFO_SIZE)) {
        return CMT2119B_ERROR;
    }

    cmt2119b_go_stby();
    cmt2119b_clear_interrupt_flags();

    cmt2119b_enable_write_fifo();
    cmt2119b_clear_tx_fifo();

    cmt2119b_write_fifo(buf, len);

    fifo_flag = cmt2119b_read_reg(CMT2119B_CUS_FIFO_FLAG);
    if ((fifo_flag & CMT2119B_MASK_TX_FIFO_NMTY_FLG) == 0) {
        cmt2119b_go_sleep();
        return CMT2119B_ERROR;
    }

    cmt2119b_go_tx();

    while (timeout_ms--) {
        if (cmt2119b_gpio3_read()) {
            cmt2119b_clear_interrupt_flags();
            cmt2119b_go_sleep();
            return CMT2119B_OK;
        }

        if (cmt2119b_read_reg(CMT2119B_CUS_INT_CLR1) & CMT2119B_MASK_TX_DONE_FLG) {
            cmt2119b_clear_interrupt_flags();
            cmt2119b_go_sleep();
            return CMT2119B_OK;
        }

        cmt2119b_delay_ms(1);
    }

    cmt2119b_go_sleep();
    return CMT2119B_ERROR;
}
