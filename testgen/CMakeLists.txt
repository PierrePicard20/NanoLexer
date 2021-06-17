cmake_minimum_required (VERSION 3.8)

project(testgen)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(SRCS
	testgen.cpp
	arithmeticLexer.cpp
	cMultilineCommentLexer.cpp
	cppLexer.cpp
	simpleLexer1.cpp
	)
	
set(HEADERS
	lexgen.h
    )

add_executable(testgen ${SRCS} ${HEADERS})

set(EXECUTABLE_OUTPUT_PATH bin/${CMAKE_BUILD_TYPE})

target_link_libraries(testgen NanoLexer)

add_custom_command(TARGET testgen POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/lexer_sk $<TARGET_FILE_DIR:testgen>/lexer_sk)
add_custom_command(TARGET testgen POST_BUILD
                   COMMAND testgen)
