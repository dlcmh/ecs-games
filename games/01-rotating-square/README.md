# 01 — rotating square

## Requirements

- A launchable C++ program that shows a slowly rotating square.

## Constraints

From [`Docs/PURPOSE.md`](../../Docs/PURPOSE.md): no engine or framework; macOS SDK only;
`make run`; roughly 1,000 lines.

## Behaviour

```sh
make run
```

- 720×720 window, black background.
- One entity: a 240×240 orange square at the centre; angle increases 20°/s.
- `NSTimer` at 1/60 s; `dt` clamped to 50 ms.
- ⌘Q or window close terminates.

## Files

| File | Lines | Contents |
|---|---|---|
| `src/ecs.hpp` | 105 | Entity alias, three components, `World`, `RenderList`, declarations |
| `src/game.cpp` | 49 | Two systems, spawn helper |
| `src/main.mm` | 169 | Window, view, timer, game loop |
| `Makefile` | 17 | Compile command |
| `compile_flags.txt` | 2 | Editor flags |

`ecs.hpp` and `game.cpp` contain no platform types. `main.mm` is the only file that
includes AppKit.

## ECS in the code

| Noun | Code | Mechanism |
|---|---|---|
| Entity | `ecs::Entity` = `std::uint32_t`, `World::nextEntity` | The square is entity `1`. |
| Component | `Transform`, `Spin`, `Rgba` | Plain structs, one table each. |
| System | `rotationSystem`, `renderSystem` | Free functions called from `-tick`. |
| World | `struct ecs::World` | Three `unordered_map<Entity, T>` members plus the ID counter. |

```cpp
struct Transform { float x, y, angle, halfSize; };  // where, how big, current angle
struct Spin      { float radiansPerSecond; };        // rate of change of angle
struct Rgba      { float r, g, b, a; };
```

An entity has a component when its ID is a key in that component's map.

## One frame

1. `-tick` computes `dt` from `CFAbsoluteTimeGetCurrent()`.
2. `rotationSystem` adds `spin × dt` to each angle.
3. `renderSystem` emits one `Quad` per entity with `Transform` and `Rgba`.
4. `-drawRect:` translates to the view centre, rotates by `quad.angle`, fills a rectangle.

20°/s is 0.3491 rad/s. One tick at 60 Hz adds `0.3491 × 0.01667 = 0.005818` rad (0.333°);
60 ticks add 20°.

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

`renderSystem` performs the same join against `colours`. Its `const World&` parameter is the
read-only access declaration.

The order is literal in `-tick`, not discovered from a registry. Rendering before rotating
would draw the previous frame's angle.

## Is ECS only four nouns

No.

1. **Queries.** Selecting "rows with Transform and Spin" is a loop plus `find`. A reusable
   query is an abstraction the four nouns do not contain.
2. **Entity lifecycle.** `createEntity` only increments. Nothing destroys or recycles IDs,
   so a stale ID cannot exist; destruction requires generation counters.
3. **Resources.** `dt` is an argument; `RenderList` is owned by the view controller.
   Neither is per-entity. Time, input, and cameras need a place outside the four nouns.
4. **Execution.** Ordering is not implied by the nouns. It is the hardcoded sequence in
   `-tick`. Larger designs derive order and parallelism from declared queries and access
   sets.
5. **Deferred changes and events.** Spawning or destroying during iteration modifies the map
   being iterated. A command buffer or event queue is needed once that happens.
6. **Storage.** One `unordered_map` per component is the layout here. Sparse sets and
   archetypes change iteration performance, not semantics.

A component may have helper methods, provided it does not read another component. A system
is a function here; in Bevy or flecs it is a declaration of query and access plus a
function.

ECS is a data model (four nouns), a query mechanism, and an execution model.

## Exercises

- **Second square.** Call `spawnSquare` twice with different positions and colours. Both
  systems compile unchanged; two quads are emitted.
- **Static square.** Spawn two entities, then `world.spins.erase(second)`. `rotationSystem`
  skips the second; `renderSystem` still draws it.
- **Reverse rotation.** Change `20.0f` to `-20.0f`. The angle decreases; no branch changes.
- **Remove the clamp.** Allow 5 s `dt` and drag the window. The square jumps by `spin × 5`.

## Editor setup

`compile_flags.txt` holds `-std=c++20` and `-fobjc-arc` for clangd.
`.vscode/c_cpp_properties.json` holds `"cppStandard": "c++20"` and
`"compilerArgs": ["-fobjc-arc"]` for Microsoft's C/C++ extension. The build reads neither;
they exist because editors do not read the `Makefile`.
