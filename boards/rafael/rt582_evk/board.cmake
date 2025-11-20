# Copyright (c) 2016 Zephyr Contributors
# SPDX-License-Identifier: Apache-2.0

# set(FLASH_START 0x8000)
# set(FLASH_SIZE  0xE8000) # 928K

# set(RAM_START 0x20000000)
# set(RAM_SIZE  0x24000) # 144K
board_runner_args(openocd "--config=${BOARD_DIR}/support/openocd.cfg")

include(${ZEPHYR_BASE}/boards/common/openocd.board.cmake)