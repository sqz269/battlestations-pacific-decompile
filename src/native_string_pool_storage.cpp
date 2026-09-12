#include "bsp/native_string_pool_storage.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <cstdlib>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native string pool storage requires MSVC Win32.
#endif

namespace bsp {
namespace {
constexpr std::uint32_t ring_mask = 0x7ffff;
constexpr std::uint32_t class_count = 0x96;

CRITICAL_SECTION* section(NativeStringPoolStorage& pool) noexcept {
    return reinterpret_cast<CRITICAL_SECTION*>(pool.critical_section_8ad484);
}
} // namespace

void* allocate_native_string_pool_00bd1120(NativeStringPoolStorage* pool,
    std::uint32_t size) {
    if (size >= class_count) return std::malloc(size);
    auto* const captured_section = section(*pool);
    EnterCriticalSection(captured_section);
    volatile auto& actual = *pool;
    actual.recursion_8ad49c = actual.recursion_8ad49c + 1u;
    volatile auto& ring = actual.ring_6acfcc;
    const auto after_tail = (ring.tail_200258[size] + 1u) & ring_mask;
    void* block;
    if (ring.head_200000[size] == after_tail) {
        const auto bump = actual.bump_6acfc8;
        // Native LEA uses DWORD wrap and does not align/check the arena.
        block = reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(pool)
            + 8u + bump);
        actual.bump_6acfc8 = bump + size;
    } else {
        ring.live_2004b0 = ring.live_2004b0 - 1u;
        const auto tail = ring.tail_200258[size]; // native post-counter reload
        block = ring.slots_000000[tail];
        ring.tail_200258[size] = (tail - 1u) & ring_mask;
    }
    actual.recursion_8ad49c = actual.recursion_8ad49c - 1u;
    LeaveCriticalSection(captured_section);
    return block;
}

void return_native_string_pool_small_00bd12a0(NativeStringPoolRingStorage& ring,
    void* const* block, std::uint32_t size_class) noexcept {
    void* moving = *block;
    volatile auto& actual = ring;
    actual.tail_200258[size_class] =
        (actual.tail_200258[size_class] + 1u) & ring_mask;
    auto last = size_class;
    auto next = (size_class + 1u) % class_count;
    while (next != size_class) {
        const auto head = actual.head_200000[next];
        if (actual.tail_200258[last] != head) break;
        const auto after_tail = (actual.tail_200258[next] + 1u) & ring_mask;
        if (head != after_tail) {
            void* const displaced = actual.slots_000000[head];
            actual.slots_000000[head] = moving;
            moving = displaced;
        }
        actual.head_200000[next] = (actual.head_200000[next] + 1u) & ring_mask;
        actual.tail_200258[next] = (actual.tail_200258[next] + 1u) & ring_mask;
        last = next;
        next = (next + 1u) % class_count;
    }
    actual.slots_000000[actual.tail_200258[last]] = moving;
    actual.live_2004b0 = actual.live_2004b0 + 1u;
    const auto live = actual.live_2004b0;
    // Native JLE is signed, even though the preceding DWORD add wraps.
    if (static_cast<std::int32_t>(live) >
        static_cast<std::int32_t>(actual.peak_2004b4)) actual.peak_2004b4 = live;
}

void return_native_string_pool_00bd1510(NativeStringPoolStorage* pool,
    void* block, std::uint32_t size,
    volatile std::uint32_t& actual_small_returns_disabled_01090aa4) noexcept {
    if (size >= class_count) {
        std::free(block);
        return;
    }
    if (actual_small_returns_disabled_01090aa4 != 0) return;
    auto* const captured_section = section(*pool);
    EnterCriticalSection(captured_section);
    volatile auto& actual = *pool;
    actual.recursion_8ad49c = actual.recursion_8ad49c + 1u;
    return_native_string_pool_small_00bd12a0(pool->ring_6acfcc, &block, size);
    actual.recursion_8ad49c = actual.recursion_8ad49c - 1u;
    LeaveCriticalSection(captured_section);
}

ActualNativeStringPoolStorage::ActualNativeStringPoolStorage(
    NativeStringPoolStorage* volatile& publication,
    volatile std::uint32_t& returns_disabled,
    SingletonLifetimeDomain& lifetime) noexcept
    : publication_(publication), returns_disabled_(returns_disabled), lifetime_(&lifetime) {}

ActualNativeStringPoolStorage::ActualNativeStringPoolStorage(
    NativeStringPoolStorage* volatile& publication,
    volatile std::uint32_t& returns_disabled,
    void* volatile& actual_manager_publication_01090aa0) noexcept
    : publication_(publication), returns_disabled_(returns_disabled),
      actual_manager_(&actual_manager_publication_01090aa0) {}

NativeStringPoolStorage* ActualNativeStringPoolStorage::get_pool() {
    if (actual_manager_) {
        return native_string_pool_get_or_create_00419cc0(publication_, *actual_manager_);
    }
    return native_string_pool_get_or_create_00419cc0(publication_, *lifetime_);
}

char* ActualNativeStringPoolStorage::allocate(std::uint32_t size) {
    auto* const owner = get_pool();
    return static_cast<char*>(allocate_native_string_pool_00bd1120(owner, size));
}

void ActualNativeStringPoolStorage::release(char* block, std::uint32_t size) noexcept {
    auto* const owner = get_pool();
    return_native_string_pool_00bd1510(owner, block, size, returns_disabled_);
}

} // namespace bsp
