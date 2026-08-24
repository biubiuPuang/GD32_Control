# CMT2219B 接收端驱动移植设计

日期：2026-07-27

## 目标

把官方 `CMT2300A_Demo_Rx` 接收端驱动思路，移植到 `d:/_Project/GD32_Control/Project` 自有工程中。

自有工程 MCU 为 GD32E230，发送端仍使用 PH2119BBA/CMT2119B，接收端新增 PH2219BBA/CMT2219B。

本次移植先按官方 Demo 的无线参数进行：

- 频率：433.920 MHz
- 调制：GFSK
- 数据速率：9.6 kbps
- 固定包长：32 字节
- 白化：关闭
- CRC：关闭

## 硬件连接

### 接收端 PH2219BBA / CMT2219B

| PH2219BBA 引脚 | 名称 | GD32E230 | 说明 |
| --- | --- | --- | --- |
| 1 | FCSB | PB12 | FIFO 片选，普通 GPIO 输出 |
| 2 | CSB | PB13 | SPI 片选，普通 GPIO 输出，低有效 |
| 3 | SDIO | PB15 | 接收模块专用 3 线 SPI 数据脚 |
| 4 | CLK | PB14 | 接收模块专用 SPI 时钟 |
| 5 | GPIO3 | PB10 | 接收中断/DOUT 输入 |
| 7 | VCC | 3V3 | 只能接 3.3V |
| 8 | GND | GND | 共地 |
| 9 | ANT | 433MHz 接收天线 | 不接 MCU |
| 10 | GND | GND | 共地，建议也接 |

### 发送端 PH2119BBA / CMT2119B，保持不变

| PH2119BBA | GD32E230 |
| --- | --- |
| FCSB | PB0 |
| CSB | PA4 |
| SDIO | PA7 |
| SCLK | PA5 |
| GPIO3 | PB1 |
| VCC | 3V3 |
| GND | GND |
| ANT | 433MHz 发射天线 |

## 推荐架构

新增目录：

```text
Hardware/cmt2219b/
```

该目录下新增接收端驱动文件：

```text
cmt2219b_port.h
cmt2219b_port.c
cmt2219b_spi.h
cmt2219b_spi.c
cmt2219b_params.h
cmt2219b.h
cmt2219b.c
radio_rx.h
radio_rx.c
```

现有 `Hardware/cmt2119b/` 发送端驱动保持不变。

## 文件职责

### cmt2219b_port.c / cmt2219b_port.h

负责 GD32E230 端口层：

- 初始化 PB12/PB13/PB14/PB15/PB10 时钟和 GPIO 模式
- 控制 FCSB、CSB、CLK 输出电平
- 控制 SDIO 输出高低电平
- 读取 SDIO 输入电平
- 切换 SDIO 输入/输出模式
- 读取 GPIO3 接收中断输入
- 提供 us/ms 延时包装

### cmt2219b_spi.c / cmt2219b_spi.h

负责 CMT2219B 专用 3 线 SPI 时序：

- 初始化 SPI 控制线默认状态
- 写寄存器
- 读寄存器
- 读 FIFO
- 如芯片时序需要，保留写 FIFO 接口

不使用 GD32E230 硬件 SPI，使用 GPIO 模拟 3 线 SPI。

### cmt2219b_params.h

保存从官方 CMT2300A Demo 移植来的无线参数表，数组命名改为 `cmt2219b` 前缀。

参数表先用于首版移植验证。后续若 CMT2219B 数据手册或 RFPDK 生成参数与官方 Demo 不一致，再替换此文件。

### cmt2219b.c / cmt2219b.h

负责芯片控制层：

- 初始化 GPIO/SPI 和寄存器参数
- 检测芯片是否存在
- 切换 Sleep/Standby/Rx 状态
- 配置 GPIO3 为接收完成相关输出
- 清接收 FIFO
- 清中断标志
- 读取 FIFO 数据
- 判断是否收到包

### radio_rx.c / radio_rx.h

负责应用层接收封装：

- `radio_rx_init()`：初始化接收端并进入 RX 模式
- `radio_rx_poll_packet(uint8_t *buf, uint8_t len)`：轮询是否收到固定长度数据包，收到则读出并重新进入 RX

上层业务只通过 `radio_rx` 接口使用接收功能，不直接访问 CMT2219B 底层寄存器。

## 接收初始化流程

```text
1. 初始化 CMT2219B GPIO 和模拟 SPI
2. 写入官方 Demo 参数表
3. 配置 GPIO3 为接收完成相关输出
4. 关闭低频 OSC 或按 Demo 配置处理
5. 进入 Sleep，使配置生效
6. 切换到 Standby
7. 使能 FIFO 读取模式
8. 清中断标志
9. 清 RX FIFO
10. 进入 RX 模式
```

## 轮询接收流程

```text
1. 读取 GPIO3
2. 如果 GPIO3 未触发，返回 0
3. 如果 GPIO3 触发：
   1. 进入 Standby
   2. 从 FIFO 读取固定长度数据
   3. 清 RX FIFO
   4. 清中断标志
   5. 重新进入 RX
   6. 返回 1
```

## 主程序使用方式

首版测试建议：

```c
#include "radio_tx.h"
#include "radio_rx.h"

static uint8_t tx_buf[32];
static uint8_t rx_buf[32];

int main(void)
{
    systick_config();

    test_packet_init();

    radio_tx_init();
    radio_rx_init();

    while (1) {
        if (radio_rx_poll_packet(rx_buf, sizeof(rx_buf))) {
            /* 收到数据后先打断点观察 rx_buf */
        }

        radio_tx_send(tx_buf, sizeof(tx_buf));
        delay_ms(1000);
    }
}
```

如果同一块板子同时接发射模块和接收模块，首版只验证驱动是否能工作。后续可根据实际业务决定是否需要半双工收发调度。

## Keil 工程集成

需要加入源码：

```text
Hardware/cmt2219b/cmt2219b_port.c
Hardware/cmt2219b/cmt2219b_spi.c
Hardware/cmt2219b/cmt2219b.c
Hardware/cmt2219b/radio_rx.c
```

需要加入头文件路径：

```text
..\Hardware\cmt2219b
```

## 验证顺序

1. 验证 GPIO 初始化：PB12/PB13/PB14/PB15/PB10 模式正确。
2. 验证模拟 SPI 波形：CSB、CLK、SDIO 是否有正确时序。
3. 验证芯片检测：`cmt2219b_is_exist()` 能读到有效芯片信息。
4. 验证参数写入：初始化时能看到连续寄存器写入波形。
5. 验证进入 RX：初始化后模块保持接收状态。
6. 验证收包：CMT2119B 每秒发送 32 字节，CMT2219B 收到后 `radio_rx_poll_packet()` 返回 1，并能读取 FIFO 数据。

## 非目标

本次不做以下内容：

- 不修改现有 CMT2119B 发射端引脚和驱动结构。
- 不把收发驱动合并成一个大 `radio` 驱动。
- 不引入硬件 SPI。
- 不做复杂业务协议解析。
- 不做低功耗收发调度。

## 风险和注意事项

1. 官方 Demo 是 CMT2300A，实际接收芯片是 CMT2219B。首版按官方参数迁移，但最终仍需要用 CMT2219B 手册或 RFPDK 参数确认寄存器兼容性。
2. SDIO 是 3 线 SPI 数据脚，读寄存器和读 FIFO 时必须切换为输入模式，写寄存器时必须切换为输出模式。
3. GPIO3 的具体含义需要和 CMT2219B 配置对应。首版按接收完成或相关中断输入处理。
4. 同板自发自收时，发射模块可能影响接收模块前端，必要时需要分时处理或拉开天线距离。
