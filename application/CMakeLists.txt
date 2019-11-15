cmake_minimum_required (VERSION 3.13)

include(.cmake/compiler.cmake)

project(totem C ASM)
set(NAME totem)

add_subdirectory(${CMAKE_SOURCE_DIR}/Source/mcu)
add_subdirectory(${CMAKE_SOURCE_DIR}/Source/totem)
add_subdirectory(${CMAKE_SOURCE_DIR}/Source/app/services)

include(.cmake/definitions.cmake)

# ==============================================================================
# Produce executables (.bin, *_AES.bin)
# ==============================================================================

# Set dependencies
set(DEPENDENCIES
          CMSIS
          Device
          pcbVersion
          emlib
          dma
          usb
          usbLib
          _stdio
          FreeRTOS
          xmodem
          hal
          mcp2515_driver
          services)

# File without bootloader
add_executable(${NAME}_no_bootloader.elf main.c)
target_link_libraries(${NAME}_no_bootloader.elf ${DEPENDENCIES})
target_link_options(${NAME}_no_bootloader.elf PRIVATE -T "${CMAKE_SOURCE_DIR}/efm32gg_no_bootloader.ld")
GENERATE_BIN(${NAME}_no_bootloader)

# File with bootloader
add_executable(${NAME}.elf main.c)
target_link_libraries(${NAME}.elf ${DEPENDENCIES})
target_link_options(${NAME}.elf PRIVATE -T "${CMAKE_SOURCE_DIR}/efm32gg_bootloader.ld")
GENERATE_BIN(${NAME})

# ==============================================================================
# Output version information
# ==============================================================================

message("\n-- Adapting Eclipse project to support cmake structure")
ADAPT_ECLIPSE_PROJECT()

message("\n")
message("** PRINT_ENABLED = ${TOTEM_BUILD_PRINT}")
message("** TRACEALYZER_ENABLED = ${TOTEM_BUILD_TRACEALYZER}\n")
message("")
message("** Totem version: " ${TOTEM_VERSION})
