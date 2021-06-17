#include "lexgen.h"

void genArithmeticExpressionLexer()
{
    try
    {
        LexerGenerator lexGen("Arithmetic");

        lexGen.addMacro("[0-9]", "Digit");
        lexGen.addMacro("[1-9]", "NonNulDigit");
        lexGen.addMacro("{Digit}+", "Integer");
        lexGen.addMacro("{Integer}\\.{Digit}+", "Float");

        lexGen.addExpression("[a-zA-Z]+", "id");
        lexGen.addExpression("{Integer}|{Float}", "number");
        lexGen.addVerbatimExpression("+", "op_plus");
        lexGen.addVerbatimExpression("-", "op_minus");
        lexGen.addVerbatimExpression("*", "op_mult");
        lexGen.addVerbatimExpression("/", "op_div");
        lexGen.addVerbatimExpression("(", "open_parenthesis");
        lexGen.addVerbatimExpression(")", "close_parenthesis");
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
