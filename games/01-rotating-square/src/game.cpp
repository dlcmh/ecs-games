// game.cpp — the two systems and the spawn helper. Plain C++; no platform headers.

#include "ecs.hpp"

#include <cmath>
#include <cstdio>

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

void printEntity(const World& world, Entity entity) {
    std::printf("entity %-3u", entity);

    if (const auto it = world.transforms.find(entity); it != world.transforms.end()) {
        const Transform& t = it->second;
        std::printf(" Transform{x=%.1f y=%.1f angle=%.3frad halfSize=%.1f}", t.x, t.y,
                    t.angle, t.halfSize);
    }
    if (const auto it = world.spins.find(entity); it != world.spins.end()) {
        std::printf(" Spin{%.3f rad/s}", it->second.radiansPerSecond);
    }
    if (const auto it = world.colours.find(entity); it != world.colours.end()) {
        const Rgba& c = it->second;
        std::printf(" Rgba{r=%.2f g=%.2f b=%.2f a=%.2f}", c.r, c.g, c.b, c.a);
    }
    std::printf("\n");
}

void printWorld(const World& world) {
    std::printf("world: transforms=%zu spins=%zu colours=%zu\n",
                world.transforms.size(), world.spins.size(), world.colours.size());
    for (const auto& [entity, transform] : world.transforms) {
        (void)transform;
        printEntity(world, entity);
    }
    std::fflush(stdout);
}

}  // namespace ecs
