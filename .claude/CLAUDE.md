# Agent Config

Agent role, response format, priorities, and example project references: see `AGENTS.md`.
Project facts (build, architecture, code style): see `CLAUDE.md`.

## Code Search

Use `ccc` for all codebase exploration. No grep/glob/Read without a ccc search first.

```bash
ccc search "<semantic query>" 2>&1 || true
```

Refine with symbols found. Read files only after narrowing target.

## Commits

Conventional Commits. Confirm with user before committing.
`fix` · `feat` · `perf` · `refactor` · `docs` · `chore`
