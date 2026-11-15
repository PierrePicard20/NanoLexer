# Spec: `nanolex`, a Lex-like frontend for NanoLexer

Status: ready-for-agent

## Problem Statement

Today, the only way to produce a lexer with NanoLexer is to write a C++ "generator" program against `LexerGenerator`, build it, copy `lexer_sk/` beside it, and run it. Every rule is a C++ string literal, so regular expressions need double escaping (`"\\*\\/"`), and the structure of the lexer (contexts, push/pop, per-rule code) is buried in method calls. Users familiar with Lex expect a declarative description file and a command-line tool.

In addition, the generated lexer has no notion of a per-lexeme *action* that decides what the lexer returns: `getNextToken()` always returns the matched `Lexeme` enum value, and there is no way to ignore a lexeme (e.g. whitespace) or to return a user-defined token object.

## Solution

A new command-line tool, `nanolex`, reads a Lex-like **description file** (`.nl`) and generates the lexer by driving a `LexerGenerator` instance. The library stays usable directly.

The description file has Lex's three sections (definitions, rules, user code), plus two NanoLexer-specific features: declaring **contexts** (`%context Name`) and attaching a context-stack command to a rule (`%push Name` / `%pop`).

The generated lexer's entry point becomes `lex()` (renamed from `getNextToken()`). Main-context **actions** run inside `lex()` and may `return` a value of the lexer's return type (`%type`, defaulting to `Lexeme`); an action without `return` makes the lexeme an **ignored lexeme** and `lex()` moves on to the next one.

Example description file:

```
%name Comments
%{
#include "Token.h"
%}
Newline     \n|(\r\n)
%%
%type Token
{Newline}                  { line++; row = 1; }
[ \t]+
[0-9]+                     { return Token(NUM, getMatchString()); }
"/*"    %push Comment      { return Token(COMMENT, getMatchString()); }
%default                   { return Token(OTHER, getMatchString()); }
<<EOF>>                    { return Token(END); }
<<UNKNOWN>>                { return Token(BAD, getMatchString()); }

%context Comment
"*/"    %pop
[^*\r\n]+
"*"
\n|(\r\n)
%%
    int line = 1, row = 1;
    int getLine() const { return line; }
```

## User Stories

1. As a lexer author, I want to describe my lexer in a single `.nl` file, so that I don't have to write and build a C++ generator program.
2. As a lexer author familiar with Lex, I want the description file to have the same three sections separated by `%%`, so that the format is immediately familiar.
3. As a lexer author, I want to run `nanolex file.nl -o <dir>`, so that the lexer header is generated in the directory of my choice.
4. As a lexer author, I want `-o` to be resolved relative to my current directory, so that the tool behaves like any other command-line compiler.
5. As a lexer author, I want to choose the target language with `-l` (default `cpp`), so that future skeleton languages can be selected.
6. As a lexer author, I want to name the lexer with `%name X` in the definitions section, so that the generated class is `XLexer` in `XLexer.h`.
7. As a lexer author, I want the lexer name to default to the description file's base name, so that a minimal file needs no `%name`.
8. As a lexer author, I want to declare macros with `Name regex` lines in the definitions section, so that I can reuse sub-expressions as `{Name}` in rules, as in Lex.
9. As a lexer author, I want the content of `%{ ... %}` blocks in the definitions section copied verbatim before the lexer class, outside its namespace, so that I can include headers and declare my token type.
10. As a lexer author, I want to write regular expressions without C++ string escaping, so that rules are readable.
11. As a lexer author, I want a rule's pattern to end at the first unquoted, unescaped blank, as in Lex, so that I can follow it with a command and an action.
12. As a lexer author, I want to write literal patterns in double quotes (`"+"`, `"/*"`), so that operators need no escaping.
13. As a lexer author, I want to attach an action `{ ... }` to a rule, possibly spanning several lines, so that I can run arbitrary C++ when it matches.
14. As a lexer author, I want braces inside strings, character literals and comments of an action to be ignored when finding the end of the action, so that any valid C++ can appear in it.
15. As a lexer author, I want to declare the lexer's return type with `%type T` in the rules section, so that `lex()` returns my own token objects.
16. As a lexer author, I want the return type to default to `Lexeme` when `%type` is absent, so that a minimal lexer still reports the matched lexemes.
17. As a lexer author, I want a main-context action to `return` a value of the return type, so that the caller of `lex()` receives it.
18. As a lexer author, I want a main-context lexeme whose action has no `return` to be ignored, so that whitespace or comments can be skipped.
19. As a lexer author, I want a main-context rule without an action to run the default action, so that I can handle all such rules uniformly.
20. As a lexer author, I want to declare the default action with `%default { ... }`, so that I control what action-less rules do.
21. As a lexer author without `%type`, I want the implicit default action to return the matched lexeme, so that every lexeme is reported as before.
22. As a lexer author with `%type`, I want the implicit default action to do nothing, so that action-less rules are ignored lexemes.
23. As a lexer author, I want to handle end of input with `<<EOF>> { ... }`, so that I can return an end-of-input token of my type.
24. As a lexer author with `%type`, I want the generation to fail if `<<EOF>>` is missing, so that `lex()` never has to invent a value of my type.
25. As a lexer author without `%type`, I want end of input to return `Lexeme::eof_` when `<<EOF>>` is absent, so that the default behaviour is unchanged.
26. As a lexer author, I want to handle unknown text with `<<UNKNOWN>> { ... }`, so that I can report characters matching no rule, distinctly from lexer errors.
27. As a lexer author with `%type`, I want unknown text to be skipped when `<<UNKNOWN>>` is absent, so that the lexer keeps going.
28. As a lexer author without `%type`, I want unknown text to return `Lexeme::unknown_` when `<<UNKNOWN>>` is absent, so that the default behaviour is unchanged.
29. As a lexer author, I want end of input and unknown text never to reach the default action, so that a non-returning default action cannot make `lex()` loop forever.
30. As a lexer author, I want to open a new context with a `%context Name` line in the rules section, so that the following rules belong to that context.
31. As a lexer author, I want rules before the first `%context` to belong to the main context, so that simple lexers need no context declaration.
32. As a lexer author, I want to write `%push Name` between a pattern and its action, so that matching that rule pushes the context `Name`.
33. As a lexer author, I want to write `%pop` between a pattern and its action, so that matching that rule pops the current context.
34. As a lexer author, I want the text consumed from a push to the matching pop to be part of the lexeme that pushed, so that a multiline comment or a string is one lexeme.
35. As a lexer author, I want the action of a pushing main-context rule to run after the pushed context has popped, so that `getMatchString()` holds the whole lexeme.
36. As a lexer author, I want `<<UNKNOWN>>` inside a `%context` section to define what happens when that context fails to match, so that I can handle e.g. invalid characters in a string.
37. As a lexer author, I want an action on a non-pushing sub-context rule to be rejected with a clear error, so that I don't get a lexer that silently drops my code (until the sub-context action bug is fixed).
38. As a lexer author, I want an unknown context name in `%push`, a `%pop` in the main context, or a duplicate `%context` to be reported as an error, so that context mistakes are caught at generation time.
39. As a lexer author, I want the user code section copied at the end of the lexer class, in a public section, so that I can add fields and accessors used by my actions.
40. As a lexer author, I want every error reported as `file.nl(line): message`, so that I can jump to it from Visual Studio.
41. As a lexer author, I want regex errors reported against the line of the rule or macro that contains them, so that I can find the faulty expression.
42. As a lexer author, I want `nanolex` to exit with a non-zero status on any error, so that a build fails when the description is wrong.
43. As a build engineer, I want `nanolex` installed with the NanoLexer package together with its `lexer_sk/` directory, so that it works right after installation.
44. As a build engineer, I want to call `nanolex` from an `add_custom_command`, so that the lexer is regenerated when the description file changes.
45. As a library user, I want to keep using `LexerGenerator` directly, so that I can build lexers programmatically.
46. As a library user, I want `LexerGenerator` to expose every feature of the description file (prologue, return type, end of input, unknown text, class members), so that the frontend is not more powerful than the library.
47. As a library user, I want the generated lexer's entry point to be `lex()`, so that the library and the frontend produce the same API.
48. As a library user, I want existing lexers without return type nor actions to behave as before (each main-context lexeme returned), so that only the method name changes for me.

## Implementation Decisions

**Modules**

- **Library (`LexerGenerator` and the C++ skeleton)**: modified, with additive API changes plus one rename in the generated code.
- **Description-file reader**: new module, a hand-written line-oriented parser (not Flex/Bison: a second Flex/Bison pair would clash with the existing `yy*` globals of the regex parser, and regexes are passed through to `addExpression` unparsed). It turns a `.nl` file into calls on a `LexerGenerator`, keeping a map from each expression/macro to its source line for error reporting. It should be a deep module with a small interface, roughly "description text + file name → configured `LexerGenerator` or list of located errors", usable by tests without the CLI.
- **`nanolex` CLI**: new executable in its own top-level directory; argument handling (`file.nl`, `-o <dir>`, `-l <language>`, default `cpp`), resolution of `-o` to an absolute path from the current directory before calling `generateFiles`, error printing, exit status.

**Description-file to library mapping**

| Description file | `LexerGenerator` |
|---|---|
| `%name X` (definitions) or file base name | constructor argument |
| `Name regex` (definitions) | `addMacro` |
| `%{ ... %}` content (definitions), copied as one block | new `addPrologue` |
| `%type T` (rules) | new `setReturnType` |
| pattern (any context) | `addExpression` (quoted literals rely on the regex syntax's own `"..."`, so `addVerbatimExpression` is not used) |
| `%push C` / `%pop` | `setPushContext` / `setPopAction` |
| `{ action }` | `addOnMatchCode` |
| `%context C` | `newContext` |
| `%default { ... }` | `addDefaultMainContextOnMatch` |
| `<<EOF>> { ... }` (main context only) | new `addOnEof` |
| `<<UNKNOWN>> { ... }` in the main context | new `addOnUnknown` |
| `<<UNKNOWN>> { ... }` in a `%context` section | `addOnCurrentContextFailure` |
| user code section | new `addMembers` |

Rule names are not written in the description file; the reader generates internal expression names (the library requires one). A trailing `%%` with an empty user code section is allowed.

**Description-file lexical rules (as in Lex)**

- A pattern starts at column 0 and ends at the first blank that is not quoted or escaped.
- An action is `{ ... }`, may span lines; brace counting ignores braces inside C++ string literals, character literals and comments.
- `%`-directives start at column 0.
- Lex's `|` action ("same action as next rule") is not supported.

**Generated lexer changes (skeleton + code generation)**

- `getNextToken()` is renamed `lex()`. No alias is kept. No ADR: the library has no users yet, so the change is cheap to reverse.
- The `onMatch()` method is removed; its switch is inlined into `lex()` at the point where it was called, after backtracking, so actions only run for final matches.
- The trailing `return accepted;` is removed: every return happens inside the switch. `lex()` loops to the next lexeme when the selected case does not return.
- The return type of `lex()` is the `setReturnType` type, defaulting to `Lexeme`.
- Switch cases:
  - `case eof_`: `addOnEof` code; else `return Lexeme::eof_;` without return type; missing `addOnEof` with a return type is a generation error.
  - `case unknown_`: `addOnUnknown` code; else `return Lexeme::unknown_;` without return type; skip the unknown text and continue with a return type.
  - one case per main-context expression that has an action;
  - `default:` the default action; implicit default is `return accepted;` with type `Lexeme`, nothing with a user return type.
- Two new skeleton placeholders, empty when unused:
  - `$(Prologue)`, after the standard includes and before the namespace;
  - `$(Members)`, at the very end of the class after a `public:` label.
- Sub-context actions keep today's behaviour (only emitted for pushing rules). The reader rejects an action on a non-pushing sub-context rule with an error referencing the sub-context actions issue.

**Errors**

- All errors (reader, regex parser, `generateLexer`, `generateFiles`) are printed as `file.nl(line): message` (MSVC format); errors with no attributable line use `file.nl: message`. Exit status is non-zero on any error.

**Build and install**

- `nanolex` gets the same POST_BUILD copy of `lexer_sk/` beside it as other generating executables; install places `lexer_sk/` next to `nanolex` in the install `bin/` directory.

**Impacted existing code**

- `checkLexemes.h`, the generated-lexer test cases and the `example/` project switch from `getNextToken()` to `lex()`.

## Testing Decisions

A good test exercises external behaviour only: given a description file, the lexer configuration it produces or the tokens the generated lexer returns. It never asserts on the reader's internal data structures or on the exact text of generated code.

Seams (agreed during design):

1. **Reader → `LexerGenerator`**: feed `.nl` text to the reader in-process and compare `getDebugString()` with that of an equivalent `LexerGenerator` built by hand. Also assert located error messages (`file.nl(line): ...`) for malformed input: unknown context, `%pop` in main, duplicate context, missing `<<EOF>>` with `%type`, action on a non-pushing sub-context rule, unterminated action, bad regex. Prior art: `LexerFixture` / `LexerTestCase.cpp`.
2. **End to end (`nanolex` → generated header → `lex()`)**: rewrite `cMultilineCommentLexer` and `arithmeticLexer` as `.nl` files, generate them with `nanolex` through an `add_custom_command(OUTPUT ...)` in `tests/CMakeLists.txt` modelled on the `testgen` rule, and assert token sequences with `checkLexemes`, comparing with the existing `testgen` versions. Add cases for `%type` with returning actions, ignored lexemes, `%default`, `<<EOF>>` and `<<UNKNOWN>>`. Prior art: `CommentLexerTestCase`, `ArithmeticLexerTestCase`, `checkLexemes.h`.

The existing DFA-structure tests and generated-lexer tests must keep passing after the `lex()` rename, with only the call site changed; they cover the "unchanged behaviour without return type" stories.

## Out of Scope

- Fixing the dropped sub-context actions and the deferred-lambda design: `.scratch/subcontext-actions/issues/01-subcontext-actions-dropped.md` and `02-deferred-action-match-snapshot.md`.
- Lex's `|` action, `BEGIN`, start conditions (`%x`, `%s`, `<SC>` prefixes), `yytext`/`yyleng`/`ECHO` aliases, `REJECT`, `yymore`, trailing context (`r/s`).
- `<<EOF>>` in sub-contexts.
- Non-C++ skeletons; GCC/Clang support of the tool beyond what the library already offers.
- Keeping a `getNextToken()` alias.

## Further Notes

- Confirmed by the user: `<<UNKNOWN>>` maps to `addOnUnknown` in the main context and to `addOnCurrentContextFailure` in a `%context` section; `<<EOF>>` is main-context only in v1. The current behaviour of the library when input ends inside a sub-context (e.g. unterminated comment) has not been checked; if it is problematic, open an issue.
- Vocabulary follows `GLOSSARY.md`: description file, context, main context, lexeme, action, default action, ignored lexeme, unknown text, macro.
- Suggested ordering of work: library changes (rename, inlined switch, return type, new hooks and placeholders) first, then the reader, then the CLI and CMake/install, then end-to-end tests.
