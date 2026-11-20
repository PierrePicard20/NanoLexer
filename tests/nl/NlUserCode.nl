%name NlUserCode
%{
#include <vector>

// declared before the lexer class, outside its namespace
struct NlUserCodeWord
{
    std::string text;
    int line;
};
%}
%%
[a-z]+          { words.push_back({ getMatchString(), line }); return accepted; }
\n              { line++; }
[ \t]+          {}
%%
    std::vector<NlUserCodeWord> words;
    int line = 1;
    int getLine() const { return line; }
