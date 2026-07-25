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
    cmt2119b_port_init();

    cmt2119b_csb_high();
    cmt2119b_fcsb_high();
    cmt2119b_sclk_low();

    cmt2119b_sdio_output();
    cmt2119b_sdio_high();

    cmt2119b_spi_delay();
}

static void cmt2119b_spi_send(uint8_t data)
{
    uint8_t i;

    for (i = 0; i < 8; i++) {
        cmt2119b_sclk_low();

        if (data & 0x80) {
            cmt2119b_sdio_high();
        } else {
            cmt2119b_sdio_low();
        }

        cmt2119b_spi_delay();

        data <<= 1;

        cmt2119b_sclk_high();
        cmt2119b_spi_delay();
    }
}

static uint8_t cmt2119b_spi_recv(void)
{
    uint8_t i;
    uint8_t data = 0;

    for (i = 0; i < 8; i++) {
        cmt2119b_sclk_low();
        cmt2119b_spi_delay();

        data <<= 1;

        cmt2119b_sclk_high();

        if (cmt2119b_sdio_read()) {
            data |= 0x01;
        }

        cmt2119b_spi_delay();
    }

    return data;
}

void cmt2119b_spi_write_reg(uint8_t addr, uint8_t dat)
{
    cmt2119b_sdio_output();
    cmt2119b_sdio_high();

    cmt2119b_sclk_low();

    /* Register access: FCSB high, CSB low */
    cmt2119b_fcsb_high();
    cmt2119b_csb_low();

    cmt2119b_spi_delay();
    cmt2119b_spi_delay();

    /* write: bit7 = 0 */
    cmt2119b_spi_send(addr & 0x7F);
    cmt2119b_spi_send(dat);

    cmt2119b_sclk_low();

    cmt2119b_spi_delay();
    cmt2119b_spi_delay();

    cmt2119b_csb_high();

    cmt2119b_sdio_high();
    cmt2119b_sdio_input();

    cmt2119b_fcsb_high();
}

uint8_t cmt2119b_spi_read_reg(uint8_t addr)
{
    uint8_t dat;

    cmt2119b_sdio_output();
    cmt2119b_sdio_high();

    cmt2119b_sclk_low();

    /* Register access: FCSB high, CSB low */
    cmt2119b_fcsb_high();
    cmt2119b_csb_low();

    cmt2119b_spi_delay();
    cmt2119b_spi_delay();

    /* read: bit7 = 1 */
    cmt2119b_spi_send(addr | 0x80);

    /* Must switch SDIO to input before reading */
    cmt2119b_sdio_input();

    dat = cmt2119b_spi_recv();

    cmt2119b_sclk_low();

    cmt2119b_spi_delay();
    cmt2119b_spi_delay();

    cmt2119b_csb_high();

    cmt2119b_sdio_input();
    cmt2119b_fcsb_high();

    return dat;
}

void cmt2119b_spi_write_fifo(const uint8_t *buf, uint16_t len)
{
    uint16_t i;

    /* FIFO access: CSB high, FCSB low */
    cmt2119b_csb_high();
    cmt2119b_fcsb_high();
    cmt2119b_sclk_low();

    cmt2119b_sdio_output();

    for (i = 0; i < len; i++) {
        cmt2119b_fcsb_low();

        cmt2119b_spi_delay();
        cmt2119b_spi_delay();

        cmt2119b_spi_send(buf[i]);

        cmt2119b_sclk_low();

        /* Datasheet/Demo requires spacing between FIFO bytes */
        cmt2119b_spi_delay_us();
        cmt2119b_spi_delay_us();
        cmt2119b_spi_delay_us();

        cmt2119b_fcsb_high();

        cmt2119b_spi_delay_us();
        cmt2119b_spi_delay_us();
        cmt2119b_spi_delay_us();
        cmt2119b_spi_delay_us();
        cmt2119b_spi_delay_us();
    }

    cmt2119b_sdio_input();
    cmt2119b_fcsb_high();
}
