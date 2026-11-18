#include <NanoLexer.h>
#include <iostream>

// This program uses NanoLexer to generate a lexer for arithmetic expressions.
// It produces a header file `ArithmeticLexer.h` in the `generated/` subdirectory.
//
// The generated header exposes a template class ArithmeticLexer<Stream> that
// can be instantiated directly in any downstream project to tokenize input.
//
// Usage of the generated lexer (once generated):
//
//   #include "generated/ArithmeticLexer.h"
//   #include <sstream>
//
//   std::istringstream input("3.14 + 2 * (x - 1)");
//   NanoLexer::ArithmeticLexer<std::istringstream> lexer(input);
//
//   NanoLexer::ArithmeticLexer<std::istringstream>::Lexeme tok;
//   while ((tok = lexer.lex()) != decltype(tok)::eof_)
//       std::cout << lexer.getLexemeName(tok) << " : " << lexer.getMatchString() << "\n";

int main()
{
    try
    {
        NanoLexer::LexerGenerator gen("Arithmetic");

        // Macros (reusable building blocks for expressions)
        gen.addMacro("[0-9]",               "Digit");
        gen.addMacro("{Digit}+",            "Integer");
        gen.addMacro("{Integer}\\.{Digit}+","Float");

        // Token definitions for the default context
        gen.addExpression("{Integer}|{Float}", "number");
        gen.addExpression("[a-zA-Z_][a-zA-Z0-9_]*", "identifier");
        gen.addVerbatimExpression("+", "op_plus");
        gen.addVerbatimExpression("-", "op_minus");
        gen.addVerbatimExpression("*", "op_mult");
        gen.addVerbatimExpression("/", "op_div");
        gen.addVerbatimExpression("(", "open_paren");
        gen.addVerbatimExpression(")", "close_paren");
        gen.addExpression("[ \t\n\r]+", "whitespace");

        gen.generateLexer();

        // "cpp" refers to the lexer_sk/cpp/ skeleton directory.
        // "generated" is the output directory (created relative to the working directory).
        gen.generateFiles("cpp", "generated");

        std::cout << "Lexer generated successfully in ./generated/\n";
        std::cout << "Include generated/ArithmeticLexer.h in your project to use it.\n";
    }
    catch (const NanoLexer::NanoLexerException& e)
    {
        for (const auto& msg : e.getMessages())
            std::cerr << "Error: " << msg << "\n";
        return 1;
    }
    return 0;
}
