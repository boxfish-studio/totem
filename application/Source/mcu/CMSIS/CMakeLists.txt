set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Release")
add_library(CMSIS INTERFACE)
target_include_directories(CMSIS INTERFACE Include/)