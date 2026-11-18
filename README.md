
# NanoLexer

NanoLexer is a library that generates lexers in C++. The code generated is straightforward, small, efficient and human readable as much as possible. The primary goal is the lexing of programming languages but not limited to.

It is already functional and usable even though developments are not complete and the first version is still not published (and not planned).

## How it works

NanoLexer follows a two-step workflow:

```
1. Define tokens with a LexerGenerator instance
          ↓
   generateFiles()
          ↓
2. Generated header (e.g. ArithmeticLexer.h)
          ↓
   Instantiate & use in your application
```

Step 1 is a small standalone program (or build step) that you write once. Step 2 is a header-only template class that you include directly — no additional library needed at that point.

To get started, have a look at the `testgen` subfolder which contains a variety of use cases, and at the `example/` directory for a minimal standalone project using NanoLexer as an external CMake dependency.

---

## Requirements

- C++17 compliant compiler (successfully tested with MSVC 2019; GCC/Clang support planned)
- CMake ≥ 3.8
- Flex
- Bison
- GoogleTest — only required when building tests (`PACKAGE_TESTS=ON`)

When an environment variable VCPKG_ROOT is defined and refers to the path of vcpkg, if the variable USE_VCPKG is true, GoogleTest is searched as a package installed with vcpkg.

On Windows you may use gnuwin32 [bison](https://gnuwin32.sourceforge.net/packages/bison.htm) and [flex](https://gnuwin32.sourceforge.net/packages/flex.htm) packages.

---

## Build & Install

```bash
cmake -B build -DCMAKE_INSTALL_PREFIX=/path/to/install
cmake --build build --config release
cmake --install build
```

To also build the tests:

```bash
cmake -B build -DUSE_VCPKG=ON -DPACKAGE_TESTS=ON -DCMAKE_INSTALL_PREFIX=/path/to/install
cmake --build build
ctest --test-dir build
```

### Integrating into your CMake project

After installation, use `find_package` in your `CMakeLists.txt`:

```cmake
find_package(NanoLexer REQUIRED)
target_link_libraries(my_target PRIVATE NanoLexer::NanoLexer)
```

---

## Examples

### Arithmetic expression lexer

```cpp
try
{
    LexerGenerator lexGen("Arithmetic");

    lexGen.addMacro("[0-9]",                "Digit");
    lexGen.addMacro("{Digit}+",             "Integer");
    lexGen.addMacro("{Integer}\\.{Digit}+", "Float");

    lexGen.addExpression("[a-zA-Z]+",    "id");
    lexGen.addExpression("{Integer}|{Float}", "number");
    lexGen.addVerbatimExpression("+", "op_plus");
    lexGen.addVerbatimExpression("-", "op_minus");
    lexGen.addVerbatimExpression("*", "op_mult");
    lexGen.addVerbatimExpression("/", "op_div");
    lexGen.addVerbatimExpression("(", "open_parenthesis");
    lexGen.addVerbatimExpression(")", "close_parenthesis");
    lexGen.addExpression("[ \t]+", "whitespace");

    lexGen.generateLexer();
    lexGen.generateFiles("cpp", outputPath);  // outputPath: destination directory
}
catch (const NanoLexerException& e)
{
    for (const auto& msg : e.getMessages())
        std::cout << msg << std::endl;
    exit(1);
}
```

... generates the following core automaton algorithm:

```cpp
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

### Using the generated lexer

Once `ArithmeticLexer.h` is generated, include it directly — no library linkage required:

```cpp
#include "ArithmeticLexer.h"
#include <sstream>
#include <iostream>

using ArithmeticLexeme = NanoLexer::ArithmeticLexer<std::istringstream>::Lexeme;

int main()
{
    std::istringstream input("3.14 + 2 * (x - 1)");
    NanoLexer::ArithmeticLexer<std::istringstream> lexer(input);

    ArithmeticLexeme tok;
    while ((tok = lexer.lex()) != ArithmeticLexeme::eof_)
    {
        if (tok != ArithmeticLexeme::whitespace_)
            std::cout << lexer.getLexemeName(tok) << " : \""
                      << lexer.getMatchString() << "\"\n";
    }
}
```

---

## Contexts

NanoLexer supports contexts that can be pushed and popped. This extends the expressive power of patterns, which is required to match symmetric constructs common in programming languages, such as strings and multiline comments.

Here is an example that illustrates the use of a context to match C multiline comments:

```cpp
try
{
    LexerGenerator lexGen("Comments");

    // Without a newContext() call, expressions are added to the main context.
    lexGen.addExpression("/\\*", "multilineComment")  // match the opening '/*'
        ->setPushContext("Comment");                  // push the 'Comment' context

    // After newContext(), expressions are added to that context.
    lexGen.newContext("Comment");
    lexGen.addExpression("[^\\*]+",  "comment1");     // any character but '*'
    lexGen.addExpression("\\*[^\\/]","comment2");     // '*' not followed by '/'
    lexGen.addExpression("\\*\\/",  "comment3")       // closing '*/' => pop context
        ->setPopAction();

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

... which generates:

```cpp
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

---

## Supported patterns

| Pattern         | Description |
|-----------------|-------------|
| `x`             | Match the character `x` |
| `.`             | Any character (byte) except newline |
| `[xyz]`         | Either `x`, `y`, or `z` |
| `[abj-oZ]`      | Either `a`, `b`, any letter from `j` through `o`, or `Z` |
| `[^A-Z]`        | Any character except those in the interval `A-Z` |
| `[^A-Z\n]`      | Any character except an uppercase letter or a newline |
| `x*`            | Zero or more `x`'s |
| `x+`            | One or more `x`'s |
| `x?`            | Zero or one `x` (optional) |
| `x{2,5}`        | Between two and five `x`'s |
| `x{2,}`         | Two or more `x`'s |
| `x{4}`          | Exactly four `x`'s |
| `{name}`        | Expansion of the macro `name` |
| `\X`            | If `X` is `a b f n r t v`: ANSI-C escape. Otherwise: literal `X` (used to escape operators such as `*`) |
| `\0`            | Null character (ASCII 0) |
| `\267`          | Character with octal value 267 |
| `\x3d`          | Character with hexadecimal value `3d` |
| `(r)`           | Match the expression `r` |
| `ab`            | Concatenation: `a` followed by `b` |
| `a\|b`          | Alternative: either `a` or `b` |
| `[:alpha:]`     | Alphabetic characters (`[A-Za-z]` in C/ASCII locale) |
| `[:blank:]`     | Blank characters: space and tab |
| `[:digit:]`     | Digits, equivalent to `[0-9]` |
| `[:xdigit:]`    | Hexadecimal digits, equivalent to `[0-9A-Fa-f]` |
| `[:lower:]`     | Lower-case letters, equivalent to `[a-z]` |
| `[:upper:]`     | Upper-case letters, equivalent to `[A-Z]` |
| `[:print:]`     | Printable characters: `[:alnum:]`, `[:punct:]`, and space |
| `[:punct:]`     | Punctuation characters |
| `[:cntrl:]`     | Control characters (octal 000–037 and 177 in ASCII) |
| `[:graph:]`     | Graphical characters: `[:alnum:]` and `[:punct:]` |

---

## Future developments

- Additional unit tests and bug fixes
- Documentation
- GCC and Clang support
- Source code generation improvements
- Refactoring of legacy code
- Additional regular expression patterns
- Multi-byte character support

---

## License

*To be defined.*
