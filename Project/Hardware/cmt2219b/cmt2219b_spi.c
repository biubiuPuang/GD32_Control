#include "cmt2219b_spi.h"
#include "cmt2219b_port.h"

static void cmt2219b_spi_delay(void)
{
    volatile uint32_t n = 20;

    while (n--) {
    }
}

static void cmt2219b_spi_delay_us(void)
{
    cmt2219b_delay_us(1);
}

void cmt2219b_spi_init(void)
{
    cmt2219b_port_init();

    cmt2219b_csb_high();
    cmt2219b_fcsb_high();
    cmt2219b_clk_low();

    cmt2219b_sdio_output();
    cmt2219b_sdio_high();

    cmt2219b_spi_delay();
}

static void cmt2219b_spi_send(uint8_t data)
{
    uint8_t i;

    for (i = 0; i < 8; i++) {
        cmt2219b_clk_low();

        if (data & 0x80) {
            cmt2219b_sdio_high();
        } else {
            cmt2219b_sdio_low();
        }

        cmt2219b_spi_delay();

        data <<= 1;

        cmt2219b_clk_high();
        cmt2219b_spi_delay();
    }
}

static uint8_t cmt2219b_spi_recv(void)
{
    uint8_t i;
    uint8_t data = 0;

    for (i = 0; i < 8; i++) {
        cmt2219b_clk_low();
        cmt2219b_spi_delay();

        data <<= 1;

        cmt2219b_clk_high();

        if (cmt2219b_sdio_read()) {
            data |= 0x01;
        }

        cmt2219b_spi_delay();
    }

    return data;
}

void cmt2219b_spi_write_reg(uint8_t addr, uint8_t dat)
{
    cmt2219b_sdio_output();
    cmt2219b_sdio_high();

    cmt2219b_clk_low();

    /*
     * 寄存器访问：
     * FCSB 保持高
     * CSB 拉低
     */
    cmt2219b_fcsb_high();
    cmt2219b_csb_low();

    cmt2219b_spi_delay();
    cmt2219b_spi_delay();

    /*
     * 写寄存器：
     * 地址 bit7 = 0
     */
    cmt2219b_spi_send(addr & 0x7F);
    cmt2219b_spi_send(dat);

    cmt2219b_clk_low();

    cmt2219b_spi_delay();
    cmt2219b_spi_delay();

    cmt2219b_csb_high();

    cmt2219b_sdio_high();
    cmt2219b_sdio_input();

    cmt2219b_fcsb_high();
}

uint8_t cmt2219b_spi_read_reg(uint8_t addr)
{
    uint8_t dat;

    cmt2219b_sdio_output();
    cmt2219b_sdio_high();

    cmt2219b_clk_low();

    /*
     * 寄存器访问：
     * FCSB 保持高
     * CSB 拉低
     */
    cmt2219b_fcsb_high();
    cmt2219b_csb_low();

    cmt2219b_spi_delay();
    cmt2219b_spi_delay();

    /*
     * 读寄存器：
     * 地址 bit7 = 1
     */
    cmt2219b_spi_send(addr | 0x80);

    /*
     * 发完地址后，SDIO 要切换成输入
     */
    cmt2219b_sdio_input();

    dat = cmt2219b_spi_recv();

    cmt2219b_clk_low();

    cmt2219b_spi_delay();
    cmt2219b_spi_delay();

    cmt2219b_csb_high();

    cmt2219b_sdio_input();
    cmt2219b_fcsb_high();

    return dat;
}

void cmt2219b_spi_write_fifo(const uint8_t *buf, uint16_t len)
{
    uint16_t i;

    /*
     * FIFO 访问：
     * CSB 保持高
     * FCSB 按字节拉低
     */
    cmt2219b_fcsb_high();
    cmt2219b_csb_high();
    cmt2219b_clk_low();

    cmt2219b_sdio_output();

    for (i = 0; i < len; i++) {
        cmt2219b_fcsb_low();

        cmt2219b_spi_delay();
        cmt2219b_spi_delay();

        cmt2219b_spi_send(buf[i]);

        cmt2219b_clk_low();

        cmt2219b_spi_delay_us();
        cmt2219b_spi_delay_us();
        cmt2219b_spi_delay_us();

        cmt2219b_fcsb_high();

        cmt2219b_spi_delay_us();
        cmt2219b_spi_delay_us();
        cmt2219b_spi_delay_us();
        cmt2219b_spi_delay_us();
        cmt2219b_spi_delay_us();
    }

    cmt2219b_sdio_input();
    cmt2219b_fcsb_high();
}

void cmt2219b_spi_read_fifo(uint8_t *buf, uint16_t len)
{
    uint16_t i;

    /*
     * FIFO 访问：
     * CSB 保持高
     * FCSB 按字节拉低
     * SDIO 切换成输入
     */
    cmt2219b_fcsb_high();
    cmt2219b_csb_high();
    cmt2219b_clk_low();

    cmt2219b_sdio_input();

    for (i = 0; i < len; i++) {
        cmt2219b_fcsb_low();

        cmt2219b_spi_delay();
        cmt2219b_spi_delay();

        buf[i] = cmt2219b_spi_recv();

        cmt2219b_clk_low();

        cmt2219b_spi_delay_us();
        cmt2219b_spi_delay_us();
        cmt2219b_spi_delay_us();

        cmt2219b_fcsb_high();

        cmt2219b_spi_delay_us();
        cmt2219b_spi_delay_us();
        cmt2219b_spi_delay_us();
        cmt2219b_spi_delay_us();
        cmt2219b_spi_delay_us();
    }

    cmt2219b_sdio_input();
    cmt2219b_fcsb_high();
}
