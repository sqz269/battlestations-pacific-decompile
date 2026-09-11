#include "bsp/native_resource_registry_destroy.hpp"

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native resource registry destruction requires MSVC Win32.
#endif

namespace bsp {
namespace {
void* volatile& word(void* storage, std::size_t offset) noexcept {
    return *reinterpret_cast<void* volatile*>(static_cast<unsigned char*>(storage) + offset);
}
volatile std::uint32_t& dword(void* storage, std::size_t offset) noexcept {
    return *reinterpret_cast<volatile std::uint32_t*>(static_cast<unsigned char*>(storage) + offset);
}
struct RegistryUnwindReset {
    void* registry;
    void* volatile& publication;
    bool armed = true;
    ~RegistryUnwindReset() noexcept {
        if (armed) {
            reset_native_resource_registry_00b19760(registry, publication);
        }
    }
};
} // namespace

void reset_native_resource_registry_00b19760(
    void* registry, void* volatile& actual_publication_00f8d41c) noexcept {
    actual_publication_00f8d41c = nullptr;
    dword(registry, 0) = 0x00ce3818;
}

void destroy_native_resource_registry_00b1b5f0(
    void* registry, void* volatile& actual_publication_00f8d41c,
    ActualNativeStringPoolStorage& strings,
    const SingletonLifetimeCallbacks& invalid_parameters) {
    auto* const captured_registry = registry;
    auto* const captured_head = word(captured_registry, 8);
    auto* const captured_minimum = word(captured_head, 0);
    auto* const tree = static_cast<unsigned char*>(captured_registry) + 4;
    NativeResourceRegistryTreeIterator output;
    const NativeResourceRegistryTreeIterator first{tree, captured_minimum};
    const NativeResourceRegistryTreeIterator last{tree, captured_head};
    // Native B1B626 arms state 0 after argument capture, immediately before
    // full range erase. CBC770 resets captured registry, not publication's value.
    RegistryUnwindReset cleanup{captured_registry, actual_publication_00f8d41c};
    erase_native_resource_registry_range_00b1a2f0(
        tree, &output, first, last, strings, invalid_parameters);
    singleton_lifetime_free(word(tree, 4));
    // Full returning-free tail B1B638..B1B65E; state remains armed throughout.
    word(tree, 4) = nullptr;
    dword(tree, 8) = 0;
    actual_publication_00f8d41c = nullptr;
    dword(captured_registry, 0) = 0x00ce3818;
    cleanup.armed = false;
}
} // namespace bsp
