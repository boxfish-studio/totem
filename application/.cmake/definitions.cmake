# ==============================================================================
# Totem version
# ==============================================================================
file(STRINGS "${CMAKE_SOURCE_DIR}/Source/totem/lib/usb/inc/descriptors.h" _TOTEM_VERSION_H_CONTENTS REGEX "#define TOTEM_VERSION_")
foreach(v RELEASE BUILD PATCH)
  if("${_TOTEM_VERSION_H_CONTENTS}" MATCHES "#define TOTEM_VERSION_${v} ([0-9]+)")
    set(TOTEM_VERSION_${v} "${CMAKE_MATCH_1}")
 else()
    message(FATAL_ERROR "Failed to retrieve the Totem version from the source code. Missing TOTEM_VERSION_${v}.")
  endif()
endforeach()
set(TOTEM_VERSION ${TOTEM_VERSION_RELEASE}.${TOTEM_VERSION_BUILD}.${TOTEM_VERSION_PATCH})

# ==============================================================================
# Macro to adapt the Eclipse project to support both compilations
# ==============================================================================

find_package(Python3 REQUIRED)

set(PYTHON_ADAPT_SCRIPT ${CMAKE_SOURCE_DIR}/.cmake/exclude_eclipse.py)

macro(ADAPT_ECLIPSE_PROJECT)
  execute_process(
    COMMAND ${Python3_EXECUTABLE} ${PYTHON_ADAPT_SCRIPT} ${CMAKE_SOURCE_DIR}/.cproject
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND echo "adapting Eclipse project to support cmake structure"
  )
endmacro(ADAPT_ECLIPSE_PROJECT)

# ==============================================================================
# Macros to generate bin files
# ==============================================================================

# Set build path
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/Release")

macro(GENERATE_BIN NAME)
	add_custom_command(TARGET ${NAME}.elf POST_BUILD
		    COMMAND ${CMAKE_OBJCOPY} -Obinary ${EXECUTABLE_OUTPUT_PATH}/${NAME}.elf ${EXECUTABLE_OUTPUT_PATH}/${NAME}.bin
        COMMAND mkdir -p ${EXECUTABLE_OUTPUT_PATH}/${TOTEM_VERSION} && cp ${EXECUTABLE_OUTPUT_PATH}/${NAME}.bin ${EXECUTABLE_OUTPUT_PATH}/${TOTEM_VERSION}/${NAME}_${TOTEM_VERSION}.bin
        COMMENT "creating bin file for ${NAME} (${NAME}_${TOTEM_VERSION}.bin)")
endmacro(GENERATE_BIN)

