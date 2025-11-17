# CMake for examples
cmake_minimum_required(VERSION 3.10)

find_package(Python3 COMPONENTS Interpreter REQUIRED)

set(GEN_SCRIPT ${CMAKE_SOURCE_DIR}/tools/gen_cmd_stat.py)
set(COMMANDS_STAT ${CMAKE_SOURCE_DIR}/examples/commands.stat)
set(GEN_OUT_H ${CMAKE_SOURCE_DIR}/examples/commands_stat.h)
set(GEN_OUT_CPP ${CMAKE_SOURCE_DIR}/examples/commands_stat.cpp)

add_custom_command(
    OUTPUT ${GEN_OUT_H} ${GEN_OUT_CPP}
    COMMAND ${Python3_EXECUTABLE} ${GEN_SCRIPT} ${COMMANDS_STAT}
    DEPENDS ${GEN_SCRIPT} ${COMMANDS_STAT}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Generating commands_stat.h/cpp from commands.stat"
    VERBATIM
)

add_executable(example main.cpp ${GEN_OUT_CPP})

target_include_directories(example PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(example PRIVATE litestat)
