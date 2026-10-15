#include "bsp/native_scene_property_assignment_storage.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>

#pragma function(memcpy)

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4, "The original assignment storage is Win32.");

__forceinline std::uint32_t load_word(const void* root, std::size_t byte_offset) noexcept {
    std::uint32_t value;
    __asm {
        mov eax, root
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov value, eax
    }
    return value;
}

__forceinline void store_word(void* root, std::size_t byte_offset, std::uint32_t value) noexcept {
    __asm {
        mov eax, root
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}

void* pointer(std::uint32_t bits) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(bits));
}
} // namespace

// Whole Original26. Keep the null path free of both provider calls and stores.
__declspec(noinline) void free_and_clear_native_owned_pointer_004357f0(
    void* actual_pointer_slot) {
    const auto old_pointer = load_word(actual_pointer_slot, 0);
    if (old_pointer != 0) {
        singleton_lifetime_free(pointer(old_pointer));
        store_word(actual_pointer_slot, 0, 0);
    }
}

// Whole Original63. The original caller ignores memcpy's residual EAX; this
// new ordinary interface returns void and admits no Original ABI transport.
__declspec(noinline) void copy_native_scene_property_array_block_008f03b0(
    void* actual_destination_block, const void* actual_source_block) {
    const auto source_pointer = load_word(actual_source_block, 0);
    const auto byte_count = load_word(actual_source_block, 4);
    const auto old_pointer = load_word(actual_destination_block, 0);
    if (old_pointer != 0) {
        singleton_lifetime_free(pointer(old_pointer));
        store_word(actual_destination_block, 0, 0);
    }
    store_word(actual_destination_block, 4, byte_count);
    void* const output = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, byte_count, byte_count});
    store_word(actual_destination_block, 0, static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(output)));
    std::memcpy(output, pointer(source_pointer), byte_count);
}

} // namespace bsp
