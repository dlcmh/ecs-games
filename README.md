# ecs-games

A workshop for building games from scratch — no engine, no framework, no tutorial — each
one written by hand on top of the ECS paradigm.

The goal is not any particular game. It is the method: being able to open an empty
directory and build the next game from entities, components, systems, and a pipeline,
because the paradigm is understood well enough to reapply to a problem shape never seen
before.

**Pac-Man is the first exercise, not the objective.** It is a small, complete, disposable
vehicle for learning the approach. Finishing it is a milestone; stopping there would miss
the point.

What every game here looks like: one self-contained directory, roughly 1,000 lines,
`make run`, a tiny hand-written ECS, systems called in a hardcoded order, and an entity
inspector built on day one. No archetypes, no templates, no shared engine.

See [`Docs/PURPOSE.md`](Docs/PURPOSE.md) for the full charter and the rules that keep the
scaffolding from becoming the product.

## Status

One exercise so far: [`games/01-rotating-square`](games/01-rotating-square) — a window, a
60 Hz loop, and a square that turns because its `Spin` component says so. Pac-Man is next.
