# 01 — rotating square

The smallest possible "game": a black window with one orange square turning slowly at
20°/second. It exists to answer a narrow question — *can a C++ program open a window, tick
at 60 Hz, and move something on screen?* — using an ECS shaped exactly like the one in
[`Docs/PURPOSE.md`](../../Docs/PURPOSE.md), with no engine, no framework, and no
dependencies beyond the macOS SDK.

```sh
make run
```

Close the window or press ⌘Q to quit (`make clean` removes `build/`).

## Files

| File | Lines | What it is |
|---|---|---|
| `src/ecs.hpp` | 105 | The whole ECS: entity ID, three components, `World`, two systems' declarations, render output |
| `src/game.cpp` | 49 | The two systems, implemented as free functions |
| `src/main.mm` | 169 | Cocoa window, view, 60 Hz timer, the game loop |
| `Makefile` | 17 | One compile command |
| `compile_flags.txt` | 1 | `-std=c++20` for clangd; the editor's fallback standard otherwise rejects structured bindings |

`ecs.hpp` and `game.cpp` are plain C++ — no platform types, no AppKit. `main.mm` is the
only file that knows a window exists. That split is the thing to protect: the ECS hands
the platform layer a list of quads and nothing else.

### Editor warnings

If VS Code underlines the structured bindings in `game.cpp` with *"decomposition
declarations are a C++17 extension"*, its language engine is parsing as C++14 — the
default it falls back to without a compilation database. `make` is unaffected: it passes
`-std=c++20`. Two files silence it:

- `compile_flags.txt` (here) — read by clangd, which searches upward from the source file.
- `.vscode/c_cpp_properties.json` (repo root) — read by Microsoft's C/C++ extension, which
  sets `"cppStandard": "c++20"`.

Reproduce the confusion deliberately to see it: `c++ -std=c++14 -Isrc -fsyntax-only
src/game.cpp` emits exactly those warnings; `-std=c++20` is clean.

## How the four nouns map onto the code

| Noun | In this code | Notes |
|---|---|---|
| **Entity** | `ecs::Entity` = `std::uint32_t`; `World::nextEntity`; the one square is entity `1` (created by `spawnSquare`) | An entity is not an object. It is a number that happens to be a key in some maps. |
| **Component** | `ecs::Transform` (position, angle, half-size), `ecs::Spin` (angular velocity), `ecs::Rgba` (colour) — all in `ecs.hpp` | Plain structs. No methods, no inheritance, no pointers to owned state. |
| **System** | `ecs::rotationSystem(World&, float dt)` and `ecs::renderSystem(const World&, RenderList&)` in `game.cpp` | Free functions. Not classes, not overrides, not registered anywhere. |
| **World** | `struct ecs::World` — one `std::unordered_map` per component type, keyed by entity ID | The maps *are* the schema: "has a Spin" means "is a key in `spins`". |

The join between components is visible in the source: `rotationSystem` runs for every
`Transform` and then skips any entity whose ID is not in `spins`. That `find` is the
query. There is no query language yet, because one loop per system is enough until a
third system needs the same loop.

### One frame, in order

`main.mm -tick:` → `rotationSystem` → `renderSystem` → `-drawRect:` → pixels.

1. **`-tick`** computes `dt` from `CFAbsoluteTimeGetCurrent()`, clamped to 50 ms.
2. **`rotationSystem`** joins `transforms` with `spins` and advances each angle by
   `radiansPerSecond * dt`, wrapping at 2π.
3. **`renderSystem`** joins `transforms` with `colours` and appends a `Quad` per match to
   a reused `RenderList`. It takes the world as `const` — that is the entire access
   declaration.
4. **`-drawRect:`** translates to the view centre, rotates, and fills a rectangle per
   quad.

The two systems never call each other, and the order is written out literally rather
than discovered from a registry.

## Is ECS just those four nouns?

Short answer: **the four nouns are the vocabulary minimum, not the whole model.** They are
a good answer to "where does this code go?" and a poor answer to "how does a system find
its data, and when does it run?" This game makes both gaps visible.

**What the four nouns leave out**

1. **Queries.** A system needs a way to say "every entity with Transform and Spin". In
   this game that is a hand-written loop with a `find` — a query expressed as control
   flow. At any real scale, the repeated join is extracted into a query helper, and query
   becomes the fifth noun, or the verb that the four nouns were missing. It is the piece
   most ECS libraries exist to provide.
2. **Entity lifecycle.** `createEntity()` hands out IDs and never recycles them. There is
   no `destroy`, so there is also no such thing as a dangling entity ID, and no need for
   generation counters. That changes the moment a bullet despawns: an ID plus a generation
   (`{index, version}`) is how a real ECS makes a stale handle detectable instead of
   silently aliasing a new entity. So "entities are IDs" is true and incomplete — they are
   *handles*, and handles need validation.
3. **Resources.** Not everything is per-entity. `dt` is one (passed as an argument here).
   `RenderList` is another (a member of the view controller). Time, input, a camera, a
   random source, the tile map — these are singletons that systems read. The four nouns
   have no place to put them, which is why every ECS grows a `Resource` concept.
4. **Execution.** The nouns say nothing about ordering, yet ordering is where the bugs
   live: rotate then render, versus render then rotate. Here it is a hardcoded call order,
   which is honest. In larger ECS designs systems declare queries and access sets and a
   scheduler derives order (and parallelism) from them — that machinery is a *consequence*
   of the nouns, not one of them.
5. **Deferred structure changes and events.** Spawning or destroying entities mid-iteration
   breaks the iteration. Real ECS designs queue those changes (command buffers) and add
   events for system-to-system messages. Neither is needed for one square; both become
   necessary as soon as collision kills something.
6. **Storage strategy.** `unordered_map<Entity, T>` per component is the naive layout. Sparse
   sets and archetypes change *performance*, not meaning. They are the most-written-about
   part of ECS and the least relevant here — for one square, they are pure ceremony.

**Two refinements to the slogan itself**

- "Components are data" is a rule about *ownership*, not absolute purity. A `Transform`
  may have a helper like `position()`, or a `Sprite` may hold a handle; the rule that
  matters is that a component never reads another component. Behaviour that needs two
  components is a system.
- "Systems are functions" is true at this scale and slightly false at scale. In Bevy or
  flecs, a system is best understood as a *declaration* — a query, an access set, and a
  function — which a scheduler runs. The function is the smallest part of it.

**So what is ECS, really?** A data model (four nouns) plus a query mechanism plus an
execution model. The nouns tell you the shape of the state; the queries and the ordering
tell you the shape of the behaviour. That is why two ECS libraries with identical
vocabulary can feel completely different to use.

## What this game deliberately does not have

No query helper, no entity destruction, no resources struct, no scheduler, no events, no
archetypes, no templates, no shared library. Each of those is added when a game's actual
problem forces it — see the rule of three in [`Docs/PURPOSE.md`](../../Docs/PURPOSE.md) §6.
The next game is where copying this file structure *by hand* is the correct move.

## Things worth trying in the code

- Add a second square at a different position with a different colour: `spawnSquare` twice.
  Notice that `rotationSystem` and `renderSystem` need no edit at all.
- Delete `world.spins[e] = ...` in `spawnSquare`. The square stops rotating but still
  renders — the join, made visible.
- Change `20.0f` to a negative value. Direction is data, not a code path.
- Break the clamp on `dt` (allow 5 s) and drag the window for a while. The square jumps:
  the fix is in the loop, not in a system.
