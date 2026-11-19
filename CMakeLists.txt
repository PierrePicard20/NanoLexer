cmake_minimum_required (VERSION 4.3.1)

option(USE_VCPKG "Use Vcpkg for dependencies" OFF)

if(USE_VCPKG AND DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
endif()

project (NanoLexer VERSION 1.0.0)

# Include sub-projects.
add_subdirectory (NanoLexer)
add_subdirectory (nanolex)

option(PACKAGE_TESTS "Build the tests" OFF)
if(PACKAGE_TESTS)
    find_package(GTest REQUIRED)
    enable_testing()
    add_subdirectory(tests)
    add_subdirectory(testgen)
endif()

# Installation & packaging
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY lexer_sk/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/NanoLexer/lexer_sk
)

install(EXPORT NanoLexerTargets
    FILE NanoLexerTargets.cmake
    NAMESPACE NanoLexer::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/NanoLexer
)

configure_package_config_file(
    cmake/NanoLexerConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/NanoLexerConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/NanoLexer
    PATH_VARS CMAKE_INSTALL_DATADIR
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/NanoLexerConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/NanoLexerConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/NanoLexerConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/NanoLexer
)
