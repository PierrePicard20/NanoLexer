#include "lexgen.h"

void genSimpleLexer1()
{
    try
    {
        LexerGenerator lexGen("Simple1");

        lexGen.addExpression("class", "class");         // a keyword
        lexGen.addExpression("[a-zA-Z0-9]+", "id");
        lexGen.addExpression("[ \t]+", "whitespace");

        lexGen.generateLexer();
        lexGen.generateFiles("cpp", outputPath);
    }
    catch (const NanoLexerException& e)
    {
        for (const auto& msg : e.getMessages())
            std::cout << msg << std::endl;
        exit(1);
    }
}
