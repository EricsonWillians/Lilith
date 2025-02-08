cmake_minimum_required(VERSION 3.10)

# Gather all test source files in the tests directory.
file(GLOB TEST_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

# Create the tests executable.
add_executable(lilith_tests ${TEST_SOURCES})

# Include the main src directory so that test files can see all project headers.
target_include_directories(lilith_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)

# If you have built a library target (e.g., lilith_static), you could link it here:
# target_link_libraries(lilith_tests PRIVATE lilith_static)

# Register the tests with CTest.
add_test(NAME AllTests COMMAND lilith_tests)
