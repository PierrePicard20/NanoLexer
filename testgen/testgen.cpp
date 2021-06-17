#include "lexgen.h"

const char* outputPath = "..\\..\\..\\tests";

int main()
{
    genSimpleLexer1();
    genArithmeticExpressionLexer();
    genCppLexer();
    genCMultilineCommentLexer();
}
