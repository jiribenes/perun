cmake_minimum_required(VERSION 3.0)
project(perun CXX)

set (CMAKE_CXX_STANDARD 14) # this project uses c++14
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")

set (PERUN_SOURCES
    "${CMAKE_SOURCE_DIR}/src/ast/node.cpp"
    "${CMAKE_SOURCE_DIR}/src/ast/printer.cpp"
    "${CMAKE_SOURCE_DIR}/src/ast/tree.cpp"
    "${CMAKE_SOURCE_DIR}/src/ast/stmt.cpp"

    "${CMAKE_SOURCE_DIR}/src/parser/parser.cpp"
    "${CMAKE_SOURCE_DIR}/src/parser/token.cpp"
    "${CMAKE_SOURCE_DIR}/src/parser/tokenizer.cpp"
    "${CMAKE_SOURCE_DIR}/src/parser/error.cpp"

    "${CMAKE_SOURCE_DIR}/src/support/util.cpp"

    "${CMAKE_SOURCE_DIR}/src/driver/driver.cpp"
    "${CMAKE_SOURCE_DIR}/src/driver/error.cpp"

    "${CMAKE_SOURCE_DIR}/src/perun/main.cpp"
    )

include_directories (
    "${CMAKE_SOURCE_DIR}/src/ast"
    "${CMAKE_SOURCE_DIR}/src/parser"
    "${CMAKE_SOURCE_DIR}/src/support"
    "${CMAKE_SOURCE_DIR}/src/driver"
    )

add_executable (perun ${PERUN_SOURCES})
