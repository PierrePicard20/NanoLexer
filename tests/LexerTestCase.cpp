#include <gtest\gtest.h>
#include "LexerFixture.h"

TEST_F(LexerFixture, Lexer01) {
    strings exprRegs{ "abc", "[a-z]+" };
    auto debugStr{ generateDebugStr(exprRegs) };

    auto expected = 
R"regexp(s0:'a'->s1;['a'-'z']->s2;
s1:'b'->s3;['a'-'z']->s2;else->accept 2;
s2:['a'-'z']->s2;else->accept 2;
s3:'c'->s4;['a'-'z']->s2;else->accept 2;
s4:['a'-'z']->s2;else->accept 1;
)regexp";
    ASSERT_EQ(debugStr, expected);
}

TEST_F(LexerFixture, Lexer02) {
    strings exprRegs{ "[0-9]+$", "[0-9]+€" };
    auto debugStr{ generateDebugStr(exprRegs) };

    auto expected =
R"regexp(s0:['0'-'9']->s1;
s1:128->accept 2;'$'->accept 1;['0'-'9']->s1;
)regexp";
    ASSERT_EQ(debugStr, expected);
}

TEST_F(LexerFixture, Lexer03) {
    strings exprRegs{ "[0-9]+$", "[0-9]+5" };
    auto debugStr{ generateDebugStr(exprRegs) };

    auto expected =
R"regexp(s0:['0'-'9']->s1;
s1:'$'->accept 1;'5'->s3;['0'-'9']->s1;
s3:'$'->accept 1;'5'->s3;['0'-'9']->s1;else->accept 2;
)regexp";
    ASSERT_EQ(debugStr, expected);
}

TEST_F(LexerFixture, Lexer04) {
    strings exprRegs{ "ab", "cd", "[a-z]+" };
    auto debugStr{ generateDebugStr(exprRegs) };

    auto expected =
R"regexp(s0:'c'->s1;'a'->s2;['a'-'z']->s3;
s1:'d'->s4;['a'-'z']->s3;else->accept 3;
s2:'b'->s5;['a'-'z']->s3;else->accept 3;
s3:['a'-'z']->s3;else->accept 3;
s4:['a'-'z']->s3;else->accept 2;
s5:['a'-'z']->s3;else->accept 1;
)regexp";
    ASSERT_EQ(debugStr, expected);
}

TEST_F(LexerFixture, Lexer05) {
    strings exprRegs{ "class", "while", "for", "using", "[a-z]+" };
    auto debugStr{ generateDebugStr(exprRegs) };

    auto expected =
R"regexp(s0:'u'->s1;'f'->s2;'w'->s3;'c'->s4;['a'-'z']->s5;
s1:'s'->s6;['a'-'z']->s5;else->accept 5;
s2:'o'->s7;['a'-'z']->s5;else->accept 5;
s3:'h'->s8;['a'-'z']->s5;else->accept 5;
s4:'l'->s9;['a'-'z']->s5;else->accept 5;
s5:['a'-'z']->s5;else->accept 5;
s6:'i'->s10;['a'-'z']->s5;else->accept 5;
s7:'r'->s11;['a'-'z']->s5;else->accept 5;
s8:'i'->s12;['a'-'z']->s5;else->accept 5;
s9:'a'->s13;['a'-'z']->s5;else->accept 5;
s10:'n'->s14;['a'-'z']->s5;else->accept 5;
s11:['a'-'z']->s5;else->accept 3;
s12:'l'->s15;['a'-'z']->s5;else->accept 5;
s13:'s'->s16;['a'-'z']->s5;else->accept 5;
s14:'g'->s17;['a'-'z']->s5;else->accept 5;
s15:'e'->s18;['a'-'z']->s5;else->accept 5;
s16:'s'->s19;['a'-'z']->s5;else->accept 5;
s17:['a'-'z']->s5;else->accept 4;
s18:['a'-'z']->s5;else->accept 2;
s19:['a'-'z']->s5;else->accept 1;
)regexp";
    ASSERT_EQ(debugStr, expected);
}

TEST_F(LexerFixture, Lexer06) {
    std::vector<std::set<int>> pop{ {},  { 2 } };
    std::vector<strings> exprRegs{ {}, { "[^\\*]+", "\\*", "\\*/" } }; // matches the content of C multiline comment
    auto debugStr{ generateDebugStrMultiContext(exprRegs, pop) };

    auto expected =
R"regexp(s0:'*'->s1;else->s2;
s1:'/'->pop;else->s0;
s2:}'*'{->s2;else->s0;
)regexp";
    ASSERT_EQ(debugStr, expected);
}

TEST_F(LexerFixture, Lexer07) {
    strings exprRegs{ "ab", "ac" };
    auto debugStr{ generateDebugStr(exprRegs) };

    auto expected =
R"regexp(s0:'a'->s1;
s1:'c'->accept 2;'b'->accept 1;
)regexp";
    ASSERT_EQ(debugStr, expected);
}

TEST_F(LexerFixture, Lexer08) {
    strings exprRegs{ "(ab)+", "(ac)+" };
    auto debugStr{ generateDebugStr(exprRegs) };

    auto expected =
R"regexp(s0:'a'->s1;
s1:'c'->s2;'b'->s3;
s2:'a'->s4;else->accept 2;
s3:'a'->s5;else->accept 1;
s4:'c'->s2;
s5:'b'->s3;
)regexp";
    ASSERT_EQ(debugStr, expected);
}

TEST_F(LexerFixture, Lexer09) {
    strings exprRegs{ "x+" };
    auto debugStr{ generateDebugStr(exprRegs) };

    auto expected =
R"regexp(s0:'x'->s1;
s1:'x'->s1;else->accept 1;
)regexp";
    ASSERT_EQ(debugStr, expected);
}

TEST_F(LexerFixture, Lexer10) {
    strings exprRegs{ "(ab)|(ac)|([a-z]+)" };
    auto debugStr{ generateDebugStr(exprRegs) };

    auto expected =
R"regexp(s0:['a'-'z']->s1;
s1:['a'-'z']->s1;else->accept 1;
)regexp";
    ASSERT_EQ(debugStr, expected);
}

TEST_F(LexerFixture, Lexer11) {
	strings exprRegs{ "a{3}" };
	auto debugStr{ generateDebugStr(exprRegs) };

	auto expected =
R"regexp(s0:'a'->s1;
s1:'a'->s2;
s2:'a'->accept 1;
)regexp";
	ASSERT_EQ(debugStr, expected);
}

TEST_F(LexerFixture, Lexer12) {
	strings exprRegs{ "a{2,4}" };
	auto debugStr{ generateDebugStr(exprRegs) };

	auto expected =
R"regexp(s0:'a'->s1;
s1:'a'->s2;
s2:'a'->s3;else->accept 1;
s3:'a'->accept 1;else->accept 1;
)regexp";
	ASSERT_EQ(debugStr, expected);
}

TEST_F(LexerFixture, Lexer13) {
    strings exprRegs{ "\"(,)\"" };
    auto debugStr{ generateDebugStr(exprRegs) };

    auto expected =
        R"regexp(s0:'('->s1;
s1:','->s2;
s2:')'->accept 1;
)regexp";
    ASSERT_EQ(debugStr, expected);
}

TEST_F(LexerFixture, Lexer14) {
    strings exprRegs{ "\"\\\"\"" };
    auto debugStr{ generateDebugStr(exprRegs) };

    auto expected =
        R"regexp(s0:'"'->accept 1;
)regexp";
    ASSERT_EQ(debugStr, expected);
}
