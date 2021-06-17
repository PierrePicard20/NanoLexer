#include <gtest\gtest.h>
#include <Cpplexer.h>
#include "checkLexemes.h"

using namespace NanoLexer;
using Lexer = CppLexer<std::istringstream>;


TEST(CppLexerTest, TestBacktrack) {
    // that text has an incorrect syntax, but is lexicaly correct.
    std::string text = "/*test";

    std::vector<Token<Lexer::Lexeme>> expectedLexemes = { {(Lexer::Lexeme)95, "/"}
                                                        , {(Lexer::Lexeme)94, "*"}
                                                        , {Lexer::Lexeme::identifier_, "test"} };
    checkLexemes<Lexer, Lexer::Lexeme>(text, expectedLexemes);
}

TEST(CppLexerTest, TestKeywordsAndIds) {
    // first and last keyword (let's check that they will not be matched as identifiers)
    std::string text = "alignas throw hello";

    std::vector<Token<Lexer::Lexeme>> expectedLexemes = { {Lexer::Lexeme::alignas_, "alignas"}
                                                        , {Lexer::Lexeme::whitespace_, " "}
                                                        , {Lexer::Lexeme::throw_, "throw"}
                                                        , {Lexer::Lexeme::whitespace_, " "}
                                                        , {Lexer::Lexeme::identifier_, "hello"} };
    checkLexemes<Lexer, Lexer::Lexeme>(text, expectedLexemes);
}