#pragma once

#include <string>
#include <memory>
#include "LexerTreeNode.h"

std::shared_ptr<RegularExpression::BaseLexerTreeNode> parseString(const char* str, const std::string& postfixMsg_);
void addMacroTree(const std::string& name, std::shared_ptr<RegularExpression::BaseLexerTreeNode> tree);

// share token _MACRO_ between lexer and parser
extern std::string macroName;

// error management
const std::vector<std::string>& getParsingErrorMessages();

