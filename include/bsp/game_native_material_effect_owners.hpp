#pragma once
#include "bsp/native_material_effect_loading.hpp"

namespace bsp::game {
// Borrow the application's SAME strings, current renderer/serial cell and
// canonical geometry/registry domain. Contexts remain stable through every
// consumer, retained failed frame and exact native/canonical retirement.
// This view neither constructs a native effect nor creates a cache/program/
// compiler/sampler caller. No registration, retain, release or retry occurs.
struct GameNativeMaterialEffectOwners {
    NativeMaterialEffectConstructionAccess& construction;
    NativeMaterialEffectDestructionAccess& lifetime;
    NativeMaterialEffectLoadOwners& owners;
};
} // namespace bsp::game
