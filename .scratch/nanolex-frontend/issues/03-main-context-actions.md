# 03: Main-context actions, `%default` and ignored lexemes

**What to build:** rules may carry an action `{ ... }`, possibly spanning several lines; the end of the action is found by brace counting that ignores braces in C++ string literals, character literals and comments. A main-context action may `return` a value; an action without `return` makes the lexeme an ignored lexeme and `lex()` moves on. `%default { ... }` in the rules section sets the default action for main-context rules without an action (mapped to `addDefaultMainContextOnMatch`); without it, the implicit default action returns the matched lexeme (return type `Lexeme`).

See spec: `.scratch/nanolex-frontend/spec.md` (Description-file lexical rules, Generated lexer changes).

**Blocked by:** 02 (Minimal `nanolex` end to end)

**Status:** resolved

- [x] Single-line and multi-line actions are parsed; braces inside strings, chars and comments do not end an action
- [x] An unterminated action is reported as a located error
- [x] End to end: a `.nl` lexer whose whitespace rule has a non-returning action never reports whitespace lexemes (`checkLexemes`)
- [x] End to end: an action-less rule runs `%default`; with no `%default`, it returns its lexeme

## Comments

From ticket 01: the generator currently appends `return accepted;` to main-context action cases and to an explicit default action (in `LexerGenerator::generateLexer`, `NanoLexer/NanoLexer.cpp`). Remove it here, and add `return accepted;` to the non-returning actions in `testgen/cMultilineCommentLexer.cpp` so the existing tests keep their token sequences.

Resolution:

- Library: main-context action cases and an explicit default action no longer get an appended `return accepted;`; each is followed by `break;`, so an action without `return` makes an ignored lexeme. Without `addDefaultMainContextOnMatch`, the default case is `return accepted;`. `addOnMatchCode("")` and `addDefaultMainContextOnMatch("")` now count as (empty) actions: a `{}` action ignores its lexeme instead of falling back to the default action. `Expression` gains `hasOnMatchCode()` and `getOnMatchCode()`. `testgen/cMultilineCommentLexer.cpp` actions end with `return accepted;`.
- Reader: an action starts with the first non-blank `{` after the pattern (or after `%default`) and its code is the text between the outer braces. Brace counting skips `//` and `/* */` comments, string and character literals (with escapes), raw strings (`R"d(...)d"` and their `u8`/`u`/`U`/`L` prefixes) and digit separators (`1'000`). An ordinary string or character literal ends at the end of its line, so a stray quote does not swallow the rest of the file. Errors: unterminated action (at the line of its `{`), text after the action, `%default` without action, duplicate `%default`; `%default` in the definitions section is an unknown directive.
- Tests: reader tests read actions back through `getOnMatchCode()`; `tests/nl/NlActions.nl` (ignored blanks, conditionally returning action with braces in literals and comments) and `tests/nl/NlDefault.nl` (`%default`, `{}`) are checked end to end.
- Known limitation: an unterminated action consumes the rest of the file, `%%` included, as in Lex.
