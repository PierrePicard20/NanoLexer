# 03: Main-context actions, `%default` and ignored lexemes

**What to build:** rules may carry an action `{ ... }`, possibly spanning several lines; the end of the action is found by brace counting that ignores braces in C++ string literals, character literals and comments. A main-context action may `return` a value; an action without `return` makes the lexeme an ignored lexeme and `lex()` moves on. `%default { ... }` in the rules section sets the default action for main-context rules without an action (mapped to `addDefaultMainContextOnMatch`); without it, the implicit default action returns the matched lexeme (return type `Lexeme`).

See spec: `.scratch/nanolex-frontend/spec.md` (Description-file lexical rules, Generated lexer changes).

**Blocked by:** 02 (Minimal `nanolex` end to end)

**Status:** ready-for-agent

- [ ] Single-line and multi-line actions are parsed; braces inside strings, chars and comments do not end an action
- [ ] An unterminated action is reported as a located error
- [ ] End to end: a `.nl` lexer whose whitespace rule has a non-returning action never reports whitespace lexemes (`checkLexemes`)
- [ ] End to end: an action-less rule runs `%default`; with no `%default`, it returns its lexeme

## Comments

From ticket 01: the generator currently appends `return accepted;` to main-context action cases and to an explicit default action (in `LexerGenerator::generateLexer`, `NanoLexer/NanoLexer.cpp`). Remove it here, and add `return accepted;` to the non-returning actions in `testgen/cMultilineCommentLexer.cpp` so the existing tests keep their token sequences.
