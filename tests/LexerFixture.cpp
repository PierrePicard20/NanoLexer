#include "LexerFixture.h"
#include "../include/NanoLexer.h"

using namespace NanoLexer;

std::string LexerFixture::generateDebugStr(const strings& expressions, const std::set<int>& popIds)
{
    LexerGenerator generator("test");

    std::string name = "_";
    int i = 0;
    for (const auto& str : expressions)
    {
        auto* expr = generator.addExpression(str, name+std::to_string(i));
        if (popIds.find(i) != popIds.end())
            expr->setPopAction();
        i++;
    }
    generator.generateLexer();
    return generator.getDebugString();
}

std::string LexerFixture::generateDebugStrMultiContext(const std::vector<strings>& expressions, const std::vector<std::set<int>>& popIds)
{
    LexerGenerator generator("test");

    std::string exprName = "_";
    int i = 0;
    std::string name = "context";
    int ctx = 1;
    auto iterPopSet = popIds.begin();
    for (const auto& arrayStr : expressions)
    {
        for (const auto& str : arrayStr)
        {
            auto* expr = generator.addExpression(str, exprName+std::to_string(i));
            if ((*iterPopSet).find(i) != (*iterPopSet).end())
                expr->setPopAction();
            i++;
        }
        iterPopSet++;
        ctx++;
        generator.newContext(name + std::to_string(ctx));
    }
    generator.generateLexer();
    return generator.getDebugString();
}

std::string LexerFixture::generateDebugStr(const strings& expressions)
{
    static std::set<int> pops;
    return generateDebugStr(expressions, pops);
}
