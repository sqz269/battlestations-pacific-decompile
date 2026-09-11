#pragma once

#include "bsp/gameplay_effect_manager.hpp"
#include "bsp/native_string.hpp"

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <new>

namespace bsp {
// Actual24h storage: vtable0, intrusive refs4, component pointer8, signed
// countC/capacity10, untouched word14, signed ID18 and NativeString1C/20.
// No parallel vector or implicit string/reference ownership is maintained.
struct alignas(4) GameplayEffectDefinition {
    std::array<std::byte, 0x24> native;
    // The native constructor starts this actual atomic object at its original
    // refs=1 write. Binding a companion only borrows it; never resets/retains.
    // Requires construct_gameplay_effect_definition_00870256_fragment first.
    std::atomic<std::int32_t>& references_04() noexcept {
        return *std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(native.data() + 4));
    }
};
static_assert(sizeof(GameplayEffectDefinition) == 0x24);
static_assert(sizeof(std::atomic<std::int32_t>) == 4);
static_assert(std::atomic<std::int32_t>::is_always_lock_free);
struct GameplayEffectComponentLifetime {
    virtual ~GameplayEffectComponentLifetime() = default;
    // Called only after real InterlockedDecrement(actual component+4)==0.
    // Dispatch CURRENT vtable+0, ECX=actual component, no stack arguments.
    virtual void zero_references_slot_00(void* actual_component) = 0;
};
struct GameplayEffectDefinitionContext {
    GameplayEffectManagerContext& manager;
    NativeStringStorage& strings;
    GameplayEffectComponentLifetime& components;
};
// Inline allocation/constructor fragment of008700E0, native00870240..00870279.
// Allocate24h and preserve untouched representation bytes14..1B. Constructor
// writes base then derived vtable, refs1, pointer/count/capacity0 and string0.
GameplayEffectDefinition* allocate_gameplay_effect_definition_00870240_fragment();
GameplayEffectDefinition& construct_gameplay_effect_definition_00870256_fragment(
    GameplayEffectDefinition&) noexcept;
//0086B870: ECX=owner, stack ID/name-header pointer; RET8. Store ID before
// resizing/copying name, including self-name assignment. Source is actual8h.
void set_gameplay_effect_definition_identity_0086b870(GameplayEffectDefinition&,
    std::int32_t id, const void* actual_name_header, NativeStringStorage&);

// These operate directly on actual12h headers, e.g. definition.native+8.
// Data pointer0, signed count4/capacity8. Valid live allocations required;
// native signed comparisons and DWORD arithmetic wrap are preserved.
//0086E770: ECX=header, stack capacity; RET4. Clamp to at least1; copy/retain
// ascending, release old slots ascending, reload/free old data, publish new
// data/capacity. Size remains whatever callbacks left in the actual header.
void reserve_gameplay_effect_components_0086e770(void* actual_header,
    std::int32_t capacity, GameplayEffectComponentLifetime&);
//0086EB60: ECX=header, stack pointer-slot address; RET4. Grow only when
// count==capacity, clear destination BEFORE reading source, retain, ++count.
void append_gameplay_effect_component_0086eb60(void* actual_header,
    const void* actual_source_slot, GameplayEffectComponentLifetime&);
//0086EDD0: ECX=header, stack size; RET4. Grow with nulls or shrink descending:
// --count before release, clear captured slot after callback, reload count.
void resize_gameplay_effect_components_0086edd0(void* actual_header,
    std::int32_t size, GameplayEffectComponentLifetime&);

//00870D00: ECX=owner; RET. Get current manager, erase current ID even when its
// cache value is another pointer; destroy name, resize components0, reload/free
// buffer, write base vtable. Freed buffer/name headers are left unchanged.
// Missing ID reaches the native STL out_of_range path; it is not ignored.
void destroy_gameplay_effect_definition_00870d00(GameplayEffectDefinition&,
    GameplayEffectDefinitionContext&);
//D0DA58[4] ->00871440: ECX=owner, stack flags; EAX original address; RET4.
// Calls full destructor; flags&1 frees owner. New C++ API, not a native vtable.
GameplayEffectDefinition* scalar_delete_gameplay_effect_definition_00871440(
    GameplayEffectDefinition*, std::uint32_t flags, GameplayEffectDefinitionContext&);
} // namespace bsp
