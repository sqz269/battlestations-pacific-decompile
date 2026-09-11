#include "bsp/native_frame_target_vector.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native frame target vector requires MSVC Win32.
#endif

namespace bsp {
namespace {
std::uint32_t word(const void* base, std::uint32_t byte_offset) noexcept {
    std::uint32_t value;
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov eax, dword ptr [eax + edx] }
    __asm { mov value, eax }
    return value;
}
void put(void* base, std::uint32_t byte_offset, std::uint32_t value) noexcept {
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov ecx, value }
    __asm { mov dword ptr [eax + edx], ecx }
}
std::int32_t signed_word(const void* base, std::uint32_t offset) noexcept {
    const auto bits = word(base, offset);
    std::int32_t value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}
void* pointer(std::uint32_t bits) noexcept { return reinterpret_cast<void*>(bits); }
std::uint32_t address(const void* value) noexcept {
    return reinterpret_cast<std::uintptr_t>(value);
}
} // namespace

void reserve_native_frame_target_vector_00b1f970(
    NativeFrameTargetVectorStorage& array, std::int32_t requested) {
    if (requested < 1) requested = 1;
    if (signed_word(&array, 8) >= requested) return;
    const auto bytes = static_cast<std::uint32_t>(requested) * 16u;
    auto* const replacement = singleton_lifetime_allocate({
        SingletonAllocationKind::object, bytes, bytes});
    auto destination = address(replacement);
    std::uint32_t index = 0;
    std::uint32_t offset = 0;
    if (signed_word(&array, 4) > 0) {
        do {
            if (destination != 0) {
                auto* const source = pointer(word(&array, 0) + offset);
                auto* const target = pointer(destination);
                put(target, 0, word(source, 0));
                put(target, 4, word(source, 4));
                put(target, 8, word(source, 8));
                put(target, 12, word(source, 12));
            }
            ++index;
            offset += 16u;
            destination += 16u;
        } while (static_cast<std::int32_t>(index) < signed_word(&array, 4));
    }
    singleton_lifetime_free(pointer(word(&array, 0)));
    // Returning-free tail B1F9E1..B1F9EF, omitted by the stored CALL_RETURN.
    put(&array, 0, address(replacement));
    put(&array, 8, static_cast<std::uint32_t>(requested));
}

void resize_native_frame_target_vector_00b1f9f0(
    NativeFrameTargetVectorStorage& array, std::int32_t requested) {
    if (requested > signed_word(&array, 8))
        reserve_native_frame_target_vector_00b1f970(array, requested);
    const auto old_count = signed_word(&array, 4);
    if (old_count < requested) {
        auto offset = static_cast<std::uint32_t>(old_count) * 16u;
        auto remaining = static_cast<std::uint32_t>(requested) -
            static_cast<std::uint32_t>(old_count);
        do {
            const auto destination = word(&array, 0) + offset;
            if (destination != 0) {
                auto* const target = pointer(destination);
                put(target, 0, 0);
                put(target, 4, 0);
                put(target, 8, 0);
                put(target, 12, 0);
            }
            offset += 16u;
            --remaining;
        } while (remaining != 0);
    }
    while (requested < signed_word(&array, 4))
        put(&array, 4, word(&array, 4) - 1u);
    put(&array, 4, static_cast<std::uint32_t>(requested));
}

void destroy_native_frame_target_vector_00b1fb90(NativeFrameTargetVectorStorage& array) {
    resize_native_frame_target_vector_00b1f9f0(array, 0);
    singleton_lifetime_free(pointer(word(&array, 0)));
}
} // namespace bsp
