#include <gtest\gtest.h>
#include "../include/NanoLexer.h"

using namespace NanoLexer;

#define CHECK_NANOLEXEREXCEPTION(c,m)       try\
                                            {\
                                                c;\
                                                FAIL() << "Expected NanoLexerException";\
                                            }\
                                            catch (const NanoLexerException& e)\
                                            {\
                                                EXPECT_STREQ(m, e.getMessages().front().c_str());\
                                            }\
                                            catch (...) {\
                                                FAIL() << "Expected NanoLexerException";\
                                            }

TEST(ErrorTest, TestDuplicateExpression) {
    LexerGenerator lexGen("test", true);
    lexGen.addExpression("hello", "hello");
    CHECK_NANOLEXEREXCEPTION(lexGen.addExpression("hello", "hello"), "Duplicate expression 'hello' in context 'main_context'");
}

TEST(ErrorTest, TestDuplicateContext) {
    LexerGenerator lexGen("test", true);
    lexGen.newContext("myContext");
    CHECK_NANOLEXEREXCEPTION(lexGen.newContext("myContext"), "Duplicate context name : 'myContext'");
}

TEST(ErrorTest, TestUnresolvedContext) {
    LexerGenerator lexGen("test", true);
    lexGen.addExpression("hello", "hello")->setPushContext("myContext");
    CHECK_NANOLEXEREXCEPTION(lexGen.generateLexer(), "Unknown context name : 'myContext'");
}

TEST(ErrorTest, TestPopError) {
    LexerGenerator lexGen("test", true);
    lexGen.addExpression("hello", "hello")->setPopAction();
    CHECK_NANOLEXEREXCEPTION(lexGen.generateLexer(), "Cannot set a pop action in main context on expression 'hello'");
}

TEST(ErrorTest, TestExpressionSyntaxError) {
    LexerGenerator lexGen("test", true);
    CHECK_NANOLEXEREXCEPTION(lexGen.addExpression("a|(b]|c", "myExpression"), "syntax error at row 5 in expression 'myExpression'");
}

TEST(ErrorTest, TestMacroSyntaxError) {
    LexerGenerator lexGen("test", true);
    CHECK_NANOLEXEREXCEPTION(lexGen.addMacro("a|(b]|c", "myMacro"), "syntax error at row 5 in macro 'myMacro'");
}

TEST(ErrorTest, TestUnknownMacro) {
    LexerGenerator lexGen("test", true);
    CHECK_NANOLEXEREXCEPTION(lexGen.addExpression("{myMacro}", "myExpression"), "Unknown macro identifier 'myMacro' in expression 'myExpression'");
}

TEST(ErrorTest, TestRepeat1) {
    LexerGenerator lexGen("test", true);
    CHECK_NANOLEXEREXCEPTION(lexGen.addExpression("a{2,1}", "myExpression"), "Maximum occurence count must be greater than the minimum count in expression 'myExpression'");
}

TEST(ErrorTest, TestRepeat2) {
    LexerGenerator lexGen("test", true);
    CHECK_NANOLEXEREXCEPTION(lexGen.addExpression("a{0,0}", "myExpression"), "Maximum occurence count must be greater than 0 in expression 'myExpression'");
}

TEST(ErrorTest, EmptyExpression1) {
    LexerGenerator lexGen("test", true);
    CHECK_NANOLEXEREXCEPTION(lexGen.addExpression("", "myExpression"), "Invalid empty expression");
}

TEST(ErrorTest, EmptyName2) {
    LexerGenerator lexGen("test", true);
    CHECK_NANOLEXEREXCEPTION(lexGen.addMacro("a", ""), "Invalid empty name");
}

TEST(ErrorTest, EmptyExpression2) {
    LexerGenerator lexGen("test", true);
    CHECK_NANOLEXEREXCEPTION(lexGen.addMacro("", "myExpression"), "Invalid empty expression");
}
