cmake_minimum_required(VERSION 3.12)
project(serialization_test)
include_directories("." "lib")
file(GLOB SOURCES "*.cpp" "lib/mtojson/*.c" "ejfp/*.c")
message(${SOURCES})
set(EXECUTABLE_NAME serialization_test)
add_executable(${EXECUTABLE_NAME} ${SOURCES})
set_property(TARGET ${EXECUTABLE_NAME} PROPERTY CXX_STANDARD 11)
target_compile_options(${EXECUTABLE_NAME} PUBLIC "-ggdb")
