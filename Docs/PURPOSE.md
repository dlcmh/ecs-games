# Purpose

This repository is a learning vehicle, not a product. Its purpose is to build one
small, complete game — Pac-Man — using a hand-written, deliberately tiny ECS in C++,
so that the ECS mental model can be learned by reading and changing a real game
end-to-end.

Everything in this document constrains what the project is allowed to become.
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
system-level behavior.** That is precisely the ECS mental model. A ghost chasing
Pac-Man via a target tile is the same shape as a T-cell following a cytokine gradient.

Tutorial-style game code is the opposite: a god-function that grows a branch per
entity type. That is why it fails to stick. It is not too hard; it is structured wrong
for the reader.

Therefore this project starts at ECS rather than at "level 2" and refactoring later.
The "ECS is overkill for four ghosts" critique is a *cost* argument, not a *fit*
argument — and if the cost of the alternative is abandoning the project again, then
ECS is by far the cheaper path.

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
2. **Structural protection against rot.** Adding a ghost personality becomes adding a
   component or a system, not editing existing systems. The Open/Closed principle
   enforced by data layout instead of class hierarchies — the same instinct as *add a
   new ERP module, don't modify the ledger*.
3. **Scaling down better than commonly claimed.** ECS is only overkill when it means
   archetypes, sparse sets, and query caching. A tiny ECS for Pac-Man is roughly 150
   lines, less code than the OOP hierarchy and far more inspectable.
4. **Explicit data flow.** Every system can be read in one sitting.

## 4. What ECS does not give

- **Game design.** ECS says how to organize entities, components, and systems; it does
  not say what the systems should be. The real content of Pac-Man — ghosts pick a
  target tile, a shared pathfinder moves them, mode is an orthogonal state machine —
  still has to be understood. ECS removes architectural churn, not domain learning.
- **Easy debugging by default.** In OOP one breaks inside `Ghost::update`. In ECS one
  must ask which system touches an entity, and in what order. That is why the entity
  inspector exists from day one.

## 5. Scope

The artifact is a **single directory, roughly 1,000 lines, `make run`, done.**

Build order:

1. **Write the tiniest possible ECS.** Entities as `uint32_t`; components as structs in
   `std::unordered_map<Entity, T>` (or `std::vector<T>` indexed by entity where entities
   are dense); systems as free functions. Target ~150 lines for the ECS core. No
   templates, no archetypes, no reuse.
2. **Model Pac-Man in ECS terms from the start.** System order:
   `inputSystem` → `modeTimerSystem` → `targetingSystem` → `movementSystem` →
   `collisionSystem` → `scoringSystem` → `renderSystem`.
   For each stage, check: does it have clear inputs/outputs? Does it match the
   pipeline instinct? If yes, the paradigm is already fluent.
3. **Use biological intuition for the tricky parts.** Frightened-mode timer is a
   hormone decay curve. Ghost-house release timer is a refractory period. Pinky's
   four-tile lookahead is feed-forward prediction. These analogies carry reasoning when
   the code is unfamiliar.
4. **Build the entity inspector on day one** — `printEntity(world, id)` dumping all
   components. About 20 lines; analogous to logging request IDs in a web pipeline.
5. **When ECS handles something awkwardly** (the maze is arguably not an entity), that
   is the lesson: it teaches the boundary of the paradigm better than any tutorial.

## 6. The discipline: the survey-form rule

The best available analogy for the failure mode is deploying Kubernetes and hardened,
containerized Java microservices to serve a one-time web survey form. It maps
one-to-one onto ECS traps:

| Over-engineering sin | Web survey version | ECS version |
|---|---|---|
| Premature generality | "Build a form-builder framework so any future survey can reuse it" | "Make components `template<typename... Ts>` so any future game can reuse the ECS" |
| Ceremony over substance | Kubernetes for one static page plus a Google Form | Archetype storage with sparse-set iteration order for four ghosts |
| Infrastructure as identity | "We're a containerized microservices shop" (about a one-page survey) | "We're an ECS codebase" (about Pac-Man) |
| The framework becomes the project | Three weeks of deploy tooling, two hours of survey | Three weeks of ECS core, two days of game |
| Onboarding cost | A new developer learns Helm charts to change a question | A new developer learns query caching to add a ghost |
| The tell | The survey cannot be demoed without explaining the infrastructure | Ghost behavior cannot be shown without explaining the storage layer |

The punchline of both: **the moment the scaffolding needs its own documentation, it has
become the product.** Separate documents explaining "ghost targeting" and "BFS" mean the
code does not speak for itself.

### The cure: YAGNI and the rule of three

Do not abstract until the concrete thing has been written three times.

1. **Write the concrete systems first, with no ECS framework** — `targetingSystem`
   operating on a `std::vector<Ghost>` and a `PacMan`. Get the game right.
2. **Notice the repeated shape** — every system is "iterate entities that have
   components X, Y, Z; mutate W." That is the abstraction. It emerges from the code; it
   is not pre-invented.
3. **Only then extract the ~150-line ECS**, and only the parts the actual systems need.
   No runtime component removal unless it is needed. No variadic queries unless three
   components are genuinely queried together.

Kubernetes after three services, never for one survey.

## 7. Red lines

Forbidden, on sight:

- ❌ `template<typename... Components>` in a query — a `for` loop over `positions` with
  `if (has<Velocity>(e))` is fine.
- ❌ Archetype storage, chunked memory, SoA layout — that is an optimization for 10k+
  entities. There are about ten. A map per component is correct.
- ❌ A `System` base class with `virtual void update(World&)` — that is OOP reinvented
  inside ECS. Systems are free functions. A registry is a
  `std::vector<std::function<void(World&)>>` *if* runtime ordering is ever needed; it
  is not.
- ❌ Serialization, reflection, editor integration — the Kubernetes of ECS.
- ❌ A `Docs/` tree that grows beyond charters and notes like this one.

Required:

- ✅ Entities as `uint32_t`, components as plain structs, systems as free functions, one
  `World` struct holding the maps.
- ✅ An entity inspector.
- ✅ Hardcoded system order in the game loop — `modeTimer(); targeting(); movement();
  collision();` — not a plugin registry.

Meta-rule: **if the ECS cannot be explained in one sentence per noun — entities are IDs,
components are data, systems are functions, world is the container — it has been
over-built.** The survey-form equivalent is "it is an HTML form and a submit handler."
If the explanation needs a diagram, the project has become the thing it was escaping.

## 8. On the claim of one paradigm for everything

"ECS for small, large, or AAA games using one singular methodology" is *almost* true.
ECS is a genuine unifying paradigm, used heavily in AAA (Unity DOTS, Unreal Mass, Bevy,
flecs). But:

- Small or simple games often do not need ECS, and forcing it adds ceremony. Feeling
  that friction is data, not failure.
- AAA games are ECS *plus* much else — asset pipelines, streaming, netcode, physics
  middleware — where ECS is one layer, not the whole. The "one methodology" framing is
  aspirational.
- Some problems are genuinely not ECS-shaped: UI layout, scripting VMs, dialogue trees.
  ECS gets composed with other patterns.

None of that undermines learning ECS first. Learning one paradigm deeply and then
discovering its boundaries is far better pedagogy than sampling five shallowly, and ECS
has the widest applicability across game sizes.

## 9. Definition of done

- A Pac-Man that runs and is playable.
- Approximately 1,000 lines total, one directory, `make run`.
- System order visible and hardcoded in the main loop.
- An entity inspector reachable while the game runs.
- Every line readable in one sitting; no hidden control flow.
- No `Docs/` folder beyond this charter.

Then **stop**. The stopping is the skill. The survey-form instinct — knowing when the
artifact is disposable and complete — is the same instinct that identifies
over-engineering, and it applies to game development too.
