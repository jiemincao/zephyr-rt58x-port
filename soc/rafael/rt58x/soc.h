/*
 * Copyright (c) 2024 Rafael Microelectronics Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _SOC_H_
#define _SOC_H_

#include <zephyr/sys/util.h>

#ifndef _ASMLANGUAGE

#include "cm3_mcu.h"

#undef __MPU_PRESENT
#if defined(CONFIG_CPU_HAS_ARM_MPU)
#define __MPU_PRESENT 1
#else
#define __MPU_PRESENT 0
#endif

#include <cmsis_core.h>

#endif /* !_ASMLANGUAGE */

#endif /* _SOC_H_ */