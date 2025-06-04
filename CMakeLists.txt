# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(TAREFA_MQTT_1 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(TAREFA_MQTT_1 TAREFA_MQTT_1.c
    inc/ssd1306.c )

# Inclui a pasta include
include_directories(${CMAKE_CURRENT_LIST_DIR}/inc)

# Adiciona as opções para incluir a pasta include como caminho de include para o compilador
# (garante que os headers em include/ sejam encontrados)
target_include_directories(TAREFA_MQTT_1 PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/inc
)
pico_set_program_name(TAREFA_MQTT_1 "TAREFA_MQTT_1")
pico_set_program_version(TAREFA_MQTT_1 "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(TAREFA_MQTT_1 1)
pico_enable_stdio_usb(TAREFA_MQTT_1 1)

# Add the standard library to the build
target_link_libraries(TAREFA_MQTT_1
        pico_stdlib
        hardware_adc
        hardware_pio
        hardware_pwm
        hardware_i2c
        pico_cyw43_arch_lwip_threadsafe_background
        pico_lwip_mqtt
        pico_mbedtls
        pico_lwip_mbedtls)

# Add the standard include files to the build
target_include_directories(TAREFA_MQTT_1 PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(TAREFA_MQTT_1 
        
        )

pico_add_extra_outputs(TAREFA_MQTT_1)

