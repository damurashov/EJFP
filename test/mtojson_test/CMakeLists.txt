cmake_minimum_required(VERSION 3.12)
project(mtojson_test)
include_directories(".")
file(GLOB SOURCES "*.cpp" "lib/mtojson/*.c" "mtojson/*.c")
message(${SOURCES})
set(EXECUTABLE_NAME mtojson_test)
add_executable(${EXECUTABLE_NAME} ${SOURCES})
set_property(TARGET ${EXECUTABLE_NAME} PROPERTY CXX_STANDARD 11)
target_compile_options(${EXECUTABLE_NAME} PUBLIC "-g")
