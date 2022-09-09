# SPDX-License-Identifier: Apache-2.0

if (CONFIG_LOCKING)

zephyr_include_directories(universal/include)
zephyr_sources(
    universal/source/locking.c
)

zephyr_sources_ifdef(CONFIG_LOCKING_SHELL
    universal/source/locking_shell.c
)

if(CONFIG_LOCKING_DEVICE_OVERRIDE)

if(CONFIG_LOCKING_DEVICE_OVERRIDE_SOURCE_FOLDER)
set(LOCKING_CUSTOM_PATH_BASE "${CMAKE_SOURCE_DIR}")
else()
set(LOCKING_CUSTOM_PATH_BASE "custom")
endif() # LOCKING_DEVICE_OVERRIDE_SOURCE_FOLDER

zephyr_include_directories(${LOCKING_CUSTOM_PATH_BASE}/${CONFIG_LOCKING_DEVICE_OVERRIDE_NAME}/include)
zephyr_sources(
    ${LOCKING_CUSTOM_PATH_BASE}/${CONFIG_LOCKING_DEVICE_OVERRIDE_NAME}/source/locking_table.c
)

else()

if(CONFIG_BOARD_PINNACLE_100_DVK)
zephyr_include_directories(custom/MG100/include)
zephyr_sources(
    custom/MG100/source/locking_table.c
)
endif()

if(CONFIG_BOARD_MG100)
zephyr_include_directories(custom/MG100/include)
zephyr_sources(
    custom/MG100/source/locking_table.c
)
endif()

if(CONFIG_BOARD_BL5340_DVK_CPUAPP OR CONFIG_BOARD_BL5340PA_DVK_CPUAPP)
zephyr_include_directories(custom/BL5340/include)
zephyr_sources(
    custom/BL5340/source/locking_table.c
)
endif()

if(CONFIG_BOARD_BT610)
zephyr_include_directories(custom/BT610/include)
zephyr_sources(
    custom/BT610/source/locking_table.c
)
endif()

if(CONFIG_BOARD_BT520)
zephyr_include_directories(custom/BT520/include)
zephyr_sources(
    custom/BT520/source/locking_table.c
)
endif()

endif() # CONFIG_LOCKING_DEVICE_OVERRIDE

endif() # CONFIG_LOCKING
