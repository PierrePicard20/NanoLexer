# 05: `%type`, `<<EOF>>` and `<<UNKNOWN>>` in the main context

**What to build:** `%type T` in the rules section sets the return type of `lex()` (new `setReturnType`; default `Lexeme`). With a user return type, the implicit default action does nothing, so action-less rules are ignored lexemes. End of input and unknown text get their own cases in `lex()` and never reach the default action:

- `<<EOF>> { ... }` (new `addOnEof`): required when `%type` is set (generation error otherwise); without `%type`, its absence returns `Lexeme::eof_`. Only allowed in the main context.
- `<<UNKNOWN>> { ... }` in the main context (new `addOnUnknown`): without `%type`, its absence returns `Lexeme::unknown_`; with `%type`, its absence skips the unknown text and continues.

Unknown text is not an error: the naming deliberately avoids "error".

See spec: `.scratch/nanolex-frontend/spec.md`.

**Blocked by:** 03 (Main-context actions, `%default` and ignored lexemes), 04 (Prologue and user code section)

**Status:** ready-for-agent

- [ ] Library users without return type see unchanged behaviour for end of input and unknown text
- [ ] Missing `<<EOF>>` with `%type` is a located generation error; `<<EOF>>` in a `%context` section is a located error
- [ ] A non-returning `%default` does not make `lex()` loop at end of input
- [ ] End to end: a lexer returning a `Token` type declared in the prologue returns the expected tokens, its `<<EOF>>` token, and its `<<UNKNOWN>>` token for unmatched input
- [ ] End to end: with `%type` and no `<<UNKNOWN>>`, unmatched input is skipped
