# Glossary

**Lexer generator**: the NanoLexer library. Builds a lexer model from regular expressions and emits a header-only lexer class.

**Frontend**: a command-line tool that reads a description file and drives the lexer generator to produce a lexer. An alternative to using the library directly; it does not replace it.

**Description file**: the Lex-like input file of the frontend (`.nl`). Has three sections: definitions, rules, user code.

**Context**: a named set of rules matched together. Contexts are managed as a stack: a rule may push another context or pop the current one. Unlike Lex start conditions, only the main context produces lexemes.

**Main context**: the context the lexer starts in, at the bottom of the context stack. Only its rules produce lexemes; the text consumed by a pushed context, up to the matching pop, becomes part of the lexeme that pushed it.

**Lexeme**: a unit of text matched by a main-context rule and reported by the lexer.

**Action**: code run when a rule matches, in any context. In the main context, an action may return a value to the caller of the lexer.

**Default action**: the action run for a main-context lexeme whose rule has no action of its own.

**Ignored lexeme**: a lexeme whose action (its own, or the default action) does not return a value. The lexer moves on to the next lexeme.

**Unknown text**: input that matches no rule of the current context. Not an error: the lexer reports or skips it as described. Distinct from an error, which is a defect of the lexer or of its description.

**Macro**: a named regular expression, not tied to any context, that rules reference by name.
