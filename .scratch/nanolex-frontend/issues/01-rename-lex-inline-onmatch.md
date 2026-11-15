# 01: Rename `getNextToken()` to `lex()` and inline the `onMatch()` switch

**What to build:** prefactoring of the library so that actions can later decide what the lexer returns. The generated lexer's entry point becomes `lex()` (no `getNextToken()` alias). The `onMatch()` method disappears: its switch is inlined into `lex()` at the point where it was called (after backtracking, so actions only run for final matches). Every return happens inside the switch; the trailing `return accepted;` is removed and the implicit `default:` is `return accepted;`. Behaviour is unchanged for existing lexers: every main-context lexeme is still returned, and main-context actions added with `addOnMatchCode` / `addDefaultMainContextOnMatch` still run.

See spec: `.scratch/nanolex-frontend/spec.md` (Generated lexer changes).

**Blocked by:** None (can start immediately)

**Status:** ready-for-agent

- [ ] Generated lexers expose `lex()`; `getNextToken()` no longer exists
- [ ] `onMatch()` is no longer generated; its cases live inside `lex()`, executed after backtracking
- [ ] The implicit `default:` case returns the accepted lexeme; `eof_` and `unknown_` are still returned as before
- [ ] `checkLexemes`, all generated-lexer tests and the `example/` project use `lex()`
- [ ] The full test suite passes with no change in expected token sequences
