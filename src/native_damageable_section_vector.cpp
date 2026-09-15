#include "bsp/native_damageable_section_vector.hpp"
#include "bsp/native_damageable_section.hpp"
#include "bsp/native_damageable_section_fill.hpp"
#include "bsp/native_alias_count_growth.hpp"

#include <intrin.h>
#include <new>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Damageable section vectors require MSVC Win32 x87.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
constexpr Word stride = 0x30;
constexpr Word maximum = 0x05555555;
static_assert(sizeof(void*) == 4 && sizeof(long) == 4);
Word address(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* pointer(Word p) noexcept { return reinterpret_cast<void*>(p); }
void* at(const void* p, Word offset) noexcept { return pointer(address(p) + offset); }
template<class T> volatile T& field(const void* p, Word offset = 0) noexcept {
    return *static_cast<volatile T*>(at(p, offset));
}
Word distance(Word first, Word end) noexcept {
    // SUB wraps before signed division, which truncates toward zero.
    return static_cast<Word>(static_cast<std::int32_t>(end - first) / 0x30);
}
using ReleaseOwner = void (__thiscall*)(void*);
using DeleteRecord = void (__thiscall*)(void*, Word);
void release_captured_owner(void* owner) {
    if (owner != nullptr && _InterlockedDecrement(&field<long>(owner, 4)) == 0)
        field<ReleaseOwner>(field<void*>(owner))(owner);
}
void invalid(const NativeDamageableSectionVectorAccess& access) {
    access.invalid_parameters.invalid_parameter(access.invalid_parameters.context);
}
} // namespace

Word size_native_damageable_section_vector_00744070(const void* header) {
    const Word begin = field<Word>(header, 4);
    return begin == 0 ? 0 : distance(begin, field<Word>(header, 8));
}

void* allocate_native_damageable_sections_00876a10(Word count) {
    if (count != 0 && 0xffffffffu / count < stride) throw std::bad_alloc();
    return singleton_lifetime_allocate(
        {SingletonAllocationKind::object, count * stride, count * stride});
}

void* assign_native_damageable_section_00878790(void* destination, const void* source) {
    field<Word>(destination, 4) = field<Word>(source, 4);
    field<Word>(destination, 8) = field<Word>(source, 8);
    __asm {
        mov eax, destination
        mov edx, source
        fld dword ptr [edx+0ch]
        fstp dword ptr [eax+0ch]
        fld dword ptr [edx+10h]
        fstp dword ptr [eax+10h]
        fld dword ptr [edx+14h]
        fstp dword ptr [eax+14h]
        fld dword ptr [edx+18h]
        fstp dword ptr [eax+18h]
        fld dword ptr [edx+1ch]
        fstp dword ptr [eax+1ch]
        fld dword ptr [edx+20h]
        fstp dword ptr [eax+20h]
    }
    void* const incoming = field<void*>(source, 0x24);
    void* const previous = field<void*>(destination, 0x24);
    if (previous != incoming) {
        field<void*>(destination, 0x24) = incoming;
        if (incoming != nullptr) _InterlockedIncrement(&field<long>(incoming, 4));
        release_captured_owner(previous);
    }
    __asm {
        mov eax, destination
        mov edx, source
        fld dword ptr [edx+28h]
        fstp dword ptr [eax+28h]
        fld dword ptr [edx+2ch]
        fstp dword ptr [eax+2ch]
    }
    return destination;
}

void destroy_native_damageable_section_range_00878ac0(void* first, const void* end) {
    while (first != end) {
        field<DeleteRecord>(field<void*>(first))(first, 0);
        first = at(first, stride);
    }
}

void* copy_backward_native_damageable_sections_00878d60(
    const void* first, const void* end, void* destination_end) {
    const Word result = address(destination_end) - distance(address(first), address(end)) * stride;
    if (first != end) {
        const Word delta = address(destination_end) - address(end);
        do {
            end = at(end, 0u - stride);
            assign_native_damageable_section_00878790(at(end, delta), end);
        } while (first != end);
    }
    return pointer(result);
}

void assign_fill_native_damageable_sections_00879390(
    void* first, const void* end, const void* source) {
    while (first != end) {
        assign_native_damageable_section_00878790(first, source);
        first = at(first, stride);
    }
}

void* uninitialized_copy_native_damageable_sections_008794f0(
    const void* first, const void* end, void* destination, Word actual_vtable) {
    void* const begin = destination;
    try {
        while (first != end) {
            // State1 placement cleanup C965C0 calls the verified bare RET401130.
            if (destination != nullptr)
                copy_construct_native_damageable_section_00878b40(destination, first, actual_vtable);
            destination = at(destination, stride);
            first = at(first, stride);
        }
    } catch (...) {
        destroy_native_damageable_section_range_00878ac0(begin, destination);
        throw;
    }
    return destination;
}

void* copy_backward_native_damageable_sections_008798c0(
    const void* first, const void* end, void* destination_end) {
    return copy_backward_native_damageable_sections_00878d60(first, end, destination_end);
}

[[noreturn]] void throw_native_damageable_section_length_0087a580() {
    NativeLegacySboStringStorage temporary;
    temporary.capacity_18 = 15;
    temporary.length_14 = 0;
    temporary.buffer_04.inline_bytes[0] = '\0';
    native_legacy_sbo_string_assign_counted_00408720(temporary, "vector<T> too long", 18);
    try {
        // Reuse the owning28h native payload/copy/destructor provider. This is
        // its new host catch type, not the original D83F98 throw metadata.
        throw NativeAliasListLengthError{temporary};
    } catch (...) {
        native_legacy_sbo_string_destroy_004072d0(temporary);
        throw;
    }
}

void* uninitialized_fill_native_damageable_sections_0087ad20(
    void* first, Word count, const void* source, Word actual_vtable) {
    fill_native_damageable_sections_008798f0(first, count, source, actual_vtable);
    return at(first, count * stride);
}

void* uninitialized_copy_native_damageable_sections_0087b610(
    const void* first, const void* end, void* destination, Word actual_vtable) {
    return uninitialized_copy_native_damageable_sections_008794f0(first, end, destination, actual_vtable);
}

void insert_native_damageable_sections_0087b950(void* header,
    const void* iterator_owner, void* position, Word count,
    const void* source, Word actual_vtable) {
    (void)iterator_owner; // Native stack+4 is not read.
    alignas(4) unsigned char temporary[stride];
    copy_construct_native_damageable_section_00878b40(temporary, source, actual_vtable);
    try {
        const Word begin = field<Word>(header, 4);
        Word capacity = begin == 0 ? 0 : distance(begin, field<Word>(header, 12));
        if (count != 0) {
            const Word size = begin == 0 ? 0 : distance(begin, field<Word>(header, 8));
            if (maximum - size < count) throw_native_damageable_section_length_0087a580();
            const Word current_size = begin == 0 ? 0 : distance(begin, field<Word>(header, 8));
            if (capacity < current_size + count) {
                capacity = maximum - (capacity >> 1) < capacity ? 0 : capacity + (capacity >> 1);
                const Word reloaded_size = begin == 0 ? 0 : distance(begin, field<Word>(header, 8));
                if (capacity < reloaded_size + count)
                    capacity = size_native_damageable_section_vector_00744070(header) + count;
                void* const allocated = allocate_native_damageable_sections_00876a10(capacity);
                void* completed = allocated;
                try {
                    completed = uninitialized_copy_native_damageable_sections_008794f0(
                        field<void*>(header, 4), position, allocated, actual_vtable);
                    completed = uninitialized_fill_native_damageable_sections_0087ad20(
                        completed, count, temporary, actual_vtable);
                    uninitialized_copy_native_damageable_sections_008794f0(
                        position, field<void*>(header, 8), completed, actual_vtable);
                } catch (...) {
                    destroy_native_damageable_section_range_00878ac0(allocated, completed);
                    singleton_lifetime_free(allocated);
                    throw;
                }
                // Catch state1 is already disarmed. Old-destruction failures
                // do not reclaim the successful new allocation or roll back.
                void* const old_begin = field<void*>(header, 4);
                const Word old_size = old_begin == nullptr ? 0 :
                    distance(address(old_begin), field<Word>(header, 8));
                const Word new_size = count + old_size;
                if (old_begin != nullptr) {
                    destroy_native_damageable_section_range_00878ac0(old_begin, field<void*>(header, 8));
                    singleton_lifetime_free(field<void*>(header, 4));
                }
                field<void*>(header, 12) = at(allocated, capacity * stride);
                field<void*>(header, 8) = at(allocated, new_size * stride);
                field<void*>(header, 4) = allocated;
            } else {
                void* const old_end = field<void*>(header, 8);
                const Word bytes = count * stride;
                if (distance(address(position), address(old_end)) < count) {
                    uninitialized_copy_native_damageable_sections_0087b610(
                        position, old_end, at(position, bytes), actual_vtable);
                    void* const reloaded_end = field<void*>(header, 8);
                    const Word fill_count = count - distance(address(position), address(reloaded_end));
                    try {
                        uninitialized_fill_native_damageable_sections_0087ad20(
                            reloaded_end, fill_count, temporary, actual_vtable);
                    } catch (...) {
                        destroy_native_damageable_section_range_00878ac0(
                            at(position, bytes), at(field<void*>(header, 8), bytes));
                        throw;
                    }
                    field<Word>(header, 8) = field<Word>(header, 8) + bytes;
                    void* const assignment_end = pointer(field<Word>(header, 8) - bytes);
                    assign_fill_native_damageable_sections_00879390(position, assignment_end, temporary);
                } else {
                    void* const split = at(old_end, 0u - bytes);
                    void* const constructed_end = uninitialized_copy_native_damageable_sections_0087b610(
                        split, old_end, old_end, actual_vtable);
                    field<void*>(header, 8) = constructed_end;
                    copy_backward_native_damageable_sections_008798c0(position, split, old_end);
                    assign_fill_native_damageable_sections_00879390(position, at(position, bytes), temporary);
                }
            }
        }
    } catch (...) {
        destroy_native_damageable_section_00878ef0(temporary, actual_vtable);
        throw;
    }
    // Normal state is -1 before this inline release. Unlike the unwind helper,
    // normal exit neither rewrites temporary.vptr nor clears temporary+24.
    release_captured_owner(field<void*>(temporary, 0x24));
}

void* insert_checked_native_damageable_section_0087c2d0(void* header,
    void* result, const void* iterator_owner, void* position,
    const void* source, const NativeDamageableSectionVectorAccess& access) {
    const Word begin = field<Word>(header, 4);
    Word offset = 0;
    if (begin != 0) {
        const Word end = field<Word>(header, 8);
        if (distance(begin, end) != 0) {
            if (end < begin) invalid(access);
            if (iterator_owner == nullptr || iterator_owner != header) invalid(access);
            offset = distance(begin, address(position));
        }
    }
    insert_native_damageable_sections_0087b950(header, iterator_owner,
        position, 1, source, access.actual_vtable_00d0df04);
    const Word new_begin = field<Word>(header, 4);
    if (new_begin > field<Word>(header, 8)) invalid(access);
    const Word new_position = new_begin + offset * stride;
    if (new_position > field<Word>(header, 8) || new_position < field<Word>(header, 4)) invalid(access);
    field<Word>(result, 4) = new_position;
    field<void*>(result) = header;
    return result;
}

void append_native_damageable_section_0087c870(void* header,
    const void* source, const NativeDamageableSectionVectorAccess& access) {
    const Word begin = field<Word>(header, 4);
    const Word size = begin == 0 ? 0 : distance(begin, field<Word>(header, 8));
    if (begin != 0 && size < distance(begin, field<Word>(header, 12))) {
        void* const end = field<void*>(header, 8);
        fill_native_damageable_sections_008798f0(end, 1, source, access.actual_vtable_00d0df04);
        field<void*>(header, 8) = at(end, stride);
        return;
    }
    void* const end = field<void*>(header, 8);
    if (begin > address(end)) invalid(access);
    Word result[2];
    insert_checked_native_damageable_section_0087c2d0(
        header, result, header, end, source, access);
}

} // namespace bsp
