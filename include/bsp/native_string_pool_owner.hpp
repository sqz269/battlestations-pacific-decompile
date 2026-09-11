#pragma once

#include "bsp/singleton_lifetime.hpp"

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace bsp {

// Actual embedded ring, not SizedStoragePool's vector projection. Construction
// writes only head/tail/live/peak; the 0x80000 pointer cells retain their bytes.
struct NativeStringPoolRingStorage {
    void* slots_000000[0x80000];
    std::uint32_t head_200000[0x96];
    std::uint32_t tail_200258[0x96];
    std::uint32_t live_2004b0;
    std::uint32_t peak_2004b4;
};

// Actual 8AD4A0h Win32 owner. No initializers, separately allocated arena,
// host vptr, cached shutdown gate, or hidden allocator state.
struct NativeStringPoolStorage {
    std::uint32_t native_vtable_00;
    std::byte untouched_04[4];
    std::byte arena_08[7000000];
    std::uint32_t bump_6acfc8;
    NativeStringPoolRingStorage ring_6acfcc;
    alignas(4) std::byte critical_section_8ad484[0x18];
    std::uint32_t recursion_8ad49c;
};

static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeStringPoolRingStorage) == 0x2004b8);
static_assert(offsetof(NativeStringPoolRingStorage, head_200000) == 0x200000);
static_assert(offsetof(NativeStringPoolRingStorage, tail_200258) == 0x200258);
static_assert(offsetof(NativeStringPoolRingStorage, live_2004b0) == 0x2004b0);
static_assert(offsetof(NativeStringPoolRingStorage, peak_2004b4) == 0x2004b4);
static_assert(sizeof(NativeStringPoolStorage) == 0x8ad4a0);
static_assert(offsetof(NativeStringPoolStorage, arena_08) == 8);
static_assert(offsetof(NativeStringPoolStorage, bump_6acfc8) == 0x6acfc8);
static_assert(offsetof(NativeStringPoolStorage, ring_6acfcc) == 0x6acfcc);
static_assert(offsetof(NativeStringPoolStorage, critical_section_8ad484) == 0x8ad484);
static_assert(offsetof(NativeStringPoolStorage, recursion_8ad49c) == 0x8ad49c);
static_assert(std::is_trivially_default_constructible_v<NativeStringPoolStorage>);
static_assert(std::is_trivially_copyable_v<NativeStringPoolStorage>);

// 00BD0F50..00BD0F92, ECX ring, EAX same ring, RET. It is an embedded-ring
// constructor, not a complete-owner base constructor.
NativeStringPoolRingStorage& construct_native_string_pool_ring_00bd0f50(
    NativeStringPoolRingStorage&) noexcept;

// 00BD1480..00BD14B1, ECX aligned raw owner allocation, EAX same owner, RET.
// Begin the trivial owner's lifetime without value-initialization. The native
// constructor has no local unwind cleanup and does not reset shared01090AA4.
NativeStringPoolStorage* construct_native_string_pool_00bd1480(void* actual_allocation);

// 00419CC0..00419D7F, no native arguments, EAX published owner, RET. References
// must be the application's one 01090AA8 slot and canonical01090AA0 domain.
// Install a real D68200/00BD1730 binding in that domain before first use (the
// binding below composes it with the application's other registered owners).
NativeStringPoolStorage* native_string_pool_get_or_create_00419cc0(
    NativeStringPoolStorage* volatile& actual_published_01090aa8,
    SingletonLifetimeDomain& actual_lifetime);

// 00BD14C0..00BD150E, ECX actual owner, RET; no specified return. Set the real
// shutdown gate, drain only positive SIGNED depth, delete the embedded section,
// clear publication unconditionally, then write baseCE3818. No unregister/free.
void destroy_native_string_pool_00bd14c0(NativeStringPoolStorage&,
    NativeStringPoolStorage* volatile& actual_published_01090aa8,
    volatile std::uint32_t& actual_small_returns_disabled_01090aa4) noexcept;

// 00BD1730..00BD174D, ECX actual owner, stack flags, EAX original owner, RET4.
// Always destroy; flags&1 additionally frees via canonical00BF65AC contract.
NativeStringPoolStorage* delete_native_string_pool_00bd1730(NativeStringPoolStorage&,
    std::uint32_t flags, NativeStringPoolStorage* volatile& actual_published_01090aa8,
    volatile std::uint32_t& actual_small_returns_disabled_01090aa4) noexcept;

// Host dispatch composition, not another lifetime domain or publication slot.
// Construct this first, construct the application's canonical domain from
// callbacks(), and retain this binding until that domain has shut down. Every
// other native profile and invalid-parameter call goes to the REQUIRED next
// callbacks. D68200 is identity data; dispatch calls rebuilt C++ directly.
class NativeStringPoolLifetimeBinding final {
public:
    NativeStringPoolLifetimeBinding(
        NativeStringPoolStorage* volatile& actual_published_01090aa8,
        volatile std::uint32_t& actual_small_returns_disabled_01090aa4,
        SingletonLifetimeCallbacks next);
    NativeStringPoolLifetimeBinding(const NativeStringPoolLifetimeBinding&) = delete;
    NativeStringPoolLifetimeBinding& operator=(const NativeStringPoolLifetimeBinding&) = delete;
    SingletonLifetimeCallbacks callbacks() noexcept;

private:
    static void destroy_registered(void*, void* owner, std::uint32_t flags) noexcept;
    static void invalid_parameter(void*);
    NativeStringPoolStorage* volatile& published_;
    volatile std::uint32_t& returns_disabled_;
    SingletonLifetimeCallbacks next_;
};

// The actual-state BD1120/BD1510/BD12A0 layer and NativeStringStorage bridge
// are provided separately by native_string_pool_storage.hpp. The owner report
// records this packet's earlier boundary; docs/NATIVE_STRING_POOL_STORAGE.md
// records its subsequent implementation and remaining integration limits.

} // namespace bsp
