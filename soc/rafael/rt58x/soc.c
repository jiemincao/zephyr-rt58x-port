/*
 * Copyright (c) 2024 Rafael Microelectronics Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <soc.h>

/* 暫存器定義 */
#define REG_GPIO_BASE     0x40000000
#define REG_SYSCTRL_BASE  0x40800000
#define REG_SYS_CLK_CTRL  (REG_SYSCTRL_BASE + 0x04)
#define OFS_GPIO_OUT_EN   0x08

#define LED_PIN           15
#define MODE_GPIO         0
#define REG32(addr)       (*((volatile uint32_t *)(addr)))

extern void z_arm_reset(void);
extern void pin_set_mode(uint32_t pin, uint32_t mode);

/* SystemInit (CMSIS 標準) */
void SystemInit(void)
{
    /* 可以在這裡做更細緻的時鐘初始化 */
}

/* Reset_Handler: 程式入口 */
void Reset_Handler(void)
{
    /* 1. 開啟所有時鐘 (為了讓 UART 和 GPIO 能動) */
    REG32(REG_SYS_CLK_CTRL) = 0xFFFFFFFF;

    /* 2. 設定 GPIO 模式 (雖然 main.c 也會設，但在這裡設也無妨) */
    /* 這一步可以保留，確保開機瞬間腳位狀態正確 */
    pin_set_mode(LED_PIN, MODE_GPIO);
    REG32(REG_GPIO_BASE + OFS_GPIO_OUT_EN) |= (1 << LED_PIN);
    
    /* --- 刪除死迴圈閃燈 --- */
    /* while(1) { ... }  <-- 兇手就是它，刪掉！ */

    /* 3. 進入 Zephyr 核心 (這會跳轉到 main) */
    SystemInit();
    z_arm_reset();
}

/* Zephyr 初始化掛鉤 */
static int rt582_init(void)
{
    return 0;
}
SYS_INIT(rt582_init, PRE_KERNEL_1, 0);