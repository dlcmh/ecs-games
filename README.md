# ecs-games

A workshop for building games from scratch — no engine, no framework, no tutorial — each
one written by hand on top of the ECS paradigm.

The goal is not any particular game. It is to learn **how the ECS paradigm scales**, from
the smallest possible program to the concerns that only appear at AAA scale:

| Rung | Scale | What gets learned |
|---|---|---|
| 0 | One entity | A window, a tick, two systems, component data reaching the screen |
| 1 | Tens to hundreds | Queries, entity lifecycle, collisions — a first complete game |
| 2 | Thousands | Spatial partitioning, resources, scheduling, scene data |
| 3 | Tens of thousands to AAA | Storage layout, parallel systems, streaming, determinism, tooling |

Each rung is entered only when a game demands it, so every piece of machinery is attached
to a problem actually felt rather than read about. The constant across all of them is the
same four nouns — entities are IDs, components are data, systems are functions, world is
the container — which is what makes the climb continuous instead of a series of rewrites.

What every game here looks like at the current rungs: one self-contained directory,
roughly 1,000 lines, `make run`, a tiny hand-written ECS, systems called in a hardcoded
order, and an entity inspector built on day one. No engine, no shared library, no
templates.

See [`Docs/PURPOSE.md`](Docs/PURPOSE.md) for the full charter, the scaling ladder, and the
rules that keep the scaffolding from becoming the product.

## Status

One exercise so far: [`games/01-rotating-square`](games/01-rotating-square) — rung 0: a
window, a 60 Hz loop, and a square that turns because its `Spin` component says so.

Next: the first complete game (rung 1), where queries, entity destruction, and collision
become necessary.
