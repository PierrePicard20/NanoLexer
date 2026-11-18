# NanoLexer — Minimal Example

This project demonstrates how to use [NanoLexer](https://github.com/your-org/NanoLexer) as an external CMake dependency.

## What this example does

`main.cpp` defines an arithmetic expression lexer using the `NanoLexer::LexerGenerator` API and generates its C++ source into a `generated/` directory. The generated header (`ArithmeticLexer.h`) can then be included in any project to tokenize arithmetic expressions.

This reflects NanoLexer's two-step workflow:

```
[define tokens with LexerGenerator]  →  generateFiles()  →  ArithmeticLexer.h
         (this example)                                      (use in your app)
```

The tokens defined in this example are:

| Token            | Pattern                  |
|------------------|--------------------------|
| `number`         | integer or float literal |
| `identifier`     | `[a-zA-Z_][a-zA-Z0-9_]*`|
| `op_plus`        | `+`                      |
| `op_minus`       | `-`                      |
| `op_mult`        | `*`                      |
| `op_div`         | `/`                      |
| `open_paren`     | `(`                      |
| `close_paren`    | `)`                      |
| `whitespace`     | spaces, tabs, newlines   |

## Prerequisites

- CMake ≥ 3.14
- A C++17 compiler
- NanoLexer installed (see below)
- Flex and Bison (required by NanoLexer at build time)

## Installing NanoLexer

```bash
git clone https://github.com/your-org/NanoLexer.git
cd NanoLexer
cmake -B build -DCMAKE_INSTALL_PREFIX=/path/to/install
cmake --build build
cmake --install build
```

## Building this example

```bash
cmake -B build -DCMAKE_PREFIX_PATH=/path/to/install
cmake --build build
```

## Running

```bash
cd build
./generate_lexer        # Linux / macOS
generate_lexer.exe      # Windows
```

The generated lexer is written to `generated/ArithmeticLexer.h` relative to
the working directory.

## Using the generated lexer

Once generated, include the header directly in your project — no additional
library is needed:

```cpp
#include "generated/ArithmeticLexer.h"
#include <sstream>
#include <iostream>

int main()
{
    std::istringstream input("3.14 + 2 * (x - 1)");
    NanoLexer::ArithmeticLexer<std::istringstream> lexer(input);

    using Lexeme = NanoLexer::ArithmeticLexer<std::istringstream>::Lexeme;
    Lexeme tok;
    while ((tok = lexer.lex()) != Lexeme::eof_)
    {
        if (tok != Lexeme::whitespace_)
            std::cout << lexer.getLexemeName(tok) << " : \""
                      << lexer.getMatchString() << "\"\n";
    }
}
```

Expected output for `3.14 + 2 * (x - 1)`:

```
number      : "3.14"
op_plus     : "+"
number      : "2"
op_mult     : "*"
open_paren  : "("
identifier  : "x"
op_minus    : "-"
number      : "1"
close_paren : ")"
```
