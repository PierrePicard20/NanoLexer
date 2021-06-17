%{
#include "tokens.h"
#include "shared.h"

/* Work around an incompatibility in flex (at least versions
2.5.31 through 2.5.33): it generates code that does
not conform to C89. See Debian bug 333231
<http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=333231>. */
# undef yywrap
# define yywrap() 1

#define isatty _isatty
#define fileno _fileno

#define register

int rowPosition;
int prevRowPosition;
bool verbatim;
%}

SpecialCharacter    \.|\(|\)|\[|\]|\-|\+|\*|\{|\}|\\|\?|\^|\||,

%%

[0-9]				{prevRowPosition = rowPosition;rowPosition++; yylval.character = yytext[0]; return _INT_DIGIT_;}
[a-fA-F]			{prevRowPosition = rowPosition;rowPosition++; yylval.character = yytext[0]; return _HEXA_LETTER_;}
\\n					{prevRowPosition = rowPosition;rowPosition++; yylval.character = '\n'; return _NEW_LINE_;}
\\t					{prevRowPosition = rowPosition;rowPosition++; yylval.character = '\t'; return _TAB_;}
\\v					{prevRowPosition = rowPosition;rowPosition++; yylval.character = '\v'; return _VTAB_;}
\\b					{prevRowPosition = rowPosition;rowPosition++; yylval.character = '\b'; return _BSPACE_;}
\\r					{prevRowPosition = rowPosition;rowPosition++; yylval.character = '\r'; return _CRET_;}
\\f					{prevRowPosition = rowPosition;rowPosition++; yylval.character = '\f'; return _FFEED_;}
\\a					{prevRowPosition = rowPosition;rowPosition++; yylval.character = '\a'; return _BEEP_;}
\[:alnum:\]			{prevRowPosition = rowPosition;rowPosition++; return _ALNUM_;}
\[:lower:\]			{prevRowPosition = rowPosition;rowPosition++; return _LOWER_;}
\[:upper:\]			{prevRowPosition = rowPosition;rowPosition++; return _UPPER_;}
(\[:digit:\])|(\\d)	{prevRowPosition = rowPosition;rowPosition++; return _DIGIT_;}
\[:xdigit:\]		{prevRowPosition = rowPosition;rowPosition++; return _XDIGIT_;}
(\[:space:\])|(\\s)	{prevRowPosition = rowPosition;rowPosition++; return _SPACE_;}
\[:blank:\]			{prevRowPosition = rowPosition;rowPosition++; return _BLANK_;}
\[:cntrl:\]			{prevRowPosition = rowPosition;rowPosition++; return _CNTRL_;}
\[:print:\]			{prevRowPosition = rowPosition;rowPosition++; return _PRINT_;}
\[:graph:\]			{prevRowPosition = rowPosition;rowPosition++; return _GRAPH_;}
\[:punct:\]			{prevRowPosition = rowPosition;rowPosition++; return _PUNCT_;}
\\x|X				{prevRowPosition = rowPosition;rowPosition++; return _PREFIX_;}
\\S					{prevRowPosition = rowPosition;rowPosition++; return _NOT_SPACE_;}
\\D					{prevRowPosition = rowPosition;rowPosition++; return _NOT_DIGIT_;}
\\w					{prevRowPosition = rowPosition;rowPosition++; return _WORD_;}
\\W					{prevRowPosition = rowPosition;rowPosition++; return _NOT_WORD_;}
{SpecialCharacter}		        {prevRowPosition = rowPosition;rowPosition++; yylval.character = yytext[0]; return verbatim?_CHAR_:yytext[0];}
\"								{rowPosition++; verbatim = !verbatim;}
[^\\]|(\\.)						{prevRowPosition = rowPosition;rowPosition++; yylval.character = (yytext[0]=='\\')?yytext[1]:yytext[0]; return _CHAR_;}
\{[a-zA-Z][a-zA-Z_\-]*\}		{prevRowPosition = rowPosition;rowPosition+=yyleng; macroName = yytext+1; macroName.pop_back(); return _MACRO_;}

%%

void initLexer()
{
	verbatim = false;
	prevRowPosition = 1;
	rowPosition = 1;
}

int getPreviousLexerPosition()
{
	return prevRowPosition;
}
