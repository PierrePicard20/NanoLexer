#include "lexgen.h"

#include <filesystem>

std::string outputPath;

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "usage: testgen <output_dir>" << std::endl;
        return 1;
    }
    outputPath = std::filesystem::absolute(argv[1]).string();

    genSimpleLexer1();
    genArithmeticExpressionLexer();
    genCppLexer();
    genCMultilineCommentLexer();
}
