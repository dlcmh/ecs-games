// game.cpp — the two systems and the spawn helper. Plain C++; no platform headers.

#include "ecs.hpp"

#include <cmath>

namespace ecs {
namespace {

constexpr float kTwoPi = 6.28318530717958647692f;

}  // namespace

Entity spawnSquare(World& world, float halfSize, float degreesPerSecond, Rgba colour) {
    const Entity e = world.createEntity();
    world.transforms[e] = Transform{0.0f, 0.0f, 0.0f, halfSize};
    world.spins[e] = Spin{degreesPerSecond * kTwoPi / 360.0f};
    world.colours[e] = colour;
    return e;
}

void rotationSystem(World& world, float dt) {
    // A query hand-written as a loop: "everything that has a Transform and a Spin."
    // The lookup in spins is the join. When the third game repeats this shape, the
    // loop is what gets extracted into a query helper — not before.
    for (auto& [entity, transform] : world.transforms) {
        const auto spin = world.spins.find(entity);
        if (spin == world.spins.end()) {
            continue;
        }
        transform.angle = std::fmod(transform.angle + spin->second.radiansPerSecond * dt,
                                    kTwoPi);
    }
}

void renderSystem(const World& world, RenderList& out) {
    out.clear();

    for (const auto& [entity, transform] : world.transforms) {
        const auto colour = world.colours.find(entity);
        if (colour == world.colours.end()) {
            continue;
        }
        out.push_back(Quad{transform.x, transform.y, transform.halfSize, transform.angle,
                           colour->second});
    }
}

}  // namespace ecs
