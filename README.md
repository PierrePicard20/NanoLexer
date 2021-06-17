
# NanoLexer
NanoLexer is a library that generates lexers in C++. On the contrary to lexer generators of the lex's familly (which are great!) it does not generate an automaton in the form of a matrix but generates an algorithm instead. Nevertheless the underlying algorithm is based on the same principles (construction of a DFA). The code generated is straightforward, small, efficient and human readable as much as possible. The primary goal is the lexing of programming languages but not limited to.

It is aldready functionnal and usable even though developments are not complete and first version still not published (and not planned).

# Requirements
* C++ 17 compliant compiler (successfuly tested only with Microsoft C++ 2019 compiler as of now)
* cmake
* flex
* bison

# Examples
Let's take a lexer specification for arithmetic expressions as an example:
```C++
    try
    {
        LexerGenerator lexGen("Arithmetic");

        lexGen.addMacro("Digit", "[0-9]");
        lexGen.addMacro("NonNulDigit", "[1-9]");
        lexGen.addMacro("Integer", "{Digit}+");
        lexGen.addMacro("Float", "{Integer}\\.{Digit}+");

        lexGen.addExpression("[a-zA-Z]+", "id");
        lexGen.addExpression("{Integer}|{Float}", "number");
        lexGen.addVerbatimExpression("+", "op_plus");
        lexGen.addVerbatimExpression("-", "op_minus");
        lexGen.addVerbatimExpression("*", "op_mult");
        lexGen.addVerbatimExpression("/", "op_div");
        lexGen.addVerbatimExpression("(", "open_parenthesis");
        lexGen.addVerbatimExpression(")", "close_parenthesis");
        lexGen.addExpression("[ \t]+", "whitespace");

        lexGen.generateLexer();
        lexGen.generateFiles("cpp", outputPath);
    }
    catch (const NanoLexerException& e)
    {
        for (const auto& msg : e.getMessages())
            std::cout << msg << std::endl;
        exit(1);
    }
```

... generates the following core automaton algorithm:
```C++
		// [...]
        bool main_context(){
            int c = get(); if (c==Traits::eof()) goto eof;
            switch (c){
            case ')': accept((Lexeme)8); return true;
            case '(': accept((Lexeme)7); return true;
            case '/': accept((Lexeme)6); return true;
            case '*': accept((Lexeme)5); return true;
            case '-': accept((Lexeme)4); return true;
            case '+': accept((Lexeme)3); return true;
            default:
                if ((c>='A'&&c<='Z')||(c>='a'&&c<='z')) goto state7;
                if ((c>='0'&&c<='9')) goto state8;
                if ((c=='\t')||(c==' ')) goto state9;
            }
            return (accepted!=Lexeme::unknown_);
        state7:
            accept((Lexeme)1);
            c = get(); if (c==Traits::eof()) goto eof;
            if ((c>='A'&&c<='Z')||(c>='a'&&c<='z')) goto state7;
            unget(); return true;
        state8:
            accept((Lexeme)2);
            c = get(); if (c==Traits::eof()) goto eof;
            switch (c){
            case '.': goto state10;
            default:
                if ((c>='0'&&c<='9')) goto state8;
            }
            unget(); return true;
        state9:
            accept((Lexeme)9);
            c = get(); if (c==Traits::eof()) goto eof;
            if ((c=='\t')||(c==' ')) goto state9;
            unget(); return true;
        state10:
            c = get(); if (c==Traits::eof()) goto eof;
            if ((c>='0'&&c<='9')) goto state11;
            return (accepted!=Lexeme::unknown_);
        state11:
            accept((Lexeme)2);
            c = get(); if (c==Traits::eof()) goto eof;
            if ((c>='0'&&c<='9')) goto state11;
            unget(); return true;
        eof:
            if (nbRead==0){
                accept(Lexeme::eof_);
                return true;
            }
            return (accepted!=Lexeme::unknown_);
        }
		// [...]
```

# Contexts
NanoLexer supports contexts of analysis that can be pushed and poped. This allows to extend the pattern's power of expression which is required to match symetric patterns that are common in programming languages, like strings and comments.

Here is another example that illustrate the use of a context to match C multiline comments:
```C++
    try
    {
        LexerGenerator lexGen("Comments");

        // by default, without having declared a new context, any new expression is added to the main context
        lexGen.addExpression("multilineComment", "/\\*")        // when the begining of a multiline C comment is matched, the context 'Comment' is pushed.
            ->setAction(LexerGenerator::ActionOnMatch::push)
            ->setPushContext("Comment");

        // after the declaration of a new context (with the method newContext() below), any new expression is added to that context
        lexGen.newContext("Comment");                           // context 'Comment' specificaly match the end of a multiline C comment
        lexGen.addExpression("comment1", "[^\\*]+");            // match any character but '*'
        lexGen.addExpression("comment2", "\\*[^\\/]");          // match '*' followed by any character but '/'
        lexGen.addExpression("comment3", "\\*\\/")              // match the closing pattern of the comment => pop the context and return to the main one
            ->setAction(LexerGenerator::ActionOnMatch::pop);

        lexGen.generateLexer();
        lexGen.generateFiles("cpp", outputPath);
    }
    catch (const NanoLexerException& e)
    {
        std::cout << e.getMessage() << std::endl;
        exit(1);
    }
```

... which generates:
```C++
		// [...]
        bool main_context(){
            int c = get(); if (c==Traits::eof()) goto eof;
            switch (c){
            case '/': goto state1;
            }
            if (accepted!=Lexeme::unknown_) return true; goto fail;
        state1:
            c = get(); if (c==Traits::eof()) goto eof;
            switch (c){
            case '*': if (Comment()){ accept((Lexeme)1); return true;}else{if (accepted!=Lexeme::unknown_) return true; goto fail;}
            }
            if (accepted!=Lexeme::unknown_) return true; goto fail;
        eof:
            if (nbRead==0){
                accept(Lexeme::eof_);
                return true;
            }
            if (accepted!=Lexeme::unknown_)
                return true;
        fail:
            return false;
        }

        bool Comment(){
            int c = get(); if (c==Traits::eof()) goto fail;
        state0_noread:
            switch (c){
            case '*': goto state1;
            default:
                goto state2;
            }
            goto fail;
        state1:
            c = get(); if (c==Traits::eof()) goto fail;
            switch (c){
            case '/': return true;
            default:
                {goto state0_noread;}
            }
            goto fail;
        state2:
            c = get(); if (c==Traits::eof()) goto fail;
            if (!((c=='*'))) goto state2;
            goto state0_noread;
        fail:
            return false;
        }
		// [...]
```

To play with it please have a look at the tests subfolder which contains a variety of use cases. All examples presented above can be found in the `testgen` subfolder.

# Patterns
Supported patterns are:
<pre>
x                           match the character ’x’
.                           any character (byte) except newline
[xyz]                       either an ’x’, a ’y’, or a ’z’
[abj-oZ]                    either an ’a’, a ’b’, any letter from ’j’through ’o’, or a ’Z’
[^A-Z]                      any character but those in the internval A-Z.
[^A-Z\n]                    any character EXCEPT an uppercase letter or a newline
x*                          zero or more x’s, where x is any regular expression
x+                          one or more x’s
x?                          zero or one x’s (that is, “an optional x”)
x{2,5}                      anywhere from two to five x’s
x{2,}                       two or more x’s
x{4}                        exactly 4 x’s
{name}                      the expansion of the macro ‘name’
\X                          if X is ‘a’, ‘b’, ‘f’, ‘n’, ‘r’, ‘t’, or ‘v’, then the ANSI-C interpretation of ‘\x’.Otherwise, a literal ‘X’ (used to escape operators such as ‘*’)
\0                          null character (ASCII code 0)
\267                        the character with octal value 267
\x3d                        the character with hexadecimal value 3d
(r)                         match the expression ‘r’
ab                          concatenation, the regular expression ‘a’ followed by the regular expression ‘b'.
a|b                         alternative, either an ‘a’ or a ‘b’.
[:alpha:]                   Alphabetic characters, union of [:lower:] and [:upper:]. In the ‘C’ locale and ASCII character encoding, this is the same as [A-Za-z].
[:blank:]                   Blank characters: space and tab.
[:digit:]                   Digits, equivalent to [0-9].
[:xdigit:]                  Hexadecimal digits, equivalent to [0-9A-Fa-f].
[:lower:]                   Lower-case letters, equivalent to [a-z] in the ‘C’ locale and ASCII character encoding.
[:upper:]                   Upper-case letters, equivalent to [A-Z] in the ‘C’ locale and ASCII character encoding.
[:print:]                   Printable characters: ‘[:alnum:]’, ‘[:punct:]’, and space.
[:punct:]                   Punctuation characters; in the ‘C’ locale and ASCII character encoding, this is ! " # $ % & ' ( ) * + , - . / : ; < = > ? @ [ \ ] ^ _ ` { | } ~.
[:cntrl:]                   Control characters. In ASCII, these characters have octal codes 000 through 037, and 177 (DEL). In other character sets, these are the equivalent characters, if any.
[:graph:]                   Graphical characters: ‘[:alnum:]’ and ‘[:punct:]’.
</pre>
# Future developments
Main areas of improvments are:
- documentation
- additional unit tests
- make it compliant with compilers gcc and clang
- source code generation
- refactoring of some old code
- additional regular expression patterns
- support of multi byte characters
