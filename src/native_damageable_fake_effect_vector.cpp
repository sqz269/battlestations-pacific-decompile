#include "bsp/native_damageable_fake_effect_vector.hpp"
#include "bsp/native_alias_count_growth.hpp"
#include "bsp/native_damageable_class_construction.hpp"
#include "bsp/native_damageable_section.hpp"

#include <cstdlib>
#include <exception>
#include <intrin.h>
#include <new>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Damageable fake-effect vectors require MSVC Win32 x87.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(long) == 4);
Word address(const void* p) noexcept { return reinterpret_cast<Word>(p); }
void* at(const void* p, Word offset) noexcept {
    return reinterpret_cast<void*>(address(p) + offset);
}
template<class T> volatile T& field(const void* p, Word offset = 0) noexcept {
    return *static_cast<volatile T*>(at(p, offset));
}
Word distance(const void* end, const void* first) noexcept {
    return static_cast<Word>(static_cast<std::int32_t>(address(end) - address(first)) >> 4);
}
Word size_from(const void* header, const void* first) noexcept {
    return first ? distance(field<void*>(header, 8), first) : 0;
}
using ReleaseOwner = void (__thiscall*)(void*);
void release_owner(void* owner) {
    if (owner && _InterlockedDecrement(&field<long>(owner, 4)) == 0) {
        void* const table = field<void*>(owner);
        field<ReleaseOwner>(table)(owner);
    }
}
void invalid(const SingletonLifetimeCallbacks& callbacks) {
    if (callbacks.invalid_parameter) callbacks.invalid_parameter(callbacks.context);
    else _invalid_parameter_noinfo();
}
void transfer_float(void* destination, const void* source) {
    // Actual FLD/FSTP, including masked sNaN quieting and x87 status/control.
    __asm {
        mov eax, source
        mov edx, destination
        fld dword ptr [eax]
        fstp dword ptr [edx]
    }
}
void assign_row(void* destination, const void* source) {
    transfer_float(destination, source);
    field<Word>(destination, 4) = field<Word>(source, 4);
    field<Word>(destination, 8) = field<Word>(source, 8);
    void* const incoming = field<void*>(source, 0x0c);
    void* const old = field<void*>(destination, 0x0c);
    if (old != incoming) {
        field<void*>(destination, 0x0c) = incoming;
        if (incoming) _InterlockedIncrement(&field<long>(incoming, 4));
        release_owner(old);
    }
}
void construct_row(void* destination, const void* source) {
    if (!destination) return;
    transfer_float(destination, source);
    field<Word>(destination, 4) = field<Word>(source, 4);
    field<Word>(destination, 8) = field<Word>(source, 8);
    field<void*>(destination, 0x0c) = nullptr;
    void* const incoming = field<void*>(source, 0x0c);
    if (incoming) {
        field<void*>(destination, 0x0c) = incoming;
        _InterlockedIncrement(&field<long>(incoming, 4));
    }
}
void unwind_row(void* row) noexcept {
    // C96810/C968E0 are FH3 unwind actions, not catch-body cleanup calls.
    // A second exception escaping a true unwind action terminates.
    try {
        destroy_native_damageable_fake_effect_row_00878a60(row);
    } catch (...) {
        std::terminate();
    }
}
} // namespace

void* allocate_native_damageable_fake_effect_rows_008769b0(Word count) {
    if (count > 0x0fffffff) throw std::bad_alloc();
    const Word bytes = count << 4;
    return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes});
}

[[noreturn]] void throw_native_damageable_fake_effect_length_0087a510() {
    NativeLegacySboStringStorage temporary;
    temporary.capacity_18 = 15;
    temporary.length_14 = 0;
    temporary.buffer_04.inline_bytes[0] = '\0';
    native_legacy_sbo_string_assign_counted_00408720(temporary, "vector<T> too long", 18);
    // C966B0: only armed after assignment returns. The existing transport has
    // the identical D69260 vtable/payload/copy/destructor; new host exception ABI.
    struct Completed {
        NativeLegacySboStringStorage& string;
        ~Completed() noexcept { native_legacy_sbo_string_destroy_004072d0(string); }
    } completed{temporary};
    throw NativeAliasListLengthError{temporary};
}

void destroy_native_damageable_fake_effect_row_00878a60(void* row) {
    release_native_ref_counted_handle_0041de40(at(row, 0x0c));
}

void fill_native_damageable_fake_effect_rows_00878c40(void* first,
    const void* end, const void* row) {
    while (first != end) {
        assign_row(first, row);
        first = at(first, 0x10);
    }
}

void* copy_native_damageable_fake_effect_rows_00878820(const void* first,
    const void* end, void* destination) {
    while (first != end) {
        assign_row(destination, first);
        first = at(first, 0x10);
        destination = at(destination, 0x10);
    }
    return destination;
}

void* copy_backward_native_damageable_fake_effect_rows_008788d0(const void* first,
    const void* end, void* destination_end) {
    while (first != end) {
        end = at(end, 0xfffffff0);
        destination_end = at(destination_end, 0xfffffff0);
        assign_row(destination_end, end);
    }
    return destination_end;
}

void* copy_native_damageable_fake_effect_rows_00879330(const void* first,
    const void* end, void* destination) {
    copy_native_damageable_fake_effect_rows_00878820(first, end, destination);
    return at(destination, distance(end, first) << 4);
}

void* copy_backward_native_damageable_fake_effect_rows_00879870(const void* first,
    const void* end, void* destination_end) {
    copy_backward_native_damageable_fake_effect_rows_008788d0(first, end, destination_end);
    return at(destination_end, 0u - (distance(end, first) << 4));
}

void* uninitialized_copy_native_damageable_fake_effect_rows_008799f0(
    const void* first, const void* end, void* destination) {
    while (first != end) {
        construct_row(destination, first);
        destination = at(destination, 0x10);
        first = at(first, 0x10);
    }
    return destination;
}

void uninitialized_fill_native_damageable_fake_effect_rows_0087ab80(
    void* destination, Word count, const void* row) {
    while (count != 0) {
        construct_row(destination, row);
        --count;
        destination = at(destination, 0x10);
    }
}

void* uninitialized_fill_native_damageable_fake_effect_rows_0087b5d0(
    void* destination, Word count, const void* row) {
    uninitialized_fill_native_damageable_fake_effect_rows_0087ab80(destination, count, row);
    return at(destination, count << 4);
}

void* uninitialized_copy_native_damageable_fake_effect_rows_0087bc50(
    const void* first, const void* end, void* destination) {
    return uninitialized_copy_native_damageable_fake_effect_rows_008799f0(first, end, destination);
}

void destroy_native_damageable_fake_effect_rows_0087b5b0(void* first, const void* end) {
    release_native_damageable_owner_rows_0087ab30(first, end);
}

NativeDamageableFakeEffectIterator* erase_native_damageable_fake_effect_vector_0087b8f0(
    void* header, NativeDamageableFakeEffectIterator* output,
    NativeDamageableFakeEffectIterator first, NativeDamageableFakeEffectIterator last,
    const SingletonLifetimeCallbacks& callbacks) {
    if (!first.owner || first.owner != last.owner) invalid(callbacks);
    if (first.position != last.position) {
        void* const next_end = copy_native_damageable_fake_effect_rows_00879330(
            last.position, field<void*>(header, 8), first.position);
        release_native_damageable_owner_rows_0087ab30(next_end, field<void*>(header, 8));
        field<void*>(header, 8) = next_end;
    }
    field<void*>(output) = first.owner;
    field<void*>(output, 4) = first.position;
    return output;
}

void insert_native_damageable_fake_effect_vector_0087c380(void* header,
    void* iterator_owner, void* position, Word count, const void* row) {
    (void)iterator_owner;
    // 87C39E..C3D1 snapshot load order is +0,+8,+4,+C. The float is MOVSS,
    // so do not quiet it here. The temporary is armed only after retention.
    const Word bits0 = field<Word>(row);
    const Word bits8 = field<Word>(row, 8);
    const Word bits4 = field<Word>(row, 4);
    void* const owner = field<void*>(row, 0x0c);
    NativeDamageableFakeEffectRow temporary{{bits0, bits4, bits8, 0}};
    if (owner) {
        field<void*>(&temporary, 0x0c) = owner;
        _InterlockedIncrement(&field<long>(owner, 4));
    }
    try {
        void* const first = field<void*>(header, 4);
        Word capacity = first ? distance(field<void*>(header, 0x0c), first) : 0;
        if (count) {
            if (0x0fffffffu - size_from(header, first) < count)
                throw_native_damageable_fake_effect_length_0087a510();
            if (capacity < size_from(header, first) + count) {
                capacity = 0x0fffffffu - (capacity >> 1) < capacity
                    ? 0 : capacity + (capacity >> 1);
                if (capacity < size_from(header, first) + count)
                    capacity = size_from(header, first) + count;
                void* const allocation = allocate_native_damageable_fake_effect_rows_008769b0(capacity);
                void* completed_end = allocation;
                try {
                    completed_end = uninitialized_copy_native_damageable_fake_effect_rows_008799f0(
                        field<void*>(header, 4), position, allocation);
                    completed_end = uninitialized_fill_native_damageable_fake_effect_rows_0087b5d0(
                        completed_end, count, &temporary);
                    uninitialized_copy_native_damageable_fake_effect_rows_008799f0(
                        position, field<void*>(header, 8), completed_end);
                } catch (...) {
                    // DC8C70 state1 catch 87C518; only COMPLETED helper calls
                    // advance the checkpoint. Never include partial suffix rows.
                    destroy_native_damageable_fake_effect_rows_0087b5b0(allocation, completed_end);
                    singleton_lifetime_free(allocation);
                    throw;
                }
                // State already 0: failures while releasing old storage do NOT
                // free the new allocation or roll back completed copies.
                void* const old_first = field<void*>(header, 4);
                const Word new_size = count + size_from(header, old_first);
                if (old_first) {
                    release_native_damageable_owner_rows_0087ab30(old_first, field<void*>(header, 8));
                    singleton_lifetime_free(field<void*>(header, 4));
                }
                field<void*>(header, 0x0c) = at(allocation, capacity << 4);
                field<void*>(header, 8) = at(allocation, new_size << 4);
                field<void*>(header, 4) = allocation;
            } else {
                void* const old_end = field<void*>(header, 8);
                const Word bytes = count << 4;
                if (distance(old_end, position) < count) {
                    uninitialized_copy_native_damageable_fake_effect_rows_0087bc50(
                        position, old_end, at(position, bytes));
                    void* const fill_first = field<void*>(header, 8);
                    try {
                        uninitialized_fill_native_damageable_fake_effect_rows_0087b5d0(
                            fill_first, count - distance(fill_first, position), &temporary);
                    } catch (...) {
                        // DC8C84 state3 catch 87C5A1 uses CURRENT header end.
                        destroy_native_damageable_fake_effect_rows_0087b5b0(
                            at(position, bytes), at(field<void*>(header, 8), bytes));
                        throw;
                    }
                    field<void*>(header, 8) = at(field<void*>(header, 8), bytes);
                    void* const fill_end = at(field<void*>(header, 8), 0u - bytes);
                    fill_native_damageable_fake_effect_rows_00878c40(position, fill_end, &temporary);
                } else {
                    void* const last_n = at(old_end, 0u - bytes);
                    field<void*>(header, 8) = uninitialized_copy_native_damageable_fake_effect_rows_0087bc50(
                        last_n, old_end, old_end);
                    copy_backward_native_damageable_fake_effect_rows_00879870(position, last_n, old_end);
                    fill_native_damageable_fake_effect_rows_00878c40(position, at(position, bytes), &temporary);
                }
            }
        }
    } catch (...) {
        unwind_row(&temporary);
        throw;
    }
    // Native state -1 is published before this release: do not release twice
    // if the actual vslot throws. Ordinary epilogue does not clear the slot.
    release_owner(field<void*>(&temporary, 0x0c));
}

void resize_native_damageable_fake_effect_vector_0087c920(void* header,
    Word new_size, NativeDamageableFakeEffectRow owned_default,
    const SingletonLifetimeCallbacks& callbacks) {
    try {
        void* const first = field<void*>(header, 4);
        if (size_from(header, first) < new_size) {
            const Word old_size = size_from(header, first);
            void* const end = field<void*>(header, 8);
            if (address(first) > address(end)) invalid(callbacks);
            insert_native_damageable_fake_effect_vector_0087c380(
                header, header, end, new_size - old_size, &owned_default);
        } else if (first) {
            void* const end = field<void*>(header, 8);
            if (new_size < distance(end, first)) {
                if (address(first) > address(end)) invalid(callbacks);
                void* const current_first = field<void*>(header, 4);
                if (address(current_first) > address(field<void*>(header, 8))) invalid(callbacks);
                void* const position = at(current_first, new_size << 4);
                if (address(position) > address(field<void*>(header, 8)) ||
                    address(position) < address(field<void*>(header, 4))) invalid(callbacks);
                NativeDamageableFakeEffectIterator output;
                erase_native_damageable_fake_effect_vector_0087b8f0(header, &output,
                    {header, position}, {header, end}, callbacks);
            }
        }
    } catch (...) {
        unwind_row(&owned_default);
        throw;
    }
    release_owner(field<void*>(&owned_default, 0x0c));
}

} // namespace bsp
