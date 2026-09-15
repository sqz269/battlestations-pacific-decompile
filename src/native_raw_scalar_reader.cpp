#include "bsp/native_raw_scalar_reader.hpp"
#include "bsp/native_memory_stream.hpp"
#include "bsp/native_physical_stream_open.hpp"

#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw scalar reader source requires MSVC Win32 x87 and four-byte pointers.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);

std::uint32_t word(const volatile void* object, std::uint32_t byte_offset = 0) noexcept {
    std::uint32_t result;
    __asm {
        mov eax, object
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov result, eax
    }
    return result;
}

// Numeric original table words are evidence-qualified dispatch identities,
// never callable host function pointers. Each native virtual site reloads
// the current object profile and its current requested slot.
__declspec(noinline) void __cdecl qualify_scalar_slot(void* stream,
    std::uint32_t offset, std::uint32_t expected,
    NativeRawScalarReaderContext& context) {
    const auto profile = word(stream);
    const volatile void* table;
    if (profile == 0x00d642c0) {
        table = context.memory->actual_stream_profile_00d642c0;
    } else if (profile == 0x00d691b0) {
        table = reinterpret_cast<const volatile void*>(profile);
    } else {
        throw std::invalid_argument("Unimplemented current raw scalar stream profile");
    }
    if (word(table, offset) != expected)
        throw std::invalid_argument("Unimplemented current raw scalar stream slot");
}

__declspec(noinline) void __cdecl read_scalar_bytes(void* stream,
    void* destination, std::uint32_t requested, std::uint32_t* actual,
    NativeRawScalarReaderContext& context) {
    const auto profile = word(stream);
    if (profile == 0x00d642c0) {
        const auto target = word(context.memory->actual_stream_profile_00d642c0, 0x24);
        if (target != 0x00bef590)
            throw std::invalid_argument("Unimplemented current memory scalar read slot");
        native_memory_stream_read_00bef590(stream, nullptr, destination, requested, actual);
    } else if (profile == 0x00d691b0) {
        const auto target = word(reinterpret_cast<const volatile void*>(profile), 0x24);
        if (target != 0x00bf5030)
            throw std::invalid_argument("Unimplemented current physical scalar read slot");
        read_native_physical_stream_00bf5030(stream, destination, requested, actual,
            *context.physical);
    } else {
        throw std::invalid_argument("Unimplemented current raw scalar read profile");
    }
}
} // namespace

// Context-bearing cdecl stack slots differ from native thiscall. Deliberately
// explicit frames retain argument-slot seeding and avoid compiler-added x87
// spills/conversions. No C++ automatic objects or cleanup scopes live here.
__declspec(naked) std::uint32_t __cdecl read_native_raw_dword_00be42e0(
    void*, std::uint32_t*, NativeRawScalarReaderContext&) {
    __asm {
        push ebp
        mov ebp, esp
        push dword ptr [ebp + 10h]
        push dword ptr [ebp + 0ch]
        push 4
        lea eax, [ebp + 0ch]
        push eax
        push dword ptr [ebp + 8]
        call read_scalar_bytes
        add esp, 14h
        mov eax, dword ptr [ebp + 0ch]
        pop ebp
        ret
    }
}

__declspec(naked) float __cdecl read_native_raw_float_00be4360(
    void*, std::uint32_t*, NativeRawScalarReaderContext&) {
    __asm {
        push ebp
        mov ebp, esp
        push dword ptr [ebp + 10h]
        push dword ptr [ebp + 0ch]
        push 4
        lea eax, [ebp + 0ch]
        push eax
        push dword ptr [ebp + 8]
        call read_scalar_bytes
        add esp, 14h
        fld dword ptr [ebp + 0ch]
        pop ebp
        ret
    }
}

__declspec(naked) std::uint32_t __cdecl read_native_raw_dword_and_debit_00bf0280(
    void*, std::uint32_t*, NativeRawScalarReaderContext&) {
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 4
        push esi
        mov eax, dword ptr [ebp + 8]
        mov dword ptr [ebp - 4], eax
        mov esi, dword ptr [eax]
        push dword ptr [ebp + 10h]
        push 00be42e0h
        push 34h
        push esi
        call qualify_scalar_slot
        add esp, 10h
        push dword ptr [ebp + 10h]
        lea eax, [ebp - 4]
        push eax
        push esi
        call read_native_raw_dword_00be42e0
        add esp, 0ch
        mov ecx, dword ptr [ebp + 0ch]
        mov edx, dword ptr [ebp - 4]
        sub dword ptr [ecx], edx
        pop esi
        mov esp, ebp
        pop ebp
        ret
    }
}

__declspec(naked) float __cdecl read_native_raw_float_and_debit_00bf02c0(
    void*, std::uint32_t*, NativeRawScalarReaderContext&) {
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 8
        push esi
        mov eax, dword ptr [ebp + 8]
        mov esi, dword ptr [eax]
        push dword ptr [ebp + 10h]
        push 00be4360h
        push 44h
        push esi
        call qualify_scalar_slot
        add esp, 10h
        push dword ptr [ebp + 10h]
        lea eax, [ebp - 8]
        push eax
        push esi
        call read_native_raw_float_00be4360
        add esp, 0ch
        fstp dword ptr [ebp - 4]
        mov eax, dword ptr [ebp + 0ch]
        fld dword ptr [ebp - 4]
        mov ecx, dword ptr [ebp - 8]
        sub dword ptr [eax], ecx
        pop esi
        mov esp, ebp
        pop ebp
        ret
    }
}

__declspec(naked) std::uint32_t __cdecl read_native_raw_node_dword_00be9a00(
    const void*, NativeRawScalarReaderContext&) {
    __asm {
        mov eax, dword ptr [esp + 4]
        mov eax, dword ptr [eax]
        mov edx, dword ptr [eax + 8]
        add eax, 20h
        push dword ptr [esp + 8]
        push eax
        push edx
        call read_native_raw_dword_and_debit_00bf0280
        add esp, 0ch
        ret
    }
}

__declspec(naked) float __cdecl read_native_raw_node_float_00be99d0(
    const void*, NativeRawScalarReaderContext&) {
    __asm {
        mov eax, dword ptr [esp + 4]
        mov eax, dword ptr [eax]
        mov edx, dword ptr [eax + 8]
        add eax, 20h
        push dword ptr [esp + 8]
        push eax
        push edx
        call read_native_raw_float_and_debit_00bf02c0
        add esp, 0ch
        ret
    }
}
} // namespace bsp
