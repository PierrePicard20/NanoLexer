# Actions of sub-context rules are dropped from the generated lexer

Status: needs-triage

## Problem

An action attached (`addOnMatchCode`) to a rule of a context other than the main context is silently missing from the generated lexer, unless that rule pushes a context.

Cause: in `NanoLexer/NanoLexer.cpp` (`generateLexer`, second loop over `contextsData`), `id2OnMatchCode` is only filled for expressions whose action is `push`.

Reproduction: `testgen/cMultilineCommentLexer.cpp` attaches `line++; row = 1; nbCharToSubstract = getMatchLength();` to the `\n` rule of context `Comment`. That code appears nowhere in the generated `CommentsLexer.h`, so line counting inside a multiline comment is wrong.

## Design notes for the fix (not decided)

Proposed direction, discussed but postponed:

- Do not run sub-context actions immediately: append them to a list of deferred lambdas, run them just before the main-context action switch in `lex()`, and clear the list when leaving `lex()`.
- Backtracking: if sub-contexts can backtrack, clearing the whole list on backtrack would discard actions of matches that were already final (e.g. `/* a\nb\n*` then backtrack: the two `\n` actions must survive). Only lambdas recorded after the last retained accept point should be dropped. To check first: whether sub-contexts backtrack at all.
- An action must only run for a final match, never for a tentative accept.

See also [02-deferred-action-match-snapshot](02-deferred-action-match-snapshot.md).
