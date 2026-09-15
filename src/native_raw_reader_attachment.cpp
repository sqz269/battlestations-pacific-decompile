#include "bsp/native_raw_reader_attachment.hpp"
#include "bsp/native_physical_stream_open.hpp"
#include "bsp/native_retained_memory_owners.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native raw reader attachment requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(LONG) == 4);

// Ordered DWORD accesses retain the raw storage and alias-visible schedule.
std::uint32_t word(const volatile void* base, std::uint32_t byte_offset = 0) noexcept {
    std::uint32_t result;
    __asm {
        mov eax, base
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov result, eax
    }
    return result;
}

void publish(void* reader, void* stream) noexcept {
    __asm {
        mov eax, reader
        mov edx, stream
        mov dword ptr [eax], edx
    }
}

volatile LONG* references(void* stream) noexcept {
    return reinterpret_cast<volatile LONG*>(
        reinterpret_cast<std::uintptr_t>(stream) + 4u);
}

void invoke_current_zero_slot(void* stream, NativeRawScalarReaderContext& context) {
    const auto profile = word(stream);
    if (profile == 0x00d642c0) {
        if (word(context.memory->actual_stream_profile_00d642c0) != 0x00bd30e0)
            throw std::invalid_argument("Unimplemented current memory attachment slot0");
        // BD30E0 reloads the current owner profile before its slot4(flags1).
        if (word(stream) != 0x00d642c0)
            throw std::invalid_argument("Unimplemented current memory attachment profile");
        if (word(context.memory->actual_stream_profile_00d642c0, 4) != 0x00bb8f90)
            throw std::invalid_argument("Unimplemented current memory attachment slot4");
        delete_native_memory_stream_00bb8f90(stream, 1, *context.memory);
        return;
    }
    if (profile == 0x00d691b0) {
        if (word(reinterpret_cast<const volatile void*>(profile)) != 0x00bf55a0)
            throw std::invalid_argument("Unimplemented current physical attachment slot0");
        // The real provider owns its next current-slot lookup, flags0, and pool.
        recycle_native_physical_stream_00bf55a0(stream, *context.physical);
        return;
    }
    throw std::invalid_argument("Unimplemented current raw reader attachment profile");
}
} // namespace

// BF0430 complete body, source ABI only. No automatic cleanup or catch scope:
// publication and retain precede old release, including a throwing terminal.
void assign_native_raw_reader_stream_00bf0430(void* actual_reader,
    void* actual_stream, NativeRawScalarReaderContext& context) {
    void* replacement;
    void* previous;
    __asm {
        mov eax, actual_stream
        mov replacement, eax
        mov ecx, actual_reader
        mov eax, dword ptr [ecx]
        mov previous, eax
    }
    if (previous == replacement) return;
    publish(actual_reader, replacement);
    if (replacement) InterlockedIncrement(references(replacement));
    if (previous && InterlockedDecrement(references(previous)) == 0)
        invoke_current_zero_slot(previous, context);
}

} // namespace bsp
