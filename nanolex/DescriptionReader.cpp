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
			bool hasAction;
			std::string action;	// code between the braces of the action
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

		bool isIdentifierChar(char c)
		{
			return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
		}

		// Identifier or number token ending just before 'pos'.
		std::string tokenBefore(const std::string& text, std::size_t pos)
		{
			auto begin = pos;
			while (begin > 0 && (isIdentifierChar(text[begin - 1]) || text[begin - 1] == '.'))
				begin--;
			return text.substr(begin, pos - begin);
		}

		// Position of the '}' closing the action whose '{' is at 'open', or npos when the action is not terminated.
		// Braces inside C++ string literals (raw ones included), character literals and comments are ignored. An
		// ordinary string or character literal ends at the end of its line, so that a stray quote cannot hide the
		// rest of the file.
		std::size_t actionEnd(const std::string& text, std::size_t open)
		{
			int depth = 0;
			for (std::size_t i = open; i < text.size(); i++)
			{
				char c = text[i];
				if (c == '{')
					depth++;
				else if (c == '}')
				{
					if (--depth == 0)
						return i;
				}
				else if (c == '/' && i + 1 < text.size() && text[i + 1] == '/')
				{
					i = text.find('\n', i);
					if (i == std::string::npos)
						return i;
				}
				else if (c == '/' && i + 1 < text.size() && text[i + 1] == '*')
				{
					i = text.find("*/", i + 2);
					if (i == std::string::npos)
						return i;
					i++;
				}
				else if (c == '"' || c == '\'')
				{
					auto prefix = tokenBefore(text, i);
					if (c == '\'' && !prefix.empty() && prefix[0] >= '0' && prefix[0] <= '9')
						continue;	// digit separator, as in 1'000
					if (c == '"' && (prefix == "R" || prefix == "u8R" || prefix == "uR" || prefix == "UR" || prefix == "LR"))	// raw string R"delim( ... )delim"
					{
						auto paren = text.find('(', i + 1);
						if (paren == std::string::npos)
							return paren;
						auto closing = ")" + text.substr(i + 1, paren - i - 1) + "\"";
						i = text.find(closing, paren);
						if (i == std::string::npos)
							return i;
						i += closing.size() - 1;	// on the closing quote
						continue;
					}
					for (i++; i < text.size() && text[i] != c && text[i] != '\n'; i++)
						if (text[i] == '\\')
							i++;
				}
			}
			return std::string::npos;
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
		int defaultLine = 0;
		std::string defaultAction;

		// actions may span several lines: they are scanned in the whole text, its lines joined by '\n'
		auto lines = splitLines(text);
		std::string joined;
		std::vector<std::size_t> lineStarts;
		for (const auto& line : lines)
		{
			lineStarts.push_back(joined.size());
			joined += line.text + "\n";
		}

		// Reads the action whose '{' is at 'column' of line 'index' into 'code'. 'index' becomes the line of the
		// closing '}', or the last line when the action is not terminated.
		auto readAction = [&](std::size_t& index, std::size_t column, std::string& code) {
			auto open = lineStarts[index] + column;
			auto close = actionEnd(joined, open);
			if (close == std::string::npos)
			{
				errors.push_back({ lines[index].number, "unterminated action, missing '}'" });
				index = lines.size() - 1;
				return false;
			}
			code = joined.substr(open + 1, close - open - 1);
			index = std::upper_bound(lineStarts.begin(), lineStarts.end(), close) - lineStarts.begin() - 1;
			auto rest = trim(lines[index].text.substr(close + 1 - lineStarts[index]));
			if (!rest.empty())
			{
				errors.push_back({ lines[index].number, "unexpected text after the action: '" + rest + "'" });
				return false;
			}
			return true;
		};

		auto section = Section::definitions;
		for (std::size_t index = 0; index < lines.size(); index++)
		{
			const auto& [number, line] = lines[index];
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
				else if (section == Section::rules && directive == "%default")
				{
					auto brace = line.find_first_not_of(" \t", directive.size());
					std::string code;
					if (brace == std::string::npos || line[brace] != '{')
						errors.push_back({ number, "%default expects an action '{ ... }'" });
					else if (readAction(index, brace, code))
					{
						if (defaultLine)
							errors.push_back({ number, "duplicate %default, already declared at line " + std::to_string(defaultLine) });
						else
						{
							defaultLine = number;
							defaultAction = code;
						}
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
				auto restStart = line.find_first_not_of(" \t", end);
				Rule rule{ number, line.substr(0, end), false, "" };
				if (restStart == std::string::npos)
					rules.push_back(rule);
				else if (line[restStart] == '{')
				{
					rule.hasAction = readAction(index, restStart, rule.action);
					if (rule.hasAction)
						rules.push_back(rule);
				}
				else
					errors.push_back({ number, "unexpected text after the pattern: '" + trim(line.substr(end)) + "'" });
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
				auto expression = generator->addExpression(rule.pattern);
				if (expression && rule.hasAction)
					expression->addOnMatchCode(rule.action);
			}
			catch (const NanoLexerException& e)
			{
				for (const auto& message : e.getMessages())
					errors.push_back({ rule.line, withoutExpressionName(message) });
			}
		}

		if (defaultLine)
			generator->addDefaultMainContextOnMatch(defaultAction);

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
