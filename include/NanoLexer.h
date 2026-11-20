#pragma once
#include <string>
#include <vector>
#include <tuple>
#include <memory>
#include <map>
#include <set>

namespace NanoLexer
{
	// class LexerGenerator allows to define the model of a lexer.
	// The generated lexers may contain several contexts, which allows a higher power of expression. 
	// This feature is required to match C multiline comments and strings.
	class LexerGenerator
	{
	public:
		enum class ActionOnMatch
		{
			none		// no specific action
			, pop		// pop the current context
			, push		// push a new context
		};

		// class Expression contains all information related to a regular expression and for its parsing
		class Expression
		{
		public:
			// Set the context that must be pushed when a match of the current regular expression is found.
			// The 'name' must be a name of a context defined at some point my the method LexerGenerator::newContext().
			Expression* setPushContext(const std::string& name);

			// Set an action Pop.
			Expression* setPopAction();

			// Add statements in the generated lexer that will be called when a match if found for the expression.
			// Overrides LexerGenerator::addDefaultMainContextOnMatch() for the present expression, even when 'code' is empty.
			// In the main context, the code may return a value from lex(); otherwise the lexeme is ignored and lex()
			// moves on to the next one.
			Expression* addOnMatchCode(const std::string& code);

			// Getters
			inline const std::string& getName() const { return name; }
			inline const std::string& getExpression() const { return expr; }
			inline ActionOnMatch getAction() const { return action; }
			inline const std::string& getPushedContext() const { return pushedContext; }
			inline bool hasOnMatchCode() const { return hasMatchCode; }
			inline const std::string& getOnMatchCode() const { return matchCode; }
		private:
			friend LexerGenerator;

			Expression(const std::string& name, const std::string& expr, LexerGenerator& lg);

			LexerGenerator& lexerGenerator;
			std::string name;
			std::string expr;
			std::string matchCode;
			bool hasMatchCode;	// set by addOnMatchCode(), even with an empty code
			ActionOnMatch	action;	// default is none
			std::string		pushedContext;	// valid only when action==ActionOnMatch::push
		};

		LexerGenerator(const std::string& name, bool throwEarly=false);
		~LexerGenerator();

		// Create a new expression associated with the current context.
		Expression* addExpression(const std::string& expr, const std::string& name="");

		// Create a new expression associated with the current context. A verbatim expression means that no special character 
		// will be interpreted (useful for example for regular expression describing operators).
		Expression* addVerbatimExpression(const std::string& expr, const std::string& name="");

		Expression* getExpression(const std::string& name);

		// Create a new macro that can be used in expressions (with the macro's name surrounded by curly brakets).
		// Macros are not associated with a specific context.
		Expression* addMacro(const std::string& expr, const std::string& name);

		Expression* getMacro(const std::string& name);

		// Close the current context and open a new one.
		void newContext(const std::string& name);

		// Generates the internal data structures of the lexer. Must be called before method generateFiles().
		void generateLexer();

		// Generates the source files of the lexer. 
		// 'language' must be a subdirectory of directory 'lexer_sk'.
		// 'outputPath' is the directory that will contain the generated sources (created if needed): either absolute,
		// or relative to the directory of the running executable.
		// Throws NanoLexerException if the skeleton files cannot be found or the output cannot be written.
		void generateFiles(const std::string& language, const std::string& outputPath);

		std::string getDebugString();

		// Name given to the constructor: the generated class is <name>Lexer.
		inline const std::string& getLexerName() const { return lexerName; }

		// Add public menbers in the generated lexer class.
		// Replace the placeholder $(PublicMembers) in the skeleton file.
		void addPublicMembers(const std::string& code);

		// Add protected menbers in the generated lexer class.
		// Replace the placeholder $(ProtectedMembers) in the skeleton file.
		void addProtectedMembers(const std::string& code);

		// Add private menbers in the generated lexer class.
		// Replace the placeholder $(PrivateMembers) in the skeleton file.
		void addPrivateMembers(const std::string& code);

		// Set code copied verbatim before the lexer class, outside its namespace (e.g. #include directives, declarations
		// of the types used by the actions). Replace the placeholder $(Prologue) in the skeleton file.
		void addPrologue(const std::string& code);

		// Set code copied verbatim at the very end of the lexer class, in a public section (fields and methods used by
		// the actions). Replace the placeholder $(Members) in the skeleton file.
		void addMembers(const std::string& code);

		inline const std::string& getPrologue() const { return prologue; }
		inline const std::string& getMembers() const { return members; }

		// Add initialization statements in the ctor of the generated lexer class.
		// Replace the placeholder $(OnCreate) in the skeleton file.
		void addOnCreate(const std::string& code);

		// Add initialization statements in the method lex() of the generated lexer class, before each lexeme is matched.
		// Replace the placeholder $(OnStartLex) in the skeleton file.
		void addOnStartLex(const std::string& code);

		// Add statements in the method handling the matching of current context expressions. These statements 
		// are executed when no match was found.
		void addOnCurrentContextFailure(const std::string& code);

		// Set the default action: statements executed when a lexeme of the main context is matched by an expression
		// without code of its own (see Expression::addOnMatchCode()). The code may return a value from lex(); otherwise
		// the lexeme is ignored. Without default action, lex() returns the matched lexeme.
		void addDefaultMainContextOnMatch(const std::string& code);

		void addErrorMessage(const std::string& msg);
		void addErrorMessages(const std::vector<std::string>& msgs);
	private:
		int								lexemeId;
		bool							throwEarly;
		std::vector<std::string>		errorMessages;
		std::vector<std::pair<std::string, std::string>> variables;
		std::string debugString;
		std::string lexerName;
		int lexemeCount;

		std::string publicMembers;
		std::string protectedMembers;
		std::string privateMembers;
		std::string prologue;
		std::string members;
		std::string onCreate;
		std::string onStartLex;
		std::string defaultOnMatch;
		bool hasDefaultOnMatch;
		std::map<std::string, std::string> contextName2contextFailure;

		std::map<std::string, std::string>														name2expr;
		std::set<std::string>																	contextNames;
		std::vector<std::tuple<std::string, std::vector<std::pair<std::string, Expression>>>>	contextsData;
		std::tuple<std::string, std::vector<std::pair<std::string, Expression>>>				currentContext;
		std::map<std::string, Expression>														macros;

		Expression* add2CurrentContext(const std::string& name, const std::string& expr);

		void closeContext();
	};

	// An exception NanoLexerException may be raised by LexerGenerator::generateLexer() for any inconsistency found in the model of the lexer.
	class NanoLexerException
	{
	public:
		NanoLexerException(const std::vector<std::string>& message);
		NanoLexerException(const std::string& message);

		inline const std::vector<std::string>& getMessages() const { return messages; }
	private:
		std::vector<std::string> messages;
	};
}
