cmake_minimum_required(VERSION 3.14)
project(NanoLexerExample VERSION 1.0.0)

find_package(NanoLexer REQUIRED)

add_executable(generate_lexer main.cpp)
target_link_libraries(generate_lexer PRIVATE NanoLexer::NanoLexer)
target_compile_features(generate_lexer PRIVATE cxx_std_17)

# Copy the lexer skeleton directory next to the executable so that
# NanoLexer::LexerGenerator::generateFiles() can find the templates at runtime.
add_custom_command(TARGET generate_lexer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${NanoLexer_LEXER_SK_DIR}"
        "$<TARGET_FILE_DIR:generate_lexer>/lexer_sk"
    COMMENT "Copying NanoLexer skeleton files"
)
