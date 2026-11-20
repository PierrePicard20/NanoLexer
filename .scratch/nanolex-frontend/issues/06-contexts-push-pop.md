# 06: Contexts with `%context`, `%push` and `%pop`

**What to build:** in the rules section, `%context Name` opens a new context (rules before the first one belong to the main context). `%push Name` or `%pop`, written between a pattern and its action, attach a context-stack command to the rule (`setPushContext` / `setPopAction`). `<<UNKNOWN>>` inside a `%context` section defines that context's failure code (`addOnCurrentContextFailure`). The text consumed from a push to its pop is part of the pushing lexeme, whose action runs after the pop.

Because of the open bug `.scratch/subcontext-actions/issues/01-subcontext-actions-dropped.md`, an action on a non-pushing sub-context rule is rejected with a located error referencing that issue.

See spec: `.scratch/nanolex-frontend/spec.md`.

**Blocked by:** 03 (Main-context actions, `%default` and ignored lexemes), 05 (`%type`, `<<EOF>>` and `<<UNKNOWN>>` in the main context)

**Status:** resolved

- [x] Reader tests compare `getDebugString()` of a multi-context description with an equivalent hand-built `LexerGenerator`
- [x] Located errors for: unknown context in `%push`, `%pop` in the main context, duplicate `%context`, action on a non-pushing sub-context rule
- [x] End to end: a `.nl` rewrite of the C multiline comment lexer gives the same token sequences as the `testgen` version (`checkLexemes`)
- [x] End to end: the action of a pushing rule sees the whole lexeme (from push to pop) in `getMatchString()`

## Comments

Resolution:

- Reader only, no library change. `%context Name` opens a context (an identifier, not `main_context`), `%push Name` / `%pop` between a pattern and its action map to `setPushContext` / `setPopAction`, `<<UNKNOWN>>` is unique per context and maps to `addOnUnknown` in the main context and to `addOnCurrentContextFailure` in a `%context` section. The contexts are filled in order, so `<<EOF>>` in a `%context` section reaches the located library error of `addOnEof` (ticket 05).
- Located errors checked by the reader (the library only reports the first two in `generateLexer`, without line): unknown context in `%push`, `%pop` in the main context, duplicate `%context`, action on a non-pushing sub-context rule (referencing `.scratch/subcontext-actions/issues/01-subcontext-actions-dropped.md`). Also: `%context` without or with an invalid name, `%context` without rule, no rule in the main context, `%push` without name, unknown command after a pattern, `%default` in a `%context` section (it applies to the main context only).
- Tests: `ContextsMatchHandBuiltGenerator` compares `getDebugString()` with a hand-built generator using three contexts and a push from a sub-context; since `LexerGenerator::getExpression()` only searches the current (last) context, the push, pop and action of the reader are read back on the last context only. End to end, `tests/nl/NlComments.nl` is `testgen/cMultilineCommentLexer.cpp` rule for rule, without the action of the `\n` rule of `Comment` (dropped by the library, rejected by the reader), with an action on the pushing rule that records `getMatchString()` (the whole comment) and a `<<UNKNOWN>>` failure action in `Comment`.
- Noticed, not changed: unlike Lex, the regex syntax rejects `[^*]` and `[^"]`; `*` and `"` must be escaped inside a class (`[^\*]`, `[^\"]`). The failure code of a `%context` section also runs when the input ends inside that context (unterminated comment).
