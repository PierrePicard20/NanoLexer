%{
#include <memory>
#include <string>
#include <map>
#include <iostream>
#include <sstream>
#include <cassert>

#define NEW_LINE "\r\n"

//#define YYDEBUG 1
#if YYDEBUG
	#define TRACE_FILE_DEBUG "trace_bison.txt"
	FILE *traceFile=nullptr;
	#define stderr traceFile
#endif YYDEBUG

#include "../../NanoLexer/LexerTreeNode.h"

void initLexer();
int getPreviousLexerPosition();

int yylex (void);
void yyerror(const char*s);

int hexaDigit2int(char h);
std::shared_ptr<RegularExpression::BaseLexerTreeNode> findMacro(const std::string &name);
std::shared_ptr<RegularExpression::BaseLexerTreeNode> createRangeMin(std::shared_ptr<RegularExpression::BaseLexerTreeNode> expReg, int min);
std::shared_ptr<RegularExpression::BaseLexerTreeNode> createRangeMinMax(std::shared_ptr<RegularExpression::BaseLexerTreeNode> expReg, int min, int max);
std::shared_ptr<RegularExpression::BaseLexerTreeNode> createRepeat(std::shared_ptr<RegularExpression::BaseLexerTreeNode> expReg, int count);

std::map<std::string, std::shared_ptr<RegularExpression::BaseLexerTreeNode>> mapTree;

std::vector<std::shared_ptr<RegularExpression::BaseLexerTreeNode>>	stackNodes;
std::vector<std::shared_ptr<RegularExpression::Set>> stackSet;

void addParsingErrorMessage(const std::string& msg);
std::vector<std::string>		errorMessages;
std::string						postfixMsg;

// shared globals
std::string macroName;

template <typename T>
std::shared_ptr<T> popSet();

std::shared_ptr<RegularExpression::BaseLexerTreeNode> popNode();
%}

%token <character> _INT_DIGIT_
%token <character> _HEXA_LETTER_
%token <character> _CHAR_
%token <id> _MACRO_
%token _PREFIX_

// character sets
%token _ALPHA_
%token _ALNUM_
%token _LOWER_
%token _UPPER_
%token _DIGIT_
%token _XDIGIT_
%token _SPACE_
%token _BLANK_
%token _CNTRL_
%token _PRINT_
%token _GRAPH_
%token _PUNCT_
%token _NOT_SPACE_
%token _NOT_DIGIT_
%token _WORD_
%token _NOT_WORD_

// special characters
%token <character> _NEW_LINE_
%token <character> _TAB_
%token <character> _VTAB_
%token <character> _BSPACE_
%token <character> _CRET_
%token <character> _FFEED_
%token <character> _BEEP_

%union {
    int character;
	int 	number;
}

%type <character> hexa_digit
%type <character> char_literal
%type <number> integer

%right '|' '?' '*' '+' '{'

%%

l_reg_exp :
  l_reg_exp reg_exp
	{
		auto node2 = popNode();
		auto node1 = popNode();
		auto concat = std::dynamic_pointer_cast<RegularExpression::ConcatNode>(node1);
		if (concat)
		{
			concat->addChild(node2);
		}
		else
		{
			concat = std::make_shared<RegularExpression::ConcatNode>();
			concat->addChild(node1);
			concat->addChild(node2);
		}
		stackNodes.push_back(concat);
	}
|           reg_exp
;

reg_exp :
  single_char
    {
		auto interval = popSet<RegularExpression::Interval>();
		auto multiInterval = std::make_shared<RegularExpression::MultiInterval>();
		multiInterval->addInterval(*interval.get());
		stackNodes.push_back(std::make_shared<RegularExpression::LeafNode>(multiInterval));
	}
| class_char
    {
		auto set = popSet<RegularExpression::Set>();
		stackNodes.push_back(std::make_shared<RegularExpression::LeafNode>(set));
	}
| '.'
    {
        auto except = std::make_shared<RegularExpression::ExceptNode>();
        except->addException(RegularExpression::Interval('\n', '\n'));
		stackNodes.push_back(except);
    }
| '[' l_charset ']'
    {
		auto set = popSet<RegularExpression::MultiInterval>();
        auto range = std::make_shared<RegularExpression::RangeNode>();
        for (const auto& interval : set->getIntervals())
            range->addInterval(interval);
		stackNodes.push_back(range);
    }
| '[' '^' l_charset ']'
    {
		auto set = popSet<RegularExpression::MultiInterval>();
        auto range = std::make_shared<RegularExpression::ExceptNode>();
        for (const auto& interval : set->getIntervals())
            range->addException(interval);
		stackNodes.push_back(range);
    }
| reg_exp '?'
    {
		auto node = popNode();
        auto or_ = std::make_shared<RegularExpression::OrNode>();
        if (node)
            or_->addChild(node);
        or_->addChild(std::make_shared<RegularExpression::EmptyNode>());
		stackNodes.push_back(or_);
    }
| reg_exp '*'
	{
		auto node = popNode();
		stackNodes.push_back(std::make_shared<RegularExpression::StarNode>(node));
	}
| reg_exp '+'
	{
		auto node = popNode();
		stackNodes.push_back(std::make_unique<RegularExpression::PlusNode>(node));
	}
| reg_exp '|' reg_exp
    {
		auto node2 = popNode();
		auto node1 = popNode();
        auto or_ = std::dynamic_pointer_cast<RegularExpression::OrNode>(node1);
        if (!or_)
        {
            or_ = std::make_shared<RegularExpression::OrNode>();
            or_->addChild(node1);
        }
        or_->addChild(node2);
		stackNodes.push_back(or_);
    }
| '(' l_reg_exp ')'
| _MACRO_ 									{ stackNodes.push_back(findMacro(macroName)); }
| reg_exp '{' integer             '}' 		{ auto node = popNode(); stackNodes.push_back(createRepeat(node, $3)); }
| reg_exp '{' integer ',' integer '}' 		{ auto node = popNode(); stackNodes.push_back(createRangeMinMax(node, $3, $5)); }
| reg_exp '{' integer ','		  '}' 		{ auto node = popNode(); stackNodes.push_back(createRangeMin(node, $3)); }
;

hexa_digit:
  _INT_DIGIT_
| _HEXA_LETTER_
;

char_literal:
  _CHAR_
| _NEW_LINE_
| _TAB_
| _VTAB_
| _BSPACE_
| _CRET_
| _FFEED_
| _BEEP_
| hexa_digit
;

single_char:
  char_literal						{ stackSet.push_back(std::make_shared<RegularExpression::Interval>($1,$1)); }
| _PREFIX_ hexa_digit hexa_digit 	{ auto val = hexaDigit2int($2) * 16 + hexaDigit2int($3); stackSet.push_back(std::make_shared<RegularExpression::Interval>(val, val)); }
;

class_char:
  _NOT_SPACE_			{ stackSet.push_back(RegularExpression::Set::getSpaceSet()->reverse()); }
| _NOT_DIGIT_			{ stackSet.push_back(RegularExpression::Set::getDigitSet()->reverse()); }
| _WORD_  				{ stackSet.push_back(RegularExpression::Set::getWordSet()); }
| _NOT_WORD_			{ stackSet.push_back(RegularExpression::Set::getWordSet()->reverse()); }
| _ALPHA_ 				{ stackSet.push_back(RegularExpression::Set::getAlphabeticSet()); }
| _ALNUM_ 				{ stackSet.push_back(RegularExpression::Set::getAlphanumericSet()); }
| _LOWER_ 				{ stackSet.push_back(RegularExpression::Set::getLowerSet()); }
| _UPPER_ 				{ stackSet.push_back(RegularExpression::Set::getUpperSet()); }
| _DIGIT_ 				{ stackSet.push_back(RegularExpression::Set::getDigitSet()); }
| _XDIGIT_ 				{ stackSet.push_back(RegularExpression::Set::getHexaDigitSet()); }
| _SPACE_ 				{ stackSet.push_back(RegularExpression::Set::getSpaceSet()); }
| _BLANK_ 				{ stackSet.push_back(RegularExpression::Set::getBlankSet()); }
| _CNTRL_ 				{ stackSet.push_back(RegularExpression::Set::getControlSet()); }
| _PRINT_ 				{ stackSet.push_back(RegularExpression::Set::getPrintSet()); }
| _GRAPH_ 				{ stackSet.push_back(RegularExpression::Set::getGraphSet()); }
| _PUNCT_ 				{ stackSet.push_back(RegularExpression::Set::getPunctSet()); }
;

integer:
	integer _INT_DIGIT_ 	{ $$ = 10*$1 + $2-'0'; }
|	        _INT_DIGIT_  	{ $$ = $1-'0'; }
;

l_charset :
  l_charset charset
	{
		auto set2 = popSet<RegularExpression::MultiInterval>();
		auto set1 = std::dynamic_pointer_cast<RegularExpression::MultiInterval>(stackSet.back());
		set1->addInterval(set2->getIntervals());
	}
|           charset
;

charset :
  single_char
    {
		auto interval = popSet<RegularExpression::Interval>();
		auto multiInterval = std::make_shared<RegularExpression::MultiInterval>();
		multiInterval->addInterval(*interval.get());
		stackSet.push_back(multiInterval);
	}
| class_char
| single_char '-' single_char
	{
		auto set2 = popSet<RegularExpression::Interval>();
		auto set1 = popSet<RegularExpression::Interval>();
		auto multiInterval = std::make_shared<RegularExpression::MultiInterval>();
		multiInterval->addInterval(RegularExpression::Interval(set1->getMin(), set2->getMin()));
		stackSet.push_back(multiInterval);
	}
;

%%

template <typename T>
std::shared_ptr<T> popSet()
{
	auto elem = std::dynamic_pointer_cast<T>(stackSet.back());
	stackSet.pop_back();
	assert(elem);
	return elem;
}

std::shared_ptr<RegularExpression::BaseLexerTreeNode> popNode()
{
	auto elem = stackNodes.back();
	stackNodes.pop_back();
	//assert(elem);
	return elem;
}

int hexaDigit2int(char h)
{
    if ((h >= '0') && (h <= '9'))
        return h - '0';
    if ((h >= 'a') && (h <= 'f'))
        return 10 + h - 'a';
    if ((h >= 'A') && (h <= 'F'))
        return 10 + h - 'A';
    return -1;
}

std::shared_ptr<RegularExpression::BaseLexerTreeNode> findMacro(const std::string &name)
{
	auto iter = mapTree.find(name);
	if (iter != mapTree.end())
	{
		return iter->second->clone();
	}
	else
	{
		std::string msg = "Unknown macro identifier '";
		msg += name;
		msg += "'";
		addParsingErrorMessage(msg);
		return nullptr;
	}
}

std::shared_ptr<RegularExpression::BaseLexerTreeNode> createRangeMin(std::shared_ptr<RegularExpression::BaseLexerTreeNode> expReg, int min)
{
	if (expReg)
	{
		if (min == 0)
		{
			return std::make_unique<RegularExpression::StarNode>(expReg);
		}
		else if (min == 1)
		{
			return std::make_unique<RegularExpression::PlusNode>(expReg);
		}
		std::vector<std::shared_ptr<RegularExpression::BaseLexerTreeNode>>  concatVector;
		for (auto i = 0; i < min-1; i++)
		{
			concatVector.push_back(expReg->clone());
		}
		auto result = std::make_shared<RegularExpression::ConcatNode>();
		result->addChild(std::make_shared<RegularExpression::ConcatNode>(std::move(concatVector)));
		result->addChild(std::make_shared<RegularExpression::PlusNode>(expReg));
		return result;
	}
	return nullptr;
}

std::shared_ptr<RegularExpression::BaseLexerTreeNode> createRangeMinMax(std::shared_ptr<RegularExpression::BaseLexerTreeNode> expReg, int min, int max)
{
	assert(expReg);
	if (expReg)
	{
		if (min > max)
		{
			addParsingErrorMessage("Maximum occurence count must be greater than the minimum count");
		}
		else if (max < 1)
		{
			addParsingErrorMessage("Maximum occurence count must be greater than 0");
		}
		else
		{
			std::vector<std::shared_ptr<RegularExpression::BaseLexerTreeNode>>  concatVector;
			for (auto i = 0; i < min; i++)
			{
				concatVector.push_back(expReg->clone());
			}

			auto baseConcat = std::make_shared<RegularExpression::ConcatNode>(std::move(concatVector));
			std::vector<std::shared_ptr<RegularExpression::BaseLexerTreeNode>>  orVector;

			orVector.push_back(std::make_shared<RegularExpression::EmptyNode>());
			while (min != max)
			{
				for (auto i = 0; i < max - min; i++)
				{
					concatVector.push_back(expReg->clone());
				}
				orVector.push_back(std::make_shared<RegularExpression::ConcatNode>(std::move(concatVector)));
				min++;
			}
			concatVector.push_back(std::move(baseConcat));
			concatVector.push_back(std::make_unique<RegularExpression::OrNode>(std::move(orVector)));
			return std::make_shared<RegularExpression::ConcatNode>(std::move(concatVector));
		}
	}
	return nullptr;
}

std::shared_ptr<RegularExpression::BaseLexerTreeNode> createRepeat(std::shared_ptr<RegularExpression::BaseLexerTreeNode> expReg, int count)
{
	if (expReg)
	{
		if (count < 1)
		{
			addParsingErrorMessage("Invalid occurence count");
		}
		else if (count == 1)
		{
			return expReg;
		}
		else
		{
			count--;
			std::vector<std::shared_ptr<RegularExpression::BaseLexerTreeNode>>  result;
			while (count)
			{
				result.push_back(expReg->clone());
				count--;
			}
			result.push_back(std::move(expReg));
			return std::make_unique<RegularExpression::ConcatNode>(std::move(result));
		}
	}
	return nullptr;
}

void yyerror(const char*s)
{
	std::string msg = "syntax error at row ";
	msg += std::to_string(getPreviousLexerPosition());
	addParsingErrorMessage(msg);
}

void addMacroTree(const std::string& name, std::shared_ptr<RegularExpression::BaseLexerTreeNode> tree)
{
    mapTree.emplace(name, tree);
}

void addParsingErrorMessage(const std::string& msg)
{
	errorMessages.push_back(msg + postfixMsg);
}

const std::vector<std::string>& getParsingErrorMessages()
{
	return errorMessages;
}

//////////////////////////////////////////////////
// The section below is for parsing of strings

typedef struct yy_buffer_state * YY_BUFFER_STATE;
extern YY_BUFFER_STATE yy_scan_string(const char *);

std::shared_ptr<RegularExpression::BaseLexerTreeNode> parseString(const char *str, const std::string& postfixMsg_)
{
	postfixMsg = postfixMsg_;
	errorMessages.clear();
    initLexer();
	stackNodes.clear();
	stackSet.clear();
    yy_scan_string(str);
#if YYDEBUG
	yydebug=1;
	traceFile = fopen( TRACE_FILE_DEBUG, "a" );
	fprintf(traceFile, "=+=+=+=+=+=+=+=+=+=+=+=+= START '%s' =+=+=+=+=+=+=+=+=+=+=+=+=" NEW_LINE, str);
#endif
    yyparse();
#if YYDEBUG
	yydebug=0;
	fprintf(traceFile, "=+=+=+=+=+=+=+=+=+=+=+=+= END =+=+=+=+=+=+=+=+=+=+=+=+=" NEW_LINE);
	fclose( traceFile );
#endif
	if (stackNodes.size()==0)
		return nullptr;
	auto node = popNode();
	//assert(stackNodes.empty());
	//assert(stackSet.empty());
    return node;
}
