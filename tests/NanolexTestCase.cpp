#include <gtest/gtest.h>
#include <ArithmeticLexer.h>
#include <NlArithmeticLexer.h>
#include <NlWordsLexer.h>
#include "checkLexemes.h"

using namespace NanoLexer;

namespace
{
    // Lexes the whole text and returns each lexeme (as its enum value) with its match string.
    template <typename Lexer>
    std::vector<std::pair<int, std::string>> lexAll(const std::string& str)
    {
        std::istringstream text(str);
        Lexer lexer(text);
        std::vector<std::pair<int, std::string>> lexemes;
        for (int lex = (int)lexer.lex(); lex != (int)Lexer::Lexeme::eof_; lex = (int)lexer.lex())
            lexemes.emplace_back(lex, lexer.getMatchString());
        return lexemes;
    }
}

// NlArithmetic.nl describes the same lexer as testgen/arithmeticLexer.cpp, rule for rule.
TEST(NanolexTest, ArithmeticLexerMatchesTestgenVersion) {
    for (const std::string text : { "0123", "0.000", "01.001", "a + b*(12.5 - c)/d", "x\t=  42 ?", "1.", "" })
    {
        auto expected = lexAll<ArithmeticLexer<std::istringstream>>(text);
        ASSERT_FALSE(text.size() && expected.empty());
        ASSERT_EQ(expected, lexAll<NlArithmeticLexer<std::istringstream>>(text)) << "text: " << text;
    }
}

// NlWords.nl has no %name: the lexer is named after the file.
TEST(NanolexTest, LexerNamedAfterFileBaseName) {
    using Lexer = NlWordsLexer<std::istringstream>;
    std::vector<Token<Lexer::Lexeme>> expectedLexemes = {
        {Lexer::Lexeme::_lexeme_1_, "ab"}, {Lexer::Lexeme::_lexeme_2_, " "}, {Lexer::Lexeme::_lexeme_1_, "cd"} };
    checkLexemes<Lexer, Lexer::Lexeme>("ab cd", expectedLexemes);
}
