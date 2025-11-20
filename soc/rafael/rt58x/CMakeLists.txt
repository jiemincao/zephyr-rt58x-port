# Copyright (c) 2020, 2021 Antony Pavlov <antonynpavlov@gmail.com>
#
# SPDX-License-Identifier: Apache-2.0

# zephyr_include_directories(${ZEPHYR_BASE}/drivers)

set(RT58X_PERIPHERAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/peripheral)

zephyr_library()

zephyr_library_sources(
  ${CMAKE_CURRENT_SOURCE_DIR}/soc.c
  ${RT58X_PERIPHERAL_DIR}/sysctrl.c
  # ${RT58X_PERIPHERAL_DIR}/uart_drv.c
  ${RT58X_PERIPHERAL_DIR}/gpio.c)


zephyr_include_directories(${RT58X_PERIPHERAL_DIR}/include)
zephyr_include_directories(${RT58X_PERIPHERAL_DIR}) 
zephyr_include_directories(.)

set(SOC_LINKER_SCRIPT ${ZEPHYR_BASE}/include/zephyr/arch/arm/cortex_m/scripts/linker.ld CACHE INTERNAL "")

