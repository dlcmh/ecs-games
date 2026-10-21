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

The longer ambition is to learn **how ECS scales, from one entity to AAA-scale worlds**:
storage layout, parallel scheduling, streaming, determinism, tooling. That climb is a
ladder of rungs in [`Docs/PURPOSE.md`](Docs/PURPOSE.md) §6, and a rung is only entered
when a game in this repository forces it. Do not import machinery from a higher rung
because it seems modern; the current red lines are scoped to rungs 0–1.

Each game lives in its own directory under `games/`, numbered in build order. Read
[`Docs/PURPOSE.md`](Docs/PURPOSE.md) before making design decisions — it is the charter,
and it takes precedence over convenience.

Key constraints from that charter:

- No generic ECS. No templates, archetypes, sparse sets, or plugin registries.
- No engine, no shared game library, no reusable ECS before a third game exists.
- Systems are free functions, invoked in a hardcoded order in the game loop.
- Each game: one directory, roughly 1,000 lines, `make run`.
- Copy-paste between games is correct until the same code appears three times.
- Lifting a red line requires the forcing game to exist and the lift to be recorded in
  the charter.

## Editor configuration

**Every game must open with zero editor diagnostics, and this is never something the user
has to ask for.** When a game is added, ship its editor configuration in the same commit.

- [`.vscode/c_cpp_properties.json`](.vscode/c_cpp_properties.json) is the shared, checked-in
  configuration for Microsoft's C/C++ extension. It globs
  `${workspaceFolder}/games/**/src`, so a new game under `games/` inherits C++20,
  `-fobjc-arc`, and the include path automatically. Do not add per-game copies of it.
- Each game still needs its own `compile_flags.txt` next to its `src/` (`-std=c++20`,
  `-fobjc-arc`). clangd searches upward from the source file and does **not** read
  `.vscode/`, so this is the one editor file that lives with the game rather than in
  `.vscode/`.
- If a game needs a flag the shared configuration cannot express — a framework header
  path, a define, a different standard — extend `.vscode/c_cpp_properties.json` so it
  applies to all games, or the game's `compile_flags.txt`, in the same commit that
  introduces the need.
- A game that builds but squiggles is not done. Verify with
  `clangd --check=<file>` on each source file before calling it finished.

## Game README pedagogy

These rules govern every game's `README.md`. They accumulate: a new rule is appended when it
is agreed, and existing READMEs are brought into line when that happens.

### Rule 1 — No fluff

Every sentence must state a requirement, a behaviour, a mechanism, or a fact about the
code. Delete anything else.

Specifically forbidden:

- Rhetorical questions as section headings or transitions ("Why not just a `Square` class?").
- Motivational or reassuring lines ("this is the important part", "the lesson is…",
  "notice how…", "the square is the smallest payload that proves it works").
- Restating the same idea in a second register — one precise statement, not a plain one
  plus a clever one. "The window is outside ECS" already says it; a following "ECS has no
  home for it" adds words but no information.
- Forward references to games not yet written, or claims about how "this will matter later".
- Analogies that do not carry a testable claim (an analogy is allowed only if it states
  something exact, such as the database/primary-key mapping).
- Second-person instructions outside the exercises section.
- Adjectives about the design (elegant, clean, honest, powerful).

Use concrete numbers, symbol names, file paths, and commands instead. If a sentence can be
deleted with no loss of information, it should be.

### Rule 2 — State the game's requirements, then stop

The README opens with what the game must do, as stated in the original request, written as
the article's opening paragraph under no heading (Rule 3). No interpretation, no
justification. The opening ends there.

- Requirements are about the game: what it must show or do.
- Meta-requirements do not belong here: how the README is structured, what the folder is
  named, or what the repository process requires.
- Repository-wide constraints (no engine, `make run`, size) are a separate paragraph, so
  the user's request is not blended with later additions.

### Rule 3 — Long-form article

The README is one long-form article: a title, then paragraphs. Structure comes from the order
of the paragraphs, not from section headings.

- No headings below the title. No bullet points, numbered lists, tables, checklists,
  definition lists, or block quotes.
- Identifiers, paths, and commands stay inline as code spans.
- An enumeration becomes a run of clauses ("the stages are input, movement, collision, and
  render"); a sequence becomes one paragraph naming the order; a mapping states both sides in
  the same sentence; a sub-point becomes another paragraph.

### Rule 4 — Architect's decomposition

After the requirements, the article shows how an architect breaks the requirement statement
into elements and classifies each one, either as an ECS element or explicitly as outside
ECS.

- Begin with the elements the requirement names, then add the ones it implies but does not
  say: a window to show in, a clock to advance the rotation, a surface to draw on, a frame
  that ties clock to window.
- Name which elements exist because of the architect's experience rather than the text.
  The clock, the frame loop, and the transfer of draw data to the platform are the usual
  cases at this size.
- Split a requirement word that hides two elements. "Slowly rotating" contains a behaviour
  (a system) and a rate (a component); an architect separates them before choosing code.
- For every element state its classification and the reason: entity when it needs
  independent identity, lifetime, and iteration; component when it is per-entity data;
  system when it is behaviour over a set; world when it is the container.
- "Outside ECS" is a classification, not a gap. Say what the element is instead — platform
  object, execution input, presentation configuration, transient buffer, build concern —
  and why it cannot be an ECS element.

### Rule 5 — Plain speech

The article should read like one person explaining the work to another, not like a paper
abstract.

- Use concrete nouns and active verbs: "the loop calls the systems", not "the systems are
  invoked by the loop".
- Never explain by tautology. "A program that shows something needs somewhere to show it"
  says nothing; "the square has to be drawn into a window" says what the window is for.
- Drop stiff connectives and nominalisations: thus, hence, thereby, whereby, whilst,
  "the implementation of", "the utilisation of", "in order to".
- Prefer "is" and "does" to "constitutes" and "performs". Contractions are allowed where
  they read naturally.
- This rule governs how things are said, not what is said; Rule 1 still decides what belongs.

### Rule 6 — No code excerpts

The article names code; it does not reproduce it.

- No fenced code blocks, no copied struct definitions, no function signatures, no
  line-by-line walkthroughs, no pseudocode.
- Refer to identifiers as inline code: `rotationSystem`, `Spin::radiansPerSecond`,
  `World::transforms`.
- Describe what the named thing does in a sentence. The reader opens the file for the body.
- Commands such as `make run` stay inline as well.

### Rule 7 — No taxonomy labels

Do not give the paradigm a group name. "The four nouns" asks the reader to hold a taxonomy
before they have met the things it counts.

- Name the concrete element: an entity, a component, a system, the world.
- No collective nouns, no counts of concepts, no unexplained acronyms.
- If a sentence only works because of the group name, rewrite the sentence.

### Rule 8 — Explain by construction

Build the explanation the way the program was built: state the goal, name the parts it
needs, then classify each part.

- Put the need before the verdict: "to show the square we need a window" comes before "the
  window is a platform object".
- Then give the verdict and the reason in that order: "it's an entity, because it's the
  thing that exists and can be referred to on its own".
- Write as the builder: "we need", "we pick", "we make this a component". First-person
  plural carries the explanations; second person stays for exercises.
- Don't compress a decision into a label. "'Shows' splits in two, one half inside ECS and
  one outside" states the conclusion with the reasoning removed. Say what each half does and
  why it lands where it does.
- Discourse markers that mark a step — "so", "now", "then", "well" — carry the reasoning and
  are not fluff.

### Rule 9 — Talk to an audience

Write as an architect presenting the design to a room, not as a specification read aloud.

- Prefer speech to note form: "The requirements mention the square and the rotation, but not
  the clock — well, the clock happens to be required to make the rotation visible" rather
  than "The requirements name A and B but not C".
- Mark the beat where the audience would ask a question: "well", "of course", "turns out",
  "happens to be", "which is why".
- Let a sentence run to the length of a spoken breath. Three clipped declaratives in a row
  read as a table, not an explanation.
- The marker has to carry reasoning, not padding: "well" is allowed when it introduces the
  answer to the objection the previous sentence raises.

### Rule 10 — Talk about requirements, not sentences

The requirements are the subject; their wording is not.

- "The requirements mention the square and the rotation", not "the sentence mentions".
- "The requirement for slowness hides two things", not "the word 'slowly' hides two things".
- "Showing the square needs two parts", not "the word 'shows' needs two parts".
- Quote requirement text only when the exact wording matters; otherwise describe what is
  asked for. "This detail" is available when a single clause is meant.

## Teaching code

Separate from the README rules, the source itself must stay teachable.

- Keep it minimal: only what the concept needs. Guards that express the mechanism stay
  (the join, the `dt` clamp, `const` access); guards against states the game cannot reach
  go. Tooling — inspectors, overlays, debug menus — arrives when a bug requires it.
- Happy path first: no pre-emptive defense against undefined behaviour, invalid input,
  overflow, null dereference, allocation failure, or misuse; no "unsafe in production"
  comments. Teaching addition is `a + b`, not overflow-checked builtins. A hazard is taught
  when the hazard is the lesson.
- Both points keep the current lesson visible. Neither forbids teaching the hazard later,
  with its own code.

## Environment

- Approval prompts are disabled in agent sessions; never request sandbox escalation.
