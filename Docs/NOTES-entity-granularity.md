# Note: what deserves to be an entity

A recurring question once real content exists: is *this* thing an entity? A smoke
particle? The maze? Each corridor? The answer is a rule about **granularity**, and getting
it wrong produces two opposite failures that look nothing alike.

## The test

An entity is a **role, not a type** — "a thing a system iterates over and can refer to
individually." Ask three questions about the thing in front of you:

1. **Does something act on it independently?** Its own lifetime, its own state changes,
   its own removal.
2. **Does anything need to reference *this specific one* later?** A target, a lock, a
   parent, a click.
3. **Does a system iterate a set of them?**

Any "yes" → entity. All "no", and it is shared or static → it is data (a resource, level
data, or a field inside a component).

## The cases

| Thing | Entity? | Why |
|---|---|---|
| Smoke particle (tens) | Yes | per-particle lifetime, position, fade |
| Smoke particle (thousands) | Forced decision — see below | storage or granularity must change |
| Wall, corridor, floor tile | No — grid/level data | static, shared, only ever read by position |
| Door that locks, opens, times out | Yes | independent state and lifetime |
| Pellet / pickup | Yes (or grid bitmask at scale) | independently consumed, triggers score |
| Background colour, camera, input | No — resource | global, exactly one |
| Level connectivity / path graph | No | a graph, read by pathfinding |

### Smoke

A single particle is a textbook entity: `Transform`, `Velocity`, `Lifetime`, `Fade`.
Three systems (`drift`, `fade`, `expire`). Nothing about that is wrong.

The caveat is scale, not semantics. At tens of particles, entities in maps are perfect.
At ten thousand, the rung-3 forcing function from
[`PURPOSE.md`](PURPOSE.md) §6 has arrived, and there are two honest answers:

- **Improve storage** — SoA arrays or sparse sets, so `fade` walks contiguous memory.
- **Change granularity** — one entity with a `ParticleEmitter` component holding a
  `std::vector<Particle>`, simulated in one system. Per-particle queries and systems are
  lost; locality and a fraction of the code are gained.

The second is a granularity choice, not a cop-out. The mistake is doing it silently and
then wondering why the particles are no longer inspectable.

### The maze

The maze is level data, not an actor. A wall has no independent state or lifetime, nothing
references wall #137, and no system says "for every wall…". Movement and collision *read*
the maze; they do not iterate it as a set of entities.

So the maze is a struct (a grid of cells, or a corridor graph) owned by the game and
passed to systems the way `dt` already is. Wrapping it in one entity adds no capability:
there is exactly one, so "query the set of mazes" is dead weight. This is why the resource
concept exists at all — and `RenderList` in
[`games/01-rotating-square`](../games/01-rotating-square) is already the first thing in the
code that is none of the four nouns.

### Corridors

Less so again. A corridor is a cell or edge in the maze's connectivity, and connectivity is
a graph, not a collection of objects. Making corridors entities means the pathfinder must
query entities to walk a graph it already knows statically: more code, worse locality, no
benefit.

Corridors become entities the moment they acquire state and behaviour — a door, a
collapsing tunnel, a trap. The general shape is:

> **Structural world** = data. **Exceptions with behaviour** = entities.

Walls and corridors live in the grid; doors, pickups, and actors live as entities.

## The two failure modes

**Entity-per-tile.** A 28×31 maze as 868 entities with `Position` + `TileType`: 868 map
insertions, 868 lookups per collision query, and not one system that ever asks "for all
walls." Ceremony — exactly what the survey-form rule in
[`PURPOSE.md`](PURPOSE.md) §7 forbids.

**Maze-as-god-entity.** One `Maze` entity that also owns doors, timers, and pellets, so
every new behaviour becomes an `if` inside it. That is the OOP god-function returning
through the back door — the thing ECS was chosen to avoid.

The middle path is usually right: structural world as data, behaviour-bearing exceptions
as entities.

## Rule of thumb

> Entities are the things that *live*; the maze is the world they live *in*.
> If a system would write a `for` loop over it, make it entities.
> If a system only ever reads it by position, make it data.

## When this note changes

- At rung 2, resources become explicit instead of being passed around as arguments.
- At rung 3, particles and pellets may become a storage decision (SoA/archetype) or a
  batch component, as above.
- If a "static" structure ever needs per-instance behaviour, that instance graduates to an
  entity — and the reason is recorded here, per the red-line rule in
  [`PURPOSE.md`](PURPOSE.md) §6.
