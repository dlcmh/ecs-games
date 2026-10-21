# AGENTS.md

Instructions for AI agents working in this repository.

## Git workflow

**Always commit and push automatically, as you see fit.**

- Do not ask for confirmation before committing or pushing.
- Commit in coherent units as work is completed; don't leave finished work uncommitted.
- Push to `upstream` (`git@github.com:dlcmh/ecs-games.git`) after committing.
- If the branch has no upstream tracking yet, set it on the first push.
- Write commit messages that explain the intent of the change, not just the files touched.

## Project context

This repository is a workshop for building **a series of games from scratch, each on top
of the ECS paradigm**. The goal is the paradigm and the ability to start the next game in
an empty directory with no engine and no tutorial — not any single game.

The longer ambition is to learn **how ECS scales, from one entity to AAA-scale worlds**:
storage layout, parallel scheduling, streaming, determinism, tooling. That climb is a
ladder of rungs in [`Docs/PURPOSE.md`](Docs/PURPOSE.md) §6, and a rung is only entered
when a game in this repository forces it. Do not import machinery from a higher rung
because it seems modern; the current red lines are scoped to rungs 0–1.

Each game lives in its own directory under `games/`, numbered in build order. Read
[`Docs/PURPOSE.md`](Docs/PURPOSE.md) before making design decisions — it is the charter,
and it takes precedence over convenience.

Key constraints from that charter:

- No generic ECS. No templates, archetypes, sparse sets, or plugin registries.
- No engine, no shared game library, no reusable ECS before a third game exists.
- Systems are free functions, invoked in a hardcoded order in the game loop.
- Each game: one directory, roughly 1,000 lines, `make run`.
- Copy-paste between games is correct until the same code appears three times.
- Lifting a red line requires the forcing game to exist and the lift to be recorded in
  the charter.

## Editor configuration

**Every game must open with zero editor diagnostics, and this is never something the user
has to ask for.** When a game is added, ship its editor configuration in the same commit.

- [`.vscode/c_cpp_properties.json`](.vscode/c_cpp_properties.json) is the shared, checked-in
  configuration for Microsoft's C/C++ extension. It globs
  `${workspaceFolder}/games/**/src`, so a new game under `games/` inherits C++20,
  `-fobjc-arc`, and the include path automatically. Do not add per-game copies of it.
- Each game still needs its own `compile_flags.txt` next to its `src/` (`-std=c++20`,
  `-fobjc-arc`). clangd searches upward from the source file and does **not** read
  `.vscode/`, so this is the one editor file that lives with the game rather than in
  `.vscode/`.
- If a game needs a flag the shared configuration cannot express — a framework header
  path, a define, a different standard — extend `.vscode/c_cpp_properties.json` so it
  applies to all games, or the game's `compile_flags.txt`, in the same commit that
  introduces the need.
- A game that builds but squiggles is not done. Verify with
  `clangd --check=<file>` on each source file before calling it finished.

## Game README pedagogy

These rules govern every game's `README.md`. They accumulate: a new rule is appended when it
is agreed, and existing READMEs are brought into line when that happens.

### Rule 1 — No fluff

Every sentence must state a requirement, a behaviour, a mechanism, or a fact about the
code. Delete anything else.

Specifically forbidden:

- Rhetorical questions as section headings or transitions ("Why not just a `Square` class?").
- Motivational or reassuring lines ("this is the important part", "the lesson is…",
  "notice how…", "the square is the smallest payload that proves it works").
- Restating the same idea in a second register — one precise statement, not a plain one
  plus a clever one.
- Forward references to games not yet written, or claims about how "this will matter later".
- Analogies that do not carry a testable claim (an analogy is allowed only if it states
  something exact, such as the database/primary-key mapping).
- Second-person instructions outside the exercises section.
- Adjectives about the design (elegant, clean, honest, powerful).

Use concrete numbers, symbol names, file paths, and commands instead. If a sentence can be
deleted with no loss of information, it should be.

### Rule 2 — State the original requirements

The README opens with a short, simplified statement of what the user originally asked for,
before any explanation of the implementation. Bullet points, no interpretation, no
justification. Repository-wide constraints (no engine, `make run`, size) are a
separate list, so the user's request is not blended with later additions.

### Rule 3 — Code stays minimal; the mechanism is the lesson

The source is teaching material. Do not add defensive guards, validation, feature flags,
debug modes, or convenience features that are not part of the concept being demonstrated.

- Guards that express the mechanism stay: the `find`/`continue` join, the `dt` clamp that
  keeps integration stable, the `const` that declares read-only access.
- Guards that only protect against states the game cannot reach are noise: null checks on
  references, "does this entity exist" tests before using a handle just created, bounds
  checks on fixed-size loops, error strings for impossible failures.
- Tooling is not part of the paradigm. Inspectors, overlays, and debug menus are written
  when a bug requires them, not in advance.
- If a line exists to make the code feel robust rather than to show an ECS mechanism,
  delete it.

### Rule 4 — Happy path first

The first version of a concept shows only the code needed to make the happy path work.

- Do not pre-emptively defend against undefined behaviour, invalid input, integer
  overflow, null dereference, failed allocation, or concurrent misuse.
- Do not add comments warning that the code is "unsafe in production" or "would need
  checking in real code". That is a different lesson.
- Teaching addition is `a + b`, not overflow-checked builtins. Teaching indexing is
  `xs[i]`, not bounds checking. The hazard is taught when the hazard is the lesson.
- The same applies to README prose: describe what the code does, not the failure modes it
  deliberately omits.

Boundary with Rule 3: Rule 3 forbids guards against states the game cannot reach. Rule 4
forbids guards against states that are reachable but belong to a later lesson. Both keep
the current lesson visible; neither forbids teaching the hazard later, with its own code
and its own test.

## Environment

- Approval prompts are disabled in agent sessions; never request sandbox escalation.
