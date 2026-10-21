# 01 — rotating square

## Requirements

Original request, simplified:

- A launchable C++ program that shows a slowly rotating square.
- The folder name is chosen by the implementer.
- The README maps ECS concepts to the parts of the code.
- The README answers whether ECS is only four nouns: entities are IDs, components are data,
  systems are functions, world is the container.

Repository constraints from [`Docs/PURPOSE.md`](../../Docs/PURPOSE.md): no engine or
framework; macOS SDK only; `make run`; an entity inspector; roughly 1,000 lines.

## Behaviour

- 720×720 window, black background.
- One entity: a 240×240 orange square at the centre, angle increasing 20°/s.
- 60 Hz timer (`NSTimer`, 1/60 s). `dt` is clamped to 50 ms.
- Window close or ⌘Q terminates. `I` prints the world to stdout.

```sh
make run
```

## Files

| File | Lines | Contents |
|---|---|---|
| `src/ecs.hpp` | 112 | Entity alias, three components, `World`, `RenderList`, declarations |
| `src/game.cpp` | 78 | Two systems, spawn helper, entity inspector |
| `src/main.mm` | 183 | Window, view, timer, game loop |
| `Makefile` | 17 | Compile command |
| `compile_flags.txt` | 2 | Editor flags |

`ecs.hpp` and `game.cpp` contain no platform types. `main.mm` is the only file that
includes AppKit.

## The four nouns in this code

| Noun | Code | Mechanism |
|---|---|---|
| Entity | `ecs::Entity` = `std::uint32_t`, `World::nextEntity` | The square is entity `1`. |
| Component | `Transform`, `Spin`, `Rgba` in `ecs.hpp` | Plain structs, one table each. |
| System | `rotationSystem`, `renderSystem` in `game.cpp` | Free functions called from `-tick`. |
| World | `struct ecs::World` | Three `unordered_map<Entity, T>` members plus the ID counter. |

Component presence is map-key presence: an entity has a `Spin` when `spins` contains its
ID. There is no flag, base class, or type tag.

The component split:

```cpp
struct Transform { float x, y, angle, halfSize; };  // where, how big, current orientation
struct Spin      { float radiansPerSecond; };        // rate of change of angle
```

`Transform` without `Spin` is a static object. `rotationSystem` therefore selects
"Transform and Spin" and `renderSystem` selects "Transform and Rgba"; neither function
mentions squares.

## The join

```cpp
for (auto& [entity, transform] : world.transforms) {
    const auto spin = world.spins.find(entity);
    if (spin == world.spins.end()) {
        continue;
    }
    transform.angle = std::fmod(transform.angle + spin->second.radiansPerSecond * dt, kTwoPi);
}
```

`find` is the query: iterate one table, keep rows that also exist in another.
`renderSystem` performs the same operation against `colours`. No query helper exists
because these are the only two joins in the game.

`renderSystem` takes `const World&`. The const is the access declaration: the system reads,
and the compiler enforces it.

## One frame

1. `-tick` computes `dt` from `CFAbsoluteTimeGetCurrent()`.
2. `rotationSystem` adds `spin × dt` to each angle.
3. `renderSystem` emits one `Quad` per entity with `Transform` and `Rgba`.
4. `-drawRect:` translates to the view centre, rotates by `quad.angle`, fills a rectangle.

At 20°/s the angular velocity is `20 × π / 180 = 0.3491` rad/s. One tick at 60 Hz adds
`0.3491 × 0.01667 = 0.005818` rad (0.333°); 60 ticks add 20°.

System order is literal in `-tick`, not discovered from a registry. Rendering before
rotating would draw the previous frame's angle.

## Inspector

`I` calls `printWorld`, which iterates `transforms` and prints every component row for each
ID. Output for the starting state after one tick:

```
world: transforms=1 spins=1 colours=1
entity 1   Transform{x=0.0 y=0.0 angle=0.006rad halfSize=120.0} Spin{0.349 rad/s} Rgba{r=0.95 g=0.55 b=0.20 a=1.00}
```

`printWorld` and `printEntity` are free functions that read only. They are not systems: they
run on a keypress, not every frame.

## Is ECS only four nouns

No. The four nouns describe storage and ownership; they do not describe selection or
scheduling.

1. **Queries.** Selecting "rows with Transform and Spin" is a loop plus `find` here. A
   reusable query is an abstraction the four nouns do not contain; most ECS libraries exist
   to provide it.
2. **Entity lifecycle.** `createEntity` only increments. Nothing destroys or recycles IDs,
   so a stale ID cannot exist. Destruction requires generation counters to distinguish a
   dead ID from a reused one.
3. **Resources.** `dt` is passed as an argument; `RenderList` is owned by the view
   controller. Neither is per-entity. Time, input, and cameras need a place outside the four
   nouns.
4. **Execution.** Ordering is not implied by the nouns. It is the hardcoded sequence in
   `-tick`; larger designs derive order and parallelism from declared queries and access
   sets.
5. **Deferred changes and events.** Spawning or destroying during iteration modifies the
   map being iterated. A command buffer or event queue is needed once that happens.
6. **Storage.** One `unordered_map` per component is the simple layout. Sparse sets and
   archetypes change iteration performance, not semantics.

Two qualifications:

- A component may have helper methods; the constraint is that it does not read another
  component. Behaviour across components is a system.
- A system is a function here. In Bevy or flecs it is a declaration (query, access set,
  function) that a scheduler executes.

## Not implemented

No query helper, destruction, resources struct, scheduler, events, archetypes, templates, or
shared library. Each is added when a game requires it ([`Docs/PURPOSE.md`](../../Docs/PURPOSE.md) §7).

## Exercises

Each states the edit and the result.

- **Second square.** Call `spawnSquare` twice with different positions and colours. Both
  systems compile unchanged; two quads are emitted.
- **Static square.** Spawn two entities, then `world.spins.erase(second)`. `rotationSystem`
  skips the second; `renderSystem` still draws it. Press `I`: the second entity has no
  `Spin`.
- **Reverse rotation.** Change `20.0f` to `-20.0f` in `spawnSquare`. The angle decreases;
  no branch changes.
- **Remove the `dt` clamp.** Allow 5 s `dt` and drag the window. The square jumps by
  `spin × 5`. The clamp belongs to the loop, not to a system.

## Editor setup

Compiler flags are in the `Makefile`, which editors do not read. Two files restate them:

- `compile_flags.txt` — `-std=c++20`, `-fobjc-arc`; read by clangd.
- `.vscode/c_cpp_properties.json` — `"cppStandard": "c++20"`,
  `"compilerArgs": ["-fobjc-arc"]`; read by Microsoft's C/C++ extension.

Without them: `game.cpp` reports "decomposition declarations are a C++17 extension"
(engine at C++14, reproducible with `c++ -std=c++14 -Isrc -fsyntax-only src/game.cpp`), and
`main.mm` reports "cannot create __weak reference in file using manual reference counting"
(ARC not applied, reproducible with
`c++ -std=c++20 -Isrc -fsyntax-only -x objective-c++ src/main.mm`).
