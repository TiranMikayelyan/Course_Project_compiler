cmake_minimum_required(VERSION 3.16)
project(course_compiler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(course_compiler
    src/main.cpp
    src/lexer.cpp
    src/parser.cpp
    src/symbols.cpp
    src/codegen.cpp
    src/exec_format.cpp
    src/vm.cpp
)

target_include_directories(course_compiler PRIVATE src)
