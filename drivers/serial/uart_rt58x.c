/*
 * Copyright (c) 2024 Rafael Microelectronics Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT rafael_rt58x_uart

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <soc.h>
#include "sysctrl.h" 
#include "gpio.h"     /* 引入 gpio.h 以使用 pin_set_mode 和 MODE_UART */

/* * 1. UART 暫存器結構 (對應硬體 Offset)
 */
typedef struct {
    volatile uint32_t RBR_THR_DLL; /* 0x00 */
    volatile uint32_t IER_DLM;     /* 0x04 */
    volatile uint32_t IIR_FCR;     /* 0x08 */
    volatile uint32_t LCR;         /* 0x0C */
    volatile uint32_t MCR;         /* 0x10 */
    volatile uint32_t LSR;         /* 0x14 */
    volatile uint32_t MSR;         /* 0x18 */
    volatile uint32_t SPR;         /* 0x1C */
} rt58x_uart_regs_t;

/* 暫存器位元定義 */
#define LCR_DLAB     (1 << 7)
#define LCR_CS8      (3 << 0)
#define LCR_1_STOP   (0 << 2)
#define LCR_NO_PARITY 0x00

#define LSR_DR       (1 << 0)
// #define LSR_THRE     (1 << 5)

#define FCR_ENABLE   (1 << 0)
#define FCR_RX_RST   (1 << 1)
#define FCR_TX_RST   (1 << 2)

/* * 2. 關鍵修正：使用原廠定義的 Baudrate 除數 
 * (抄自 uart_drv.h)
 */
#define RT58X_BAUD_115200    35
#define RT58X_BAUD_9600      417
#define RT58X_BAUD_1000000   4

struct uart_rt58x_config {
    uint32_t base;
    uint32_t baudrate;
    uint8_t  uart_id;
};

struct uart_rt58x_data {};

#define DEV_CFG(dev) ((const struct uart_rt58x_config *)(dev)->config)
#define UART_REG(dev) ((rt58x_uart_regs_t *)(DEV_CFG(dev)->base))

/*
 * Poll Out: 輸出一個字元
 */
static void uart_rt58x_poll_out(const struct device *dev, unsigned char c)
{
    rt58x_uart_regs_t *uart = UART_REG(dev);

    /* 等待 TX Empty */
    while (!(uart->LSR & LSR_THRE));

    /* 寫入資料 */
    uart->RBR_THR_DLL = c;
}

/*
 * Poll In: 讀取一個字元
 */
static int uart_rt58x_poll_in(const struct device *dev, unsigned char *c)
{
    rt58x_uart_regs_t *uart = UART_REG(dev);

    /* 檢查 RX Ready */
    if (uart->LSR & LSR_DR) {
        *c = (unsigned char)(uart->RBR_THR_DLL & 0xFF);
        return 0;
    }
    return -1;
}

static const struct uart_driver_api uart_rt58x_driver_api = {
    .poll_in = uart_rt58x_poll_in,
    .poll_out = uart_rt58x_poll_out,
};

/*
 * 初始化
 */
static int uart_rt58x_init(const struct device *dev)
{
    const struct uart_rt58x_config *config = DEV_CFG(dev);
    rt58x_uart_regs_t *uart = UART_REG(dev);
    uint32_t divisor;

    /* 1. 開啟 Clock (System Control) */
    /* UART0=16, UART1=17, UART2=18 */
    enable_perclk(16 + config->uart_id);

    /* 2. 設定 Pinmux (使用 gpio.h 的 pin_set_mode) */
    /* 如果 MODE_UART 沒定義，請在 gpio.h 裡找，或者暫時用 1 */
    #ifndef MODE_UART
    #define MODE_UART 1 
    #endif

    if (config->uart_id == 0) {
        pin_set_mode(16, MODE_UART); /* UART0 RX */
        pin_set_mode(17, MODE_UART); /* UART0 TX */
    } else if (config->uart_id == 1) {
        pin_set_mode(28, MODE_UART); /* UART1 TX */
        pin_set_mode(29, MODE_UART); /* UART1 RX */
        /* main.c 還有設定 RTS/CTS，我們暫時不需要 */
    }

    /* 3. 設定 Baudrate (使用原廠魔術數字) */
    switch (config->baudrate) {
        case 115200: divisor = RT58X_BAUD_115200; break;
        case 9600:   divisor = RT58X_BAUD_9600;   break;
        case 1000000:divisor = RT58X_BAUD_1000000;break;
        default:     divisor = RT58X_BAUD_115200; break;
    }

    /* LCR: DLAB=1 */
    uart->LCR = LCR_DLAB | LCR_CS8;
    /* 寫入 Divisor */
    uart->RBR_THR_DLL = divisor & 0xFF;
    uart->IER_DLM     = (divisor >> 8) & 0xFF;
    /* LCR: DLAB=0, 8N1 */
    uart->LCR = LCR_CS8 | LCR_1_STOP | LCR_NO_PARITY;

    /* 4. FIFO Reset */
    uart->IIR_FCR = FCR_ENABLE | FCR_RX_RST | FCR_TX_RST;

    /* 5. 關閉中斷 */
    uart->IER_DLM = 0;

    return 0;
}

/* 補上基底位址 (如果 soc.h 沒定義) */
#ifndef UART0_BASE
#define UART0_BASE 0xA0000000
#define UART1_BASE 0xA0500000
#define UART2_BASE 0xA0600000
#endif

#define RT58X_UART_INIT(n)                                                          \
    static const struct uart_rt58x_config uart_rt58x_cfg_##n = {                    \
        .base = DT_INST_REG_ADDR(n),                                                \
        .baudrate = DT_INST_PROP(n, current_speed),                                 \
        .uart_id = (DT_INST_REG_ADDR(n) == UART0_BASE) ? 0 :                        \
                   (DT_INST_REG_ADDR(n) == UART1_BASE) ? 1 :                        \
                   2,                                                               \
    };                                                                              \
    static struct uart_rt58x_data uart_rt58x_data_##n;                              \
    DEVICE_DT_INST_DEFINE(n,                                                        \
                          uart_rt58x_init,                                          \
                          NULL,                                                     \
                          &uart_rt58x_data_##n,                                     \
                          &uart_rt58x_cfg_##n,                                      \
                          PRE_KERNEL_1,                                             \
                          CONFIG_SERIAL_INIT_PRIORITY,                              \
                          &uart_rt58x_driver_api);

DT_INST_FOREACH_STATUS_OKAY(RT58X_UART_INIT)