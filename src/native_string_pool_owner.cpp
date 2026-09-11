#include "bsp/native_string_pool_owner.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <cstring>
#include <new>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native string pool ownership requires MSVC Win32.
#endif

namespace bsp {
namespace {
constexpr std::uint32_t pool_profile = 0x00d68200;
constexpr std::uint32_t base_profile = 0x00ce3818;
static_assert(sizeof(CRITICAL_SECTION) == 0x18);
static_assert(alignof(CRITICAL_SECTION) == 4);

CRITICAL_SECTION* section(NativeStringPoolStorage& owner) noexcept {
    return reinterpret_cast<CRITICAL_SECTION*>(owner.critical_section_8ad484);
}
} // namespace

NativeStringPoolRingStorage& construct_native_string_pool_ring_00bd0f50(
    NativeStringPoolRingStorage& ring) noexcept {
    volatile auto& actual = ring;
    std::uint32_t head = 0;
    for (std::uint32_t i = 0; i < 0x96; ++i) {
        actual.head_200000[i] = head;
        actual.tail_200258[i] = head - 1u;
        head += 0xda7;
    }
    actual.live_2004b0 = 0;
    actual.peak_2004b4 = 0;
    actual.tail_200258[0] = 0x7ffff;
    return ring;
}

NativeStringPoolStorage* construct_native_string_pool_00bd1480(void* actual_allocation) {
    auto* const owner = ::new (actual_allocation) NativeStringPoolStorage;
    volatile auto& actual = *owner;
    actual.native_vtable_00 = pool_profile;
    construct_native_string_pool_ring_00bd0f50(owner->ring_6acfcc);
    ::new (owner->critical_section_8ad484) CRITICAL_SECTION;
    InitializeCriticalSection(section(*owner));
    actual.recursion_8ad49c = 0;
    actual.bump_6acfc8 = 0;
    return owner;
}

NativeStringPoolStorage* native_string_pool_get_or_create_00419cc0(
    NativeStringPoolStorage* volatile& actual_published_01090aa8,
    SingletonLifetimeDomain& actual_lifetime) {
    if (auto* const owner = actual_published_01090aa8) return owner;
    auto* const captured_section = actual_lifetime.get_manager_00415350()
        ->system_owner().section_10;
    if (captured_section) {
        singleton_enter_critical_section(*captured_section);
        ++captured_section->recursion_18;
    }
    void* allocation = nullptr;
    unsigned unwind_state = 0;
    // FuncInfo D841EC: state1 frees raw storage (C5E118), then state0 releases
    // the captured section (C5E110 -> 00411EE0). Registration is in state0.
    // __finally preserves cleanup for both C++ and Win32 unwinding.
    __try {
        if (!actual_published_01090aa8) {
            allocation = singleton_lifetime_allocate({SingletonAllocationKind::object,
                0x8ad4a0, sizeof(NativeStringPoolStorage)});
            unwind_state = 1;
            auto* const owner = allocation
                ? construct_native_string_pool_00bd1480(allocation) : nullptr;
            unwind_state = 0;
            actual_published_01090aa8 = owner;
            auto* const registration_manager = actual_lifetime.get_manager_00415350();
            registration_manager->register_object(actual_published_01090aa8);
        }
    } __finally {
        if (unwind_state == 1) singleton_lifetime_free(allocation);
        if (captured_section) {
            --captured_section->recursion_18;
            singleton_leave_critical_section(*captured_section);
        }
    }
    return actual_published_01090aa8; // Reload AFTER unlocking.
}

void destroy_native_string_pool_00bd14c0(NativeStringPoolStorage& owner,
    NativeStringPoolStorage* volatile& actual_published_01090aa8,
    volatile std::uint32_t& actual_small_returns_disabled_01090aa4) noexcept {
    volatile auto& actual = owner;
    actual.native_vtable_00 = pool_profile;
    actual_small_returns_disabled_01090aa4 = 1;
    // Native JLE/JG use signed depth; arithmetic itself wraps as a DWORD.
    while (static_cast<std::int32_t>(actual.recursion_8ad49c) > 0) {
        actual.recursion_8ad49c = actual.recursion_8ad49c - 1u;
        LeaveCriticalSection(section(owner));
    }
    DeleteCriticalSection(section(owner));
    actual_published_01090aa8 = nullptr;
    actual.native_vtable_00 = base_profile;
}

NativeStringPoolStorage* delete_native_string_pool_00bd1730(NativeStringPoolStorage& owner,
    std::uint32_t flags, NativeStringPoolStorage* volatile& actual_published_01090aa8,
    volatile std::uint32_t& actual_small_returns_disabled_01090aa4) noexcept {
    auto* const original_address = &owner;
    destroy_native_string_pool_00bd14c0(owner, actual_published_01090aa8,
        actual_small_returns_disabled_01090aa4);
    if ((flags & 1u) != 0) {
        owner.~NativeStringPoolStorage();
        singleton_lifetime_free(original_address);
    }
    // Ghidra misses ADD ESP,4 at 00BD1745 after incorrectly treating _free as
    // no-return. Verified bytes continue with EAX=this and RET4 through174D.
    return original_address;
}

NativeStringPoolLifetimeBinding::NativeStringPoolLifetimeBinding(
    NativeStringPoolStorage* volatile& actual_published_01090aa8,
    volatile std::uint32_t& actual_small_returns_disabled_01090aa4,
    SingletonLifetimeCallbacks next)
    : published_(actual_published_01090aa8),
      returns_disabled_(actual_small_returns_disabled_01090aa4), next_(next) {
    if (!next_.destroy_registered || !next_.invalid_parameter) {
        throw std::invalid_argument("String pool lifetime binding requires other-owner callbacks");
    }
}

SingletonLifetimeCallbacks NativeStringPoolLifetimeBinding::callbacks() noexcept {
    return {this, &destroy_registered, &invalid_parameter};
}

void NativeStringPoolLifetimeBinding::destroy_registered(void* context, void* owner,
    std::uint32_t flags) noexcept {
    auto& binding = *static_cast<NativeStringPoolLifetimeBinding*>(context);
    std::uint32_t profile;
    std::memcpy(&profile, owner, sizeof(profile));
    if (profile == pool_profile) {
        delete_native_string_pool_00bd1730(*static_cast<NativeStringPoolStorage*>(owner),
            flags, binding.published_, binding.returns_disabled_);
    } else {
        binding.next_.destroy_registered(binding.next_.context, owner, flags);
    }
}

void NativeStringPoolLifetimeBinding::invalid_parameter(void* context) {
    auto& binding = *static_cast<NativeStringPoolLifetimeBinding*>(context);
    binding.next_.invalid_parameter(binding.next_.context);
}

} // namespace bsp
