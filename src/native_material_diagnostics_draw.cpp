#include "bsp/native_material_diagnostics_draw.hpp"

#include "bsp/native_instance_collection.hpp"
#include "bsp/native_material_diagnostics_record_init.hpp"
#include "bsp/native_material_record_vector.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_renderer_begin_frame.hpp"
#include "bsp/native_string_pool_storage.hpp"

#include <cstdlib>
#include <exception>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native material diagnostics require MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(Word) == 4);
constexpr Word stride = 300;

Word bits(const void* value) noexcept { return reinterpret_cast<Word>(value); }
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
void* at(const void* value, Word byte_offset) noexcept {
    return pointer(bits(value) + byte_offset);
}
__declspec(noinline) Word load(const void* value, Word byte_offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(at(value, byte_offset));
}
__declspec(noinline) void store(void* value, Word byte_offset, Word data) noexcept {
    *reinterpret_cast<volatile Word*>(at(value, byte_offset)) = data;
}
__declspec(noinline) void invalid_parameter() { _invalid_parameter_noinfo(); }

struct CompletedRecord {
    void* value;
    NativeStringRawPoolContext& strings;
    bool armed = true;
    ~CompletedRecord() noexcept {
        if (armed) {
            try { destroy_native_renderer_record_00b10740(value, strings); }
            catch (...) { std::terminate(); }
        }
    }
};
struct CompletedHeader {
    void* value;
    NativeStringRawPoolContext& strings;
    bool armed = true;
    ~CompletedHeader() noexcept {
        if (armed) {
            try { destroy_native_string_header_0041dd20(value, strings); }
            catch (...) { std::terminate(); }
        }
    }
};

// Preserve five single-memory ADDs and the interleaved actual argument
// reads at B1715D..B17194. The mode is captured once; no range check.
__declspec(naked) void __fastcall add_draw_counters(void*, const void*) noexcept {
    __asm {
        push ebx
        mov ebx, ecx
        mov eax, dword ptr [edx + 4]
        mov ecx, dword ptr [edx + 8]
        add dword ptr [ebx + eax * 4 + 3Ch], ecx
        mov ecx, dword ptr [edx + 0Ch]
        add dword ptr [ebx + eax * 4 + 74h], ecx
        mov ecx, dword ptr [edx + 10h]
        add dword ptr [ebx + eax * 4 + 4], 1
        add dword ptr [ebx + eax * 4 + 0ACh], ecx
        mov edx, dword ptr [edx + 14h]
        add dword ptr [ebx + eax * 4 + 0E4h], edx
        pop ebx
        ret
    }
}

// args names the public entry's original six stack words. This body uses
// new private locals and source C++ EH; it does not alias the native spills.
__declspec(noinline) void __cdecl record_draw_body(
    void* diagnostics, NativeStringRawPoolContext* strings, const void* args) {
    if (*reinterpret_cast<const volatile unsigned char*>(at(diagnostics, 0x681)) == 0)
        return;
    void* const vector = at(diagnostics, 0x68c);
    Word cursor = load(vector, 4);
    Word selected = 0;
    if (cursor > load(vector, 8)) invalid_parameter();
    for (;;) {
        const Word end = load(vector, 8);
        if (load(vector, 4) > end) invalid_parameter();
        // Native CMP ESI,ESI / JZ always skips the impossible owner error.
        if (cursor == end) break;
        if (cursor >= load(vector, 8)) invalid_parameter();
        const Word effect = load(args);
        if (load(pointer(cursor)) == effect) {
            if (cursor >= load(vector, 8)) invalid_parameter();
            if (equal_native_string_headers_00435c40(
                    pointer(cursor + 0x11c), at(diagnostics, 0x684))) {
                if (cursor >= load(vector, 8)) invalid_parameter();
                selected = cursor;
            }
        }
        if (cursor >= load(vector, 8)) invalid_parameter();
        cursor += stride;
    }
    if (selected == 0) {
        alignas(4) unsigned char temporary[stride];
        initialize_native_material_diagnostics_record_00b106c0(temporary);
        CompletedRecord record_cleanup{temporary, *strings}; // native state0
        append_native_material_record_00b15610(vector, *strings, temporary);
        const Word end = load(vector, 8);
        if (load(vector, 4) > end) invalid_parameter();
        const Word last = end - stride;
        if (last > load(vector, 8) || last < load(vector, 4)) invalid_parameter();
        if (last >= load(vector, 8)) invalid_parameter();
        void* const group = at(diagnostics, 0x684);
        const Word effect = load(args);
        void* const first_header = pointer(last + 0x11c);
        selected = last;
        store(pointer(last), 0, effect);
        // The two native inlined assignments have exactly425F40's identity,
        // resize, fresh-length/data load and overlap-copy schedule.
        assign_native_string_header_00425f40(first_header, group, *strings);
        const void* const current_effect = pointer(load(pointer(last)));
        const void* const effect_name = native_effect_name_00b172d0(current_effect);
        Word concatenated[2];
        void* const result = concatenate_native_string_headers_004261a0(
            first_header, concatenated, effect_name, *strings);
        void* const second_header = pointer(last + 0x124);
        CompletedHeader header_cleanup{concatenated, *strings}; // native state1
        assign_native_string_header_00425f40(second_header, result, *strings);

        // Native captures the data pointer before leaving state1, then reads
        // the current length before the singleton getter. It never retries.
        void* const data = pointer(load(concatenated, 4));
        header_cleanup.armed = false; // state0, including a null data pointer
        if (data) {
            const Word size = load(concatenated) + 1u;
            auto* const pool = native_string_pool_get_or_create_00419cc0(
                strings->actual_published_01090aa8,
                strings->actual_manager_publication_01090aa0);
            return_native_string_pool_00bd1510(pool, data, size,
                strings->actual_small_returns_disabled_01090aa4);
        }
        record_cleanup.armed = false; // native state-1 before normal destruction
        destroy_native_renderer_record_00b10740(temporary, *strings);
    }
    add_draw_counters(pointer(selected), args);
}
} // namespace

__declspec(naked) void __fastcall record_native_material_diagnostics_draw_00b16f80(
    void*, NativeStringRawPoolContext&, const void*, std::uint32_t,
    std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t) {
    __asm {
        lea eax, [esp + 4]
        push eax
        push edx
        push ecx
        call record_draw_body
        add esp, 0Ch
        ret 18h
    }
}
} // namespace bsp
