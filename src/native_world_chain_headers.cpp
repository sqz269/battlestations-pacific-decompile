#include "bsp/native_world_chain_headers.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>
#include <new>
#include <type_traits>

namespace bsp {

static_assert(sizeof(void*) == 4 && sizeof(std::size_t) == 4);
static_assert(std::is_standard_layout_v<NativeWorldChainHeader>);
static_assert(std::is_trivially_default_constructible_v<NativeWorldChainHeader>);
static_assert(std::is_trivially_destructible_v<NativeWorldChainHeader>);
static_assert(sizeof(NativeWorldChainHeader) == native_world_chain_header_bytes);
static_assert(offsetof(NativeWorldChainHeader, first) == 0);
static_assert(offsetof(NativeWorldChainHeader, last) == 4);
static_assert(offsetof(NativeWorldChainHeader, count) == 8);

NativeWorldChainHeader* allocate_native_world_chain_headers_009037f0(
    void* actual_world_storage, std::uint32_t active_word,
    std::uint32_t /*unused_word*/) {
    auto* const world = static_cast<unsigned char*>(actual_world_storage);
    const std::uint32_t zero = 0;
    const auto active = static_cast<std::uint8_t>(active_word);
    std::memcpy(world + 0x4a8, &zero, sizeof(zero));
    std::memcpy(world + 0x4a4, &active, sizeof(active));

    const SingletonAllocationRequest request{
        SingletonAllocationKind::object, native_world_chain_header_bytes,
        native_world_chain_header_bytes};
    void* const first_storage = singleton_lifetime_allocate(request);
    NativeWorldChainHeader* first = nullptr;
    if (first_storage != nullptr) {
        // Default initialization only begins the trivial object's lifetime.
        // The Native stores then occur in the order +4, +0, +8.
        first = ::new (first_storage) NativeWorldChainHeader;
        first->last = nullptr;
        first->first = nullptr;
        first->count = 0;
    }
    std::memcpy(world + 4, &first, sizeof(first));

    // No owning local or catch: a throw here leaves World+4 published.
    void* const second_storage = singleton_lifetime_allocate(request);
    NativeWorldChainHeader* second = nullptr;
    if (second_storage != nullptr) {
        second = ::new (second_storage) NativeWorldChainHeader;
        second->last = nullptr;
        second->first = nullptr;
        second->count = 0;
    }
    std::memcpy(world + 8, &second, sizeof(second));
    return second;
}

} // namespace bsp
