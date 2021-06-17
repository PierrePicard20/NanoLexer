cmake_minimum_required (VERSION 3.8)

project (NanoLexer)

# Include sub-projects.
add_subdirectory (NanoLexer)

option(PACKAGE_TESTS "Build the tests" OFF)
if(PACKAGE_TESTS)
    enable_testing()
    add_subdirectory(tests)
    add_subdirectory(testgen)
	option(BUILD_GMOCK "Builds the googlemock subproject" OFF)
	add_subdirectory(googletest)
endif()
