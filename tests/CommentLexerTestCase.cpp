#include <gtest\gtest.h>
#include <CommentsLexer.h>
#include "checkLexemes.h"

using namespace NanoLexer;
using Lexer = CommentsLexer<std::istringstream>;


TEST(CommentsLexerTest, TestFail) {
    std::string text = "/*test";

    std::vector<Token<Lexer::Lexeme>> expectedLexemes = { {Lexer::Lexeme::unknown_, "/*test"} };
    checkLexemes<Lexer, Lexer::Lexeme>(text, expectedLexemes);
}

TEST(CommentsLexerTest, TestSimpleOk) {
    std::string text = "/*test*/";

    std::vector<Token<Lexer::Lexeme>> expectedLexemes = { {Lexer::Lexeme::multilineComment_, "/*test*/"} };
    checkLexemes<Lexer, Lexer::Lexeme>(text, expectedLexemes);
}
