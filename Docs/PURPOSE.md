# Purpose

This repository is a workshop, not a product. Its purpose is to **build a series of
games from scratch, each one written by hand on top of the ECS paradigm**, until
thinking in entities, components, systems, and pipelines is the default way game code
is written here.

No single game is the goal. The ambition is to learn **how the ECS paradigm scales** —
from a window showing one rotating square, through small complete games, up to the
concerns that only surface at AAA scale: tens of thousands of entities, parallel system
execution, storage layout, streaming worlds, and the tooling that surrounds them. Each
rung of that ladder is reached because a game demanded it, not because a blog post
recommended it.

The durable output of this repository is not a game. It is the ability to sit down in
an empty directory and build the next game from scratch — no engine, no framework, no
tutorial — and to know, from having felt the pressure, which ECS machinery the problem
in front of you actually requires and which is ceremony.

Everything in this document constrains what that work is allowed to become.
If the code and this document disagree, this document wins.

## 1. Why ECS, and why first

Previous attempts at learning game development stalled "a few pages or lines of code
in." That is not a knowledge gap. It is a paradigm mismatch: mainstream tutorials are
written imperatively — update each entity in a loop, add an `if` branch for each type —
and that structure is the opposite of how this author already reasons about systems.

The domains already understood fluently:

| Domain | What is actually understood |
|---|---|
| Web request/response | A pipeline of stateless stages, each with clear inputs/outputs, composed in order |
| Oracle ERP | A set of orthogonal modules that transform records, with explicit integration points |
| Human energy system | A state machine over substrates (glycogen → fat, aerobic/anaerobic thresholds) with feedback loops and rate limiting |
| Immune response to a virus | A staged pipeline (innate → adaptive) with agents (T-cells, antibodies) that have local rules producing emergent system behavior |

The common shape is: **entities with local rules, processed in stages, producing
system-level behavior.** That is precisely the ECS mental model. An enemy homing on the
player via a target tile is the same shape as a T-cell following a cytokine gradient.

Tutorial-style game code is the opposite: a god-function that grows a branch per
entity type. That is why it fails to stick. It is not too hard; it is structured wrong
for the reader.

Therefore this work starts at ECS rather than at "level 2" and refactoring later.
The "ECS is overkill for a handful of entities" critique is a *cost* argument, not a
*fit* argument — and if the cost of the alternative is abandoning the work again, then
ECS is by far the cheaper path.

The choice of ECS is also the choice of a *repeatable* method. The point of learning one
paradigm deeply is that every subsequent game starts from the same four nouns, so
effort goes into the game rather than into relearning an architecture. The first game
proves the method works. The second proves it transfers. The third proves it is a method
rather than a coincidence.

## 2. The model to internalize

ECS has a canonical shape that fits in one's head:

- **Entities** — IDs. Just integers.
- **Components** — plain data structs, no behavior.
- **Systems** — functions that query entities with certain components and mutate them.
- **World** — the container that holds all of it.

Four nouns, one verb. It is *more* uniform than OOP, which carries inheritance,
virtual dispatch, composition, interfaces, and lifetime rules. For a reader who thinks
in pipelines and orthogonal modules, ECS is the simpler model; it is merely unfamiliar
in game-dev circles.

The mapping onto things already known is exact:

| ECS concept | Equivalent already familiar |
|---|---|
| System | A middleware stage: input shape, output shape, fixed order |
| Query ("all entities with Position + Target") | SQL `SELECT ... WHERE`; sets and predicates are ERP muscle memory |
| Component composition | Joining tables; an entity with Position+Target+Personality is a row across three tables |
| System ordering | Pipeline ordering (auth → routing → handler → response); stages are ordered and stateless per call |

The work is therefore mostly *renaming* what is already done, not learning something
new from scratch.

## 3. What ECS buys this project

1. **A canonical, holdable shape.** Uniform nouns and one verb; no hidden `this`.
2. **Structural protection against rot.** Adding an enemy personality becomes adding a
   component or a system, not editing existing systems. The Open/Closed principle
   enforced by data layout instead of class hierarchies — the same instinct as *add a
   new ERP module, don't modify the ledger*.
3. **Scaling down better than commonly claimed.** ECS is only overkill when it means
   archetypes, sparse sets, and query caching. A tiny ECS is roughly 150 lines, less
   code than an OOP hierarchy and far more inspectable.
4. **Reapplication.** Because the shape is fixed, a new game is a new set of components
   and systems, not a new architecture. That is what makes "build the next game from
   scratch" a realistic plan instead of a fresh start each time.
5. **An honest scaling path.** The same nouns hold at one entity and at ten thousand;
   what changes is the machinery around them. That makes "small to AAA" a sequence of
   concrete, forceable steps rather than an ambition.
6. **Explicit data flow.** Every system can be read in one sitting.

## 4. What ECS does not give

- **Game design.** ECS says how to organize entities, components, and systems; it does
  not say what the systems should be. The real content of a game — who chases whom, how
  paths are found, when modes change — still has to be understood. ECS removes
  architectural churn, not domain learning. Every game brings its own domain problem,
  and that problem is the point.
- **Easy debugging by default.** In OOP one breaks inside `Enemy::update`. In ECS one
  must ask which system touches an entity, and in what order. That is why the entity
  inspector exists from day one of every game.
- **Performance for free.** A map per component is the right first implementation and
  the wrong last one. Which one you need is a question for a profiler, not for
  convention — see §6.

## 5. How the work is organised

Each game is **one self-contained directory, roughly 1,000 lines, `make run`, done.**
No shared engine, no shared runtime library, no build system spanning games. Games may
copy small pieces of each other by hand — that is deliberate; copying is how the
repeated shape becomes visible.

Every game follows the same build order:

1. **Write the tiniest possible ECS** in that game's directory. Entities as `uint32_t`;
   components as structs in `std::unordered_map<Entity, T>` (or `std::vector<T>` indexed
   by entity where entities are dense); systems as free functions. Target ~150 lines for
   the ECS core. No templates, no archetypes, no reuse.
2. **Model the game in ECS terms from the start.** Write the system pipeline down before
   writing the systems — the equivalent of naming the middleware stages first.
3. **Use existing intuition for the tricky parts.** Biological and web-pipeline
   analogies carry reasoning when the code is unfamiliar: decay timers are hormone
   curves, spawn delays are refractory periods, lookahead is feed-forward prediction.
4. **Build the entity inspector on day one** — `printEntity(world, id)` dumping all
   components. About 20 lines; analogous to logging request IDs in a web pipeline.
5. **When ECS handles something awkwardly**, that is the lesson. It teaches the boundary
   of the paradigm better than any tutorial, and the boundary is knowledge the next game
   inherits.

### Exercise 0: a rotating square

Before any game logic, prove the boring layer works: a window, a 60 Hz tick, a `dt`
clamped against stalls, and one component-driven value reaching the screen
([`games/01-rotating-square`](../games/01-rotating-square)). Two systems, one order, no
queries worth extracting. Its README records the one question this raised — whether ECS
really is only four nouns — and the answer, which is that the nouns describe the data
model and leave out queries and execution.

### Exercise 1: a first complete game

Small enough to finish, large enough to force the first missing pieces: real input,
entities appearing and disappearing, several systems with a meaningful order, and a win
or loss condition. Suggested pipeline to write down before the systems themselves:

`inputSystem` → `aiSystem` → `movementSystem` → `collisionSystem` → `spawnSystem` →
`scoringSystem` → `renderSystem`

For each stage, check: does it have clear inputs/outputs? Does it match the pipeline
instinct? If yes, the paradigm is already fluent.

Domain specifics that belong to the game rather than to ECS: how enemies choose
targets, whether a pathfinder is shared, which modes exist and how they transition, and
what in the world is not an entity at all. That awkwardness is worth feeling.

### After the first game

The next game changes one variable, not all of them: a different problem shape, written
from scratch in a new directory, using the same four nouns. Finishing one game and
stopping would fail this document's purpose — it would reproduce the original failure at
a larger scale.

## 6. Scaling the paradigm: one square to AAA

The repository's learning curve is the ladder below. It is not a reading list; it is a
sequence of games, each of which cannot be completed without the capability in the right
column. A rung is only claimed when a game has forced it, and that game exists in this
repository.

| Rung | Scale | What the game forces | What the ECS gains |
|---|---|---|---|
| 0 | 1 entity | Prove the platform: window, tick, `dt`, one component reaching the screen | Maps as storage, two systems, hardcoded order. **Current.** |
| 1 | Tens to hundreds | A first complete game: input, entities dying and spawning, collisions, score | Extracted queries, entity destruction with generation-validated handles, deferred spawn/despawn, the first events |
| 2 | Thousands | A bullet-heavy or crowd game: many overlapping entities, level data, persistence | Spatial partitioning/broadphase, resources (time, input, camera), an explicit schedule, scene load/save |
| 3 | Tens of thousands to AAA | A world too big to fit the previous rung: streaming, tools, determinism | Archetype or sparse-set storage chosen against a benchmark, parallel systems with declared access, streaming, netcode-ready determinism, editor and inspector tooling |

Three rules govern the climb:

1. **One rung at a time.** Each step adds exactly one capability, forced by a concrete
   game. Skipping a rung produces machinery with no problem attached — the survey-form
   failure again, one level up.
2. **The rule of three still applies.** Extraction happens when the third game repeats
   the same code, not when the ladder suggests it might. The ladder says *what becomes
   legitimate*; the rule of three says *when*.
3. **Lift red lines in writing.** The red lines in §8 describe rungs 0–1. When a game
   forces a lift — say, archetype storage at rung 3 — the change is recorded here, next
   to the game that forced it, with the measurement that justified it. Crossing a red
   line without that record is the failure mode this document exists to prevent.

AAA is not a separate curriculum. It is this ladder continued: the same four nouns, with
storage, scheduling, streaming, and tooling added under pressure. The point of climbing
slowly is not caution for its own sake — it is that at every rung, the author knows
exactly which problem the added machinery solves, because they have felt that problem in
their own code.

## 7. The discipline: the survey-form rule

The best available analogy for the failure mode is deploying Kubernetes and hardened,
containerized Java microservices to serve a one-time web survey form. It maps
one-to-one onto ECS traps:

| Over-engineering sin | Web survey version | ECS version |
|---|---|---|
| Premature generality | "Build a form-builder framework so any future survey can reuse it" | "Make components `template<typename... Ts>` so any future game can reuse the ECS" |
| Ceremony over substance | Kubernetes for one static page plus a Google Form | Archetype storage with sparse-set iteration order for a handful of entities |
| Infrastructure as identity | "We're a containerized microservices shop" (about a one-page survey) | "We're an ECS codebase" (about a small game) |
| The framework becomes the project | Three weeks of deploy tooling, two hours of survey | Three weeks of ECS core, two days of game |
| Onboarding cost | A new developer learns Helm charts to change a question | A new developer learns query caching to add an enemy |
| The tell | The survey cannot be demoed without explaining the infrastructure | Entity behavior cannot be shown without explaining the storage layer |

The punchline of both: **the moment the scaffolding needs its own documentation, it has
become the product.** Separate documents explaining "enemy targeting" and "BFS" mean the
code does not speak for itself.

### The cure: YAGNI and the rule of three

Do not abstract until the concrete thing has been written three times.

1. **Write the concrete systems first, with no ECS framework** — `targetingSystem`
   operating on a `std::vector<Enemy>` and a `Player`. Get the game right.
2. **Notice the repeated shape** — every system is "iterate entities that have
   components X, Y, Z; mutate W." That is the abstraction. It emerges from the code; it
   is not pre-invented.
3. **Only then extract the ~150-line ECS**, and only the parts the actual systems need.
   No runtime component removal unless it is needed. No variadic queries unless three
   components are genuinely queried together.

The same rule governs *between* games, which is where the temptation is strongest. Two
games sharing code is a coincidence. Three games sharing the same code is a pattern, and
only then may a small shared core be extracted — still small, still the parts the games
actually needed. Until the third game, each directory is written independently and
copy-paste is the correct engineering choice.

Kubernetes after three services, never for one survey.

## 8. Red lines (rungs 0–1)

Forbidden, on sight, until a game at a higher rung forces the lift and §6 records it:

- ❌ `template<typename... Components>` in a query — a `for` loop over `positions` with
  `if (has<Velocity>(e))` is fine.
- ❌ Archetype storage, chunked memory, SoA layout — that is an optimization for 10k+
  entities. At this rung a game has tens. A map per component is correct.
- ❌ A `System` base class with `virtual void update(World&)` — that is OOP reinvented
  inside ECS. Systems are free functions. A registry is a
  `std::vector<std::function<void(World&)>>` *if* runtime ordering is ever needed; it
  is not.
- ❌ An engine, a shared game library, or a "reusable" ECS written before the third
  game exists.
- ❌ Serialization, reflection, editor integration — the Kubernetes of ECS at this rung.
- ❌ A `Docs/` tree that grows beyond charters and notes like this one.

Required:

- ✅ Entities as `uint32_t`, components as plain structs, systems as free functions, one
  `World` struct holding the maps — re-derived per game until the rule of three allows
  otherwise.
- ✅ An entity inspector in every game.
- ✅ Hardcoded system order in the game loop — `movement(); collision();` — not a plugin
  registry.

Meta-rule: **if the ECS cannot be explained in one sentence per noun — entities are IDs,
components are data, systems are functions, world is the container — it has been
over-built.** The survey-form equivalent is "it is an HTML form and a submit handler."
If the explanation needs a diagram, the project has become the thing it was escaping.

## 9. On the claim of one paradigm for everything

"ECS for small, large, or AAA games using one singular methodology" is *almost* true.
ECS is a genuine unifying paradigm, used heavily in AAA (Unity DOTS, Unreal Mass, Bevy,
flecs). But:

- Small or simple games often do not need ECS, and forcing it adds ceremony. Feeling
  that friction is data, not failure — and a game here is allowed to be small enough
  that the ceremony is visible.
- AAA games are ECS *plus* much else — asset pipelines, streaming, netcode, physics
  middleware — where ECS is one layer, not the whole. The "one methodology" framing is
  aspirational.
- Some problems are genuinely not ECS-shaped: UI layout, scripting VMs, dialogue trees.
  ECS gets composed with other patterns.

None of that undermines learning ECS first. Learning one paradigm deeply and then
discovering its boundaries is far better pedagogy than sampling five shallowly, and ECS
has the widest applicability across game sizes. Discovering where it stops working —
game by game, not by reading — is part of the curriculum.

## 10. Definition of done

Per game:

- The game runs and is playable.
- Approximately 1,000 lines, one directory, `make run`.
- System order visible and hardcoded in the main loop (until §6 says otherwise).
- An entity inspector reachable while the game runs.
- Every line readable in one sitting; no hidden control flow.
- No `Docs/` folder inside the game's directory.

For the repository:

- More than one finished game, each built from scratch rather than adapted from the
  last.
- Evidence that the paradigm transfers: a game whose problem shape is unlike the first
  game's, expressed in the same four nouns.
- At least one game that has climbed a rung of §6, with the forcing problem and any
  lifted red line recorded alongside it.
- If, and only if, the third game repeats the same code, a small extracted core — with
  the games still runnable standalone.

Meeting the first game's items is the first milestone. It is not the goal, and it is not
a reason to stop. The stopping that matters here is knowing when a *game* is complete and
disposable; the repository itself keeps going, one rung at a time.

## 11. Where the work stands

- `games/01-rotating-square` — rung 0, done. Window, 60 Hz loop, `Transform` + `Spin` +
  `Rgba`, two systems, one entity. The platform layer is proven; no game logic yet.
- Next: exercise 1, a first complete game — the rung where queries, entity destruction,
  and collision first become necessary.
