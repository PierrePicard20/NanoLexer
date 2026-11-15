# Deferred sub-context actions see the wrong match

Status: needs-triage

Depends on the deferred-lambda design of [01-subcontext-actions-dropped](01-subcontext-actions-dropped.md).

## Problem

If sub-context actions are deferred as lambdas capturing only `this`, by the time they run the lexer state no longer describes their own match. Example: `nbCharToSubstract = getMatchLength();` on the `\n` rule of context `Comment` would read the length of the whole lexeme instead of the length of the `\n`.

## Idea (not decided)

Each lambda captures a snapshot of its match (text, length) by value, and `getMatchString()` / `getMatchLength()` return that snapshot while the lambda runs. The user wants to think about this further; for now lambdas capture `this` only.
