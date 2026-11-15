# 06: Contexts with `%context`, `%push` and `%pop`

**What to build:** in the rules section, `%context Name` opens a new context (rules before the first one belong to the main context). `%push Name` or `%pop`, written between a pattern and its action, attach a context-stack command to the rule (`setPushContext` / `setPopAction`). `<<UNKNOWN>>` inside a `%context` section defines that context's failure code (`addOnCurrentContextFailure`). The text consumed from a push to its pop is part of the pushing lexeme, whose action runs after the pop.

Because of the open bug `.scratch/subcontext-actions/issues/01-subcontext-actions-dropped.md`, an action on a non-pushing sub-context rule is rejected with a located error referencing that issue.

See spec: `.scratch/nanolex-frontend/spec.md`.

**Blocked by:** 03 (Main-context actions, `%default` and ignored lexemes), 05 (`%type`, `<<EOF>>` and `<<UNKNOWN>>` in the main context)

**Status:** ready-for-agent

- [ ] Reader tests compare `getDebugString()` of a multi-context description with an equivalent hand-built `LexerGenerator`
- [ ] Located errors for: unknown context in `%push`, `%pop` in the main context, duplicate `%context`, action on a non-pushing sub-context rule
- [ ] End to end: a `.nl` rewrite of the C multiline comment lexer gives the same token sequences as the `testgen` version (`checkLexemes`)
- [ ] End to end: the action of a pushing rule sees the whole lexeme (from push to pop) in `getMatchString()`
