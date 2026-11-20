#include <gtest/gtest.h>
#include <ArithmeticLexer.h>
#include <NlArithmeticLexer.h>
#include <NlWordsLexer.h>
#include <NlActionsLexer.h>
#include <NlDefaultLexer.h>
#include <NlUserCodeLexer.h>
#include <NlTypedLexer.h>
#include <NlTypedSkipLexer.h>
#include <CommentsLexer.h>
#include <NlCommentsLexer.h>
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

// NlActions.nl: the blank rule has an action without return, the number rule returns only for non-zero numbers.
TEST(NanolexTest, LexemesWithNonReturningActionAreIgnored) {
    using Lexer = NlActionsLexer<std::istringstream>;
    std::vector<Token<Lexer::Lexeme>> expectedLexemes = {
        {Lexer::Lexeme::_lexeme_2_, "ab"}, {Lexer::Lexeme::_lexeme_3_, "12"}, {Lexer::Lexeme::_lexeme_2_, "cd"} };
    checkLexemes<Lexer, Lexer::Lexeme>("  ab 12\t0 \r\ncd\n", expectedLexemes);
    checkLexemes<Lexer, Lexer::Lexeme>(" \n0  ", {});
}

// NlDefault.nl: the action-less word rule runs %default, which ignores "skip"; the number rule has its own action.
TEST(NanolexTest, ActionLessRulesRunDefaultAction) {
    using Lexer = NlDefaultLexer<std::istringstream>;
    std::vector<Token<Lexer::Lexeme>> expectedLexemes = {
        {Lexer::Lexeme::_lexeme_1_, "abc"}, {Lexer::Lexeme::_lexeme_2_, "12"}, {Lexer::Lexeme::_lexeme_1_, "skipped"} };
    checkLexemes<Lexer, Lexer::Lexeme>("abc skip 12 skipped skip", expectedLexemes);
}

// NlUserCode.nl: NlUserCodeWord is declared in %{ %}; the fields and getLine() come from the user code section and
// are updated by the actions.
TEST(NanolexTest, PrologueAndUserCodeAreUsableByActions) {
    using Lexer = NlUserCodeLexer<std::istringstream>;
    std::istringstream text("ab cd\n\nef");
    Lexer lexer(text);
    while (lexer.lex() != Lexer::Lexeme::eof_)
        ;
    ASSERT_EQ(3, lexer.getLine());
    ASSERT_EQ(3u, lexer.words.size());
    ASSERT_EQ("cd", lexer.words[1].text);
    ASSERT_EQ(1, lexer.words[1].line);
    ASSERT_EQ("ef", lexer.words[2].text);
    ASSERT_EQ(3, lexer.words[2].line);
}

// NlTyped.nl: lex() returns NlTypedToken, declared in %{ %}; the blank rule runs the non-returning %default.
TEST(NanolexTest, TypedLexerReturnsTokensEofAndUnknown) {
    std::istringstream text("ab 12 ?! cd\n");
    NlTypedLexer<std::istringstream> lexer(text);
    std::vector<std::pair<NlTypedKind, std::string>> expected = { {NlTypedKind::word, "ab"}, {NlTypedKind::number, "12"},
        {NlTypedKind::unknown, "?!"}, {NlTypedKind::word, "cd"}, {NlTypedKind::end, ""} };
    for (const auto& [kind, str] : expected)
    {
        auto token = lexer.lex();
        ASSERT_EQ(kind, token.kind) << "expected: " << str;
        ASSERT_EQ(str, token.text);
    }
    ASSERT_EQ(4, lexer.blanks);
    // end of input never reaches the non-returning %default: lex() returns the <<EOF>> token again
    ASSERT_EQ(NlTypedKind::end, lexer.lex().kind);
    ASSERT_EQ(4, lexer.blanks);
}

// NlTypedSkip.nl: with %type and no <<UNKNOWN>>, unknown text is skipped.
TEST(NanolexTest, TypedLexerSkipsUnknownTextWithoutUnknownAction) {
    std::istringstream text("ab ?? cde!");
    NlTypedSkipLexer<std::istringstream> lexer(text);
    ASSERT_EQ(2, lexer.lex());
    ASSERT_EQ(3, lexer.lex());
    ASSERT_EQ(-1, lexer.lex());
}

// NlComments.nl describes the same lexer as testgen/cMultilineCommentLexer.cpp, rule for rule (without the action of
// the new line rule of context Comment, dropped by the library), plus a failure action in context Comment.
TEST(NanolexTest, CommentsLexerMatchesTestgenVersion) {
    for (const std::string text : { "/*test*/", "/*test", "a\n/* x\r\n * y **/b\n", "/* a *\n */", "/**/ /*/", "" })
    {
        auto expected = lexAll<CommentsLexer<std::istringstream>>(text);
        ASSERT_FALSE(text.size() && expected.empty());
        ASSERT_EQ(expected, lexAll<NlCommentsLexer<std::istringstream>>(text)) << "text: " << text;
    }
}

// NlComments.nl: the action of the pushing rule runs after the pop, so getMatchString() holds the whole comment.
TEST(NanolexTest, PushingRuleActionSeesTheWholeLexeme) {
    std::istringstream text("a/* x\n y */b/**/");
    NlCommentsLexer<std::istringstream> lexer(text);
    while (lexer.lex() != NlCommentsLexer<std::istringstream>::Lexeme::eof_)
        ;
    ASSERT_EQ((std::vector<std::string>{ "/* x\n y */", "/**/" }), lexer.comments);
    ASSERT_EQ(0, lexer.failures);
}

// NlComments.nl: <<UNKNOWN>> in %context Comment runs when the context fails to match.
TEST(NanolexTest, ContextUnknownActionRunsOnContextFailure) {
    std::istringstream text("/* a *\n */");
    NlCommentsLexer<std::istringstream> lexer(text);
    ASSERT_EQ(NlCommentsLexer<std::istringstream>::Lexeme::unknown_, lexer.lex());
    ASSERT_EQ(1, lexer.failures);
}
