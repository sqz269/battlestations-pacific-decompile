#include "bsp/native_raw_node_detach.hpp"
#include "bsp/native_filestore_open.hpp"
#include "bsp/native_physical_stream_conversion.hpp"

#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw node detach requires MSVC Win32 and actual four-byte pointers.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);

std::uint32_t word(const volatile void* owner, std::uint32_t byte_offset = 0) noexcept {
    std::uint32_t result;
    __asm {
        mov eax, owner
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov result, eax
    }
    return result;
}
void* pointer(const void* owner, std::uint32_t byte_offset = 0) noexcept {
    return reinterpret_cast<void*>(word(owner, byte_offset));
}
void put(void* owner, std::uint32_t byte_offset, std::uint32_t value) noexcept {
    __asm {
        mov eax, owner
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}
void subtract(void* owner, std::uint32_t byte_offset, std::uint32_t amount) noexcept {
    __asm {
        mov eax, owner
        mov edx, byte_offset
        mov ecx, amount
        sub dword ptr [eax + edx], ecx
    }
}

// The established naked BEF540 source is declared void, but the original
// adapter forwards its EAX. Capture those actual bits, not a Boolean success.
std::uint32_t memory_relative_seek_result(void* stream, std::uint32_t distance_low) noexcept {
    const auto entry = &native_memory_stream_seek_00bef540;
    std::uint32_t result;
    __asm {
        push 1
        push 0
        push distance_low
        mov ecx, stream
        xor edx, edx
        mov eax, entry
        call eax
        mov result, eax
    }
    return result;
}
} // namespace

__declspec(noinline) std::uint32_t skip_native_raw_reader_relative_00bf03e0(
    void* actual_reader, std::uint32_t distance_low, void*,
    NativeRetainedMemoryOwnerContext& memory) {
    void* stream = pointer(actual_reader);
    const auto profile = word(stream);
    if (profile == 0x00d642c0) {
        const auto target = word(memory.actual_stream_profile_00d642c0, 0x1c);
        if (target != 0x00bef540)
            throw std::invalid_argument("Unimplemented current memory detach seek slot");
        return memory_relative_seek_result(stream, distance_low);
    }
    if (profile != 0x00d691b0)
        throw std::invalid_argument("Unimplemented current raw detach stream profile");
    const auto target = word(reinterpret_cast<const volatile void*>(profile), 0x1c);
    if (target != 0x00bf4f20)
        throw std::invalid_argument("Unimplemented current physical detach seek slot");
    return static_cast<std::uint32_t>(seek_native_physical_stream_00bf4f20(
        stream, distance_low, 0, 1));
}

__declspec(noinline) void* skip_and_detach_native_raw_node_00be9c40(
    const void* actual_wrapper, NativeRetainedMemoryOwnerContext& memory) {
    void* node = pointer(actual_wrapper);
    void* parent = pointer(node, 0x0c);
    if (parent) subtract(parent, 0x20, word(node, 0x1c));
    const auto remaining = word(node, 0x20);
    if (remaining != 0) {
        void* reader = pointer(node, 8);
        std::uint32_t ignored = 0;
        (void)skip_native_raw_reader_relative_00bf03e0(reader, remaining, &ignored, memory);
        put(node, 0x20, 0);
    }
    void* reader = pointer(node, 8);
    subtract(reader, 0x60, 1);
    put(node, 8, 0);
    return reader;
}
} // namespace bsp
