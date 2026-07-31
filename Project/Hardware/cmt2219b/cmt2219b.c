#include "cmt2219b.h"
#include "cmt2219b_spi.h"
#include "cmt2219b_port.h"
#include "cmt2219b_params.h"

/* Register bank base address */
#define CMT2219B_CMT_BANK_ADDR              0x00
#define CMT2219B_SYSTEM_BANK_ADDR           0x0C
#define CMT2219B_FREQUENCY_BANK_ADDR        0x18
#define CMT2219B_DATA_RATE_BANK_ADDR        0x20
#define CMT2219B_BASEBAND_BANK_ADDR         0x38
#define CMT2219B_TX_BANK_ADDR               0x55
#define CMT2219B_LBD_BANK_ADDR              0x5F

/* Registers */
/* 
这里新增了三个寄存器：// 手动快速调频相关
#define CMT2219B_CUS_FSK4                   0x27  // AFC 阈值
#define CMT2219B_CUS_FREQ_CHNL              0x63  // 跳频信道
#define CMT2219B_CUS_FREQ_OFS               0x64  // 跳频步进
*/
#define CMT2219B_CUS_CMT10                  0x09
#define CMT2219B_CUS_SYS2                   0x0D
#define CMT2219B_CUS_FSK4                   0x27
#define CMT2219B_CUS_PKT17                  0x48
#define CMT2219B_CUS_MODE_CTL               0x60
#define CMT2219B_CUS_MODE_STA               0x61
#define CMT2219B_CUS_EN_CTL                 0x62
#define CMT2219B_CUS_FREQ_CHNL              0x63
#define CMT2219B_CUS_FREQ_OFS               0x64
#define CMT2219B_CUS_IO_SEL                 0x65
#define CMT2219B_CUS_INT2_CTL               0x67
#define CMT2219B_CUS_INT_EN                 0x68
#define CMT2219B_CUS_FIFO_CTL               0x69
#define CMT2219B_CUS_INT_CLR1               0x6A
#define CMT2219B_CUS_INT_CLR2               0x6B
#define CMT2219B_CUS_FIFO_CLR               0x6C

/* Mode command */
#define CMT2219B_GO_STBY                    0x02
#define CMT2219B_GO_RX                      0x08
#define CMT2219B_GO_SLEEP                   0x10

/* Mode status */
#define CMT2219B_STA_SLEEP                  0x01
#define CMT2219B_STA_STBY                   0x02
#define CMT2219B_STA_RX                     0x05

/* Mode status masks */
#define CMT2219B_MASK_CHIP_MODE_STA         0x0F
#define CMT2219B_MASK_CFG_RETAIN            0x10
#define CMT2219B_MASK_RSTN_IN_EN            0x20
#define CMT2219B_MASK_LOCKING_EN            0x20

/* LFOSC masks */
#define CMT2219B_MASK_LFOSC_RECAL_EN        0x80
#define CMT2219B_MASK_LFOSC_CAL1_EN         0x40
#define CMT2219B_MASK_LFOSC_CAL2_EN         0x20

/* GPIO / interrupt config */
#define CMT2219B_GPIO3_SEL_INT2             0x20
#define CMT2219B_MASK_INT2_SEL              0x1F
#define CMT2219B_INT_SEL_PKT_DONE           0x19

#define CMT2219B_MASK_PREAM_OK_EN           0x10
#define CMT2219B_MASK_SYNC_OK_EN            0x08
#define CMT2219B_MASK_PKT_DONE_EN           0x01

/* FIFO config */
#define CMT2219B_MASK_SPI_FIFO_RD_WR_SEL    0x01
#define CMT2219B_MASK_FIFO_RX_TX_SEL        0x04
#define CMT2219B_MASK_FIFO_CLR_RX           0x02

/* Interrupt clear masks */
#define CMT2219B_MASK_SL_TMO_CLR            0x02
#define CMT2219B_MASK_RX_TMO_CLR            0x01
#define CMT2219B_MASK_TX_DONE_CLR           0x04

#define CMT2219B_MASK_LBD_CLR               0x20
#define CMT2219B_MASK_PREAM_OK_CLR          0x10
#define CMT2219B_MASK_SYNC_OK_CLR           0x08
#define CMT2219B_MASK_NODE_OK_CLR           0x04
#define CMT2219B_MASK_CRC_OK_CLR            0x02
#define CMT2219B_MASK_PKT_DONE_CLR          0x01

static void cmt2219b_config_reg_bank(uint8_t start_addr, const uint8_t *buf, uint8_t len)
{
    uint8_t i;

    for (i = 0; i < len; i++) {
        cmt2219b_write_reg((uint8_t)(start_addr + i), buf[i]);
    }
}

static void cmt2219b_soft_reset(void)
{
    cmt2219b_write_reg(0x7F, 0xFF);
}

static uint8_t cmt2219b_get_chip_status(void)
{
    return cmt2219b_read_reg(CMT2219B_CUS_MODE_STA) & CMT2219B_MASK_CHIP_MODE_STA;
}

static uint8_t cmt2219b_auto_switch_status(uint8_t cmd, uint8_t wait_status)
{
    uint8_t i;

    cmt2219b_write_reg(CMT2219B_CUS_MODE_CTL, cmd);

    for (i = 0; i < 100; i++) {
        cmt2219b_delay_us(100);

        if (cmt2219b_get_chip_status() == wait_status) {
            return CMT2219B_OK;
        }
    }

    return CMT2219B_ERROR;
}

static void cmt2219b_enable_lfosc(uint8_t enable)
{
    uint8_t tmp;

    tmp = cmt2219b_read_reg(CMT2219B_CUS_SYS2);

    if (enable) {
        tmp |= CMT2219B_MASK_LFOSC_RECAL_EN;
        tmp |= CMT2219B_MASK_LFOSC_CAL1_EN;
        tmp |= CMT2219B_MASK_LFOSC_CAL2_EN;
    } else {
        tmp &= (uint8_t)(~CMT2219B_MASK_LFOSC_RECAL_EN);
        tmp &= (uint8_t)(~CMT2219B_MASK_LFOSC_CAL1_EN);
        tmp &= (uint8_t)(~CMT2219B_MASK_LFOSC_CAL2_EN);
    }

    cmt2219b_write_reg(CMT2219B_CUS_SYS2, tmp);
}

static void cmt2219b_config_gpio_interrupt(void)
{
    uint8_t tmp;

    /*
     * 只使用 GPIO3。
     * 配置 GPIO3 输出 INT2。
     */
    cmt2219b_write_reg(CMT2219B_CUS_IO_SEL, CMT2219B_GPIO3_SEL_INT2);

    /*
     * 配置 INT2 = PKT_DONE。
     */
    tmp = cmt2219b_read_reg(CMT2219B_CUS_INT2_CTL);
    tmp &= (uint8_t)(~CMT2219B_MASK_INT2_SEL);
    tmp |= CMT2219B_INT_SEL_PKT_DONE;
    cmt2219b_write_reg(CMT2219B_CUS_INT2_CTL, tmp);

    /*
     * 使能接收相关中断。
     */
    cmt2219b_write_reg(CMT2219B_CUS_INT_EN,
                       CMT2219B_MASK_PKT_DONE_EN |
                       CMT2219B_MASK_PREAM_OK_EN |
                       CMT2219B_MASK_SYNC_OK_EN);
}

uint8_t cmt2219b_init(void)
{
    uint8_t tmp;

    cmt2219b_spi_init();

    cmt2219b_soft_reset();
    cmt2219b_delay_ms(20);

    if (cmt2219b_go_stby() != CMT2219B_OK) {
        return CMT2219B_ERROR;
    }

    /*
     * Enable CFG_RETAIN, disable RSTN_IN.
     */
    tmp = cmt2219b_read_reg(CMT2219B_CUS_MODE_STA);
    tmp |= CMT2219B_MASK_CFG_RETAIN;
    tmp &= (uint8_t)(~CMT2219B_MASK_RSTN_IN_EN);
    cmt2219b_write_reg(CMT2219B_CUS_MODE_STA, tmp);

    /*
     * Enable LOCKING_EN.
     */
    tmp = cmt2219b_read_reg(CMT2219B_CUS_EN_CTL);
    tmp |= CMT2219B_MASK_LOCKING_EN;
    cmt2219b_write_reg(CMT2219B_CUS_EN_CTL, tmp);

    cmt2219b_enable_lfosc(0);
    cmt2219b_clear_interrupt_flags();

    /*
     * 写入官方 Demo 参数表。
     */
    cmt2219b_config_reg_bank(CMT2219B_CMT_BANK_ADDR,
                             g_cmt2219b_cmt_bank,
                             CMT2219B_CMT_BANK_SIZE);

    cmt2219b_config_reg_bank(CMT2219B_SYSTEM_BANK_ADDR,
                             g_cmt2219b_system_bank,
                             CMT2219B_SYSTEM_BANK_SIZE);

    cmt2219b_config_reg_bank(CMT2219B_FREQUENCY_BANK_ADDR,
                             g_cmt2219b_frequency_bank,
                             CMT2219B_FREQUENCY_BANK_SIZE);

    cmt2219b_config_reg_bank(CMT2219B_DATA_RATE_BANK_ADDR,
                             g_cmt2219b_data_rate_bank,
                             CMT2219B_DATA_RATE_BANK_SIZE);

    cmt2219b_config_reg_bank(CMT2219B_BASEBAND_BANK_ADDR,
                             g_cmt2219b_baseband_bank,
                             CMT2219B_BASEBAND_BANK_SIZE);

    cmt2219b_config_reg_bank(CMT2219B_TX_BANK_ADDR,
                             g_cmt2219b_tx_bank,
                             CMT2219B_TX_BANK_SIZE);
                             
    cmt2219b_config_reg_bank(CMT2219B_LBD_BANK_ADDR,
                            g_cmt2219b_lbd_bank,
                            CMT2219B_LBD_BANK_SIZE);

    /*
     * 官方 Demo 里有这个额外配置：
     * xosc_aac_code[2:0] = 2
     */
    tmp = cmt2219b_read_reg(CMT2219B_CUS_CMT10);
    tmp &= (uint8_t)(~0x07);
    tmp |= 0x02;
    cmt2219b_write_reg(CMT2219B_CUS_CMT10, tmp);

    cmt2219b_config_gpio_interrupt();

    /*
     * 配置完成后进入 Sleep，使配置生效。
     */
    cmt2219b_go_sleep();

    return CMT2219B_OK;
}

uint8_t cmt2219b_is_exist(void)
{
    uint8_t back;
    uint8_t dat;

    /*
     * 通过读写 0x48 寄存器判断 SPI 是否正常。
     */
    back = cmt2219b_read_reg(CMT2219B_CUS_PKT17);

    cmt2219b_write_reg(CMT2219B_CUS_PKT17, 0xAA);
    dat = cmt2219b_read_reg(CMT2219B_CUS_PKT17);

    cmt2219b_write_reg(CMT2219B_CUS_PKT17, back);

    if (dat == 0xAA) {
        return CMT2219B_OK;
    }

    return CMT2219B_ERROR;
}

uint8_t cmt2219b_read_reg(uint8_t addr)
{
    return cmt2219b_spi_read_reg(addr);
}

void cmt2219b_write_reg(uint8_t addr, uint8_t dat)
{
    cmt2219b_spi_write_reg(addr, dat);
}

/*
// 手动快速调频相关 
这三个函数对应官方 Demo 里的：
CMT2300A_SetFrequencyChannel()
CMT2300A_SetFrequencyStep()
CMT2300A_SetAfcOvfTh()
*/
void cmt2219b_set_frequency_channel(uint8_t channel)
{
    cmt2219b_write_reg(CMT2219B_CUS_FREQ_CHNL, channel);
}

void cmt2219b_set_frequency_step(uint8_t step)
{
    cmt2219b_write_reg(CMT2219B_CUS_FREQ_OFS, step);
}

void cmt2219b_set_afc_ovf_th(uint8_t afc_ovf_th)
{
    cmt2219b_write_reg(CMT2219B_CUS_FSK4, afc_ovf_th);
}

void cmt2219b_read_fifo(uint8_t *buf, uint8_t len)
{
    if (len > CMT2219B_MAX_FIFO_SIZE) {
        len = CMT2219B_MAX_FIFO_SIZE;
    }

    cmt2219b_spi_read_fifo(buf, len);
}

void cmt2219b_write_fifo(const uint8_t *buf, uint8_t len)
{
    if (len > CMT2219B_MAX_FIFO_SIZE) {
        len = CMT2219B_MAX_FIFO_SIZE;
    }

    cmt2219b_spi_write_fifo(buf, len);
}

uint8_t cmt2219b_go_sleep(void)
{
    return cmt2219b_auto_switch_status(CMT2219B_GO_SLEEP, CMT2219B_STA_SLEEP);
}

uint8_t cmt2219b_go_stby(void)
{
    return cmt2219b_auto_switch_status(CMT2219B_GO_STBY, CMT2219B_STA_STBY);
}

uint8_t cmt2219b_go_rx(void)
{
    return cmt2219b_auto_switch_status(CMT2219B_GO_RX, CMT2219B_STA_RX);
}

void cmt2219b_enable_read_fifo(void)
{
    uint8_t tmp;

    /*
     * 配置 SPI 读 RX FIFO。
     */
    tmp = cmt2219b_read_reg(CMT2219B_CUS_FIFO_CTL);
    tmp &= (uint8_t)(~CMT2219B_MASK_SPI_FIFO_RD_WR_SEL);
    tmp &= (uint8_t)(~CMT2219B_MASK_FIFO_RX_TX_SEL);
    cmt2219b_write_reg(CMT2219B_CUS_FIFO_CTL, tmp);
}

void cmt2219b_clear_rx_fifo(void)
{
    cmt2219b_write_reg(CMT2219B_CUS_FIFO_CLR, CMT2219B_MASK_FIFO_CLR_RX);
}

void cmt2219b_clear_interrupt_flags(void)
{
    cmt2219b_write_reg(CMT2219B_CUS_INT_CLR1,
                       CMT2219B_MASK_TX_DONE_CLR |
                       CMT2219B_MASK_SL_TMO_CLR |
                       CMT2219B_MASK_RX_TMO_CLR);

    cmt2219b_write_reg(CMT2219B_CUS_INT_CLR2,
                       CMT2219B_MASK_LBD_CLR |
                       CMT2219B_MASK_PREAM_OK_CLR |
                       CMT2219B_MASK_SYNC_OK_CLR |
                       CMT2219B_MASK_NODE_OK_CLR |
                       CMT2219B_MASK_CRC_OK_CLR |
                       CMT2219B_MASK_PKT_DONE_CLR);
}

uint8_t cmt2219b_packet_received(void)
{
    /*
     * 0x6D 是 INT_FLAG 寄存器。
     * bit0 = 1 表示收到完整数据包。
     */
    if (cmt2219b_read_reg(0x6D) & 0x01) {
        return CMT2219B_OK;
    }

    return CMT2219B_ERROR;
}

