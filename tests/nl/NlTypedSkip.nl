%name NlTypedSkip
%%
%type int
[a-z]+          { return (int)getMatchString().size(); }
<<EOF>>         { return -1; }
