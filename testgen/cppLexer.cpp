#include "lexgen.h"

void genCppLexer()
{
    try
    {
        LexerGenerator lexGen("Cpp");

        // keywords
        std::vector<std::string> keywords = { "alignas", "continue", "friend", "register", "true", "alignof", "decltype", "goto", "reinterpret_cast", "try", "asm", "default", "if", "return", "type"
            , "delete", "inline", "short", "typeid", "bool", "do", "int", "signed", "typename", "break", "double", "long", "sizeof", "union", "case"
            , "dynamic_cast", "mutable", "static", "unsigned", "catch", "else", "namespace", "static_assert", "using", "char", "enum", "new", "static_cast", "virtual"
            , "char16_t", "explicit", "noexcept", "struct", "void", "char32_t", "export", "nullptr", "switch", "volatile", "class", "extern", "operator", "template"
            , "wchar_t", "const", "false", "private", "this", "while", "constexpr", "float", "protected", "thread_local", "const_cast", "for", "public", "throw" };

        // punctuation and operators
        std::vector<std::string> operators = { "{", "}", "[", "]", "(", ")", "<:", ":>", "<%", "%>", "%:", "%:%:", ";", ":", "...", "?", "::", ".", ".*", "+", "-", "*", "/", "%", "ˆ", "&", "|", "~", "!", "="
            , "<", ">", "+=", "-=", "*=", "/=", "%=", "ˆ=", "&=", "|=", "<<", ">>", ">>=", "<<=", "==", "!=", "<=", ">=", "&&", "||", "++", "--", ",", "->*", "->" };

        for (const auto& kw : keywords)
        {
            lexGen.addExpression(kw, kw);
        }

        auto id = 1;
        for (const auto& op : operators)
        {
            std::string name = "op" + std::to_string(id);
            lexGen.addVerbatimExpression(op, name);
            id++;
        }
        lexGen.addExpression("[ \t]+", "whitespace");
        lexGen.addExpression("\n|(\r\n)", "new_line");

        lexGen.addMacro("[a-zA-Z_]", "NonDigit");
        lexGen.addMacro("[0-9]", "Digit");

        lexGen.addExpression("{NonDigit}({NonDigit}|{Digit})*", "identifier");
        lexGen.addExpression("/\\*", "multilineComment")
            ->setPushContext("Comment");

        lexGen.newContext("Comment");
        lexGen.addExpression("[^\\*]+");
        lexGen.addExpression("\\*[^\\/]");
        lexGen.addExpression("\\*\\/")->setPopAction();

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
