// ecs.hpp — the entire ECS for this game, in one file.
//
// The whole point is that this file stays readable in one sitting. There are no
// templates, no archetypes, no query caching, and no reusable "engine". Just the four
// nouns and the two systems this game actually needs.
//
// Portability: this header is plain C++ with no platform types. The render system
// produces a plain list of quads; main.mm turns those into pixels.

#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace ecs {

// ---------------------------------------------------------------------------
// Entity: an ID. Nothing else. No class, no state, no behaviour.
// ---------------------------------------------------------------------------
using Entity = std::uint32_t;

// ---------------------------------------------------------------------------
// Components: plain data. No methods, no constructors with logic, no pointers to
// anything owned. If a component needs behaviour, that behaviour belongs in a
// system.
// ---------------------------------------------------------------------------

// Where something is and how big it is, in screen units (points, origin at centre,
// y pointing up).
struct Transform {
    float x = 0.0f;
    float y = 0.0f;
    float angle = 0.0f;     // radians, counter-clockwise
    float halfSize = 0.0f;  // the square is 2 * halfSize wide
};

// How fast the angle changes. Split from Transform on purpose: position data and
// motion data are different things, and a static entity simply has no Spin.
struct Spin {
    float radiansPerSecond = 0.0f;
};

// What it looks like. Rendering reads this; nothing else does.
struct Rgba {
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float a = 1.0f;
};

// ---------------------------------------------------------------------------
// World: the container. One map per component type, keyed by entity ID, plus the
// counter that hands out IDs.
//
// This is the naive storage strategy, and it is the correct one here: a game with a
// handful of entities does not need sparse sets or archetypes. The maps are the
// schema; an entity "has" a Transform when that ID is a key in transforms.
// ---------------------------------------------------------------------------
struct World {
    std::unordered_map<Entity, Transform> transforms;
    std::unordered_map<Entity, Spin> spins;
    std::unordered_map<Entity, Rgba> colours;

    // Entities are never recycled in this game, so no generation/version bookkeeping
    // is needed yet. That bookkeeping is one of the things you add when a game needs
    // to destroy entities at runtime.
    Entity nextEntity = 1;

    Entity createEntity() { return nextEntity++; }
};

// ---------------------------------------------------------------------------
// Render output. Not a component: it is not per-entity state that persists between
// frames. It is a scratch buffer that the render system fills and the platform layer
// consumes. This is the first thing in any ECS that is not one of the four nouns.
// ---------------------------------------------------------------------------
struct Quad {
    float x = 0.0f;
    float y = 0.0f;
    float halfSize = 0.0f;
    float angle = 0.0f;
    Rgba colour;
};

using RenderList = std::vector<Quad>;

// ---------------------------------------------------------------------------
// Systems: free functions that query the world and mutate it. Called in a fixed
// order from the game loop in main.mm. No base class, no registry, no virtual
// dispatch.
// ---------------------------------------------------------------------------

// Spawn helper: creates an entity and attaches its components. Not a system — it is
// called once at startup, not every frame.
Entity spawnSquare(World& world, float halfSize, float degreesPerSecond, Rgba colour);

// For every entity with both Transform and Spin, integrate the angle.
void rotationSystem(World& world, float dt);

// For every entity with Transform and Rgba, emit a quad. Does not mutate the world,
// so it takes a const reference — that constness is the access declaration.
void renderSystem(const World& world, RenderList& out);

}  // namespace ecs
