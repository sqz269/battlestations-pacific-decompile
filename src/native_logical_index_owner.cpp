#include "bsp/native_logical_index_owner.hpp"
#include "bsp/native_render_buffer_unregistration.hpp"

#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native logical-index owners require MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(LONG) == 4);
static_assert(sizeof(NativeRendererOptionalGuardStorage) == 8);

void* at(const void* base, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
volatile std::uint32_t& word(const void* base, std::uint32_t offset = 0) noexcept {
    return *static_cast<volatile std::uint32_t*>(at(base, offset));
}
volatile std::uint16_t& half(const void* base, std::uint32_t offset) noexcept {
    return *static_cast<volatile std::uint16_t*>(at(base, offset));
}
void* pointer(const void* base, std::uint32_t offset = 0) noexcept {
    return reinterpret_cast<void*>(word(base, offset));
}
std::int32_t signed_bits(std::uint32_t bits) noexcept {
    std::int32_t result;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}
std::uint32_t saved_guard_word(const NativeRendererOptionalGuardStorage& guard) noexcept {
    std::uint32_t result;
    __asm {
        mov eax, guard
        mov eax, dword ptr [eax]
        mov result, eax
    }
    return result;
}
void destroy_base(void* logical) noexcept { word(logical) = 0x00ceb130u; }

const volatile std::uint32_t* profile_view(std::uint32_t profile,
    const NativeLogicalIndexPhysicalProfiles& profiles) noexcept {
    if (profile == 0x00d61e10u) return profiles.private_index_00d61e10;
    if (profile == 0x00d61e58u) return profiles.pooled_index_00d61e58;
    __assume(0); // No arbitrary profile or callback fallback.
}
void invoke_physical_index_deleting_slot(void* physical,
    NativeLogicalIndexOwnerContext& context) {
    // Both observed immutable profiles have BD30E0 at slot0. That invoker
    // rereads the current physical profile before loading its deleting slot.
    const auto* const invoker_table = profile_view(word(physical), context.actual_physical_profiles);
    const auto invoker = invoker_table[0];
    __assume(invoker == 0x00bd30e0u);
    const auto* const terminal_table = profile_view(word(physical), context.actual_physical_profiles);
    const auto terminal = terminal_table[1];
    if (terminal == 0x00b4bb20u) {
        (void)delete_native_private_index_buffer_00b4bb20(physical, 1, context.actual_physical);
        return;
    }
    if (terminal == 0x00b4c210u) {
        (void)delete_native_pooled_index_buffer_00b4c210(physical, 1, context.actual_physical);
        return;
    }
    __assume(0); // Other physical profiles have no recovered terminal contract.
}
void unwind_guard(const NativeRendererOptionalGuardStorage& guard,
    NativeRendererSynchronizationGlobals& globals) noexcept {
    // Original FH3 invokes B21110 as an unwind action: another exception here
    // terminates instead of replacing the exception already being unwound.
    destroy_native_renderer_optional_guard_00b21110(guard, globals);
}
void leave_guard(const NativeRendererOptionalGuardStorage& guard,
    NativeRendererSynchronizationGlobals& globals, bool entry_enabled) {
    if (globals.mode_00 != 0) {
        __assume(entry_enabled);
        // Preserve the native whole-DWORD load, including padding, through an
        // isolated MOV. B33B00 ignores it; no C++ indeterminate scalar is read.
        const std::uint32_t ignored = saved_guard_word(guard);
        const void* const retained_renderer = guard.renderer_04;
        leave_native_renderer_optional_guard_00b33b00(retained_renderer,
            ignored, globals);
    }
}
} // namespace

void destroy_native_logical_index_stream_00b4b6f0(void* logical,
    NativeLogicalIndexOwnerContext& context) {
    word(logical) = 0x00d61de0u;
    auto& globals = context.actual_synchronization_0108d6dc;
    const bool entry_enabled = globals.mode_00 != 0;
    NativeRendererOptionalGuardStorage guard;
    try { // Native state0: base cleanup is active before optional entry.
        if (entry_enabled) {
            guard.renderer_04 = context.actual_renderer_00f8d394;
            guard.entered_00 = enter_native_renderer_optional_guard_00b33ad0(
                guard.renderer_04, globals);
        }
        const auto flags = word(logical, 0x10);
        void* const captured_physical = pointer(logical, 8);
        try { // Native state1 begins only after these two storage reads.
            if ((flags & 0xf000u) == 0x1000u) {
                unregister_native_physical_index_stream_00b4b390(captured_physical,
                    static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(logical)),
                    context.actual_renderer_00f8d394, globals);
            } else {
                (void)word(captured_physical, 4); // Native otherwise-unused MOV.
            }
            (void)unregister_native_renderer_index_stream_00b26900(
                const_cast<void*>(context.actual_renderer_00f8d394),
                static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(logical)));
            void* const current_physical = pointer(logical, 8);
            auto* const references = static_cast<volatile LONG*>(at(current_physical, 4));
            if (InterlockedDecrement(references) == 0)
                invoke_physical_index_deleting_slot(current_physical, context);
        } catch (...) {
            unwind_guard(guard, globals);
            throw;
        }
        leave_guard(guard, globals, entry_enabled); // State0: guard disarmed.
    } catch (...) {
        destroy_base(logical);
        throw;
    }
    destroy_base(logical); // State-1 before normal base destruction.
}

void* delete_native_pooled_logical_index_stream_00b4c1f0(void* logical,
    std::uint32_t flags, NativeLogicalIndexOwnerContext& context) {
    destroy_native_logical_index_stream_00b4b6f0(logical, context);
    if ((flags & 1u) != 0)
        return_native_logical_index_slot_00b495e0(
            context.actual_logical_index_pool_0108fe50, logical);
    return logical;
}

void return_native_logical_index_slot_00b495e0(void* pool, void* slot) {
    auto* const section = static_cast<CRITICAL_SECTION*>(at(pool, 0x0c));
    EnterCriticalSection(section);
    word(section, 0x18) = word(section, 0x18) + 1u;
    const auto slab_index = word(slot, 0x24);
    void* const slabs = pointer(pool, 0x28);
    void* const slab = pointer(slabs, slab_index * 4u);
    const auto delta = reinterpret_cast<std::uintptr_t>(slot) -
        reinterpret_cast<std::uintptr_t>(slab);
    const auto index = signed_bits(delta) / 0x28;
    const auto old_free_count = half(slab, 0x540);
    half(slab, 0x500u + static_cast<std::uint32_t>(old_free_count) * 2u) =
        static_cast<std::uint16_t>(index);
    half(slab, 0x540) = static_cast<std::uint16_t>(half(slab, 0x540) + 1u);
    if (slab_index < word(pool, 0x34)) word(pool, 0x34) = slab_index;
    word(section, 0x18) = word(section, 0x18) - 1u;
    LeaveCriticalSection(section);
}
} // namespace bsp
