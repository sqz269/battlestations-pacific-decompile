#pragma once

#include "bsp/live_effect_event_registry.hpp"
#include "bsp/generated_model_lifetime.hpp"
#include "bsp/native_render_context.hpp"

namespace bsp {

// Actual 3Ch Win32 owner. The shared prefix contains the sole actual +04
// reference count; +10 is borrowed subject, +14 borrowed definition. Native
// construction leaves +0D..+0F and +20..+23 untouched. No companion count.
struct NativeRegisteredType4EffectStorage {
    NativeRegisteredEffectPrefixStorage prefix_00;
    std::uint32_t state_1c;
    std::uint32_t untouched_20;
    float value_24;
    float value_28;
    float value_2c;
    float value_30;
    void* node_34;
    void* owned_38;
};
static_assert(sizeof(NativeRegisteredType4EffectStorage) == 0x3c);
static_assert(offsetof(NativeRegisteredType4EffectStorage, prefix_00) == 0);
static_assert(offsetof(NativeRegisteredType4EffectStorage, state_1c) == 0x1c);
static_assert(offsetof(NativeRegisteredType4EffectStorage, untouched_20) == 0x20);
static_assert(offsetof(NativeRegisteredType4EffectStorage, value_24) == 0x24);
static_assert(offsetof(NativeRegisteredType4EffectStorage, value_30) == 0x30);
static_assert(offsetof(NativeRegisteredType4EffectStorage, node_34) == 0x34);
static_assert(offsetof(NativeRegisteredType4EffectStorage, owned_38) == 0x38);

// Existing application associations only. Every nonnull +34 must already have
// its actual node virtual18 bound in this runtime. Actual +38 is released via
// the canonical owner domain, resolving its current virtual0 only at count0.
struct RegisteredType4EffectLifetimeBindings {
    LiveEffectEventRegistryBindings registry;
    GeneratedModelLifetimeRuntime& nodes;
    NativeRenderActualOwners& actual_owners;
};

// Complete 8744A0..87453D: ECX=event, stack(definition,subject), RET8.
// Register the same raw owner with the same manager/lock globals. Registration
// failure performs base-only unwind; neither borrowed pointer is retained.
NativeRegisteredType4EffectStorage& construct_registered_type4_effect_008744a0(
    NativeRegisteredType4EffectStorage&, const void* definition, void* subject,
    LiveEffectEventRegistryBindings);

// Complete 868D70..868DD5: ECX=definition, stack subject, RET4. Allocate actual
// 3Ch, construct, free raw storage after constructor unwind on an exception.
NativeRegisteredType4EffectStorage* create_registered_type4_effect_00868d70(
    const void* definition, void* subject, LiveEffectEventRegistryBindings);

// Complete 874540..8745E3: ECX=event, RET. Unlink/release captured +34 then
// clear it; reload/release +38 then clear it; unregister; restore base tables.
// Exceptions restore only base tables, preserving the native partial cleanup.
void destroy_registered_type4_effect_00874540(
    NativeRegisteredType4EffectStorage&, RegisteredType4EffectLifetimeBindings);

// Complete 874610..87462D: ECX=event, stack flags, RET4. Successful destruction
// precedes ordinary CRT free iff flags&1; returns the original address.
NativeRegisteredType4EffectStorage* delete_registered_type4_effect_00874610(
    NativeRegisteredType4EffectStorage*, std::uint32_t flags,
    RegisteredType4EffectLifetimeBindings);

// These lifetime bodies are complete; this is not the complete event family.
// Current D0DE50 virtual28=872790 and virtual30=872060 remain outside these
// entry points. No frame-dispatch binding or successful unsupported event stub
// is provided. New C++ interfaces are not original executable ABI replacements.
} // namespace bsp
