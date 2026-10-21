# 01 — rotating square

A black window with one orange square turning slowly at 20°/second. It exists to answer a
narrow question — *can a C++ program open a window, tick at 60 Hz, and move something on
screen?* — using an ECS shaped exactly like the one in
[`Docs/PURPOSE.md`](../../Docs/PURPOSE.md), with no engine, no framework, and no
dependencies beyond the macOS SDK.

```sh
make run
```

Close the window or press ⌘Q to quit (`make clean` removes `build/`).

## What this program is really showing

Not "a square rotates". This: **one frame is a pipeline of independent functions, each
reading a slice of plain data, mutating it or emitting output, in a fixed order.**

The square is the smallest payload that proves the pipeline works. Everything that matters
later — ten enemies, a hundred pellets, ten thousand particles — is the same pipeline with
more rows in the tables. If the shape is clear here, it stays clear there.

## Why not just a `Square` class?

The obvious version, and it works for exactly one square:

```cpp
struct Square { float x, y, angle, halfSize; };

void update(Square& s, float dt) {
    s.angle += 0.349f * dt;   // 20°/s
}
```

Now the requirements arrive the way they always do. A second square that *doesn't* spin.
A smoke puff that fades. An enemy that chases the player. Each one is a small, reasonable
request, and each one edits `update`:

```cpp
void update(Thing& t, float dt) {
    if (t.spins)        t.angle += t.spinRate * dt;
    if (t.kind == GHOST) t.target = chooseTarget(t);
    if (t.kind == SMOKE) t.alpha -= 0.5f * dt;
    ...
}
```

Every new behaviour touches the same function, so every change can break every other
behaviour. That is the god-function that makes tutorials rot — not because the code is
bad, but because there is nowhere else to put anything. Inheritance doesn't save it: you
replace the `if` chain with a `switch` wherever the base type is used.

ECS makes the other choice: **data is inert; behaviour lives outside it, and selects the
data it works on.**

| Question | OOP answer | ECS answer |
|---|---|---|
| Where does state live? | Inside the object | Rows in the world's tables |
| Where does behaviour live? | A method on the object | A free function outside |
| Who decides what runs? | Virtual dispatch, or the caller | The fixed order in the game loop |
| Add a new kind of thing | New class, or a new branch | New component, or new system |
| Change how something behaves | Edit the existing class | Add or edit a system |

## The four nouns

### Entity — a name, not a thing

An entity is a number: `using Entity = std::uint32_t`. That's it. The square is entity
`1` because `spawnSquare` took the next ID from a counter. Nothing marks entity `1` as a
square except which tables happen to have a row for `1`.

The closest familiar thing is a **primary key**. It identifies a row across tables without
containing any data itself. Two entities with identical components are still distinct
entities — the ID is the identity, the components are the content.

### Component — a row in a table, not a class

Open `src/ecs.hpp` and the components are three plain structs:

```cpp
struct Transform { float x, y, angle, halfSize; };   // where and how big
struct Spin      { float radiansPerSecond; };         // how fast the angle changes
struct Rgba      { float r, g, b, a; };               // what it looks like
```

Each component *type* is a table keyed by entity ID; an entity's component is its row. The
world's storage makes this literal:

```cpp
std::unordered_map<Entity, Transform> transforms;
std::unordered_map<Entity, Spin> spins;
std::unordered_map<Entity, Rgba> colours;
```

"Entity 1 has a Spin" means "`spins` has a key `1`". Presence is the whole of it — no
inheritance, no base class, no flags for "does it spin?".

The rule that makes this work: **a component never reads another component.** If behaviour
needs two components together, that behaviour is a system, not a component method.

Why `Transform` and `Spin` are separate rather than one `MovingSquare`: position and motion
vary independently. A static wall has `Transform` and no `Spin`. Because they are separate
tables, "rotating things" is expressible as a query — and that query is what a system
iterates.

### System — a function that runs over a set

```cpp
void rotationSystem(World& world, float dt);              // mutate
void renderSystem(const World& world, RenderList& out);   // read, emit
```

Read them as sentences:

- `rotationSystem`: *for every entity that has a Transform and a Spin, add
  `spin * dt` to its angle.*
- `renderSystem`: *for every entity that has a Transform and an Rgba, emit a quad.*

Neither function mentions squares, or this game at all. They know component types, not
entity kinds. That is why adding a second square would need **zero** changes to either
system: another row appears in the same tables, and the same queries pick it up.

The `const World&` on `renderSystem` is not decoration. It is the entire access
declaration: this system reads and never writes. In bigger ECS designs that same idea
becomes a declaration the scheduler uses to decide what may run in parallel.

### World — the database handle

`World` owns the maps and the ID counter and is passed to systems. It is not a god object:
it has no behaviour, no update method, no knowledge of squares. It is closer to a database
connection than to a game manager.

### The inversion, in one sentence

> OOP asks the object to act on itself. ECS selects the rows and acts on them.

## One frame, with real numbers

The loop is `main.mm -tick:`, and it does exactly three things: measure `dt`, call the
systems in order, ask for a redraw.

At startup, the world contains one entity:

| Table | Row |
|---|---|
| `transforms[1]` | `{x:0, y:0, angle:0, halfSize:120}` |
| `spins[1]` | `{radiansPerSecond: 0.3491}` (20°/s) |
| `colours[1]` | orange |

**Tick 1**, with `dt ≈ 0.01667` s (60 Hz):

1. `rotationSystem` joins `transforms` with `spins`. Entity 1 matches, so
   `angle = 0 + 0.3491 × 0.01667 = 0.005818` rad — 0.33°.
2. `renderSystem` joins `transforms` with `colours`. Entity 1 matches, so it appends
   `Quad{0, 0, 120, 0.005818, orange}` to the render list.
3. `-drawRect:` translates to the view centre, rotates by 0.005818 rad, and fills a
   240×240 rectangle.

After 60 ticks the angle is 0.3491 rad, or 20° — the square has turned at exactly the rate
its `Spin` component claims. Nothing in the systems knows "20° per second"; they only know
"spin times dt".

The join is the concept worth staring at. Add an entity with a `Transform` and a colour but
no `Spin`: `rotationSystem` skips it, `renderSystem` draws it. Same tables, different query,
different behaviour — and not one `if` about types.

## Now the code

| File | Lines | What it is |
|---|---|---|
| `src/ecs.hpp` | 105 | Entity alias, three components, `World`, `RenderList`, system declarations |
| `src/game.cpp` | 49 | The two systems, as free functions |
| `src/main.mm` | 169 | Cocoa window, view, 60 Hz timer, the game loop |
| `Makefile` | 17 | One compile command |
| `compile_flags.txt` | 2 | `-std=c++20`, `-fobjc-arc` for the editor |

`ecs.hpp` and `game.cpp` are plain C++ — no platform types, no AppKit. `main.mm` is the
only file that knows a window exists; the ECS hands it a list of quads and nothing else.
That split is the thing to protect, because it is what makes the ECS portable and the
platform layer replaceable.

| Noun | In this code | Notes |
|---|---|---|
| **Entity** | `ecs::Entity` = `std::uint32_t`; `World::nextEntity`; the square is entity `1` | Not an object. A number that happens to be a key in some maps. |
| **Component** | `Transform`, `Spin`, `Rgba` in `ecs.hpp` | Plain structs: no methods, no inheritance, no owned pointers. |
| **System** | `rotationSystem`, `renderSystem` in `game.cpp` | Free functions. Not classes, not overrides, not registered anywhere. |
| **World** | `struct ecs::World` — one map per component type, keyed by ID | The maps are the schema. |

### The frame order

`-tick` → `rotationSystem` → `renderSystem` → `-drawRect:` → pixels

The two systems never call each other, and the order is written out literally rather than
discovered from a registry. Rotating before rendering is correct here; rendering before
rotating would show last frame's angle. That ordering decision is part of the design, and
at this size the honest place to record it is the call site.

## Is ECS just those four nouns?

Short answer: **the four nouns are the vocabulary minimum, not the whole model.** They are
a good answer to "where does this code go?" and a poor answer to "how does a system find
its data, and when does it run?" This game makes both gaps visible.

1. **Queries.** A system needs a way to say "every entity with Transform and Spin". Here
   that is a hand-written loop with a `find` — a query expressed as control flow. At any
   real scale the repeated join is extracted into a query helper, and query becomes the
   fifth noun, or the verb the four nouns were missing. It is the piece most ECS libraries
   exist to provide.
2. **Entity lifecycle.** `createEntity()` hands out IDs and never recycles them, so there is
   no `destroy` and no dangling-ID problem. That changes the moment a bullet despawns: an ID
   plus a generation (`{index, version}`) is how a real ECS makes a stale handle detectable
   instead of silently aliasing a new entity. "Entities are IDs" is true and incomplete —
   they are *handles*, and handles need validation.
3. **Resources.** Not everything is per-entity. `dt` is one (passed as an argument here);
   `RenderList` is another (owned by the view controller). Time, input, a camera, a random
   source, the tile map — singletons that systems read. The four nouns have no place for
   them, which is why every ECS grows a `Resource` concept.
4. **Execution.** The nouns say nothing about ordering, yet ordering is where the bugs live.
   Here it is a hardcoded call order. In larger designs systems declare queries and access
   sets, and a scheduler derives order (and parallelism) from them — a *consequence* of the
   nouns, not one of them.
5. **Deferred structure changes and events.** Spawning or destroying entities mid-iteration
   breaks the iteration. Real ECS designs queue those changes and add events for
   system-to-system messages. Neither is needed for one square; both are needed as soon as
   collision kills something.
6. **Storage strategy.** One map per component is the naive layout. Sparse sets and
   archetypes change performance, not meaning — the most-written-about part of ECS and the
   least relevant here.

Two refinements to the slogan itself:

- "Components are data" is a rule about *ownership*, not purity. A component may have a
  helper method; what matters is that it never reads another component.
- "Systems are functions" is true at this scale and slightly false at scale. In Bevy or
  flecs a system is best understood as a *declaration* — query, access set, function — that
  a scheduler runs. The function is the smallest part of it.

**So what is ECS, really?** A data model (the four nouns) plus a query mechanism plus an
execution model. The nouns shape the state; queries and ordering shape the behaviour. That
is why two libraries with identical vocabulary can feel completely different.

## What this game deliberately does not have

No query helper, no entity destruction, no resources struct, no scheduler, no events, no
archetypes, no templates, no shared library. Each is added when a game's actual problem
forces it — see the rule of three in [`Docs/PURPOSE.md`](../../Docs/PURPOSE.md) §7. The
next game is where copying this structure *by hand* is the correct move.

## Things worth trying

- **Add a second square**: call `spawnSquare` twice with different positions and colours.
  Both systems stay untouched — that is the lesson.
- **Delete the Spin row**: remove `world.spins[e] = ...` in `spawnSquare`. The square stops
  rotating but still renders. The join, made visible.
- **Make one square static**: spawn two, then `world.spins.erase(second)`. One turns, one
  doesn't, and no system was edited.
- **Reverse the direction**: change `20.0f` to `-20.0f`. Direction is data, not a code path.
- **Break the `dt` clamp**: allow 5 s and drag the window. The square jumps. The fix belongs
  in the loop, not in a system — a small preview of why execution order and timing are part
  of the design.

## Editor setup

The compiler flags live in the `Makefile`, which editors do not read. Two editor-side files
restate the ones that matter, or the squiggles lie about code that builds fine:

- `compile_flags.txt` (here) — `-std=c++20` and `-fobjc-arc`, for clangd, which searches
  upward from the source file.
- `.vscode/c_cpp_properties.json` (repo root) — `"cppStandard": "c++20"` and
  `"compilerArgs": ["-fobjc-arc"]`, for Microsoft's C/C++ extension.

Without them, two false alarms appear:

- **"decomposition declarations are a C++17 extension"** in `game.cpp` — the engine fell
  back to C++14. Reproduce with `c++ -std=c++14 -Isrc -fsyntax-only src/game.cpp`.
- **"cannot create __weak reference in file using manual reference counting"** in
  `main.mm` — the engine did not apply ARC. Reproduce with
  `c++ -std=c++20 -Isrc -fsyntax-only -x objective-c++ src/main.mm`; adding `-fobjc-arc`
  clears it.
