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
- Each game: one directory, roughly 1,000 lines, `make run`, with an entity inspector.
- Copy-paste between games is correct until the same code appears three times.
- Lifting a red line requires the forcing game to exist and the lift to be recorded in
  the charter.

## Environment

- Approval prompts are disabled in agent sessions; never request sandbox escalation.
