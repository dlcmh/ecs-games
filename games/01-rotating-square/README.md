# 01 — rotating square

A launchable C++ program that shows a slowly rotating square.

The requirement is one sentence. Read as a build instruction, it splits into parts: some it
names, some it only hints at, and some it never mentions but experience says must be there.
It names the square and the rotation, and calls the rotation slow. It implies a window to
draw into and pixels to draw with. It never mentions the clock that makes the rotation
visible, the frame loop that ties the clock to the window, or the handoff that carries
drawable data from the game to the platform. Those come from experience, because a program
shaped like this cannot run without them.

The square is an entity. It is the thing that exists, persists across frames, and can be
referred to on its own. The requirement asks for one; the shape allows any number. Its
identity is not its geometry: the entity is the number `1` that `World::nextEntity` handed
out when `spawnSquare` created it. Strip every component off entity `1` and the ID is still
there.

Position, size, and current orientation are a component, `Transform`. They belong to the
square, change together, and mean nothing on their own. `World::transforms` holds one row
per entity; being in that map is what having a position means.

The orange is a component, `Rgba`. The requirement says nothing about colour; orange was
picked while building it. Once it exists it is per-entity data: two squares can differ, it
does no work, and nothing refers to it on its own. It is not an entity because it has no
identity or lifetime, and not a system because it does nothing.

Rotation is a system, `rotationSystem`. It runs on every entity that has both a `Transform`
and a `Spin`, adds each one's rate times `dt` to its angle, and knows nothing about squares
or colours. It is not a component because it is behaviour, not data, and not an entity
because it has no identity and nothing iterates over rotations.

"Slowly" hides two elements. The behaviour is the system. The pace is data:
`Spin::radiansPerSecond`, set to 20 degrees per second by `spawnSquare`. The rate is a
component because each entity can have its own and the system has to accept any value,
including zero and negative. If the rate lived inside the system, every square would turn at
the same speed and changing one would mean editing the system.

The window is outside ECS. It is a platform object owned by `GameController`, with an
OS-defined lifetime, exactly one instance, and no per-instance iteration; no system queries
it.

The clock is outside ECS. `dt` reaches every system as an argument, computed from
`CFAbsoluteTimeGetCurrent()` in `-tick`. It is not attached to an entity, it does no work,
and it is not a container, so none of the four nouns fit. It is an input to execution.

The frame loop is outside ECS. `NSTimer` fires sixty times a second, `-tick` measures the
time since the last frame, calls the systems in a fixed order, and asks the view to redraw.
The four nouns say nothing about ordering; the loop decides it.

"Shows" splits in two, one half inside ECS and one outside. `renderSystem` decides what to
draw by iterating entities that have a `Transform` and an `Rgba` and emitting a `Quad` for
each; picking from a set is a system. `-drawRect:` turns those quads into pixels with
CoreGraphics and knows nothing about entities; drawing is platform code. `RenderList` sits
between them, rebuilt every frame and owned by the view controller. It is a transient
buffer, not a component, because it is not per-entity state that persists, and not an
entity, because nothing refers to it by identity.

The window size and the black background are outside ECS. Both are single values set once
when the window is built; neither has behaviour or identity, and no system reads them. They
are presentation configuration.

`World` is not part of the requirement. It is the container that holds the elements: three
maps keyed by entity ID plus the counter that issues IDs. It is ECS infrastructure.

"Launchable" is outside ECS entirely. It names the entry point in `main`, the build command
in the `Makefile`, and the process lifecycle AppKit manages.

Classification comes down to three tests and one exclusion. An element that needs
independent identity, lifetime, and iteration is an entity; data that belongs to an entity
is a component; behaviour over a set is a system; the thing that holds them is the world.
Anything there is only one of, or that belongs to the platform, the loop, or configuration,
stays outside ECS. Some of these calls are judgement, not deduction: the square could be one
entity holding a list of squares, and the colour could be a constant inside the renderer.
