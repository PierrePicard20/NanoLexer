# 01: Rename `getNextToken()` to `lex()` and inline the `onMatch()` switch

**What to build:** prefactoring of the library so that actions can later decide what the lexer returns. The generated lexer's entry point becomes `lex()` (no `getNextToken()` alias). The `onMatch()` method disappears: its switch is inlined into `lex()` at the point where it was called (after backtracking, so actions only run for final matches). Every return happens inside the switch; the trailing `return accepted;` is removed and the implicit `default:` is `return accepted;`. Behaviour is unchanged for existing lexers: every main-context lexeme is still returned, and main-context actions added with `addOnMatchCode` / `addDefaultMainContextOnMatch` still run.

See spec: `.scratch/nanolex-frontend/spec.md` (Generated lexer changes).

**Blocked by:** None (can start immediately)

**Status:** resolved

- [x] Generated lexers expose `lex()`; `getNextToken()` no longer exists
- [x] `onMatch()` is no longer generated; its cases live inside `lex()`, executed after backtracking
- [x] The implicit `default:` case returns the accepted lexeme; `eof_` and `unknown_` are still returned as before
- [x] `checkLexemes`, all generated-lexer tests and the `example/` project use `lex()`
- [x] The full test suite passes with no change in expected token sequences

## Comments

Implemented on the `nanolex-frontend` integration branch.

- To keep behaviour unchanged, the generator still appends `return accepted;` to every main-context action case and to an explicit default action (`addDefaultMainContextOnMatch`). Ticket 03 must remove that suffix to get ignored lexemes; the `lex()` loop already moves on when a case does not return. The `testgen` lexers whose actions don't `return` will then need `return accepted;` added to keep their expected token sequences.
- `eof_` and `unknown_` now have their own cases in the switch, so the default action no longer runs at end of input (it used to, via `onMatch()`'s `default:`). No token sequence changes.
- Unknown text now goes through the switch: `accepted` is set to `unknown_` before it, so `getAcceptedLexeme()` matches what `lex()` returns.
- `$(OnStartNextToken)` now runs at the start of each loop iteration, i.e. before each lexeme matched, ignored ones included.
