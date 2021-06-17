cmake_minimum_required (VERSION 3.8)

project(NanoLexer)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set (CMAKE_INCLUDE_CURRENT_DIR ON)

set(SRCS
	LexerBuilder.cpp
	LexerTreeNode.cpp
	NanoLexer.cpp
	${CMAKE_CURRENT_BINARY_DIR}/NanoLexerParser.cpp
	${CMAKE_CURRENT_BINARY_DIR}/NanoLexerLexer.cpp
	)

set (HEADERS
	LexerBuilder.h
	LexerTreeNode.h
	../include/NanoLexer.h
	shared.h
	unistd.h
	${CMAKE_CURRENT_BINARY_DIR}/tokens.h
	)

set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/NanoLexerParser.cpp
							${CMAKE_CURRENT_BINARY_DIR}/tokens.h
							${CMAKE_CURRENT_BINARY_DIR}/NanoLexerLexer.cpp
							PROPERTIES GENERATED TRUE
							)

FIND_PACKAGE(BISON REQUIRED)
SET(BisonOutput ${CMAKE_CURRENT_BINARY_DIR}/NanoLexerParser.cpp)
IF(BISON_FOUND)
    ADD_CUSTOM_COMMAND(
      OUTPUT ${BisonOutput}
      COMMAND ${BISON_EXECUTABLE}
              --defines=${CMAKE_CURRENT_BINARY_DIR}/tokens.h
              --output=${BisonOutput}
			  --report=all
              ${CMAKE_SOURCE_DIR}/NanoLexer/NanoLexer.y
      COMMENT "Generating NanoLexerParser.cpp"
    )
ENDIF()

FIND_PACKAGE(FLEX REQUIRED)
SET(FlexOutput ${CMAKE_CURRENT_BINARY_DIR}/NanoLexerLexer.cpp)
IF(FLEX_FOUND)
    ADD_CUSTOM_COMMAND(
      OUTPUT ${FlexOutput}
      COMMAND ${FLEX_EXECUTABLE}
              -o${FlexOutput}
              ${CMAKE_SOURCE_DIR}/NanoLexer/NanoLexer.l
      COMMENT "Generating NanoLexerLexer.cpp"
    )
ENDIF()

add_library(NanoLexer STATIC ${SRCS} ${HEADERS} NanoLexer.y NanoLexer.l ../README.md)

set(EXECUTABLE_OUTPUT_PATH bin/${CMAKE_BUILD_TYPE})

target_compile_features(NanoLexer PRIVATE cxx_std_17)
