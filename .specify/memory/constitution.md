<!--
SYNC IMPACT REPORT
==================
Version change: 1.0.0 → 1.1.0
Bump rationale: MINOR — two new principles added, two existing principles
materially expanded; no existing rule removed or weakened.

Modified principles:
  II. Manual Verification & Testing → II. Minimal, Targeted Verification
      (manual-verification mandate kept; added minimal/incremental/no-repeat)
  V. CCC-First Code Search — added feature-scoped search, inspect-only-relevant,
      stop-when-understood

Added principles:
  VI. Narrow Scope
  VII. Context & Delegation Economy

Modified sections:
  Development Workflow — quality-gate bullet references targeted verification

Templates requiring updates:
  ✅ .specify/templates/plan-template.md (Constitution Check gate references
      this file dynamically — no edit needed)
  ✅ .specify/templates/spec-template.md (no principle-specific edits required)
  ✅ .specify/templates/tasks-template.md (no principle-specific edits required)

Follow-up TODOs: none
-->

# GBC C Starter Kit Constitution

## Core Principles

### I. Code Quality Discipline

All code MUST conform to the project Code Style in `CLAUDE.md`: C99, no heap,
no `<stdbool.h>` (`uint8_t` as bool), 4-space indent, brace on same line.
Private symbols MUST be `static`; ROM data MUST be `const`. Naming follows the
documented table (snake_case functions, PascalCase types, UPPER_SNAKE_CASE
macros, `g_` globals, `FILENAME_H` guards). Explicit narrowing casts MUST be
used for arithmetic that overflows the target width: `(uint8_t)(x + y)`.
Hardware registers MUST use unsigned literals (`0u`, `0xFFu`). When a GBDK
technique has a canonical sample under `examples/`, the implementation MUST
follow that sample's idiom rather than inventing a new one.

Rationale: Style uniformity on a constrained Z80/GBC target prevents subtle
overflow, signedness, and banking bugs that the toolchain will not catch.

### II. Minimal, Targeted Verification

There is no automated test suite. Testing MUST stay minimal and pragmatic:
verify only critical paths and regression-prone logic, using the smallest
relevant check first (build, then the specific screen/flow touched in BGB or
SameBoy) and escalating only when that is insufficient. Broad coverage goals,
full screen-flow walkthroughs without cause, and repeating a check that
already passed are PROHIBITED. Every behavioral change MUST still be observed
before it is considered done: the build MUST be green
(`cmake --build --preset build-rom`) and the ROM MUST boot. CGB-only features
MUST be checked to degrade gracefully on DMG (`_cpu == CGB_TYPE` guards). The
verification performed (emulator, flow exercised, result) MUST be stated when
reporting completion — never claim "works" unobserved.

Rationale: With no CI gate, the emulator run is the only safety net, but cost
should track risk; an unstated verification is an untested change, and a
redundant one is wasted effort.

### III. UX Consistency

Player-facing behavior MUST stay consistent with the established screen flow
(splash → intro → title → [options] → gameplay → game over/win → credits →
title). Text rendering MUST use the shared helpers (`print_centered`,
dialogue typewriter, `FONT_OFFSET`) rather than ad-hoc tile writes. Fonts,
palettes, and the sliding dialogue box MUST match existing screens. New
screens MUST honor input via `get_pressed()` / `flush_input()` and respect
`g_settings` persistence.

Rationale: A starter kit's value is a coherent template; divergent UI patterns
make it harder to learn from and extend.

### IV. Memory & Resource Optimization

WRAM, VRAM, and ROM are scarce and MUST be treated as a budget. Every change
MUST avoid net growth of WRAM/SRAM unless the feature requires it, and that
cost MUST be stated. Dead or unused modules (e.g. `sprite.c`, `background.c`)
MUST stay excluded from the build. Prefer ROM `const` data and LUTs over
runtime computation that consumes RAM. Reuse existing buffers and sprite/OAM
slots before allocating new ones. Banking MUST follow the single-module
`#pragma bank N` rule — multi-module banks corrupt the ROM. Memory-sensitive
techniques MUST be modeled on the relevant `examples/` sample (e.g.
`hblank_copy`, `ram_function`, `hicolor`, `linkerfile`).

Rationale: The platform offers ~8 KB WRAM; uncontrolled allocation silently
breaks the ROM and is hard to diagnose after the fact.

### V. CCC-First Code Search (NON-NEGOTIABLE)

Using `Skill(ccc)` is MANDATORY for all codebase exploration. No `grep`,
`glob`, or file `Read` may precede a `ccc` search. Search strategy MUST be:
(1) high-level semantic query, (2) extract symbols, (3) refine with at least
one identifier, (4) traverse callers/callees, (5) open files only after
narrowing. After code changes, the index MUST be refreshed via `ccc`. This
does not apply to external/general knowledge (use Context7 for library docs).

Search MUST be scoped strictly to the current feature or bug. Once `ccc`
identifies the relevant files and dependencies, inspect only those directly;
unrelated repository areas MUST NOT be scanned. Exploration MUST stop as soon
as the implementation surface and its dependencies are understood.

Rationale: Semantic search over the indexed codebase finds the right callers
and patterns faster and avoids blind, speculative edits; bounded scope keeps
exploration cost proportional to the task.

### VI. Narrow Scope

Each change MUST be the smallest coherent change that satisfies the
requirement. It MUST reuse existing project conventions and implementations
and MUST preserve behavior outside the requested scope. Unrelated refactors,
cleanup, architectural changes, and new dependencies are PROHIBITED unless the
requirement cannot be met without them, in which case they MUST be justified in
the plan's Complexity Tracking section.

Rationale: Small, convention-matching diffs are easy to review and cannot
silently regress a ROM budget or screen that was not in scope. Code quality,
maintainability, and UX consistency (I, III) remain the primary criteria.

### VII. Context & Delegation Economy

Context usage MUST be minimized. Analysis already captured in specs, plans,
tasks, docs, or the current session MUST NOT be repeated. Large files or
outputs MUST NOT be re-read without a concrete need; prefer reading only the
relevant range of code directly. Subagents MUST NOT be spawned unless
necessary; when delegation is needed for simple work, the cheaper/smaller
model MUST be preferred.

Rationale: Context and model cost are finite; redundant reads and needless
delegation add cost without adding information.

## Hardware & Performance Constraints

Target is Game Boy Color via GBDK (gbdk-2020). Priority order is CPU cycles
(target 60 FPS) → RAM/ROM → battery → maintainability. Code MUST respect:

- VRAM writes only outside LCD Mode 3 (`wait_vbl_done()` or H-Blank ISR).
- OAM index < 40; palette index 0–7.
- `uint8_t` loop counters; LUTs in ROM over runtime div/mod.
- Multi-module `#pragma bank N` is FORBIDDEN — it corrupts the ROM. Use
  single-module banks only.
- WRAM is scarce (~8 KB): track and minimize per-feature RAM cost.

The example projects under `examples/` (per `AGENTS.md`) are the canonical
reference for GBDK techniques — palettes (`apa_image`, `colorbar`, `dscan`,
`hicolor`), scrolling (`galaxy`), VRAM/H-Blank (`hblank_copy`), interrupts
(`irq`, `lcd_isr_wobble`), banking (`linkerfile`, `ram_function`), link cable
(`comm`), sound (`sound`), and PRNG (`rand`). Consult the matching example
before implementing a technique from scratch.

## Development Workflow

- Read a file before editing it; search all callers (via `ccc`) before
  modifying a function signature or contract.
- Match existing patterns and the relevant `examples/` sample — no blind or
  speculative edits.
- Commits follow Conventional Commits 1.0.0 (`fix`, `feat`, `refactor`,
  `docs`, `perf`, `chore`); breaking changes use `!` or a
  `BREAKING CHANGE:` footer. Confirm with the user before committing.
- Build → targeted manual emulator verification (Principle II) is the quality
  gate for every change.

## Governance

This constitution supersedes other ad-hoc practices for this repository. All
changes MUST be checked against these principles before merge; deviations MUST
be justified in the relevant plan's Complexity Tracking section or rejected.

Amendments require: a written change, a version bump per the policy below, and
update of this file's Sync Impact Report plus any affected `.specify`
templates. Versioning is semantic — MAJOR for backward-incompatible principle
removals/redefinitions, MINOR for a new principle or materially expanded
guidance, PATCH for clarifications and wording fixes.

Runtime development guidance lives in `CLAUDE.md` and `AGENTS.md`; this
constitution governs the non-negotiable rules those documents elaborate.

**Version**: 1.1.0 | **Ratified**: 2026-06-29 | **Last Amended**: 2026-10-13
