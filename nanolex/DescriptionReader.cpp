#include "DescriptionReader.h"
#include <algorithm>
#include <climits>
#include <filesystem>
#include <set>

namespace NanoLexer
{
	namespace
	{
		struct Line
		{
			int number;
			std::string text;
		};

		struct Macro
		{
			int line;
			std::string name;
			std::string regex;
		};

		struct Rule
		{
			int line;
			std::string pattern;
		};

		struct LocatedError
		{
			int line;	// 0 when the error has no line
			std::string message;
		};

		bool isBlank(char c)
		{
			return c == ' ' || c == '\t';
		}

		bool isBlankLine(const std::string& line)
		{
			return std::all_of(line.begin(), line.end(), isBlank);
		}

		std::string trim(const std::string& str)
		{
			auto begin = std::find_if_not(str.begin(), str.end(), isBlank);
			auto end = std::find_if_not(str.rbegin(), str.rend(), isBlank).base();
			return (begin < end) ? std::string(begin, end) : std::string();
		}

		bool isSeparator(const std::string& line)
		{
			return line.compare(0, 2, "%%") == 0 && isBlankLine(line.substr(2));
		}

		bool isIdentifier(const std::string& str)
		{
			auto isAlpha = [](char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; };
			auto isAlnum = [&](char c) { return isAlpha(c) || (c >= '0' && c <= '9'); };
			return !str.empty() && isAlpha(str[0]) && std::all_of(str.begin() + 1, str.end(), isAlnum);
		}

		std::vector<Line> splitLines(const std::string& text)
		{
			std::vector<Line> lines;
			std::size_t start = 0;
			for (int number = 1; start < text.size(); number++)
			{
				auto end = text.find('\n', start);
				if (end == std::string::npos)
					end = text.size();
				auto line = text.substr(start, end - start);
				if (!line.empty() && line.back() == '\r')
					line.pop_back();
				lines.push_back({ number, line });
				start = end + 1;
			}
			return lines;
		}

		// Splits a line into its first word and the rest, without surrounding blanks.
		std::pair<std::string, std::string> splitWord(const std::string& line)
		{
			auto end = std::find_if(line.begin(), line.end(), isBlank);
			return { std::string(line.begin(), end), trim(std::string(end, line.end())) };
		}

		// Position of the end of the pattern starting a rule line: as in Lex, the first blank that is not
		// escaped, quoted or inside a character class.
		std::size_t patternEnd(const std::string& line)
		{
			bool quoted = false;
			bool inClass = false;
			for (std::size_t i = 0; i < line.size(); i++)
			{
				char c = line[i];
				if (c == '\\')
					i++;
				else if (c == '"')
					quoted = !quoted;
				else if (quoted)
					continue;
				else if (inClass)
				{
					if (c == ']')
						inClass = false;
					else if (c == '[' && i + 1 < line.size() && line[i + 1] == ':')	// [:alnum:] and the like
					{
						auto close = line.find(":]", i + 2);
						if (close != std::string::npos)
							i = close + 1;
					}
				}
				else if (c == '[')
					inClass = true;
				else if (isBlank(c))
					return i;
			}
			return line.size();
		}

		// Regex errors of a rule end with " in expression '<internal name>'": the line identifies the rule better.
		std::string withoutExpressionName(const std::string& message)
		{
			auto pos = message.rfind(" in expression '");
			if (pos == std::string::npos || message.back() != '\'')
				return message;
			return message.substr(0, pos);
		}
	}

	std::string formatError(const std::string& fileName, int line, const std::string& message)
	{
		if (line > 0)
			return fileName + "(" + std::to_string(line) + "): " + message;
		return fileName + ": " + message;
	}

	Description readDescription(const std::string& text, const std::string& fileName)
	{
		enum class Section { definitions, rules, userCode };

		std::vector<LocatedError> errors;
		std::string lexerName = std::filesystem::path(fileName).stem().string();
		int nameLine = 0;
		std::vector<Macro> macros;
		std::vector<Rule> rules;
		bool userCodeReported = false;

		auto section = Section::definitions;
		for (const auto& [number, line] : splitLines(text))
		{
			if (section == Section::userCode)
			{
				if (!isBlankLine(line) && !userCodeReported)
				{
					errors.push_back({ number, "the user code section is not supported yet" });
					userCodeReported = true;
				}
				continue;
			}
			if (isSeparator(line))
			{
				section = (section == Section::definitions) ? Section::rules : Section::userCode;
				continue;
			}
			if (isBlankLine(line))
				continue;
			if (isBlank(line[0]))
			{
				errors.push_back({ number, (section == Section::definitions)
					? "unexpected indented line in the definitions section"
					: "unexpected indented line, a pattern must start at column 0" });
				continue;
			}
			if (line[0] == '%')
			{
				auto [directive, argument] = splitWord(line);
				if (section == Section::definitions && directive == "%name")
				{
					if (nameLine)
						errors.push_back({ number, "duplicate %name, already declared at line " + std::to_string(nameLine) });
					else if (argument.empty())
						errors.push_back({ number, "%name expects a lexer name" });
					else
					{
						lexerName = argument;
						nameLine = number;
					}
				}
				else
					errors.push_back({ number, "unknown directive '" + directive + "'" });
				continue;
			}
			if (section == Section::definitions)
			{
				auto [name, regex] = splitWord(line);
				if (regex.empty())
					errors.push_back({ number, "missing regular expression for macro '" + name + "'" });
				else
					macros.push_back({ number, name, regex });
			}
			else
			{
				auto end = patternEnd(line);
				auto rest = trim(line.substr(end));
				if (!rest.empty())
					errors.push_back({ number, "unexpected text after the pattern: '" + rest + "'" });
				else
					rules.push_back({ number, line.substr(0, end) });
			}
		}
		if (section == Section::definitions)
			errors.push_back({ 0, "missing '%%' between the definitions and rules sections" });
		else if (rules.empty() && errors.empty())
			errors.push_back({ 0, "no rule in the rules section" });
		if (!isIdentifier(lexerName))
			errors.push_back({ nameLine, "invalid lexer name '" + lexerName + "'" + (nameLine ? "" : ", use %name to set one") });

		// throwEarly: each faulty call throws, so its errors can be attributed to the line being processed
		auto generator = std::make_unique<LexerGenerator>(lexerName, true);
		std::set<std::string> macroNames;
		for (const auto& macro : macros)
		{
			if (!macroNames.insert(macro.name).second)
			{
				errors.push_back({ macro.line, "duplicate macro '" + macro.name + "'" });
				continue;
			}
			try
			{
				generator->addMacro(macro.regex, macro.name);
			}
			catch (const NanoLexerException& e)
			{
				for (const auto& message : e.getMessages())
					errors.push_back({ macro.line, message });
			}
		}
		for (const auto& rule : rules)
		{
			try
			{
				generator->addExpression(rule.pattern);
			}
			catch (const NanoLexerException& e)
			{
				for (const auto& message : e.getMessages())
					errors.push_back({ rule.line, withoutExpressionName(message) });
			}
		}

		// errors without line come last
		std::stable_sort(errors.begin(), errors.end(), [](const LocatedError& a, const LocatedError& b) {
			return (a.line ? a.line : INT_MAX) < (b.line ? b.line : INT_MAX);
		});

		Description description;
		for (const auto& error : errors)
			description.errors.push_back(formatError(fileName, error.line, error.message));
		if (description.errors.empty())
			description.generator = std::move(generator);
		return description;
	}
}
