 /******************************************************************************
   * 测试硬件：立创开发板·GD32E230C8T6    使用主频72Mhz    晶振8Mhz
   * 版 本 号: V1.0
   * 修改作者: www.lckfb.com
   * 修改日期: 2023年11月02日
   * 功能介绍:      
   *****************************************************************************
   * 梁山派软硬件资料与相关扩展板软硬件资料官网全部开源  
   * 开发板官网：www.lckfb.com   
   * 技术支持常驻论坛，任何技术问题欢迎随时交流学习  
   * 立创论坛：club.szlcsc.com   
   * 其余模块移植手册：【立创·GD32E230C8T6开发板】模块移植手册
   * 关注bilibili账号：【立创开发板】，掌握我们的最新动态！
   * 不靠卖板赚钱，以培养中国工程师为己任
  ******************************************************************************/
#include "bsp_usart.h"
#include "stdio.h"
#include "string.h"


/* 原有命令接收缓冲区，继续给rf_uart_config_process()使用 */
uint8_t g_recv_buff[USART_RECEIVE_LENGTH];
volatile uint16_t g_recv_length = 0U;
volatile uint8_t g_recv_complete_flag = 0U;


/*
 * USART0 RX DMA临时缓冲区。
 * DMA只往这里写；IDLE中断确认一帧结束后，
 * 再复制到上面的g_recv_buff。
 */
static uint8_t g_usart_rx_dma_buffer[USART_RX_DMA_LENGTH];

/* USART0 TX DMA环形队列 */
static uint8_t g_usart_tx_queue[USART_TX_QUEUE_LENGTH];
static volatile uint16_t g_usart_tx_read = 0U;
static volatile uint16_t g_usart_tx_write = 0U;
static volatile uint16_t g_usart_tx_dma_length = 0U;
static volatile uint8_t g_usart_tx_dma_busy = 0U;

/* 通信异常统计 */
volatile uint32_t g_usart_tx_overflow_count = 0U;
volatile uint32_t g_usart_rx_overflow_count = 0U;
volatile uint32_t g_usart_dma_error_count = 0U;

static void usart_rx_dma_init(void);
static void usart_rx_dma_restart(void);
static void usart_tx_dma_start(void);
static void usart_tx_dma_complete(void);

static void usart_rx_dma_init(void)
{
    dma_parameter_struct dma_init_struct;

    /* 先复位DMA_CH2 */
    dma_deinit(BSP_USART_RX_DMA_CHANNEL);

    /* 给DMA配置结构体填默认值 */
    dma_struct_para_init(&dma_init_struct);

    /*
     * USART0接收数据寄存器地址。
     * DMA从这里读取收到的字节。
     */
    dma_init_struct.periph_addr = (uint32_t)&USART_RDATA(BSP_USART);

    /*
     * DMA把数据写入临时缓冲区。
     */
    dma_init_struct.memory_addr = (uint32_t)g_usart_rx_dma_buffer;

    /* USART数据和内存数组都是按字节收发 */
    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;

    /* 本次最多接收128字节 */
    dma_init_struct.number = USART_RX_DMA_LENGTH;

    /* USART接收数据寄存器地址固定，不能递增 */
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;

    /* 数组地址随每个字节递增 */
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;

    /* 方向：USART外设 → 内存 */
    dma_init_struct.direction = DMA_PERIPHERAL_TO_MEMORY;

    /* 接收优先级设高 */
    dma_init_struct.priority = DMA_PRIORITY_HIGH;

    dma_init(BSP_USART_RX_DMA_CHANNEL, &dma_init_struct);

    /*
     * 不使用循环模式。
     * 每次IDLE中断后重新装载DMA计数，开始接收下一帧。
     */
    dma_circulation_disable(BSP_USART_RX_DMA_CHANNEL);

    /* 清除DMA_CH2可能残留的标志 */
    dma_flag_clear(BSP_USART_RX_DMA_CHANNEL, DMA_FLAG_G);

    /* 允许USART0向DMA发出接收请求 */
    usart_dma_receive_config(BSP_USART, USART_DENR_ENABLE);

    /* 最后启动DMA_CH2 */
    dma_channel_enable(BSP_USART_RX_DMA_CHANNEL);
}


static void usart_rx_dma_restart(void)
{
    /* 修改DMA计数和地址前，必须先关闭DMA通道 */
    dma_channel_disable(BSP_USART_RX_DMA_CHANNEL);

    /* 接收地址仍然使用DMA临时缓冲区 */
    dma_memory_address_config(BSP_USART_RX_DMA_CHANNEL,
                              (uint32_t)g_usart_rx_dma_buffer);

    /* 重新装载接收数量 */
    dma_transfer_number_config(BSP_USART_RX_DMA_CHANNEL,
                               USART_RX_DMA_LENGTH);

    /* 清除DMA标志 */
    dma_flag_clear(BSP_USART_RX_DMA_CHANNEL, DMA_FLAG_G);

    /* 再次启动DMA，继续等待下一帧 */
    dma_channel_enable(BSP_USART_RX_DMA_CHANNEL);
}

static void usart_tx_dma_start(void)
{
    dma_parameter_struct dma_init_struct;
    uint16_t continuous_length;

    /*
     * 已有DMA正在发送，不能重复启动。
     */
    if (g_usart_tx_dma_busy != 0U)
    {
        return;
    }

    /*
     * 读指针和写指针相同，说明队列为空。
     */
    if (g_usart_tx_read == g_usart_tx_write)
    {
        usart_dma_transmit_config(BSP_USART, USART_DENT_DISABLE);
        return;
    }

    /*
     * 环形队列回绕时，一次DMA只能发送物理内存连续的一段。
     */
    if (g_usart_tx_write > g_usart_tx_read)
    {
        continuous_length = g_usart_tx_write - g_usart_tx_read;
    }
    else
    {
        continuous_length = USART_TX_QUEUE_LENGTH - g_usart_tx_read;
    }

    g_usart_tx_dma_busy = 1U;
    g_usart_tx_dma_length = continuous_length;

    /* 重新配置DMA_CH1前先关闭并复位 */
    dma_channel_disable(BSP_USART_TX_DMA_CHANNEL);
    dma_deinit(BSP_USART_TX_DMA_CHANNEL);
    dma_struct_para_init(&dma_init_struct);

    /*
     * DMA写到USART0发送数据寄存器。
     */
    dma_init_struct.periph_addr = (uint32_t)&USART_TDATA(BSP_USART);

    /*
     * DMA从环形队列当前读位置取数据。
     */
    dma_init_struct.memory_addr =
        (uint32_t)&g_usart_tx_queue[g_usart_tx_read];

    dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.number = continuous_length;

    /* USART发送数据寄存器地址固定 */
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;

    /* 队列数组地址随发送字节递增 */
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;

    /* 方向：内存 → USART外设 */
    dma_init_struct.direction = DMA_MEMORY_TO_PERIPHERAL;

    dma_init_struct.priority = DMA_PRIORITY_MEDIUM;

    dma_init(BSP_USART_TX_DMA_CHANNEL, &dma_init_struct);

    /* 清除旧状态 */
    dma_flag_clear(BSP_USART_TX_DMA_CHANNEL, DMA_FLAG_G);

    /* 开启“本段发送完成”和“DMA错误”中断 */
    dma_interrupt_enable(BSP_USART_TX_DMA_CHANNEL, DMA_INT_FTF);
    dma_interrupt_enable(BSP_USART_TX_DMA_CHANNEL, DMA_INT_ERR);

    /* 允许USART0向DMA发出发送请求 */
    usart_dma_transmit_config(BSP_USART, USART_DENT_ENABLE);

    /* 最后启动DMA_CH1 */
    dma_channel_enable(BSP_USART_TX_DMA_CHANNEL);
}

static void usart_tx_dma_complete(void)
{
    /* 当前DMA段已完成，先关闭通道 */
    dma_channel_disable(BSP_USART_TX_DMA_CHANNEL);

    /*
     * 读指针跨过本次已经发送的数据。
     */
    g_usart_tx_read += g_usart_tx_dma_length;

    if (g_usart_tx_read >= USART_TX_QUEUE_LENGTH)
    {
        g_usart_tx_read = 0U;
    }

    g_usart_tx_dma_length = 0U;
    g_usart_tx_dma_busy = 0U;

    /*
     * 继续检查队列：
     * - 有数据：开始发送下一段
     * - 没数据：usart_tx_dma_start()会关闭TX DMA请求
     */
    usart_tx_dma_start();
}

/************************************************
函数名称 ： usart_gpio_config
功    能 ： 串口配置GPIO
参    数 ： band_rate:波特率
返 回 值 ： 无
作    者 ： LC
*************************************************/
void usart_gpio_config(uint32_t band_rate)
{
	/* 开启时钟 */
	rcu_periph_clock_enable(BSP_USART_TX_RCU);   // 开启串口时钟
	rcu_periph_clock_enable(BSP_USART_RX_RCU);   // 开启端口时钟
	rcu_periph_clock_enable(BSP_USART_RCU);      // 开启端口时钟
	rcu_periph_clock_enable(RCU_DMA);			// 开启DMA
	
	/* 配置GPIO复用功能 */
 	gpio_af_set(BSP_USART_TX_PORT,BSP_USART_AF,BSP_USART_TX_PIN);	
	gpio_af_set(BSP_USART_RX_PORT,BSP_USART_AF,BSP_USART_RX_PIN);	
	
	/* 配置GPIO的模式 */
	/* 配置TX为复用模式 上拉模式 */
	gpio_mode_set(BSP_USART_TX_PORT,GPIO_MODE_AF,GPIO_PUPD_PULLUP,BSP_USART_TX_PIN);
	/* 配置RX为复用模式 上拉模式 */
	gpio_mode_set(BSP_USART_RX_PORT, GPIO_MODE_AF,GPIO_PUPD_PULLUP,BSP_USART_RX_PIN);
	
	/* 配置TX为推挽输出 50MHZ */
	gpio_output_options_set(BSP_USART_TX_PORT,GPIO_OTYPE_PP,GPIO_OSPEED_50MHZ,BSP_USART_TX_PIN);
	/* 配置RX为推挽输出 50MHZ */
	gpio_output_options_set(BSP_USART_RX_PORT,GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_USART_RX_PIN);

	/* 配置串口的参数 */
	usart_deinit(BSP_USART);                                 // 复位串口
	usart_baudrate_set(BSP_USART,band_rate);                 // 设置波特率
	usart_parity_config(BSP_USART,USART_PM_NONE);            // 没有校验位
	usart_word_length_set(BSP_USART,USART_WL_8BIT);          // 8位数据位
	usart_stop_bit_set(BSP_USART,USART_STB_1BIT);     			 // 1位停止位

  /* 使能串口 */
	usart_enable(BSP_USART);                                 // 使能串口
	usart_transmit_config(BSP_USART,USART_TRANSMIT_ENABLE);  // 使能串口发送
	usart_receive_config(BSP_USART,USART_RECEIVE_ENABLE);    // 使能串口接收
	
	/*
	* 接收数据由DMA_CH2搬运。
	* 因此关闭RBNE逐字节接收中断，避免CPU和DMA同时读取接收寄存器。
	*/
	usart_interrupt_disable(BSP_USART, USART_INT_RBNE);

	/*
	* IDLE：上位机停发一段时间，表示一帧结束。
	* ERR：检测串口溢出、噪声和帧错误。
	*/
	usart_interrupt_enable(BSP_USART, USART_INT_IDLE);
	usart_interrupt_enable(BSP_USART, USART_INT_ERR);

	/*
	* DMA_CH1和DMA_CH2共用DMA_Channel1_2_IRQn。
	* DMA中断优先级高于USART IDLE中断。
	*/
	nvic_irq_enable(BSP_USART_DMA_IRQ, 1U);
	nvic_irq_enable(BSP_USART_IRQ, 2U);

	/* 最后启动USART0接收DMA */
	usart_rx_dma_init();
}

uint16_t usart_send_buffer(const uint8_t *data, uint16_t length)
{
    uint16_t written = 0U;
    uint16_t next_write;

    if ((data == NULL) || (length == 0U))
    {
        return 0U;
    }

    /*
     * 主循环在写队列；
     * DMA完成中断会修改读指针并启动下一段DMA。
     * 所以只暂停DMA_CH1_2中断，保护队列读写指针。
     */
    nvic_irq_disable(BSP_USART_DMA_IRQ);

    while (written < length)
    {
        next_write = g_usart_tx_write + 1U;

        if (next_write >= USART_TX_QUEUE_LENGTH)
        {
            next_write = 0U;
        }

        /*
         * 下一个写位置追上读位置，说明队列满。
         * 不等待，不死循环，直接退出。
         */
        if (next_write == g_usart_tx_read)
        {
            g_usart_tx_overflow_count++;
            break;
        }

        g_usart_tx_queue[g_usart_tx_write] = data[written];
        g_usart_tx_write = next_write;
        written++;
    }

    /*
     * 数据已写入队列；如果DMA当前空闲，立即从队列取数据发送。
     * 此处DMA中断仍关闭，避免启动DMA时与DMA完成中断竞争。
     */
    usart_tx_dma_start();

    nvic_irq_enable(BSP_USART_DMA_IRQ, 1U);

    return written;
}

/************************************************
函数名称 ： usart_send_data
功    能 ： 串口重发送一个字节
参    数 ： ucch：要发送的字节
返 回 值 ： 
作    者 ： LC
*************************************************/
void usart_send_data(uint8_t ucch)
{
	(void)usart_send_buffer(&ucch, 1U);
}


/************************************************
函数名称 ： usart_send_String
功    能 ： 串口发送字符串
参    数 ： ucstr:要发送的字符串
返 回 值 ： 
作    者 ： LC
*************************************************/
void usart_send_string(uint8_t *ucstr)
{
    uint16_t length = 0U;

    if (ucstr == NULL)
    {
        return;
    }

    while (ucstr[length] != '\0')
    {
        length++;
    }

    (void)usart_send_buffer(ucstr, length);
}

/************************************************
函数名称 ： fputc
功    能 ： 串口重定向函数
参    数 ： 
返 回 值 ： 
作    者 ： LC
*************************************************/
int fputc(int ch, FILE *f)
{
     usart_send_data(ch);
     // 等待发送数据缓冲区标志置位
     return ch;
}

void BSP_USART_IRQHandler(void)
{
    uint16_t received_length;
    uint8_t usart_error = 0U;

    /*
     * 先处理USART错误。
     * DMA正常接收时，不要再手动读取USART接收寄存器，
     * 否则会抢走DMA本应读取的数据。
     */
    if (usart_flag_get(BSP_USART, USART_FLAG_ORERR) == SET)
    {
        usart_interrupt_flag_clear(BSP_USART, USART_INT_FLAG_ERR_ORERR);
        usart_error = 1U;
    }

    if (usart_flag_get(BSP_USART, USART_FLAG_NERR) == SET)
    {
        usart_interrupt_flag_clear(BSP_USART, USART_INT_FLAG_ERR_NERR);
        usart_error = 1U;
    }

    if (usart_flag_get(BSP_USART, USART_FLAG_FERR) == SET)
    {
        usart_interrupt_flag_clear(BSP_USART, USART_INT_FLAG_ERR_FERR);
        usart_error = 1U;
    }

    if (usart_error != 0U)
    {
        g_usart_dma_error_count++;
        usart_rx_dma_restart();
        return;
    }

    /*
     * IDLE表示上位机已经停止发送一段时间，
     * 把它作为一帧命令结束标志。
     */
    if (usart_interrupt_flag_get(BSP_USART, USART_INT_FLAG_IDLE) == SET)
    {
        /*
         * 先停止DMA，冻结DMA剩余数量。
         */
        dma_channel_disable(BSP_USART_RX_DMA_CHANNEL);

        received_length = USART_RX_DMA_LENGTH -
            (uint16_t)dma_transfer_number_get(BSP_USART_RX_DMA_CHANNEL);

        /*
         * 清除IDLE标志。
         * 不需要usart_data_receive()，DMA已经读取了接收寄存器。
         */
        usart_interrupt_flag_clear(BSP_USART, USART_INT_FLAG_IDLE);

        /*
         * 长度为0：只是线路空闲，没有有效命令。
         * 长度达到DMA缓冲区上限：认为本帧超长。
         * 上一帧未处理完成：丢弃当前帧，保护上一帧命令。
         */
        if ((received_length == 0U) ||
            (received_length >= USART_RX_DMA_LENGTH) ||
            (g_recv_complete_flag != 0U))
        {
            if (received_length >= USART_RX_DMA_LENGTH)
            {
                g_usart_rx_overflow_count++;
            }
        }
        else
        {
            /*
             * 复制到原来的g_recv_buff。
             * 后面的rf_uart_config_process()可继续使用原有变量。
             */
            memcpy(g_recv_buff,
                   g_usart_rx_dma_buffer,
                   received_length);

            g_recv_buff[received_length] = '\0';
            g_recv_length = received_length;
            g_recv_complete_flag = 1U;
        }

        /*
         * 无论本帧是否有效，都恢复DMA，准备接收下一帧。
         */
        usart_rx_dma_restart();
    }
}

void DMA_Channel1_2_IRQHandler(void)
{
    /*
     * DMA_CH1：USART0发送DMA。
     */
    if (dma_interrupt_flag_get(BSP_USART_TX_DMA_CHANNEL,
                               DMA_INT_FLAG_FTF) == SET)
    {
        /* 清除DMA发送完成标志 */
        dma_interrupt_flag_clear(BSP_USART_TX_DMA_CHANNEL,
                                 DMA_INT_FLAG_FTF);

        /* 当前连续段已发送完，推进读指针并继续下一段 */
        usart_tx_dma_complete();
    }

    if (dma_interrupt_flag_get(BSP_USART_TX_DMA_CHANNEL,
                               DMA_INT_FLAG_ERR) == SET)
    {
        dma_interrupt_flag_clear(BSP_USART_TX_DMA_CHANNEL,
                                 DMA_INT_FLAG_ERR);

        dma_channel_disable(BSP_USART_TX_DMA_CHANNEL);
        usart_dma_transmit_config(BSP_USART, USART_DENT_DISABLE);

        /*
         * DMA错误时，为避免TX队列一直卡死，
         * 丢弃当前待发送的日志数据并记录错误。
         */
        g_usart_tx_read = g_usart_tx_write;
        g_usart_tx_dma_length = 0U;
        g_usart_tx_dma_busy = 0U;
        g_usart_dma_error_count++;
    }

    /*
     * DMA_CH2是RX DMA。
     * 当前方案不启用CH2的DMA完成中断，
     * RX帧结束由USART0的IDLE中断处理。
     */
}
