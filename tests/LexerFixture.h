#ifndef LEXER_FIXTURE_H
#define LEXER_FIXTURE_H
#include <gtest\gtest.h>

using strings = std::vector<std::string>;

class LexerFixture : public ::testing::Test
{
public:
    std::string generateDebugStr(const strings& expressions, const std::set<int>& popIds);
    std::string generateDebugStrMultiContext(const std::vector<strings>& expressions, const std::vector<std::set<int>>& popIds);
    std::string generateDebugStr(const strings& expressions);
};

#endif
