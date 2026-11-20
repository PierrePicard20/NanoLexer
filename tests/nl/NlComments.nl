%{
#include <string>
#include <vector>
%}
%%
\n|(\r\n)                   { line++; row = 1; return accepted; }
/\*         %push Comment   { comments.push_back(getMatchString()); row += getMatchLength(); return accepted; }
%default                    { row += getMatchLength(); return accepted; }

%context Comment
\n|(\r\n)
[^\*\r\n]+
\*[^\/\r\n]
\*\/        %pop
<<UNKNOWN>>                 { failures++; }
%%
    int line = 1, row = 1;
    int failures = 0;
    std::vector<std::string> comments;  // whole comments, as seen by the action of the pushing rule
