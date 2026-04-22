cmake_minimum_required(VERSION 3.10)

# Gather all source files from src (excluding main.c which has its own main).
file(GLOB_RECURSE SRC_SOURCES
    "${CMAKE_SOURCE_DIR}/src/*.c"
)
list(REMOVE_ITEM SRC_SOURCES "${CMAKE_SOURCE_DIR}/src/main.c")

# Build lexer tests separately.
add_executable(test_lexer test_lexer.c ${SRC_SOURCES})
target_include_directories(test_lexer PRIVATE ${CMAKE_SOURCE_DIR}/src)
add_test(NAME LexerTests COMMAND test_lexer)

# Build parser tests separately.
add_executable(test_parser test_parser.c ${SRC_SOURCES})
target_include_directories(test_parser PRIVATE ${CMAKE_SOURCE_DIR}/src)
add_test(NAME ParserTests COMMAND test_parser)
