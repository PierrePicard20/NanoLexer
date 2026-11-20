%name NlDefault
%%
[a-z]+
[0-9]+          { return accepted; }
" "             {}
%default        { if (getMatchString() != "skip") return accepted; }
