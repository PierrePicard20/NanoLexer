#include "lexgen.h"

void genCMultilineCommentLexer()
{
    try
    {
        LexerGenerator lexGen("Comments");

        auto actionOnNewLine = "line++; row = 1;";
        auto actionOnAllOtherLexeme = "row += getMatchLength()-nbCharToSubstract;";

        lexGen.addOnStartNextToken("            nbCharToSubstract = 0;");
        lexGen.addPrivateMembers("        int line,row,nbCharToSubstract;");      // add private fields to the lexer
        lexGen.addOnCreate("            line = 1; row = 1;");                       // initialize the fields in the lexer constructor
        lexGen.addPublicMembers(R"*(        inline int getCurrentLine() const {return line;}
            inline int getCurrentRow() const {return row;})*");
        lexGen.addDefaultMainContextOnMatch(actionOnAllOtherLexeme);

        // by default, without having declared a new context, any new expression is added to the main context
        lexGen.addExpression("\\n|(\\r\\n)", "new_line")
            ->addOnMatchCode(actionOnNewLine);
        lexGen.addExpression("/\\*", "multilineComment")        // when the begining of a multiline C comment is matched, the context 'Comment' is pushed.
            ->setPushContext("Comment");

        // after the declaration of a new context (with the method newContext() below), any new expression is added to that context
        // 
        lexGen.newContext("Comment");               // context 'Comment' purpose is to specifically match the rest of a multiline C comment
        lexGen.addExpression("\\n|(\\r\\n)")
            ->addOnMatchCode("line++; row = 1; nbCharToSubstract = getMatchLength();");
        lexGen.addExpression("[^\\*\\r\\n]+");      // match any character but '*' or new line
        lexGen.addExpression("\\*[^\\/\\r\\n]");    // match '*' followed by any character but '/' or new line
        lexGen.addExpression("\\*\\/")              // match the closing pattern of the comment ('*/') => pop the context and return to the main one
            ->setPopAction();

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
