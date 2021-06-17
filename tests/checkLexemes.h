#include <gtest\gtest.h>

template <typename Lexeme>
struct Token
{
    Lexeme lex;
    std::string str;
};

template <typename Lexer, typename Lexeme>
void checkLexemes(const std::string& str, const std::vector<Token<Lexeme>>& lexemes)
{
    std::istringstream  text(str);
    Lexer lexer(text);

    int count = 0;
    Lexeme  lex;
    while ((lex = lexer.getNextToken()) != Lexeme::eof_)
    {
        ASSERT_FALSE(lexemes.size() <= count);  // Found more lexemes than expected
        ASSERT_EQ(lexemes[count].lex, lex);
        ASSERT_EQ(lexemes[count].str, lexer.getMatchString());
        count++;
    }
    ASSERT_FALSE(lexemes.size() > count);     // Found less lexemes than expected
}
