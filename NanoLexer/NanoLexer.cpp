#include <iostream>
#include <fstream>
#include <memory>
#include "../include/NanoLexer.h"
#include "shared.h"
#include "LexerBuilder.h"
#include <filesystem>
#include <cassert>
#ifdef WIN32
#include <windows.h>

std::string getexepath()
{
	char result[MAX_PATH];
	return std::string(result, GetModuleFileName(NULL, result, MAX_PATH));
}
#else
#include <limits.h>
#include <unistd.h>

std::string getexepath()
{
	char result[PATH_MAX];
	ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
	return std::string(result, (count > 0) ? count : 0);
}
#endif

namespace
{
	std::map<std::string, std::map<std::string, std::shared_ptr<RegularExpression::BaseLexerTreeNode>>> generator2expressions;
	void addLexerTree(const std::string& generatorName, const std::string& exprName, std::shared_ptr<RegularExpression::BaseLexerTreeNode> tree)
	{
		auto iter = generator2expressions.emplace(generatorName, std::map<std::string, std::shared_ptr<RegularExpression::BaseLexerTreeNode>>()).first;
		iter->second.emplace(exprName, tree);
	}
	std::shared_ptr<RegularExpression::BaseLexerTreeNode> getLexerTree(const std::string& generatorName, const std::string& exprName)
	{
		auto iter = generator2expressions.find(generatorName);
		if (iter == generator2expressions.end())
			return nullptr;
		auto iterTree = iter->second.find(exprName);
		if (iterTree == iter->second.end())
			return nullptr;
		return iterTree->second;
	}

	bool checkExpressionAndName(const std::string& name, const std::string& expr, NanoLexer::LexerGenerator& lexgen)
	{
		if (expr.empty())
		{
			lexgen.addErrorMessage("Invalid empty expression");
			return false;
		}
		if (name.empty())
		{
			lexgen.addErrorMessage("Invalid empty name");
			return false;
		}
		int count = 0;
		for (auto c : name)
		{
			if (count == 0)
			{
				if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_')))
				{
					std::string msg = "Invalid identifier '";
					msg += name;
					msg += "'";
					lexgen.addErrorMessage(msg);
					return false;
				}
			}
			else
			{
				if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || (c == '_')))
				{
					std::string msg = "Invalid identifier '";
					msg += name;
					msg += "'";
					lexgen.addErrorMessage(msg);
					return false;
				}
			}
			count++;
		}
		return true;
	}

	const char* mainContextName = "main_context";
}

namespace NanoLexer
{
	// class NanoLexer::Expression
	LexerGenerator::Expression::Expression(const std::string& name, const std::string& expr, LexerGenerator& lg)
		:name{ name }, expr{ expr }, action{ ActionOnMatch::none }, lexerGenerator{ lg }
	{
	}

	LexerGenerator::Expression* LexerGenerator::Expression::setPushContext(const std::string& name)
	{
		action = ActionOnMatch::push;
		pushedContext = name;
		return this;
	}

	LexerGenerator::Expression* LexerGenerator::Expression::setPopAction()
	{
		action = ActionOnMatch::pop;
		pushedContext.clear();
		return this;
	}

	LexerGenerator::Expression* LexerGenerator::Expression::addOnMatchCode(const std::string& code)
	{
		matchCode = code;
		return this;
	}

	std::shared_ptr<RegularExpression::BaseLexerTreeNode> parse(const std::string& expr,const std::string& postfixMsg, LexerGenerator& lexerGenerator)
	{
		auto tree = parseString(expr.c_str(), postfixMsg);
		if (getParsingErrorMessages().size())
		{
			lexerGenerator.addErrorMessages(getParsingErrorMessages());
		}
		return tree;
	}

	// class NanoLexer
	LexerGenerator::LexerGenerator(const std::string& name, bool throwEarly_)
		:lexemeCount{ 0 }, currentContext{ mainContextName , {} }, lexerName{ name }, throwEarly{ throwEarly_ }
	{
		variables.emplace_back("$(LexerName)", name);
		lexemeId = 1;
	}

	LexerGenerator::~LexerGenerator()
	{
		generator2expressions.erase(lexerName);
	}

	LexerGenerator::Expression* LexerGenerator::addMacro(const std::string& expr, const std::string& name)
	{
		if (!checkExpressionAndName(name, expr, *this))
			return nullptr;

		auto macro = getMacro(name);
		if (macro)
			return nullptr;
		auto i = macros.emplace(name, Expression(name, expr, *this)).first;
		std::string postfixMsg = " in macro '";
		postfixMsg += name;
		postfixMsg += "'";
		auto tree = parse(expr, postfixMsg, *this);
		addMacroTree(name, tree);
		return &i->second;
	}

	LexerGenerator::Expression* LexerGenerator::getMacro(const std::string& name)
	{
		auto i = macros.find(name);
		if (i == macros.end())
			return nullptr;
		else
			return &i->second;
	}

	LexerGenerator::Expression* LexerGenerator::addExpression(const std::string& expr, const std::string& name)
	{
		std::string name_ = name;

		if (name_.empty())
		{
			name_ = "_lexeme_";
			name_ += std::to_string(lexemeId);
		}
		if (!checkExpressionAndName(name_, expr, *this))
			return nullptr;

		lexemeId++;
		if (name2expr.find(name_) == name2expr.end())
			name2expr.emplace(name_, expr);

		auto expreg = add2CurrentContext(name_, expr);
		if (!expreg)
			return nullptr;
		std::string postfixMsg = " in expression '";
		postfixMsg += name_;
		postfixMsg += "'";
		auto tree = parse(expr, postfixMsg, *this);
		addLexerTree(lexerName, name_, tree);
		return expreg;
	}

	LexerGenerator::Expression* LexerGenerator::addVerbatimExpression(const std::string& expr, const std::string& name)
	{
		std::string verbatimStr = "\"";
		verbatimStr += expr;
		verbatimStr += "\"";
		name2expr.emplace(name, expr);
		return addExpression(verbatimStr, name);
	}

	LexerGenerator::Expression* LexerGenerator::getExpression(const std::string& name)
	{
		for (auto& pair : std::get<1>(currentContext))
		{
			if (pair.first == name)
			{
				return &(pair.second);
			}
		}
		return nullptr;
	}

	void LexerGenerator::addErrorMessage(const std::string& msg)
	{
		if (throwEarly)
			throw NanoLexerException(msg);
		else
			errorMessages.emplace_back(msg);
	}

	void LexerGenerator::addErrorMessages(const std::vector<std::string>& msgs)
	{
		if (msgs.empty())
			return;

		if (throwEarly)
		{
			throw NanoLexerException(msgs);
		}
		else
		{
			for (const auto& str : msgs)
				errorMessages.emplace_back(str);
		}
	}

	void LexerGenerator::newContext(const std::string& name)
	{
		closeContext();
		if (contextNames.find(name) != contextNames.end())
		{
			std::string msg = "Duplicate context name : '";
			msg += name;
			msg += "'";
			addErrorMessage(msg);
		}
		std::get<0>(currentContext) = name;
		contextNames.insert(name);
	}

	std::string normalizeExpression(const std::string& expr)
	{
		std::string ret;
		for (auto c : expr)
		{
			switch (c)
			{
			case '\r': ret += "\\\\r"; break;
			case '\n': ret += "\\\\n"; break;
			case '\t': ret += "\\\\t"; break;
			case '\v': ret += "\\\\v"; break;
			case '\b': ret += "\\\\b"; break;
			case '\f': ret += "\\\\f"; break;
			case '\a': ret += "\\\\a"; break;
			case '\\': ret += "\\\\"; break;
			default: ret += c;
			}
		}
		return ret;
	}

	void LexerGenerator::generateLexer()
	{
		// if there is expressions in the current context, then we add that context
		if (std::get<1>(currentContext).size())
			closeContext();

		int id = 1;
		std::string source;
		std::string declarations;
		std::ostringstream lexNames, nameCases, exprCases, onMatch;
		std::map<std::string, std::shared_ptr<RegularExpression::LexerContext>> contexts;

		debugString = "";
		lexemeCount = 0;
		for (auto& context : contextsData)
		{
			std::vector<std::shared_ptr<RegularExpression::BaseLexerTreeNode>>  expRegTrees;
			std::set<int> popIds;
			auto& expressions = std::get<1>(context);
			auto& contextName = std::get<0>(context);
			auto isMainContext = (contextName == mainContextName);
			for (auto& expr : expressions)
			{
				switch (expr.second.getAction())
				{
				case ActionOnMatch::pop:
					if (contextName == mainContextName)
					{
						std::string msg = "Cannot set a pop action in main context on expression '";
						msg += expr.first;
						msg += "'";
						addErrorMessage(msg);
					}
					popIds.insert(id);
					break;
				case ActionOnMatch::push:
					auto& pushedName = expr.second.getPushedContext();
					if (contextNames.find(pushedName) == contextNames.end())
					{
						std::string msg = "Unknown context name : '";
						msg += pushedName;
						msg += "'";
						addErrorMessage(msg);
					}
					break;
				}

				if (!expr.second.matchCode.empty() && isMainContext)
				{
					onMatch << "			case Lexeme::" << expr.second.getName() << "_:" << std::endl;
					onMatch << "				{" << expr.second.matchCode << " break;}" << std::endl;
				}
				if (isMainContext)
				{
					if (id > 1)
					{
						lexNames << std::endl << "            ";
						nameCases << std::endl << "                ";
						exprCases << std::endl << "                ";
					}
					lexNames << expr.first << "_,";
					nameCases << "case Lexeme::" << expr.first << "_: return \"" << expr.first << "\";";
					auto iter = name2expr.find(expr.first);
					exprCases << "case Lexeme::" << expr.first << "_: return \"" << normalizeExpression(iter->second) << "\";";
				}
				auto tree = getLexerTree(lexerName, expr.first);
				auto concatNode = std::dynamic_pointer_cast<RegularExpression::ConcatNode>(tree);
				if (!concatNode)
				{
					concatNode = std::make_shared<RegularExpression::ConcatNode>();
					concatNode->addChild(tree);
				}
				concatNode->addChild(std::make_unique<RegularExpression::EndNode>(id));
				expRegTrees.push_back(std::move(concatNode));
				id++;
			}
			auto pair = contexts.emplace(contextName, std::make_shared<RegularExpression::LexerContext>(contextName, (int)expressions.size(), popIds));
			assert(pair.second);
			auto context = pair.first->second;
			context->setExpRegList(std::move(expRegTrees));
			context->ComputeLexer();

			debugString += context->getDebugString(popIds);
			lexemeCount += context->getLexemeCount();
		}
		onMatch << "			default:" << std::endl;
		onMatch << "				{" << defaultOnMatch << "}" << std::endl;

		std::map<int, std::string>	id2OnMatchCode;
		id = 1;
		for (auto& context : contextsData)
		{
			auto& currentContextName = std::get<0>(context);
			auto iter = contexts.find(currentContextName);
			assert(iter != contexts.end());
			auto currentContext = iter->second;

			auto& expressions = std::get<1>(context);
			id2OnMatchCode.clear();
			for (auto& expr : expressions)
			{
				if (expr.second.getAction() == ActionOnMatch::push)
				{
					auto& pushedContextName = expr.second.getPushedContext();
					auto iter = contexts.find(pushedContextName);
					assert(iter != contexts.end());
					auto pushedContext = iter->second;
					currentContext->addPushContext(id, pushedContext.get());
					if (currentContextName != mainContextName)
					{
						if (!expr.second.matchCode.empty())
						{
							id2OnMatchCode.emplace(id, expr.second.matchCode);
						}
					}
				}
				id++;
			}
			declarations += currentContext->declareToCpp();
			auto iterFailure = contextName2contextFailure.find(currentContextName);
			std::string contextFailure = (iterFailure != contextName2contextFailure.end()) ? iterFailure->second: "";
			source += currentContext->bodyToCpp(contextFailure, id2OnMatchCode);
		}
		variables.emplace_back("$(PublicMembers)", publicMembers);
		variables.emplace_back("$(ProtectedMembers)", protectedMembers);
		variables.emplace_back("$(PrivateMembers)", privateMembers);
		variables.emplace_back("$(OnCreate)", onCreate);
		variables.emplace_back("$(OnStartNextToken)", onStartNextToken);
		variables.emplace_back("$(OnMatch)", onMatch.str());

		variables.emplace_back("$(Declarations)", declarations);
		variables.emplace_back("$(LexerContextsMethods)", source);
		variables.emplace_back("$(LexemeNames)", lexNames.str());
		variables.emplace_back("$(GetLexemeNamesCases)", nameCases.str());
		variables.emplace_back("$(GetLexemeExpressionsCases)", exprCases.str());

		if (!errorMessages.empty())
			throw NanoLexerException(errorMessages);
	}

	std::string readFile(const std::filesystem::path& file)
	{
		std::ostringstream skeleton;
		std::ifstream sk(file.string());
		for (std::string line; std::getline(sk, line); ) {
			skeleton << line << std::endl;
		}
		sk.close();
		return skeleton.str();
	}

	void writeFile(const std::filesystem::path& file, const std::string& str)
	{
		std::ofstream myfile;
		myfile.open(file.string());
		myfile << str;
		myfile.close();
	}

	void LexerGenerator::generateFiles(const std::string& language, const std::string& outputPath)
	{
		std::filesystem::path exePath = getexepath();
		exePath.remove_filename();
		auto lexer_sk = exePath;

		lexer_sk = lexer_sk / "lexer_sk";
		if (!std::filesystem::is_directory(lexer_sk))
		{
			std::cout << "Bad install, missing directory lexer_sk " << lexer_sk.string() << std::endl;
			return;
		}

		auto cpp = lexer_sk / language;
		if (!std::filesystem::is_directory(cpp))
		{
			std::cout << "Missing directory lexer_sk/" << language << std::endl;
			return;
		}

		std::string line;
		for (auto& file : std::filesystem::directory_iterator(cpp))
		{
			auto str = readFile(file.path());
			for (const auto& pair : variables)
			{
				std::size_t n = 0;
				while ((n = str.find(pair.first, n)) != std::string::npos)
				{
					str.replace(n, pair.first.size(), pair.second);
				}
			}

			auto filename = lexerName + file.path().filename().string();
			auto myPath = exePath / outputPath / filename;
			writeFile(myPath, str);
		}
		std::cout << "Lexer " << lexerName << " generated." << std::endl;
	}

	std::string LexerGenerator::getDebugString()
	{
		return debugString;
	}

	void LexerGenerator::addPublicMembers(const std::string& code)
	{
		publicMembers = code;
	}
	void LexerGenerator::addProtectedMembers(const std::string& code)
	{
		protectedMembers = code;
	}
	void LexerGenerator::addPrivateMembers(const std::string& code)
	{
		privateMembers = code;
	}
	void LexerGenerator::addOnCreate(const std::string& code)
	{
		onCreate = code;
	}
	void LexerGenerator::addOnStartNextToken(const std::string& code)
	{
		onStartNextToken = code;
	}
	void LexerGenerator::addOnCurrentContextFailure(const std::string& code)
	{
		contextName2contextFailure.emplace(std::get<0>(currentContext), code);
	}

	void LexerGenerator::addDefaultMainContextOnMatch(const std::string& code)
	{
		defaultOnMatch = code;
	}

	///////////////////////////////////////////////////////// private methods ////////////////////////////////////////////////////

	void LexerGenerator::closeContext()
	{
		contextsData.emplace_back(std::move(currentContext));
	}

	LexerGenerator::Expression* LexerGenerator::add2CurrentContext(const std::string& name, const std::string& expr)
	{
		auto e = getExpression(name);
		if (e)
		{
			std::string msg = "Duplicate expression '";
			msg += name;
			msg += "' in context '";
			msg += std::get<0>(currentContext);
			msg += "'";
			addErrorMessage(msg);
		}
		std::get<1>(currentContext).emplace_back(name, Expression(name, expr, *this));
		return &std::get<1>(currentContext).back().second;
	}

	///////////////////////////////////////////////////////// NanoLexerException ////////////////////////////////////////////////////

	NanoLexerException::NanoLexerException(const std::vector<std::string>& messages_) :messages{ messages_ } {}
	NanoLexerException::NanoLexerException(const std::string& message) : messages{ message } {}
}
