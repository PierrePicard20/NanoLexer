%name NlArithmetic
Digit       [0-9]
NonNulDigit [1-9]
Integer     {Digit}+
Float       {Integer}\.{Digit}+

%%
[a-zA-Z]+
{Integer}|{Float}
"+"
"-"
"*"
"/"
"("
")"
[ \t]+
%%
