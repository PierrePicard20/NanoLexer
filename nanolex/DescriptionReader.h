#pragma once
#include <memory>
#include <string>
#include <vector>
#include "NanoLexer.h"

namespace NanoLexer
{
	// Outcome of reading a description file (.nl).
	struct Description
	{
		// Configured generator, ready for generateLexer(). Null when errors is not empty.
		// It is created with throwEarly, so generateLexer() throws at the first error.
		std::unique_ptr<LexerGenerator> generator;

		// Errors sorted by line, each formatted by formatError().
		std::vector<std::string> errors;
	};

	// Reads the content of a description file. 'fileName' is used in error messages and, without %name,
	// gives the lexer name (its base name without extension).
	Description readDescription(const std::string& text, const std::string& fileName);

	// Formats an error the way MSVC does: "file.nl(line): message", or "file.nl: message" when line is 0.
	std::string formatError(const std::string& fileName, int line, const std::string& message);
}
