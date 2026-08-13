#include "cmt2119b_spi.h"
#include "cmt2119b_port.h"

static void cmt2119b_spi_delay(void)
{
    volatile uint32_t n = 20;
    while (n--) {
    }
}

static void cmt2119b_spi_delay_us(void)
{
    cmt2119b_delay_us(1);
}

void cmt2119b_spi_init(void)
{
    // 参数含义：
    // SPI_MASTER                         GD32 是主机
    // SPI_TRANSMODE_BDTRANSMIT           单线半双工，初始为发送方向
    // SPI_FRAMESIZE_8BIT                 每次传输 8 位
    // SPI_NSS_SOFT                       不使用硬件 NSS
    // SPI_ENDIAN_MSB                     高位先发送
    // SPI_CK_PL_LOW_PH_1EDGE             Mode 0：低电平空闲，上升沿采样
    // SPI_PSC_16                         72MHz / 16 = 4.5MHz
    spi_parameter_struct spi_init_struct;

    cmt2119b_port_init();

    spi_i2s_deinit(CMT2119B_SPI_PERIPH);
    spi_struct_para_init(&spi_init_struct);

    spi_init_struct.device_mode = SPI_MASTER;
    spi_init_struct.trans_mode = SPI_TRANSMODE_BDTRANSMIT;
    spi_init_struct.frame_size = SPI_FRAMESIZE_8BIT;
    spi_init_struct.nss = SPI_NSS_SOFT;
    spi_init_struct.endian = SPI_ENDIAN_MSB;
    spi_init_struct.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;
    spi_init_struct.prescale = SPI_PSC_16;

    spi_init(CMT2119B_SPI_PERIPH, &spi_init_struct);
    spi_enable(CMT2119B_SPI_PERIPH);

    cmt2119b_csb_high();
    cmt2119b_fcsb_high();
}

// static void cmt2119b_spi_send(uint8_t data)
// {
//     uint8_t i;

//     for (i = 0; i < 8; i++) {
//         cmt2119b_sclk_low();

//         if (data & 0x80) {
//             cmt2119b_sdio_high();
//         } else {
//             cmt2119b_sdio_low();
//         }

//         cmt2119b_spi_delay();

//         data <<= 1;

//         cmt2119b_sclk_high();
//         cmt2119b_spi_delay();
//     }
// }

static void cmt2119b_spi_send_byte(uint8_t data)
{
    while (spi_i2s_flag_get(CMT2119B_SPI_PERIPH, SPI_FLAG_TBE) == RESET) {
    }

    spi_i2s_data_transmit(CMT2119B_SPI_PERIPH, data);

    while (spi_i2s_flag_get(CMT2119B_SPI_PERIPH, SPI_FLAG_TRANS) != RESET) {
    }
}

static uint8_t cmt2119b_spi_recv_byte(void);

void cmt2119b_spi_write_reg(uint8_t addr, uint8_t dat)
{
    cmt2119b_fcsb_high();
    cmt2119b_csb_low();

    cmt2119b_delay_us(1);

    cmt2119b_spi_send_byte(addr & 0x7FU);
    cmt2119b_spi_send_byte(dat);

    cmt2119b_delay_us(1);

    cmt2119b_csb_high();
}

uint8_t cmt2119b_spi_read_reg(uint8_t addr)
{
     uint8_t data;

    cmt2119b_fcsb_high();
    cmt2119b_csb_low();

    cmt2119b_delay_us(1);

    /* bit7=1 means register read */
    cmt2119b_spi_send_byte(addr | 0x80U);

    /* SPI0 releases PA7 and receives one byte from CMT2119B */
    data = cmt2119b_spi_recv_byte();

    cmt2119b_delay_us(1);

    cmt2119b_csb_high();

    return data;
}

void cmt2119b_spi_write_fifo(const uint8_t *buf, uint16_t len)
{
     uint16_t i;

    cmt2119b_csb_high();

    for (i = 0; i < len; i++) {
        cmt2119b_fcsb_low();

        /* FCSB low to first SCLK rising edge: >= 1 SCLK cycle */
        cmt2119b_delay_us(1);

        cmt2119b_spi_send_byte(buf[i]);

        /* Last SCLK falling edge to FCSB high: >= 2us */
        cmt2119b_delay_us(2);

        cmt2119b_fcsb_high();

        /* FCSB high before next FIFO byte: >= 4us */
        cmt2119b_delay_us(4);
    }
}

static uint8_t cmt2119b_spi_recv_byte(void)
{
    uint8_t data;

    spi_disable(CMT2119B_SPI_PERIPH);

    spi_bidirectional_transfer_config(CMT2119B_SPI_PERIPH,
                                      SPI_BIDIRECTIONAL_RECEIVE);

    spi_enable(CMT2119B_SPI_PERIPH);

    while (spi_i2s_flag_get(CMT2119B_SPI_PERIPH, SPI_FLAG_RBNE) == RESET) {
    }

    data = (uint8_t)spi_i2s_data_receive(CMT2119B_SPI_PERIPH);

    spi_disable(CMT2119B_SPI_PERIPH);

    spi_bidirectional_transfer_config(CMT2119B_SPI_PERIPH,
                                      SPI_BIDIRECTIONAL_TRANSMIT);

    spi_enable(CMT2119B_SPI_PERIPH);

    return data;
}
