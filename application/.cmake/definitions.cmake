# ==============================================================================
# Totem version
# ==============================================================================
execute_process(
    COMMAND git describe --always --dirty --tags
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE TOTEM_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# ==============================================================================
# Macro to adapt the Eclipse project to support both compilations
# ==============================================================================

set(ADAPT_ECLIPSE_SCRIPT ${CMAKE_SOURCE_DIR}/.cmake/exclude_eclipse.sh)

macro(ADAPT_ECLIPSE_PROJECT)
  execute_process(
    COMMAND bash ${ADAPT_ECLIPSE_SCRIPT} ${CMAKE_SOURCE_DIR}/.cproject
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND echo "\n\n-- Adapting Eclipse project to support cmake structure"
  )
endmacro(ADAPT_ECLIPSE_PROJECT)

# ==============================================================================
# Macros to generate bin files
# ==============================================================================

# Set build path
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/Release")

macro(GENERATE_BIN NAME)
	add_custom_command(TARGET ${NAME}.elf POST_BUILD
		    COMMAND ${CMAKE_OBJCOPY} -Obinary ${EXECUTABLE_OUTPUT_PATH}/${NAME}.elf ${EXECUTABLE_OUTPUT_PATH}/${NAME}_${TOTEM_VERSION}.bin
        COMMAND mkdir -p ${EXECUTABLE_OUTPUT_PATH}/${TOTEM_VERSION} && cp ${EXECUTABLE_OUTPUT_PATH}/${NAME}_${TOTEM_VERSION}.bin ${EXECUTABLE_OUTPUT_PATH}/${TOTEM_VERSION}/${NAME}_${TOTEM_VERSION}.bin
        COMMENT "creating bin file for ${NAME} (${NAME}_${TOTEM_VERSION}.bin)")
endmacro(GENERATE_BIN)
