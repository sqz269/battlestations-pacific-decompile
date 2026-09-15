#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include "bsp/native_material_record_copy.hpp"

#include <cstring>
#include <exception>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native material record copy requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
constexpr Word record_stride = 300;
static_assert(record_stride == 0x12c);

// Separate native REP blocks matter: one large memcpy can reverse overlap or
// read ahead across block boundaries. The Win32 ABI supplies clear DF.
__declspec(naked) void __fastcall copy_prefix_11c(void*, const void*) noexcept {
    __asm {
        push ebx
        push esi
        push edi
        mov ebx, ecx
        mov eax, dword ptr [edx]
        mov dword ptr [ebx], eax
        lea esi, [edx + 4]
        lea edi, [ebx + 4]
        mov ecx, 0Eh
        rep movsd
        lea esi, [edx + 3Ch]
        lea edi, [ebx + 3Ch]
        mov ecx, 0Eh
        rep movsd
        lea esi, [edx + 74h]
        lea edi, [ebx + 74h]
        mov ecx, 0Eh
        rep movsd
        lea esi, [edx + 0ACh]
        lea edi, [ebx + 0ACh]
        mov ecx, 0Eh
        rep movsd
        lea esi, [edx + 0E4h]
        lea edi, [ebx + 0E4h]
        mov ecx, 0Eh
        rep movsd
        pop edi
        pop esi
        pop ebx
        ret
    }
}

Word word(const volatile void* base, Word byte_offset = 0) noexcept {
    Word result;
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov eax, dword ptr [eax + edx] }
    __asm { mov result, eax }
    return result;
}
void put(void* base, Word byte_offset, Word value) noexcept {
    __asm { mov eax, base }
    __asm { mov edx, byte_offset }
    __asm { mov ecx, value }
    __asm { mov dword ptr [eax + edx], ecx }
}
void* at(const void* base, Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(base) + offset);
}
void* pointer(Word bits) noexcept { return reinterpret_cast<void*>(bits); }

void construct_header(void* destination, const void* source,
    NativeStringRawPoolContext& strings) {
    Word same_address;
    __asm {
        mov eax, destination
        xor ecx, ecx
        cmp eax, source
        sete cl
        mov same_address, ecx
    }
    put(destination, 0, 0);
    put(destination, 4, 0);
    if (same_address != 0) return;
    resize_native_string_header_0041dd40(destination, strings, word(source), true);
    if (word(source) != 0) {
        const Word copied = word(destination);
        const void* const current_source = pointer(word(source, 4));
        void* const current_destination = pointer(word(destination, 4));
        // Original BF7680 memcpy has a backward path for overlap. Keep the
        // native call even for a current zero-byte destination length.
        std::memmove(current_destination, current_source, copied);
    }
}
int cleanup_exception(unsigned long code) noexcept {
    if (code == 0xe06d7363u) std::terminate();
    return EXCEPTION_CONTINUE_SEARCH;
}
void unwind_first_header(void* header, NativeStringRawPoolContext& strings) noexcept {
    __try { destroy_native_string_header_0041dd20(header, strings); }
    __except (cleanup_exception(GetExceptionCode())) { __assume(0); }
}
struct FirstHeaderCleanup {
    void* actual_first_header;
    NativeStringRawPoolContext& strings;
    bool armed = true;
    ~FirstHeaderCleanup() noexcept {
        if (armed) unwind_first_header(actual_first_header, strings);
    }
};
} // namespace

void* __fastcall construct_native_material_record_00b13070(
    void* destination, NativeStringRawPoolContext& strings, const void* source) {
    copy_prefix_11c(destination, source);

    void* const first_destination = at(destination, 0x11c);
    const void* const first_source = at(source, 0x11c);
    construct_header(first_destination, first_source, strings);

    FirstHeaderCleanup cleanup{first_destination, strings};
    void* const second_destination = at(destination, 0x124);
    const void* const second_source = at(source, 0x124);
    construct_header(second_destination, second_source, strings);
    cleanup.armed = false;
    return destination;
}

__declspec(naked) std::int32_t __fastcall get_native_material_record_vector_size_00b0d230(
    const void*) noexcept {
    __asm {
        mov eax, dword ptr [ecx + 4]
        test eax, eax
        jnz nonempty
        ret
    nonempty:
        mov ecx, dword ptr [ecx + 8]
        sub ecx, eax
        mov eax, 1B4E81B5h
        imul ecx
        sar edx, 5
        mov eax, edx
        shr eax, 1Fh
        add eax, edx
        ret
    }
}
} // namespace bsp
