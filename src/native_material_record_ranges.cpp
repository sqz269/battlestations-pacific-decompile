#include "bsp/native_material_record_ranges.hpp"
#include "bsp/native_material_record_copy.hpp"
#include "bsp/native_renderer_begin_frame.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native material record ranges require MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
using Context = NativeStringRawPoolContext;
constexpr Word stride = 0x12c;
static_assert(sizeof(void*) == sizeof(Word));

Word bits(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(p); }

Word read_argument(const void* slot) noexcept {
    Word value;
    __asm { mov eax, slot }
    __asm { mov eax, dword ptr [eax] }
    __asm { mov value, eax }
    return value;
}

void publish_argument(void* slot, Word value) noexcept {
    __asm { mov eax, slot }
    __asm { mov edx, value }
    __asm { mov dword ptr [eax], edx }
}

void destroy_ascending(Word first, Word last, Context& strings) {
    while (first != last) {
        destroy_native_renderer_record_00b10740(pointer(first), strings);
        first += stride;
    }
}

void __cdecl fill_body(void* output, Word count, Context* strings,
    const void* source_slot) {
    const Word initial_output = bits(output);
    Word current_output = initial_output;
    volatile Word completed_output = initial_output;
    try {
        while (count != 0) {
            if (current_output != 0) {
                const void* const source = pointer(read_argument(source_slot));
                construct_native_material_record_00b13070(
                    pointer(current_output), *strings, source);
            }
            --count;
            current_output += stride;
            completed_output = current_output;
        }
    } catch (...) {
        const Word cleanup_end = completed_output;
        destroy_ascending(initial_output, cleanup_end, *strings);
        throw;
    }
}

void* __cdecl copy_body(const void* first, const void* last, Context* strings,
    void* output_slot) {
    Word current_input = bits(first);
    const Word input_end = bits(last);
    const Word initial_output = read_argument(output_slot);
    Word current_output = initial_output;
    try {
        while (current_input != input_end) {
            if (current_output != 0) {
                construct_native_material_record_00b13070(
                    pointer(current_output), *strings, pointer(current_input));
            }
            current_output += stride;
            publish_argument(output_slot, current_output);
            current_input += stride;
        }
    } catch (...) {
        const Word cleanup_end = read_argument(output_slot);
        destroy_ascending(initial_output, cleanup_end, *strings);
        throw;
    }
    return pointer(current_output);
}
} // namespace

__declspec(naked) void __fastcall fill_construct_native_material_records_00b13b00(
    void*, Word, const void*, Context&) {
    __asm { lea eax, [esp + 4] }
    __asm { push eax }
    __asm { mov eax, dword ptr [esp + 0ch] }
    __asm { push eax }
    __asm { push edx }
    __asm { push ecx }
    __asm { call fill_body }
    __asm { add esp, 10h }
    __asm { ret 8 }
}

__declspec(naked) void* __fastcall copy_construct_native_material_records_00b13920(
    const void*, const void*, void*, Context&) {
    __asm { lea eax, [esp + 4] }
    __asm { push eax }
    __asm { mov eax, dword ptr [esp + 0ch] }
    __asm { push eax }
    __asm { push edx }
    __asm { push ecx }
    __asm { call copy_body }
    __asm { add esp, 10h }
    __asm { ret 8 }
}

__declspec(naked) void* __stdcall fill_construct_native_material_record_tail_00b14360(
    void*, Word, const void*, Context&) {
    __asm {
        push esi
        mov esi, dword ptr [esp + 0ch]
        push edi
        mov edi, dword ptr [esp + 0ch]
        mov edx, esi
        mov ecx, edi
        push dword ptr [esp + 018h]
        push dword ptr [esp + 018h]
        call fill_construct_native_material_records_00b13b00
        mov eax, esi
        imul eax, eax, 012ch
        add eax, edi
        pop edi
        pop esi
        ret 010h
    }
}

void __stdcall destroy_native_material_record_range_00b143a0(
    void* first, const void* last, Context& strings) {
    destroy_ascending(bits(first), bits(last), strings);
}
} // namespace bsp
