#include <gtest\gtest.h>
#include <ArithmeticLexer.h>
#include "checkLexemes.h"

using namespace NanoLexer;
using Lexer = ArithmeticLexer<std::istringstream>;


TEST(CppLexerTest, TestInteger1) {
    std::string text = "0123";
    std::vector<Token<Lexer::Lexeme>> expectedLexemes = { {Lexer::Lexeme::number_, "0123"} };
    checkLexemes<Lexer, Lexer::Lexeme>(text, expectedLexemes);
}

TEST(CppLexerTest, TestInteger2) {
    std::string text = "0";
    std::vector<Token<Lexer::Lexeme>> expectedLexemes = { {Lexer::Lexeme::number_, "0"} };
    checkLexemes<Lexer, Lexer::Lexeme>(text, expectedLexemes);
}

TEST(CppLexerTest, TestFloat1) {
    std::string text = "0.000";
    std::vector<Token<Lexer::Lexeme>> expectedLexemes = { {Lexer::Lexeme::number_, "0.000"} };
    checkLexemes<Lexer, Lexer::Lexeme>(text, expectedLexemes);
}

TEST(CppLexerTest, TestFloat2) {
    std::string text = "00.001";
    std::vector<Token<Lexer::Lexeme>> expectedLexemes = { {Lexer::Lexeme::number_, "00.001"} };
    checkLexemes<Lexer, Lexer::Lexeme>(text, expectedLexemes);
}

TEST(CppLexerTest, TestFloat3) {
    std::string text = "01.001";
    std::vector<Token<Lexer::Lexeme>> expectedLexemes = { {Lexer::Lexeme::number_, "01.001"} };
    checkLexemes<Lexer, Lexer::Lexeme>(text, expectedLexemes);
}
