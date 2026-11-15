# 04: Prologue and user code section

**What to build:** the content of `%{ ... %}` blocks in the definitions section is copied as one block before the lexer class, outside its namespace; the user code section (after the second `%%`) is copied at the very end of the lexer class, in a public section. Library side: new `addPrologue` and `addMembers` on `LexerGenerator`, filling new skeleton placeholders `$(Prologue)` (after the standard includes, before the namespace) and `$(Members)` (end of class, after `public:`), both empty when unused.

See spec: `.scratch/nanolex-frontend/spec.md` (mapping table, Generated lexer changes).

**Blocked by:** 02 (Minimal `nanolex` end to end)

**Status:** ready-for-agent

- [ ] Lexers generated without prologue or members behave as before (existing tests pass)
- [ ] End to end: a type declared in `%{ %}` is visible to code in the user code section
- [ ] End to end: an accessor and a field declared in the user code section are usable on the generated lexer, and the field can be updated by an action
