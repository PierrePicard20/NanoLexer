# 04: Prologue and user code section

**What to build:** the content of `%{ ... %}` blocks in the definitions section is copied as one block before the lexer class, outside its namespace; the user code section (after the second `%%`) is copied at the very end of the lexer class, in a public section. Library side: new `addPrologue` and `addMembers` on `LexerGenerator`, filling new skeleton placeholders `$(Prologue)` (after the standard includes, before the namespace) and `$(Members)` (end of class, after `public:`), both empty when unused.

See spec: `.scratch/nanolex-frontend/spec.md` (mapping table, Generated lexer changes).

**Blocked by:** 02 (Minimal `nanolex` end to end)

**Status:** resolved

- [x] Lexers generated without prologue or members behave as before (existing tests pass)
- [x] End to end: a type declared in `%{ %}` is visible to code in the user code section
- [x] End to end: an accessor and a field declared in the user code section are usable on the generated lexer, and the field can be updated by an action

## Comments

Resolution:

- Library: `addPrologue` and `addMembers` (with `getPrologue()` / `getMembers()`), like the other code setters they replace the previous value. Skeleton placeholders `$(Prologue)` (after `#include <string>`, before the namespace) and `$(Members)` (after a `public:` label at the very end of the class). Both variables are substituted last, so user code is never searched for the other placeholders; the substitution loop now resumes after the inserted text, so a value containing its own placeholder no longer loops forever.
- Reader: a `%{` line (column 0, possibly followed by blanks) in the definitions section opens a block closed by a `%}` line; the lines between are copied verbatim, all blocks joined into one prologue. An unterminated block is a located error. `%{` in the rules section is still an unknown directive (Lex's code local to `yylex` is not supported). Everything after the second `%%` is copied verbatim as members, a later `%%` included; a blank user code section adds nothing.
- Tests: reader tests read the prologue and members back; `tests/nl/NlUserCode.nl` declares a type in `%{ %}` used by a field of the user code section, updated by actions, checked end to end.
