#include "bsp/native_file_provider_base.hpp"

#include <cstdint>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native file provider base requires MSVC Win32.
#endif

namespace bsp {
namespace {
void* at(void* owner, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(owner) + offset);
}
const void* at(const void* owner, std::uint32_t offset) noexcept {
    return reinterpret_cast<const void*>(reinterpret_cast<std::uintptr_t>(owner) + offset);
}
std::uint32_t word(const void* owner, std::uint32_t offset = 0) noexcept {
    return *static_cast<const volatile std::uint32_t*>(at(owner, offset));
}
void put(void* owner, std::uint32_t offset, std::uint32_t value) noexcept {
    *static_cast<volatile std::uint32_t*>(at(owner, offset)) = value;
}

struct ReferenceBaseCleanup {
    void* owner;
    bool armed = true;
    ~ReferenceBaseCleanup() noexcept {
        // Complete BD30F0 and the only action in each native one-state map.
        if (armed) put(owner, 0, 0x00ceb130u);
    }
};
} // namespace

void* construct_native_file_provider_base_00bb5590(void* owner,
    const void* source, NativeStringStorage& storage) {
    put(owner, 0, 0x00ceb130u);
    put(owner, 4, 1);
    auto* const target = at(owner, 8);
    const bool same_header = target == source; // Native CMP precedes the stores.
    put(owner, 0, 0x00d641a0u);
    ReferenceBaseCleanup cleanup{owner};
    put(target, 0, 0);
    put(target, 4, 0);
    if (!same_header) {
        resize_native_string_header_0041dd40(target, storage, word(source), true);
        // BB55EE reloads source length AFTER the allocation/copy boundary.
        if (word(source) != 0) {
            const auto count = word(target);
            const auto* const source_bytes = reinterpret_cast<const void*>(word(source, 4));
            auto* const target_bytes = reinterpret_cast<void*>(word(target, 4));
            // Original BF7680 handles overlap. Match the existing actual-header
            // API's zero-byte-copy policy; preserve all preceding field reads.
            if (count != 0) std::memmove(target_bytes, source_bytes, count);
        }
    }
    put(owner, 0x10, 0xffffffffu);
    cleanup.armed = false;
    return owner;
}

void destroy_native_file_provider_base_00bb5380(void* owner,
    NativeStringStorage& storage) noexcept {
    put(owner, 0, 0x00d641a0u);
    ReferenceBaseCleanup cleanup{owner};
    destroy_native_string_header_0041dd20(at(owner, 8), storage);
}

} // namespace bsp
