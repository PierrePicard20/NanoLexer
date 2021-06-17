cmake_minimum_required (VERSION 3.8)

project(tests)

include_directories(${gtest_SOURCE_DIR}
					${gtest_SOURCE_DIR}/include)
	
set (CMAKE_INCLUDE_CURRENT_DIR ON)

set (SRCS
	IntervalTestCase.cpp
	LexerTestCase.cpp
	MultiAntiIntervalTestCase.cpp
	MultiIntervalTestCase.cpp
	SimpleLexer1TestCase.cpp
	CppLexerTestCase.cpp
	ErrorMngtTestCase.cpp
	ArithmeticLexerTestCase.cpp
	CharSetExprTestCase.cpp
	CommentLexerTestCase.cpp
	LexerFixture.cpp
	)

set (HEADERS
	checkLexemes.h
	LexerFixture.h
	${CMAKE_CURRENT_BINARY_DIR}/CppLexer.h
	${CMAKE_CURRENT_BINARY_DIR}/Simple1Lexer.h
	${CMAKE_CURRENT_BINARY_DIR}/ArithmeticLexer.h
	${CMAKE_CURRENT_BINARY_DIR}/CommentsLexer.h
	)

set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/CppLexer.h
							${CMAKE_CURRENT_BINARY_DIR}/Simple1Lexer.h
							${CMAKE_CURRENT_BINARY_DIR}/ArithmeticLexer.h
							${CMAKE_CURRENT_BINARY_DIR}/CommentsLexer.h
							PROPERTIES GENERATED TRUE
							)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_executable(tests ${SRCS} ${HEADERS})
target_link_libraries(tests NanoLexer gtest_main)
add_test(NAME tests
		 COMMAND $<TARGET_FILE:tests>
		 WORKING_DIRECTORY ${PROJECT_BINARY_DIR})

set(EXECUTABLE_OUTPUT_PATH bin/${CMAKE_BUILD_TYPE})

add_dependencies(tests NanoLexer)
add_dependencies(tests testgen)

add_custom_command(TARGET tests POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/lexer_sk $<TARGET_FILE_DIR:tests>/lexer_sk)
