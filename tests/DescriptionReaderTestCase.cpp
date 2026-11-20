#include <gtest/gtest.h>
#include "../include/NanoLexer.h"
#include "DescriptionReader.h"

using namespace NanoLexer;

namespace
{
    std::string debugStringOf(LexerGenerator& generator)
    {
        generator.generateLexer();
        return generator.getDebugString();
    }

    std::vector<std::string> errorsOf(const std::string& text)
    {
        return readDescription(text, "test.nl").errors;
    }

    // Action of the n-th rule (1-based) of a description without errors; "<none>" when the rule has no action.
    std::string actionOf(const std::string& text, int n)
    {
        auto description = readDescription(text, "test.nl");
        if (!description.errors.empty())
            return description.errors.front();
        auto expression = description.generator->getExpression("_lexeme_" + std::to_string(n));
        if (!expression)
            return "<no rule>";
        return expression->hasOnMatchCode() ? expression->getOnMatchCode() : "<none>";
    }
}

TEST(DescriptionReaderTest, MainContextRulesMatchHandBuiltGenerator) {
    std::string expected;
    {
        LexerGenerator handBuilt("ReaderHandBuilt");
        handBuilt.addMacro("[0-9]", "ReaderDigit");
        handBuilt.addMacro("{ReaderDigit}+", "ReaderInteger");
        handBuilt.addExpression("[a-zA-Z]+");
        handBuilt.addExpression("{ReaderInteger}");
        handBuilt.addExpression("\"+\"");
        handBuilt.addExpression("\"/*\"");
        handBuilt.addExpression("[ \\t]+");
        handBuilt.addExpression("a\\ b");
        expected = debugStringOf(handBuilt);
    }

    auto description = readDescription(
R"nl(%name ReaderParsed
ReaderDigit     [0-9]
ReaderInteger   {ReaderDigit}+

%%
[a-zA-Z]+
{ReaderInteger}
"+"
"/*"
[ \t]+
a\ b
)nl", "test.nl");

    ASSERT_TRUE(description.errors.empty()) << description.errors.front();
    ASSERT_TRUE(description.generator);
    ASSERT_EQ(expected, debugStringOf(*description.generator));
}

TEST(DescriptionReaderTest, TrailingSeparatorWithEmptyUserCodeIsAllowed) {
    auto description = readDescription("%%\nabc\n%%\n\n", "test.nl");
    ASSERT_TRUE(description.errors.empty()) << description.errors.front();
    ASSERT_TRUE(description.generator);
}

TEST(DescriptionReaderTest, CrLfLineEndingsAreAccepted) {
    auto description = readDescription("%name CrLf\r\n%%\r\nabc\r\n", "test.nl");
    ASSERT_TRUE(description.errors.empty()) << description.errors.front();
    ASSERT_EQ("CrLf", description.generator->getLexerName());
}

TEST(DescriptionReaderTest, NameDirectiveGivesLexerName) {
    auto description = readDescription("%name Arith\n%%\nabc\n", "dir/other.nl");
    ASSERT_TRUE(description.generator);
    ASSERT_EQ("Arith", description.generator->getLexerName());
}

TEST(DescriptionReaderTest, LexerNameDefaultsToFileBaseName) {
    auto description = readDescription("%%\nabc\n", "dir/Words.nl");
    ASSERT_TRUE(description.generator);
    ASSERT_EQ("Words", description.generator->getLexerName());
}

TEST(DescriptionReaderTest, InvalidFileBaseNameIsReported) {
    auto description = readDescription("%%\nabc\n", "my-lexer.nl");
    ASSERT_FALSE(description.generator);
    ASSERT_EQ(std::vector<std::string>{ "my-lexer.nl: invalid lexer name 'my-lexer', use %name to set one" }, description.errors);
}

TEST(DescriptionReaderTest, BadRegexInRuleIsLocated) {
    ASSERT_EQ(std::vector<std::string>{ "test.nl(4): syntax error at row 5" }, errorsOf("%%\nabc\n\na|(b]|c\n"));
}

TEST(DescriptionReaderTest, UnknownMacroInRuleIsLocated) {
    ASSERT_EQ(std::vector<std::string>{ "test.nl(2): Unknown macro identifier 'ReaderUndefined'" }, errorsOf("%%\n{ReaderUndefined}\n"));
}

TEST(DescriptionReaderTest, BadRegexInMacroIsLocated) {
    ASSERT_EQ(std::vector<std::string>{ "test.nl(2): syntax error at row 5 in macro 'ReaderBad'" },
              errorsOf("%name BadMacro\nReaderBad a|(b]|c\n%%\nabc\n"));
}

TEST(DescriptionReaderTest, MissingSeparatorIsReported) {
    ASSERT_EQ(std::vector<std::string>{ "test.nl: missing '%%' between the definitions and rules sections" }, errorsOf("%name NoRules\n"));
}

TEST(DescriptionReaderTest, EmptyRulesSectionIsReported) {
    ASSERT_EQ(std::vector<std::string>{ "test.nl: no rule in the rules section" }, errorsOf("%%\n\n%%\n"));
}

TEST(DescriptionReaderTest, TextAfterPatternIsReported) {
    ASSERT_EQ(std::vector<std::string>{ "test.nl(3): unexpected text after the pattern: 'def'" }, errorsOf("%%\nabc\nabc def\n"));
}

TEST(DescriptionReaderTest, IndentedRuleIsReported) {
    ASSERT_EQ(std::vector<std::string>{ "test.nl(2): unexpected indented line, a pattern must start at column 0" }, errorsOf("%%\n  abc\n"));
}

TEST(DescriptionReaderTest, UnknownDirectiveIsReported) {
    ASSERT_EQ(std::vector<std::string>{ "test.nl(1): unknown directive '%foo'" }, errorsOf("%foo bar\n%%\nabc\n"));
}

TEST(DescriptionReaderTest, MacroWithoutRegexIsReported) {
    ASSERT_EQ(std::vector<std::string>{ "test.nl(1): missing regular expression for macro 'Lonely'" }, errorsOf("Lonely\n%%\nabc\n"));
}

TEST(DescriptionReaderTest, UserCodeSectionGivesMembers) {
    auto description = readDescription("%%\nabc\n%%\n    int x;\n\n%%\n    int y;\n", "test.nl");
    ASSERT_TRUE(description.errors.empty()) << description.errors.front();
    ASSERT_EQ("    int x;\n\n%%\n    int y;\n", description.generator->getMembers());
}

TEST(DescriptionReaderTest, BlankUserCodeSectionGivesNoMembers) {
    auto description = readDescription("%%\nabc\n%%\n  \n\t\n", "test.nl");
    ASSERT_TRUE(description.errors.empty()) << description.errors.front();
    ASSERT_EQ("", description.generator->getMembers());
}

TEST(DescriptionReaderTest, PrologueBlocksAreJoined) {
    auto description = readDescription(
        "%{\n#include <vector>\n  // %% and {Macro} are copied as is\n%}\nReaderPrologueDigit [0-9]\n%{ \n struct T;\n%}\t\n%%\n{ReaderPrologueDigit}\n",
        "test.nl");
    ASSERT_TRUE(description.errors.empty()) << description.errors.front();
    ASSERT_EQ("#include <vector>\n  // %% and {Macro} are copied as is\n struct T;\n", description.generator->getPrologue());
}

TEST(DescriptionReaderTest, UnterminatedPrologueIsLocated) {
    ASSERT_EQ((std::vector<std::string>{ "test.nl(2): unterminated '%{' block, missing '%}'", "test.nl: missing '%%' between the definitions and rules sections" }),
              errorsOf("%name Unterminated\n%{\n#include <vector>\n%%\nabc\n"));
}

TEST(DescriptionReaderTest, PrologueInRulesSectionIsReported) {
    ASSERT_EQ(std::vector<std::string>{ "test.nl(3): unknown directive '%{'" }, errorsOf("%%\nabc\n%{\n"));
}

TEST(DescriptionReaderTest, ErrorsAreSortedByLine) {
    ASSERT_EQ((std::vector<std::string>{ "test.nl(2): unknown directive '%foo'", "test.nl(4): syntax error at row 5" }),
              errorsOf("%name Sorted\n%foo\n%%\na|(b]|c\n"));
}

TEST(DescriptionReaderTest, SingleLineActionIsAttachedToItsRule) {
    auto text = "%%\nabc     { return accepted; }\ndef\n";
    ASSERT_EQ(" return accepted; ", actionOf(text, 1));
    ASSERT_EQ("<none>", actionOf(text, 2));
}

TEST(DescriptionReaderTest, EmptyActionIsAnAction) {
    ASSERT_EQ("", actionOf("%%\nabc {}\n", 1));
}

TEST(DescriptionReaderTest, PatternWithBlankAndBracesIsNotAnAction) {
    ASSERT_EQ("{}", actionOf("%%\n\"{ }\"\t{{}}\n", 1));
}

TEST(DescriptionReaderTest, MultiLineActionIsAttachedToItsRule) {
    auto text = "%%\nabc {\n    if (x) { y(); }\n}\ndef { z(); }\n";
    ASSERT_EQ("\n    if (x) { y(); }\n", actionOf(text, 1));
    ASSERT_EQ(" z(); ", actionOf(text, 2));
}

TEST(DescriptionReaderTest, BracesInLiteralsAndCommentsDoNotEndAnAction) {
    std::string action = R"--( s = "}\"}"; c = '}'; d = '\''; e = '"'; // }
    /* } */ r = R"x(})" })x"; n = 1'000; )--";
    auto text = "%%\nabc {" + action + "}\ndef\n";
    ASSERT_EQ(action, actionOf(text, 1));
    ASSERT_EQ("<none>", actionOf(text, 2));
}

TEST(DescriptionReaderTest, UnterminatedActionIsLocated) {
    ASSERT_EQ(std::vector<std::string>{ "test.nl(3): unterminated action, missing '}'" }, errorsOf("%%\nabc\ndef { if (x) {\n}\n%%\n"));
    ASSERT_EQ(std::vector<std::string>{ "test.nl(2): unterminated action, missing '}'" }, errorsOf("%%\nabc { // }\n"));
}

TEST(DescriptionReaderTest, TextAfterActionIsLocated) {
    ASSERT_EQ(std::vector<std::string>{ "test.nl(3): unexpected text after the action: 'def'" }, errorsOf("%%\nabc {\n} def\n"));
}

TEST(DescriptionReaderTest, DefaultActionErrorsAreLocated) {
    ASSERT_EQ(std::vector<std::string>{ "test.nl(3): %default expects an action '{ ... }'" }, errorsOf("%%\nabc\n%default\n"));
    ASSERT_EQ(std::vector<std::string>{ "test.nl(4): duplicate %default, already declared at line 2" },
              errorsOf("%%\n%default {\n}\n%default {}\nabc\n"));
    ASSERT_EQ(std::vector<std::string>{ "test.nl(1): unknown directive '%default'" }, errorsOf("%default {}\n%%\nabc\n"));
}
