#ifndef LEXGEN_H
#define LEXGEN_H
#include <iostream>
#include "../include/NanoLexer.h"

using namespace NanoLexer;

extern const char* outputPath;

void genSimpleLexer1();
void genArithmeticExpressionLexer();
void genCppLexer();
void genCMultilineCommentLexer();

#endif
