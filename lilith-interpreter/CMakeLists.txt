cmake_minimum_required(VERSION 3.10)
project(LilithInterpreter VERSION 1.0 LANGUAGES C)

# Set the C standard and enforce strict conformance.
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Enable warnings and optimizations.
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O2)
endif()

# Include directories for the entire project.
include_directories(${CMAKE_SOURCE_DIR}/src)

# Gather all source files recursively from the src directory.
file(GLOB_RECURSE LILITH_SOURCES
    "${CMAKE_SOURCE_DIR}/src/*.c"
)

# Create the main executable for the interpreter.
add_executable(lilith ${LILITH_SOURCES})
target_include_directories(lilith PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Optionally, install the interpreter to the bin folder.
install(TARGETS lilith DESTINATION bin)

# Enable testing and add the tests subdirectory.
enable_testing()
add_subdirectory(tests)
