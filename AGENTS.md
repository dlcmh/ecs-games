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

This repository is a learning vehicle: a small, complete Pac-Man written in C++ with a
deliberately tiny hand-written ECS. Read [`Docs/PURPOSE.md`](Docs/PURPOSE.md) before
making design decisions — it is the charter, and it takes precedence over convenience.

Key constraints from that charter:

- No generic ECS. No templates, archetypes, sparse sets, or plugin registries.
- Systems are free functions, invoked in a hardcoded order in the game loop.
- Target roughly 1,000 lines total, one directory, `make run`.

## Environment

- Approval prompts are disabled in agent sessions; never request sandbox escalation.
