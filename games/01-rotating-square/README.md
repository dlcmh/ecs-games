# 01 — rotating square

A launchable C++ program that shows a slowly rotating square.

To build that we need a handful of things. The sentence mentions the square and the rotation,
and it calls the rotation slow. It doesn't mention the window we draw into, the pixels we
draw with, the clock that makes the rotation visible, the loop that ties the clock to the
window, or the handoff that carries drawable data to the platform. Well, some of those are
implied by what the sentence asks for. The rest happen to be required, because a program
shaped like this won't run without them.

The square comes first. It's an entity, because it's the thing that exists, persists across
frames, and can be referred to on its own. The requirement asks for one, though the code
allows any number. Its identity isn't its geometry: the entity is the number `1` that
`World::nextEntity` handed out when `spawnSquare` created it. Strip every component off
entity `1` and the ID is still there.

To place the square we need its position, its size, and its current angle. Those are a
component, `Transform`. They belong to the square, change together, and mean nothing on
their own. `World::transforms` holds one row per entity, and being in that map is what
having a position means.

The requirement never says the square is orange; we picked orange while building it. That
colour is another component, `Rgba`. Once it's there it's per-entity data: two squares can
differ, it does no work, and nothing refers to it on its own. It isn't an entity, because it
has no identity or lifetime, and it isn't a system, because it does nothing.

For the rotation itself we need a system. `rotationSystem` runs on every entity that has
both a `Transform` and a `Spin`, adds each one's rate times `dt` to its angle, and knows
nothing about squares or colours. It isn't a component because it's behaviour, not data, and
it isn't an entity because it has no identity and nothing iterates over rotations.

The word "slowly" hides two things. The behaviour is the system we just named. The pace is
data: `Spin::radiansPerSecond`, set to 20 degrees per second by `spawnSquare`. We make the
rate a component because each entity can have its own, and the system has to accept any
value, including zero and negative. If the rate lived inside the system, every square would
turn at the same speed, and changing one would mean editing the system.

Now for the parts outside ECS. To show anything we need a window, and the window is a
platform object owned by `GameController`, with an OS-defined lifetime, exactly one
instance, and no per-instance iteration; no system queries it.

To make the rotation visible we need a clock. `dt` reaches every system as an argument,
computed from `CFAbsoluteTimeGetCurrent()` in `-tick`. It isn't attached to an entity, it
does no work, and it isn't a container, so it's an input to execution.

We also need something to drive the clock and call the systems in order. `NSTimer` fires
sixty times a second, `-tick` measures the time since the last frame, calls the systems in a
fixed order, and asks the view to redraw. Ordering belongs to the loop, not to ECS.

Then there's the word "shows", which needs two parts. To decide what to draw we have
`renderSystem`: it iterates entities with a `Transform` and an `Rgba` and emits a `Quad` for
each. Picking from a set is what makes it a system. To turn those quads into pixels,
`-drawRect:` calls CoreGraphics, and it knows nothing about entities; drawing is platform
code. The `RenderList` in between is rebuilt every frame and owned by the view controller.
That makes it a transient buffer, not a component, because it isn't per-entity state that
persists, and not an entity, because nothing refers to it by identity.

The window size and the black background are set once when the window is built. Neither has
behaviour or identity, and no system reads them, so they're presentation configuration,
outside ECS.

`World` isn't part of the requirement at all. It's the container that holds the elements:
three maps keyed by entity ID plus the counter that issues IDs. It's ECS infrastructure.

"Launchable" points outside ECS entirely: the entry point in `main`, the build command in
the `Makefile`, and the process lifecycle AppKit manages.

So the classification comes down to three tests and one exclusion. An element that needs
independent identity, lifetime, and iteration is an entity; data that belongs to an entity
is a component; behaviour over a set is a system; the thing that holds them is the world.
Anything there's only one of, or that belongs to the platform, the loop, or configuration,
stays outside ECS. Some of these calls are judgement, not deduction: the square could be one
entity holding a list of squares, and the colour could be a constant inside the renderer.
