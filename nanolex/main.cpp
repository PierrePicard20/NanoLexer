// nanolex: generates a lexer from a description file (.nl).
// usage: nanolex <file.nl> [-o <output directory>] [-l <language>]
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include "DescriptionReader.h"

namespace
{
	int usage()
	{
		std::cerr << "usage: nanolex <file.nl> [-o <output directory>] [-l <language>]" << std::endl;
		return 2;
	}
}

int main(int argc, char* argv[])
{
	std::string input;
	std::string output = ".";
	std::string language = "cpp";
	for (int i = 1; i < argc; i++)
	{
		std::string arg = argv[i];
		if (arg == "-o" || arg == "-l")
		{
			if (i + 1 >= argc)
				return usage();
			(arg == "-o" ? output : language) = argv[++i];
		}
		else if (arg.empty() || arg[0] == '-' || !input.empty())
			return usage();
		else
			input = arg;
	}
	if (input.empty())
		return usage();

	std::ifstream file(input, std::ios::binary);
	if (!file)
	{
		std::cerr << NanoLexer::formatError(input, 0, "cannot open file") << std::endl;
		return 1;
	}
	std::ostringstream text;
	text << file.rdbuf();

	auto description = NanoLexer::readDescription(text.str(), input);
	for (const auto& error : description.errors)
		std::cerr << error << std::endl;
	if (!description.generator)
		return 1;

	try
	{
		description.generator->generateLexer();
		// generateFiles() resolves a relative path from the executable's directory: -o is relative to the current one
		description.generator->generateFiles(language, std::filesystem::absolute(output).string());
	}
	catch (const NanoLexer::NanoLexerException& e)
	{
		for (const auto& message : e.getMessages())
			std::cerr << NanoLexer::formatError(input, 0, message) << std::endl;
		return 1;
	}
	return 0;
}
