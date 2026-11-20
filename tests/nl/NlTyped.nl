%name NlTyped
%{
enum class NlTypedKind { word, number, end, unknown };

struct NlTypedToken
{
    NlTypedKind kind;
    std::string text;
};
%}
%%
%type NlTypedToken
[a-z]+          { return { NlTypedKind::word, getMatchString() }; }
[0-9]+          { return { NlTypedKind::number, getMatchString() }; }
[ \t\n]+
%default        { blanks++; }
<<EOF>>         { return { NlTypedKind::end, "" }; }
<<UNKNOWN>>     { return { NlTypedKind::unknown, getMatchString() }; }
%%
    int blanks = 0;
