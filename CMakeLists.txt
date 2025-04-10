#
# Copyright (c) 2022 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

# Flag which defines whether application is compiled as gateway/dongle or headset
add_compile_definitions(HEADSET=1)
add_compile_definitions(GATEWAY=2)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(EmbeddedAppsPoC)

string(TIMESTAMP NRF5340_AUDIO_CORE_APP_COMP_DATE "%a %b %d %H:%M:%S %Y")

# Generate fw_info_app.c
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/utils/fw_info_app.c.in"
               "${CMAKE_BINARY_DIR}/fw_info_app.c"
               @ONLY)

# Target sources below are specific to the nRF5340 Audio DK HW
target_sources(app PRIVATE
    ${CMAKE_BINARY_DIR}/fw_info_app.c
    )

if (CONFIG_BT_BAP_BROADCAST_SINK)
	add_subdirectory(broadcast_sink)
endif()

if (CONFIG_BT_BAP_BROADCAST_SOURCE)
	add_subdirectory(broadcast_source)
endif()

if (CONFIG_BT_BAP_UNICAST_CLIENT)
	add_subdirectory(unicast_client)
endif()

if (CONFIG_BT_BAP_UNICAST_SERVER)
	add_subdirectory(unicast_server)
endif()


# Include application events and configuration headers
zephyr_library_include_directories(
    include
    src/audio
    src/bluetooth
    src/drivers
    src/modules
    src/utils
    src/utils/macros
    # src/microPython
)

zephyr_library_include_directories(app PRIVATE
    ${ZEPHYR_NRF_MODULE_DIR}/boards/arm/nrf5340_audio_dk_nrf5340)

# Application sources
add_subdirectory(src/audio)
add_subdirectory(src/bluetooth)
add_subdirectory(src/drivers)
add_subdirectory(src/modules)
add_subdirectory(src/utils)
# add_subdirectory(src/microPython)

# MicroPython LLEXT
# add_llext_target(micropython_ext
#     OUTPUT ${PROJECT_BINARY_DIR}/micropython_ext.llext 
#     SOURCES $<TARGET_OBJECTS:micropython_objects>
# )

# llext hello world
add_llext_target(hello_world
    OUTPUT ${PROJECT_BINARY_DIR}/hello_world.llext 
    SOURCES src/llext_hello_world/hello_world.c
)
# Create an intermediate target to generate the .inc file before build starts
add_custom_command(
    OUTPUT ${PROJECT_BINARY_DIR}/hello_world_ext.inc
    COMMAND xxd -i -n hello_world_llext ${PROJECT_BINARY_DIR}/hello_world.llext | sed 's/^unsigned char /const unsigned char /' > ${PROJECT_BINARY_DIR}/hello_world_ext.inc
    DEPENDS hello_world
    COMMENT "Generating hello_world_ext.inc with clean symbol name"
)
add_custom_target(hello_world_blob_inc
    DEPENDS ${PROJECT_BINARY_DIR}/hello_world_ext.inc
)

add_dependencies(app hello_world_blob_inc)

target_include_directories(app PRIVATE ${PROJECT_BINARY_DIR})

## Cirrus Logic
if (CONFIG_HW_CODEC_CIRRUS_LOGIC)
    if (ZEPHYR_CIRRUS_LOGIC_MODULE_DIR)
        add_subdirectory(${ZEPHYR_CIRRUS_LOGIC_MODULE_DIR} cirrus_logic_bin_dir)
    else()
        message(FATAL_ERROR "Cirrus Logic/sdk-mcu-drivers repository not found\n")
    endif()
endif()
