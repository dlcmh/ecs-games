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

Pac-Man is the **first exercise, not the objective**. It is a vehicle for learning the
method; finishing it is a milestone, and stopping there fails the purpose. Each game
lives in its own directory. Read [`Docs/PURPOSE.md`](Docs/PURPOSE.md) before making design
decisions — it is the charter, and it takes precedence over convenience.

Key constraints from that charter:

- No generic ECS. No templates, archetypes, sparse sets, or plugin registries.
- No engine, no shared game library, no reusable ECS before a third game exists.
- Systems are free functions, invoked in a hardcoded order in the game loop.
- Each game: one directory, roughly 1,000 lines, `make run`, with an entity inspector.
- Copy-paste between games is correct until the same code appears three times.

## Environment

- Approval prompts are disabled in agent sessions; never request sandbox escalation.
