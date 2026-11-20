%name NlActions
%%
[ \t\r\n]+      { /* blanks are ignored lexemes } */ }
[a-z]+
[0-9]+          {
                    // zeros are ignored: a '}' in a comment, "}" in a string or '}' in a char does not end the action
                    const char* closing = "}";
                    if (getMatchString() != "0" && closing[0] == '}')
                        return accepted;
                }
