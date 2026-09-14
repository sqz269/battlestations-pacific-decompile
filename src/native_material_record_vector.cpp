#include "bsp/native_material_record_vector.hpp"

#include "bsp/native_material_record_allocation.hpp"
#include "bsp/native_material_record_assignment.hpp"
#include "bsp/native_material_record_copy.hpp"
#include "bsp/native_material_record_ranges.hpp"
#include "bsp/native_renderer_begin_frame.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstdlib>
#include <exception>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native material record vectors require MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
constexpr Word stride = 300;
constexpr Word maximum_count = 0x00da740d;
static_assert(sizeof(void*) == 4 && sizeof(Word) == 4);

Word bits(const void* address) noexcept {
    return static_cast<Word>(reinterpret_cast<std::uintptr_t>(address));
}
void* pointer(Word address) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(address));
}
void* at(const void* address, Word byte_offset) noexcept {
    return pointer(bits(address) + byte_offset);
}

__declspec(noinline) Word load(const void* address, Word byte_offset = 0) noexcept {
    return *reinterpret_cast<volatile Word*>(at(address, byte_offset));
}
__declspec(noinline) void store(void* address, Word byte_offset, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(at(address, byte_offset)) = value;
}

// Keep the original single DWORD read/modify/write at B1482B.
__declspec(naked) void __fastcall add_current(void*, Word) noexcept {
    __asm {
        add dword ptr [ecx], edx
        ret
    }
}

// EAX is the32-bit pattern of signed32(wrapped byte difference)/300.
__declspec(naked) Word __fastcall record_quotient(Word) noexcept {
    __asm {
        mov eax, 1B4E81B5h
        imul ecx
        sar edx, 5
        mov eax, edx
        shr eax, 1Fh
        add eax, edx
        ret
    }
}

__declspec(noinline) void __cdecl current_invalid_parameter() {
    _invalid_parameter_noinfo();
}

// Native state0/CBC370 destroys the completed temporary on unwind. Normal
// destruction is explicitly disarmed first and remains allowed to throw.
struct CompletedTemporary {
    void* record;
    NativeStringRawPoolContext& strings;
    bool armed = true;
    ~CompletedTemporary() noexcept {
        if (armed) {
            try {
                destroy_native_renderer_record_00b10740(record, strings);
            } catch (...) {
                std::terminate();
            }
        }
    }
};

// args points into the PUBLIC naked adapter's original four argument words:
// ignored owner+0, position+4, count/current cursor+8, source/allocation+Ch.
__declspec(noinline) void __cdecl insert_body(
    void* vector, NativeStringRawPoolContext* strings, void* args) {
    alignas(4) unsigned char temporary[stride];
    const Word source = load(args, 12);
    construct_native_material_record_00b13070(temporary, *strings, pointer(source));
    const Word begin = load(vector, 4);
    CompletedTemporary cleanup{temporary, *strings};
    const Word capacity = begin ? record_quotient(load(vector, 12) - begin) : 0;
    const Word count = load(args, 8);
    if (count != 0) {
        const Word size = begin ? record_quotient(load(vector, 8) - begin) : 0;
        if (maximum_count - size < count) STL_xlen_throw_00b135c0();
        const Word repeated_size = begin ? record_quotient(load(vector, 8) - begin) : 0;
        if (capacity < repeated_size + count) {
            const Word half = capacity >> 1;
            Word growth = maximum_count - half < capacity ? 0 : capacity + half;
            const Word size_for_growth = begin ? record_quotient(load(vector, 8) - begin) : 0;
            if (growth < size_for_growth + count) {
                growth = static_cast<Word>(
                    get_native_material_record_vector_size_00b0d230(vector)) + count;
            }
            const Word allocation = bits(allocate_native_material_records_00b0d3b0(growth, nullptr));
            const Word copy_begin = load(vector, 4);
            store(args, 8, allocation);
            const Word copy_position = load(args, 4);
            store(args, 12, allocation);
            // Native state1: only these three construction calls are covered.
            try {
                Word cursor = bits(copy_construct_native_material_records_00b13920(
                    pointer(copy_begin), pointer(copy_position), pointer(allocation), *strings));
                store(args, 8, cursor);
                cursor = bits(fill_construct_native_material_record_tail_00b14360(
                    pointer(cursor), count, temporary, *strings));
                const Word copy_end = load(vector, 8);
                store(args, 8, cursor);
                const Word current_position = load(args, 4);
                copy_construct_native_material_records_00b13920(
                    pointer(current_position), pointer(copy_end), pointer(cursor), *strings);
            } catch (...) {
                // B1479B reads the actual reused argument slots on catch entry.
                const Word completed_end = load(args, 8);
                const Word captured_allocation = load(args, 12);
                destroy_native_material_record_range_00b143a0(
                    pointer(captured_allocation), pointer(completed_end), *strings);
                singleton_lifetime_free(pointer(captured_allocation));
                throw;
            }
            // Native state0 before old-record destruction. No allocation guard
            // extends across this phase; an old destructor failure is not repaired.
            const Word old_begin = load(vector, 4);
            const Word old_size = old_begin ? record_quotient(load(vector, 8) - old_begin) : 0;
            const Word new_size = count + old_size;
            if (old_begin) {
                const Word old_end = load(vector, 8);
                destroy_native_material_record_range_00b143a0(
                    pointer(old_begin), pointer(old_end), *strings);
                singleton_lifetime_free(pointer(load(vector, 4)));
            }
            const Word current_allocation = load(args, 12);
            store(vector, 12, current_allocation + growth * stride);
            store(vector, 8, current_allocation + new_size * stride);
            store(vector, 4, current_allocation);
        } else {
            const Word old_end = load(vector, 8);
            const Word position = load(args, 4);
            const Word trailing = record_quotient(old_end - position);
            const bool short_tail = trailing < count;
            store(args, 12, old_end);
            const Word count_bytes = count * stride;
            if (short_tail) {
                store(args, 12, count_bytes);
                copy_construct_native_material_record_tail_00b14550(
                    pointer(position), pointer(old_end), pointer(position + count_bytes), *strings);
                const Word current_end = load(vector, 8);
                const Word remaining = count - record_quotient(current_end - position);
                // Native state3 starts only after the preceding tail copy.
                try {
                    fill_construct_native_material_record_tail_00b14360(
                        pointer(current_end), remaining, temporary, *strings);
                } catch (...) {
                    const Word cleanup_bytes = load(args, 8) * stride;
                    const Word cleanup_end = load(vector, 8) + cleanup_bytes;
                    const Word cleanup_first = load(args, 4) + cleanup_bytes;
                    destroy_native_material_record_range_00b143a0(
                        pointer(cleanup_first), pointer(cleanup_end), *strings);
                    throw;
                }
                const Word current_count_bytes = load(args, 12);
                add_current(at(vector, 8), current_count_bytes);
                const Word assignment_end = load(vector, 8) - current_count_bytes;
                fill_assign_native_material_records_00b13ad0(
                    pointer(position), pointer(assignment_end), temporary, *strings);
            } else {
                const Word first_of_tail = old_end - count_bytes;
                store(args, 8, first_of_tail);
                const Word constructed_end = bits(copy_construct_native_material_record_tail_00b14550(
                    pointer(first_of_tail), pointer(old_end), pointer(old_end), *strings));
                const Word current_first_of_tail = load(args, 8);
                store(vector, 8, constructed_end);
                const Word captured_old_end = load(args, 12);
                copy_assign_native_material_record_head_wrapper_00b13f50(
                    pointer(position), pointer(current_first_of_tail), pointer(captured_old_end), *strings);
                fill_assign_native_material_records_00b13ad0(
                    pointer(position), pointer(position + count_bytes), temporary, *strings);
            }
        }
    }
    cleanup.armed = false;
    destroy_native_renderer_record_00b10740(temporary, *strings);
}

// B150D0 public slots: output pair+0, iterator owner+4, position+8, source+Ch.
__declspec(noinline) void* __cdecl insert_one_body(
    void* vector, NativeStringRawPoolContext* strings, void* args) {
    const Word position = load(args, 8);
    const Word begin = load(vector, 4);
    Word end = 0;
    Word size = 0;
    if (begin) {
        end = load(vector, 8);
        size = record_quotient(end - begin);
    }
    Word iterator_owner;
    Word index = 0;
    if (size != 0) {
        if (begin > end) current_invalid_parameter();
        iterator_owner = load(args, 4);
        if (iterator_owner == 0 || iterator_owner != bits(vector)) current_invalid_parameter();
        index = record_quotient(position - begin);
    } else {
        iterator_owner = load(args, 4);
    }
    const Word source = load(args, 12);
    insert_native_material_records_00b145e0(
        vector, *strings, pointer(iterator_owner), pointer(position), 1, pointer(source));
    const Word captured_begin = load(vector, 4);
    const Word current_end = load(vector, 8);
    if (captured_begin > current_end) current_invalid_parameter();
    const Word result_position = captured_begin + index * stride;
    const bool above_end = result_position > load(vector, 8);
    store(args, 8, captured_begin);
    if (above_end || result_position < load(vector, 4)) current_invalid_parameter();
    void* const output_pair = pointer(load(args));
    store(output_pair, 0, bits(vector));
    store(output_pair, 4, result_position);
    return output_pair;
}

__declspec(noinline) void __cdecl append_body(
    void* vector, NativeStringRawPoolContext* strings, void* args) {
    const Word begin = load(vector, 4);
    const Word size = begin ? record_quotient(load(vector, 8) - begin) : 0;
    const bool spare = begin && size < record_quotient(load(vector, 12) - begin);
    if (spare) {
        (void)load(args); // Native duplicate input read used for ignored child padding.
        const Word source = load(args);
        const Word captured_end = load(vector, 8);
        fill_construct_native_material_records_00b13b00(
            pointer(captured_end), 1, pointer(source), *strings);
        store(vector, 8, captured_end + stride);
    } else {
        const Word captured_end = load(vector, 8);
        if (begin > captured_end) current_invalid_parameter();
        const Word source = load(args);
        Word output_pair[2];
        insert_one_native_material_record_00b150d0(
            vector, *strings, output_pair, vector, pointer(captured_end), pointer(source));
    }
}
} // namespace

__declspec(naked) void __fastcall insert_native_material_records_00b145e0(
    void*, NativeStringRawPoolContext&, void*, void*, std::uint32_t, const void*) {
    __asm {
        lea eax, [esp + 4]
        push eax
        push edx
        push ecx
        call insert_body
        add esp, 0Ch
        ret 10h
    }
}

__declspec(naked) void* __fastcall insert_one_native_material_record_00b150d0(
    void*, NativeStringRawPoolContext&, void*, void*, void*, const void*) {
    __asm {
        lea eax, [esp + 4]
        push eax
        push edx
        push ecx
        call insert_one_body
        add esp, 0Ch
        ret 10h
    }
}

__declspec(naked) void __fastcall append_native_material_record_00b15610(
    void*, NativeStringRawPoolContext&, const void*) {
    __asm {
        lea eax, [esp + 4]
        push eax
        push edx
        push ecx
        call append_body
        add esp, 0Ch
        ret 4
    }
}
} // namespace bsp
