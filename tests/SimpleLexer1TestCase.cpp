#include <gtest\gtest.h>
#include <Simple1lexer.h>
#include "checkLexemes.h"

using namespace NanoLexer;
using Lexer = Simple1Lexer<std::istringstream>;

TEST(LexerTest1, TestHelloWorld) {
    std::string text = "Hello world";
    std::vector<Token<Lexer::Lexeme>> expectedLexemes = { {Lexer::Lexeme::id_, "Hello"}
                                                 , {Lexer::Lexeme::whitespace_, " "}
                                                 , {Lexer::Lexeme::id_, "world"} };
    checkLexemes<Lexer, Lexer::Lexeme>(text, expectedLexemes);
}

TEST(LexerTest1, TestUnknown1) {
    std::string text = "Hello ***";
    std::vector<Token<Lexer::Lexeme>> expectedLexemes = { {Lexer::Lexeme::id_, "Hello"}
                                                        , {Lexer::Lexeme::whitespace_, " "}
                                                        , {Lexer::Lexeme::unknown_, "***"} };
    checkLexemes <Lexer, Lexer::Lexeme> (text, expectedLexemes);
}

TEST(LexerTest1, TestUnknown2) {
    std::string text = "Hello ***world";
    std::vector<Token<Lexer::Lexeme>> expectedLexemes = { {Lexer::Lexeme::id_, "Hello"}
                                                        , {Lexer::Lexeme::whitespace_, " "}
                                                        , {Lexer::Lexeme::unknown_, "***"}
                                                        , {Lexer::Lexeme::id_, "world" } };
    checkLexemes<Lexer, Lexer::Lexeme>(text, expectedLexemes);
}

TEST(LexerTest1, TestKeyword) {
    std::string text = "class world";
    std::vector<Token<Lexer::Lexeme>> expectedLexemes = { {Lexer::Lexeme::class_, "class"}
                                                        , {Lexer::Lexeme::whitespace_, " "}
                                                        , {Lexer::Lexeme::id_, "world"} };
    checkLexemes<Lexer, Lexer::Lexeme>(text, expectedLexemes);
}
