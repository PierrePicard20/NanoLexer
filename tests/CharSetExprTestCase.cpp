#include <gtest\gtest.h>
#include "LexerFixture.h"

TEST_F(LexerFixture, Digits) {
    strings exprRegs{ "\\d" };
    auto debugStr{ generateDebugStr(exprRegs) };

    auto expected =
        R"regexp(s0:['0'-'9']->accept 1;
)regexp";
    ASSERT_EQ(debugStr, expected);
}

TEST_F(LexerFixture, Spaces) {
    strings exprRegs{ "\\s" };
    auto debugStr{ generateDebugStr(exprRegs) };

    auto expected =
        R"regexp(s0:{['\t'-'\r'],' '}->accept 1;
)regexp";
    ASSERT_EQ(debugStr, expected);
}

TEST_F(LexerFixture, Words) {
    strings exprRegs{ "\\w" };
    auto debugStr{ generateDebugStr(exprRegs) };

    auto expected =
        R"regexp(s0:{['0'-'9'],['A'-'Z'],'_',['a'-'z']}->accept 1;
)regexp";
    ASSERT_EQ(debugStr, expected);
}

TEST_F(LexerFixture, NotDigits) {
    strings exprRegs{ "\\D" };
    auto debugStr{ generateDebugStr(exprRegs) };

    auto expected =
        R"regexp(s0:}['0'-'9']{->accept 1;
)regexp";
    ASSERT_EQ(debugStr, expected);
}

TEST_F(LexerFixture, NotSpaces) {
    strings exprRegs{ "\\S" };
    auto debugStr{ generateDebugStr(exprRegs) };

    auto expected =
        R"regexp(s0:}['\t'-'\r'],' '{->accept 1;
)regexp";
    ASSERT_EQ(debugStr, expected);
}

TEST_F(LexerFixture, NotWords) {
    strings exprRegs{ "\\W" };
    auto debugStr{ generateDebugStr(exprRegs) };

    auto expected =
        R"regexp(s0:}['0'-'9'],['A'-'Z'],'_',['a'-'z']{->accept 1;
)regexp";
    ASSERT_EQ(debugStr, expected);
}

TEST_F(LexerFixture, Digits2) {
    strings exprRegs{ "[:digit:]" };
    auto debugStr{ generateDebugStr(exprRegs) };

    auto expected =
        R"regexp(s0:['0'-'9']->accept 1;
)regexp";
    ASSERT_EQ(debugStr, expected);
}

TEST_F(LexerFixture, Spaces2) {
    strings exprRegs{ "[:space:]" };
    auto debugStr{ generateDebugStr(exprRegs) };

    auto expected =
        R"regexp(s0:{['\t'-'\r'],' '}->accept 1;
)regexp";
    ASSERT_EQ(debugStr, expected);
}

TEST_F(LexerFixture, HexaDigits) {
    strings exprRegs{ "[:xdigit:]" };
    auto debugStr{ generateDebugStr(exprRegs) };

    auto expected =
        R"regexp(s0:{['0'-'9'],['A'-'F'],['a'-'f']}->accept 1;
)regexp";
    ASSERT_EQ(debugStr, expected);
}

TEST_F(LexerFixture, Blank) {
    strings exprRegs{ "[:blank:]" };
    auto debugStr{ generateDebugStr(exprRegs) };

    auto expected =
        R"regexp(s0:{'\t',' '}->accept 1;
)regexp";
    ASSERT_EQ(debugStr, expected);
}

TEST_F(LexerFixture, Alnum) {
    strings exprRegs{ "[:alnum:]" };
    auto debugStr{ generateDebugStr(exprRegs) };

    auto expected =
        R"regexp(s0:{['0'-'9'],['A'-'Z'],['a'-'z']}->accept 1;
)regexp";
    ASSERT_EQ(debugStr, expected);
}

TEST_F(LexerFixture, Lower) {
    strings exprRegs{ "[:lower:]" };
    auto debugStr{ generateDebugStr(exprRegs) };

    auto expected =
        R"regexp(s0:['a'-'z']->accept 1;
)regexp";
    ASSERT_EQ(debugStr, expected);
}

TEST_F(LexerFixture, Upper) {
    strings exprRegs{ "[:upper:]" };
    auto debugStr{ generateDebugStr(exprRegs) };

    auto expected =
        R"regexp(s0:['A'-'Z']->accept 1;
)regexp";
    ASSERT_EQ(debugStr, expected);
}

TEST_F(LexerFixture, Cntrl) {
    strings exprRegs{ "[:cntrl:]" };
    auto debugStr{ generateDebugStr(exprRegs) };

    auto expected =
        R"regexp(s0:{[0-31],127}->accept 1;
)regexp";
    ASSERT_EQ(debugStr, expected);
}

TEST_F(LexerFixture, Print) {
    strings exprRegs{ "[:print:]" };
    auto debugStr{ generateDebugStr(exprRegs) };

    auto expected =
        R"regexp(s0:[' '-'~']->accept 1;
)regexp";
    ASSERT_EQ(debugStr, expected);
}

TEST_F(LexerFixture, Graph) {
    strings exprRegs{ "[:graph:]" };
    auto debugStr{ generateDebugStr(exprRegs) };

    auto expected =
        R"regexp(s0:['!'-'~']->accept 1;
)regexp";
    ASSERT_EQ(debugStr, expected);
}

TEST_F(LexerFixture, Punct) {
    strings exprRegs{ "[:punct:]" };
    auto debugStr{ generateDebugStr(exprRegs) };

    auto expected =
        R"regexp(s0:{['!'-'/'],[':'-'@'],['['-'`'],['{'-'~']}->accept 1;
)regexp";
    ASSERT_EQ(debugStr, expected);
}
