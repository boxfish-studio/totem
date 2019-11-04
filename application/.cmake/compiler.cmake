# ==============================================================================
# ARM Cross Compiler configuration
# ==============================================================================

# Cross Compiler ARM metadata
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)
set(CMAKE_CROSSCOMPILING 1)

# Select compiler
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_AR arm-none-eabi-ar)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)

# ==============================================================================
# Totem build options
# ==============================================================================
option(TOTEM_BUILD_PRINT "Print for debug over SWO" OFF)
option(TOTEM_BUILD_TRACEALYZER "Print traces with Tracealyzer" OFF)

# Set compiler / linker flags
set(CMAKE_C_FLAGS "-mcpu=cortex-m3 \
-mthumb \
-O0 \
-fmessage-length=0 \
-fsigned-char \
-ffunction-sections \
-fdata-sections \
-Wunused \
-Wuninitialized \
-Wall \
-Wmissing-declarations \
-g3")
set(CMAKE_EXE_LINKER_FLAGS "-Xlinker \
--gc-sections \
-Wl,-Map,\"totem.map\" \
--specs=nano.specs \
-u _printf_float \
-u _scanf_float \
-lgcc \
-lc \
-lrdimon")

# Check Compiler flags options
if(TOTEM_BUILD_PRINT)
  add_definitions(-DPRINT_ENABLED=1)
else()
  add_definitions(-DPRINT_ENABLED=0)
endif()

if(TOTEM_BUILD_TRACEALYZER)
  add_definitions(-DTRACE_ENABLED=1)
else()
  add_definitions(-DTRACE_ENABLED=0)
endif()

add_definitions(-DEFM32GG990F1024 -std=c99)

