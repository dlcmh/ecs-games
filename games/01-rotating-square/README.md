# 01 — rotating square

A launchable C++ program that shows a slowly rotating square.

Read as an architecture rather than an instruction, that sentence decomposes into parts, and
the parts fall into three groups: those the text names, those it implies, and those it does
not contain but which experience says must exist. The text names the square and the
rotation, and qualifies the rotation with "slowly". It implies a window, because a program
that shows something needs somewhere to show it, and a surface of pixels, because a window
alone displays nothing. It does not name the clock that makes rotation observable, the frame
loop that connects clock to window, or the channel that carries drawable data from the game
state to the platform; an architect supplies those because every program of this shape
needs them.

The square is an entity. It is the thing that exists, persists across frames, and can be
referred to individually. The requirement asks for exactly one, and the shape allows any
number. Its identity is not its geometry: the entity is the number `1`, which
`World::nextEntity` handed out when `spawnSquare` created it, and removing every component
from entity `1` would leave the ID still existing.

Position, size, and current orientation are a component, `Transform`. They belong to the
square, change together, and mean nothing on their own. `World::transforms` holds one row
per entity, and presence in that map is what "has a position" means.

The orange is a component, `Rgba`. The requirement names no colour, so the colour is an
addition rather than a derivation, but once present it is per-entity data: it can differ
between two squares, it does not act, and nothing addresses it independently. It is not an
entity because it has no identity or lifetime of its own, and not a system because it does
no work.

Rotation is a system, `rotationSystem`. It acts on every entity that has both a `Transform`
and a `Spin`, advancing each angle by its own rate times `dt`, and it mentions neither
squares nor colours. It is not a component because it is behaviour rather than data, and not
an entity because it has no identity and nothing iterates over rotations as a set.

"Slowly" hides two elements. The behaviour is the system; the pace is data,
`Spin::radiansPerSecond`, set to 20 degrees per second by `spawnSquare`. The rate is a
component because it varies per entity and the system must accept any value, including zero
and negative values. Putting the rate inside the system would make every square turn
identically and require editing the system to change one of them.

The window is outside ECS. It is a platform object owned by `GameController`, with an
OS-defined lifetime, exactly one instance, and no per-instance iteration; no system queries
it.

The clock is outside ECS. `dt` arrives as an argument to every system, derived from
`CFAbsoluteTimeGetCurrent()` in `-tick`. It is not a component, because it is not attached
to an entity; not a system, because it does no work; not the world, because it does not
contain anything. It is an execution input.

The frame loop is outside ECS. `NSTimer` fires sixty times a second, `-tick` measures the
elapsed time, calls the systems in a fixed order, and asks the view to redraw. Ordering is a
property of the execution model, which the four nouns do not express.

"Shows" splits into a part inside ECS and a part outside it. Selecting what to draw is
`renderSystem`, which iterates entities that have a `Transform` and an `Rgba` and emits a
`Quad` for each; selection over a set is a system. Producing pixels is `-drawRect:`, which
calls CoreGraphics and knows nothing about entities; drawing is platform code. Between them
sits `RenderList`, rebuilt every frame and owned by the view controller, which makes it a
transient buffer rather than a component, since it is not per-entity persistent state, and
not an entity, since nothing refers to it by identity.

The window size and the black background are outside ECS. Both are single values set once
when the window is constructed; neither has behaviour or identity, and no system reads them.
They are presentation configuration.

`World` is not an element of the requirement. It is the container that holds the elements:
three maps keyed by entity ID plus the counter that issues IDs. It is ECS infrastructure.

"Launchable" is outside ECS entirely. It names the entry point in `main`, the build command
in the `Makefile`, and the process lifecycle AppKit manages.

The classification follows three tests and one exclusion. An element that needs independent
identity, lifetime, and iteration is an entity; per-entity data is a component; behaviour
over a set is a system; the container that holds them is the world. Anything single,
platform-bound, execution-bound, or configuration is outside ECS. Some calls are judgement
rather than derivation: the square could be modelled as one entity holding a list of squares,
and the colour could be a constant inside the renderer.
