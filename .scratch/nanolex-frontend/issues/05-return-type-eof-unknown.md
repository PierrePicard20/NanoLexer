# 05: `%type`, `<<EOF>>` and `<<UNKNOWN>>` in the main context

**What to build:** `%type T` in the rules section sets the return type of `lex()` (new `setReturnType`; default `Lexeme`). With a user return type, the implicit default action does nothing, so action-less rules are ignored lexemes. End of input and unknown text get their own cases in `lex()` and never reach the default action:

- `<<EOF>> { ... }` (new `addOnEof`): required when `%type` is set (generation error otherwise); without `%type`, its absence returns `Lexeme::eof_`. Only allowed in the main context.
- `<<UNKNOWN>> { ... }` in the main context (new `addOnUnknown`): without `%type`, its absence returns `Lexeme::unknown_`; with `%type`, its absence skips the unknown text and continues.

Unknown text is not an error: the naming deliberately avoids "error".

See spec: `.scratch/nanolex-frontend/spec.md`.

**Blocked by:** 03 (Main-context actions, `%default` and ignored lexemes), 04 (Prologue and user code section)

**Status:** resolved

- [x] Library users without return type see unchanged behaviour for end of input and unknown text
- [x] Missing `<<EOF>>` with `%type` is a located generation error; `<<EOF>>` in a `%context` section is a located error
- [x] A non-returning `%default` does not make `lex()` loop at end of input
- [x] End to end: a lexer returning a `Token` type declared in the prologue returns the expected tokens, its `<<EOF>>` token, and its `<<UNKNOWN>>` token for unmatched input
- [x] End to end: with `%type` and no `<<UNKNOWN>>`, unmatched input is skipped

## Comments

Resolution:

- Library: `setReturnType`, `addOnEof` and `addOnUnknown` (with `getReturnType()`, `hasOnEof()` / `getOnEof()`, `hasOnUnknown()` / `getOnUnknown()`). Skeleton placeholder `$(ReturnType)` for the return type of `lex()` (`Lexeme` by default, substituted with the user code). The `eof_` and `unknown_` cases of the `lex()` switch are now generated (`$(LexCases)`) instead of being written in the skeleton: without return type nor action they still return `Lexeme::eof_` / `Lexeme::unknown_`. With a return type, a missing `addOnEof` is a generation error, a missing `addOnUnknown` skips the unknown text, and the implicit default action ignores the lexeme. `addOnEof` / `addOnUnknown` called outside the main context are errors (for `addOnUnknown`, the message points to `addOnCurrentContextFailure`).
- Reader: `%type T` in the rules section (the rest of the line, so `std::pair<int, int>` works), `<<EOF>> { ... }` and `<<UNKNOWN>> { ... }` rules, each declared at most once; `%default` shares the same parsing. `%type` without `<<EOF>>` is reported at the `%type` line. The library errors of `addOnEof` / `addOnUnknown` are attributed to the `<<EOF>>` / `<<UNKNOWN>>` line: since `%context` does not exist yet in the reader, the located error for `<<EOF>>` in a `%context` section is checked on the library side only, and will be reached by the reader in ticket 06 provided it registers `<<EOF>>` while the context is current.
- An `<<EOF>>` action that does not return makes `lex()` loop at the end of the input, as documented on `addOnEof`.
- Tests: library errors in `ErrorMngtTestCase.cpp`; reader tests read the return type and actions back and check the located errors; end to end, `tests/nl/NlTyped.nl` returns a token type declared in `%{ %}` with `<<EOF>>`, `<<UNKNOWN>>` and a non-returning `%default`, and `tests/nl/NlTypedSkip.nl` checks that unknown text is skipped without `<<UNKNOWN>>`.
