cmake_minimum_required (VERSION 3.8)

project(nanolex)

# Description-file reader, kept apart from the executable so that the tests can use it in-process
add_library(nanolex_reader STATIC DescriptionReader.cpp DescriptionReader.h)
target_link_libraries(nanolex_reader PUBLIC NanoLexer)
target_include_directories(nanolex_reader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(nanolex_reader PUBLIC cxx_std_17)

add_executable(nanolex main.cpp)
target_link_libraries(nanolex PRIVATE nanolex_reader)

set(EXECUTABLE_OUTPUT_PATH bin/${CMAKE_BUILD_TYPE})

# generateFiles() looks for lexer_sk/ next to the running executable
add_custom_command(TARGET nanolex POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/lexer_sk $<TARGET_FILE_DIR:nanolex>/lexer_sk)

include(GNUInstallDirs)

install(TARGETS nanolex
    EXPORT NanoLexerTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/lexer_sk/
    DESTINATION ${CMAKE_INSTALL_BINDIR}/lexer_sk
)
