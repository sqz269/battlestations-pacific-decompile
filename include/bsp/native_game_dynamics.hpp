#pragma once
#include "bsp/dyn_world_factory.hpp"
namespace bsp {
// Borrow the existing engine/profile publication cells, memory service and
// world/scene dispatch owners. They must remain valid through construction and
// subsequent use. This binding does not create replacement globals or tables.
struct NativeGameDynamicsContext {
    const DynEngineRuntimeContext& engine;
    const DynWorldRuntimeContext& world;
};
// C31A40..C31A49: ECX=world, stack callback owner, RET4. Parent4DDB90
// supplies game+1C after publishing world at game+18. Exactly one pointer
// store at world+24; null owner clears it. The world pointer must be valid.
// New C++ interface, not a binary-compatible native entry.
void set_native_dyn_world_callback_owner_00c31a40(void* world,void* callback_owner) noexcept;
} // namespace bsp
