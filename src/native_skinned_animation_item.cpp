#include "bsp/native_skinned_animation_item.hpp"
#include "bsp/native_camera_group_resource.hpp"
#include "bsp/native_resource_value_reads.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native animation resource parsing requires MSVC Win32 x87 assembly.
#endif
namespace bsp {
namespace {
using U = std::uint32_t;
static_assert(sizeof(void*) == 4);
void* ptr(U v) noexcept { return reinterpret_cast<void*>(v); }
U bits(const void* p) noexcept { return static_cast<U>(reinterpret_cast<std::uintptr_t>(p)); }
void* at(const void* p, U offset = 0) noexcept { return ptr(bits(p) + offset); }
U word(const void* p, U offset = 0) noexcept { return *static_cast<const volatile U*>(at(p, offset)); }
void put(void* p, U offset, U value) noexcept { *static_cast<volatile U*>(at(p, offset)) = value; }
std::int32_t signed_word(U v) noexcept { return static_cast<std::int32_t>(v); }
U grow(U capacity) noexcept { const U doubled = capacity * 2u; return signed_word(doubled) > 1 ? doubled : 1u; }
void* allocate(U bytes) { return singleton_lifetime_allocate({SingletonAllocationKind::object, bytes, bytes}); }
void give_back(void* data, U bytes, NativeStringRawPoolContext& strings) {
    auto* const pool = native_string_pool_get_or_create_00419cc0(
        strings.actual_published_01090aa8, strings.actual_manager_publication_01090aa0);
    return_native_string_pool_00bd1510(pool, data, bytes, strings.actual_small_returns_disabled_01090aa4);
}
void zero_vector(void* v) noexcept { put(v, 0, 0); put(v, 4, 0); put(v, 8, 0); }
void copy_values(void* destination, const void* source) noexcept {
    // Original REP MOVSD is a forward twelve-DWORD copy, including overlap.
    for (U n = 0; n != 0x30; n += 4) put(destination, n, word(source, n));
}
void copy_name(void* destination, const void* source, NativeStringRawPoolContext& strings) {
    put(destination, 0, 0); put(destination, 4, 0);
    if (destination == source) return;
    resize_native_string_header_0041dd40(destination, strings, word(source), true);
    if (word(source) != 0)
        std::memmove(ptr(word(destination, 4)), ptr(word(source, 4)), word(destination));
}
void clear_values(void* v) {
    if (signed_word(word(v, 8)) < 0) reserve_native_animation_values_00b92440(v, 0);
    while (signed_word(word(v, 4)) > 0) put(v, 4, word(v, 4) - 1u);
    put(v, 4, 0);
}
void append_values(void* v, const void* source) {
    if (word(v, 4) == word(v, 8)) reserve_native_animation_values_00b92440(v, grow(word(v, 8)));
    void* const destination = ptr(word(v) + word(v, 4) * 0x30u);
    if (destination) copy_values(destination, source);
    put(v, 4, word(v, 4) + 1u);
}
__declspec(naked) void __cdecl read_float_store(void*, void*, NativeResourceStreamReadContext&) {
    __asm {
        push ebp
        mov ebp, esp
        push dword ptr [ebp+16]
        push dword ptr [ebp+12]
        call read_native_resource_node_float_00be99d0
        add esp, 8
        mov eax, dword ptr [ebp+8]
        fstp dword ptr [eax]
        pop ebp
        ret
    }
}
__declspec(naked) void __cdecl copy_float_store(void*, const void*) {
    __asm {
        mov eax, dword ptr [esp+8]
        fld dword ptr [eax]
        mov eax, dword ptr [esp+4]
        fstp dword ptr [eax]
        ret
    }
}
}

void* read_native_animation_values_00b92300(void* output, void* handle, NativeResourceStreamReadContext& reads) {
    for (U offset = 0; offset != 0x30; offset += 4) read_float_store(at(output, offset), handle, reads);
    return output;
}
void reserve_native_animation_values_00b92440(void* v, U capacity) {
    if (signed_word(capacity) < 1) capacity = 1;
    if (signed_word(capacity) <= signed_word(word(v, 8))) return;
    void* const storage = allocate(capacity * 0x30u);
    U destination = bits(storage), offset = 0;
    for (U i = 0; signed_word(i) < signed_word(word(v, 4)); ++i, offset += 0x30, destination += 0x30)
        if (destination) copy_values(ptr(destination), ptr(word(v) + offset));
    singleton_lifetime_free(ptr(word(v)));
    put(v, 0, bits(storage)); put(v, 8, capacity);
}
void* assign_native_animation_values_00b926e0(void* v, const void* source) {
    clear_values(v);
    reserve_native_animation_values_00b92440(v, word(source, 4));
    for (U i = 0, offset = 0; signed_word(i) < signed_word(word(source, 4)); ++i, offset += 0x30) {
        const void* const captured_source = ptr(word(source) + offset);
        append_values(v, captured_source);
    }
    return v;
}
void destroy_native_animation_record_00b928b0(void* record, NativeStringRawPoolContext& strings) {
    try {
        clear_values(at(record, 0xc));
        singleton_lifetime_free(ptr(word(record, 0xc)));
    } catch (...) {
        try { destroy_native_string_header_0041dd20(at(record, 4), strings); }
        catch (...) { std::terminate(); }
        throw;
    }
    destroy_native_string_header_0041dd20(at(record, 4), strings);
}
void* construct_native_animation_record_00b92970(void* record, U value, void* argument, NativeStringRawPoolContext& strings) {
    const U length = word(argument);
    put(record, 0, value); put(record, 4, 0); put(record, 8, 0);
    try {
        if (at(record, 4) != argument) {
            resize_native_string_header_0041dd40(at(record, 4), strings, length, true);
            if (length != 0) std::memmove(ptr(word(record, 8)), ptr(word(argument, 4)), word(record, 4));
        }
        zero_vector(at(record, 0xc));
    } catch (...) {
        try { destroy_native_string_header_0041dd20(argument, strings); }
        catch (...) { std::terminate(); }
        throw;
    }
    void* const data = ptr(word(argument, 4));
    if (data) give_back(data, length + 1u, strings);
    return record;
}
void* copy_native_animation_record_00b92a50(void* record, const void* source, NativeStringRawPoolContext& strings) {
    copy_float_store(record, source);
    copy_name(at(record, 4), at(source, 4), strings);
    zero_vector(at(record, 0xc));
    try { assign_native_animation_values_00b926e0(at(record, 0xc), at(source, 0xc)); }
    catch (...) {
        try { destroy_native_string_header_0041dd20(at(record, 4), strings); }
        catch (...) { std::terminate(); }
        throw;
    }
    return record;
}
void reserve_native_animation_records_00b92ae0(void* v, U capacity, NativeStringRawPoolContext& strings) {
    if (signed_word(capacity) < 1) capacity = 1;
    if (signed_word(capacity) <= signed_word(word(v, 8))) return;
    void* const storage = allocate(capacity * 0x18u);
    // Native EH only calls the no-op placement-delete 401130. No rollback.
    for (U i = 0; signed_word(i) < signed_word(word(v, 4)); ++i) {
        void* const destination = at(storage, i * 0x18u);
        if (destination) copy_native_animation_record_00b92a50(destination, ptr(word(v) + i * 0x18u), strings);
    }
    for (U i = 0, offset = 0; signed_word(i) < signed_word(word(v, 4)); ++i, offset += 0x18)
        destroy_native_animation_record_00b928b0(ptr(word(v) + offset), strings);
    singleton_lifetime_free(ptr(word(v)));
    put(v, 0, bits(storage)); put(v, 8, capacity);
}
void resize_native_animation_records_00b92bc0(void* v, U count, NativeStringRawPoolContext& strings) {
    if (signed_word(count) > signed_word(word(v, 8))) reserve_native_animation_records_00b92ae0(v, count, strings);
    const U old_count = word(v, 4);
    if (signed_word(old_count) < signed_word(count)) {
        U offset = old_count * 0x18u;
        for (U left = count - old_count; left != 0; --left, offset += 0x18) {
            void* const record = ptr(word(v) + offset);
            if (record) { put(record, 4, 0); put(record, 8, 0); zero_vector(at(record, 0xc)); }
        }
    }
    while (signed_word(count) < signed_word(word(v, 4))) {
        put(v, 4, word(v, 4) - 1u);
        destroy_native_animation_record_00b928b0(ptr(word(v) + word(v, 4) * 0x18u), strings);
    }
    put(v, 4, count);
}
void append_native_animation_record_00b92c40(void* v, const void* source, NativeStringRawPoolContext& strings) {
    if (word(v, 4) == word(v, 8)) reserve_native_animation_records_00b92ae0(v, grow(word(v, 8)), strings);
    void* const record = ptr(word(v) + word(v, 4) * 0x18u);
    if (record) copy_native_animation_record_00b92a50(record, source, strings);
    put(v, 4, word(v, 4) + 1u);
}
void read_native_skinned_animation_item_00b92cd0(void* item, void* handle, NativeResourceStreamReadContext& reads) {
    put(item, 8, read_native_resource_node_dword_00be9a00(handle, reads));
    while (word(ptr(word(handle)), 0x20) != 0) {
        U value; read_float_store(&value, handle, reads);
        U name[2]; read_native_resource_handle_string_00bea010(handle, name, reads);
        U record[6]; int state = 0;
        try {
            U argument[2]; copy_name(argument, name, reads.strings);
            // Native reader FLD/FSTP32 before constructor's bitwise MOVSS.
            U rounded; copy_float_store(&rounded, &value);
            construct_native_animation_record_00b92970(record, rounded, argument, reads.strings);
            state = 1;
            append_native_animation_record_00b92c40(at(item, 0xc), record, reads.strings);
            state = 0;
            destroy_native_animation_record_00b928b0(record, reads.strings);
            U left = read_native_resource_node_dword_00be9a00(handle, reads);
            if (signed_word(left) > 0) {
                do {
                    void* const last = ptr(word(item, 0xc) + word(item, 0x10) * 0x18u - 0x18u);
                    U values[12]; read_native_animation_values_00b92300(values, handle, reads);
                    append_values(at(last, 0xc), values);
                } while (--left != 0);
            }
        } catch (...) {
            try {
                if (state == 1) destroy_native_animation_record_00b928b0(record, reads.strings);
                destroy_native_string_header_0041dd20(name, reads.strings);
            } catch (...) { std::terminate(); }
            throw;
        }
        destroy_native_string_header_0041dd20(name, reads.strings);
    }
}
void destroy_native_skinned_animation_item_00b92ec0(void* item, NativeStringRawPoolContext& strings) {
    try {
        resize_native_animation_records_00b92bc0(at(item, 0xc), 0, strings);
        singleton_lifetime_free(ptr(word(item, 0xc)));
    } catch (...) { destroy_native_resource_item_base_00b86890(item); throw; }
    destroy_native_resource_item_base_00b86890(item);
}
void* parse_native_skinned_animation_item_00b92f20(void* handle, NativeResourceStreamReadContext& reads) {
    void* const storage = allocate(0x18);
    void* item = nullptr;
    if (storage) {
        item = construct_native_resource_item_base_00b868b0(storage);
        put(item, 0, 0x00d63700); zero_vector(at(item, 0xc));
    }
    read_native_skinned_animation_item_00b92cd0(item, handle, reads);
    return item;
}
void* delete_native_skinned_animation_item_00b92fa0(void* item, U flags, NativeStringRawPoolContext& strings) {
    destroy_native_skinned_animation_item_00b92ec0(item, strings);
    if ((flags & 1u) != 0) singleton_lifetime_free(item);
    return item;
}
} // namespace bsp
