#include "app_211.h"

// �����ֽڵĳ��ȹ̶�32�ֽ�
#define TEST_PACKET_LEN 32

/*
 * �⴮����Ϊ�������� 11 22 33 44 55 ��ӡ�����������ʾ��ǰ���͵��ǲ�������
 * ��Ҫ���͵����ݾ�������ġ�
 * ע�ⳤ�ȱ��� 32 �ֽڣ���Ϊ��ǰ RFPDK ���õ��ǹ̶� 32 �ֽڰ���
 */
static uint8_t tx_buf_data[TEST_PACKET_LEN] = {
    0x11,
    0x22,
    0x33,
    0x44,
    0x55,
    0x66,
    0x77,
    0x88,
    0x99,
    0xAA,
    0xBB,
    0xCC,
    0xDD,
    0xEE,
    0xFF,
    0x00,
    0x01,
    0x02,
    0x03,
    0x04,
    0x05,
    0x06,
    0x07,
    0x08,
    0x09,
    0x0A,
    0x0B,
    0x0C,
    0x0D,
    0x0E,
    0x0F,
    0x10,
};

static uint8_t tx_ok;
static uint8_t send_ret;
static uint32_t tx_count = 0;

/**
 * @brief ��ȡGD32E230оƬ��ΨһUID
 *
 * GD32E230��ΨһID��96λ
 * @param uid��С:3�ֽ�
 */
void gd32_get_uid(uint32_t uid[3])
{
    volatile const uint32_t *p = (volatile const uint32_t *)GD32_UID_BASE;

    uid[0] = p[0]; // 0x1FFF F7AC
    uid[1] = p[1]; // 0x1FFF F7B0
    uid[2] = p[2]; // 0x1FFF F7B4
}

/**
 *
 * @brief ���ڴ�ӡ,��һ���ֽڰ���16���ƴ�ӡ����
 * ��ͨ��printf��ӡ������ɲ����� ����16���������������������
 * @param buf ��Ҫ��ӡ���ֽ�
 * @param len ��Ҫ��ӡ���ֽڳ���
 */
void print_buf(uint8_t *buf, uint8_t len)
{
    uint8_t i;

    for (i = 0; i < len; i++)
    {
        printf("%02X ", buf[i]);
    }

    printf("\r\n");
}

/**
 * @brief 211����оƬ��ʼ������
 *
 * @return uint8_t ��ʼ���Ƿ�ɹ��ı�־λ
 */
void app_211_init(void)
{
    // ʱ������
    systick_config();
    // ���ڳ�ʼ������
    usart_gpio_config(115200);

    // 211����оƬ��ʼ��
    tx_ok = radio_tx_init();
    // �ж�211����оƬ��ʼ���Ƿ�ɹ�
    if (tx_ok)
    {
        printf("TX init OK\r\n");
    }
    else
    {
        printf("TX init ERROR\r\n");
    }
}

/**
 * @brief 211����оƬ����32�ֽڵ�����
 *
 */
void app_211_send_data(void)
{
    printf("User TX data: ");
    // ��ӡ��ǰ�û���д������,���ӡ��11 22 33...���ʾ�ǳ�ʼ���Ĳ�������
    print_buf(tx_buf_data, TEST_PACKET_LEN);

    // �������� tx_buf ������д������
    if (tx_ok)
    {
        send_ret = radio_tx_send(tx_buf_data, TEST_PACKET_LEN);
        tx_count++;

        if (send_ret)
        {
            printf("\r\nTX OK, count=%u\r\n", tx_count);
        }
        else
        {
            printf("\r\nTX ERROR, count=%u\r\n", tx_count);
        }
    }
}
