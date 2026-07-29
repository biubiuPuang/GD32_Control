#include "app_221.h"

extern void print_buf(uint8_t *buf, uint8_t len);

// �����ֽڳ��� �̶�32�ֽ�
#define TEST_PACKET_LEN 32
// �����ֽ�����
static uint8_t rx_buf[TEST_PACKET_LEN];
static uint8_t rx_ok;
static uint32_t rx_count = 0;

// 221����оƬ��ʼ��
void app_221_init(void)
{
    // ʱ������
    systick_config();
    // ���ڳ�ʼ������
    usart_gpio_config(115200);

    // �ж�221����оƬ��ʼ���Ƿ�ɹ�
    rx_ok = radio_rx_init();
    if (rx_ok)
    {
        printf("RX init OK\r\n");
    }
    else
    {
        printf("RX init ERROR\r\n");
    }
}

// 221����оƬ����32���ֽڵ�����
void app_221_receive_data(void)
{
    /*
     * �����жϵ��ǽ���оƬ�ڲ� INT_FLAG �Ĵ�����
     * 0x6D �� INT_FLAG��bit0 = 1 ��ʾ����оƬ�յ��������ݰ���
     * ���������ӡ���������ݣ��Ǵ� PH2219BBA ����оƬ FIFO ����������ġ�
     */
    if (rx_ok && (cmt2219b_read_reg(0x6D) & 0x01))
    {
        cmt2219b_go_stby();

        /*
         * �ӽ���оƬ FIFO ��ȡ���ݣ�����ֱ�Ӵ�ӡ tx_buf��
         */
        cmt2219b_read_fifo(rx_buf, TEST_PACKET_LEN);

        cmt2219b_clear_rx_fifo();
        cmt2219b_clear_interrupt_flags();
        cmt2219b_go_rx();

        rx_count++;

        printf("RX chip FIFO data, count=%u: ", rx_count);
        print_buf(rx_buf, TEST_PACKET_LEN);
    }
    else
    {
        printf("RX chip no packet\r\n");
    }
}
