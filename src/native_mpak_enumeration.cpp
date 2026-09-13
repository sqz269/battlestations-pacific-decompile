#include "bsp/native_mpak_enumeration.hpp"

#include "bsp/native_lua_script_overrides.hpp"
#include "bsp/native_particle_type_property.hpp"
#include "bsp/native_path_canonicalizer.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_string.hpp"

#include <cstring>

namespace bsp {
namespace {
using U = std::uint32_t;
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeString) == 8);
static_assert(sizeof(NativeStringVectorStorage) == 0xc);
U address(const void* value) noexcept { return reinterpret_cast<U>(value); }
void* pointer(U value) noexcept { return reinterpret_cast<void*>(value); }
void* at(const void* value, U offset) noexcept { return pointer(address(value) + offset); }
U word(const void* value, U offset = 0) noexcept {
    return *static_cast<const volatile U*>(at(value, offset));
}
void put(void* value, U offset, U data) noexcept {
    *static_cast<volatile U*>(at(value, offset)) = data;
}
// These are the callers' inline string copies, using the existing actual
// resize. BF7680 preserves overlapping byte ranges through its reverse branch.
void copy_name(void* destination, const void* source, NativeStringStorage& strings) {
    if (destination == source) return;
    resize_native_string_header_0041dd40(destination, strings, word(source), true);
    if (word(source) != 0) {
        const U bytes = word(destination);
        const void* const input = pointer(word(source, 4));
        void* const output = pointer(word(destination, 4));
        std::memmove(output, input, bytes);
    }
}
void return_string(void* captured_data, U captured_bytes,
    NativeMpakDirectoryContext& context) {
    auto* const pool = context.allocation_and_pool.string_pool_00419cc0();
    context.allocation_and_pool.return_string_00bd1510(
        pool, captured_data, captured_bytes, 1);
}
U file_count(void* provider, U captured_begin) noexcept {
    return static_cast<U>(static_cast<std::int32_t>(
        word(provider, 0x24) - captured_begin) / 0x24);
}
} // namespace

bool matches_native_provider_enumeration_00bee340(const void* prefix,
    const void* extension, U flags, const void* candidate,
    const char* null_pattern) {
    if (!matches_native_particle_string_at_0043e9a0(
        candidate, prefix, 0, null_pattern)) return false;
    if ((flags & 0xffu) == 0) {
        const U slash = reverse_find_native_string_bytes_004bcb80(
            *static_cast<const NativeString*>(candidate), "/", 0x7fffffff);
        if (slash > word(prefix)) return false;
    }
    const U start = word(candidate) - word(extension);
    return matches_native_particle_string_at_0043e9a0(
        candidate, extension, start, null_pattern);
}

void append_native_mpak_names_00bb5f40(void* provider, const void* prefix,
    const void* extension, U flags, NativeStringVectorStorage& output,
    NativeMpakDirectoryContext& context, const char* null_pattern) {
    NativeString temporary;
    NativeString slash;
    int state = -1;
    try {
        copy_name(&temporary, prefix, context.strings);
        state = 0;
        const U length = word(&temporary);
        if (length != 0 && *static_cast<const volatile char*>(
            pointer(word(&temporary, 4) + length - 1u)) != '/') {
            construct_native_string_cstring_0041e870(&slash, "/", context.strings);
            const U slash_length = word(&slash);
            void* const slash_data = pointer(word(&slash, 4));
            state = 1;
            if (slash_length != 0) {
                const U prefix_length = word(&temporary);
                resize_native_string_header_0041dd40(&temporary, context.strings,
                    slash_length + prefix_length, true);
                std::memmove(pointer(word(&temporary, 4) + prefix_length),
                    slash_data, slash_length);
            }
            state = 0;
            if (slash_data) return_string(slash_data, slash_length + 1u, context);
        }
        U index = 0;
        U offset = 0;
        for (;;) {
            U begin = word(provider, 0x20);
            if (!begin || index >= file_count(provider, begin)) break;
            begin = word(provider, 0x20);
            if (!begin || index >= file_count(provider, begin)) {
                context.containers.invalid_parameter_00bf6713();
            }
            const void* const row = pointer(word(provider, 0x20) + offset);
            if (matches_native_provider_enumeration_00bee340(prefix,
                extension, flags, row, null_pattern)) {
                append_native_string_vector_004cdc20(output,
                    *static_cast<const NativeString*>(row), context.strings);
            }
            ++index;
            offset += 0x24u;
        }
        void* const data = pointer(word(&temporary, 4));
        state = -1;
        if (data) return_string(data, word(&temporary) + 1u, context);
    } catch (...) {
        // FuncInfo DFDE10: state1 CC44B8 destroys slash then state0
        // CC44B0 destroys prefix, both through existing41DD20 source.
        if (state >= 1) destroy_native_string_header_0041dd20(&slash, context.strings);
        if (state >= 0) destroy_native_string_header_0041dd20(&temporary, context.strings);
        throw;
    }
}

bool select_native_mpak_member_directory_00bb68f0(void* provider,
    const void* input, void* output, NativeMpakDirectoryContext& context,
    NativeMpakDirectorySearchLibrary& search) {
    alignas(4) std::uint8_t temporary[0x14];
    put(temporary, 0, 0);
    put(temporary, 4, 0);
    copy_name(temporary, input, context.strings);
    void* const vector = at(provider, 0x2c);
    const U end = word(vector, 8);
    const bool invalid_end = word(vector, 4) > end;
    put(temporary, 8, 0);
    put(temporary, 0xc, 0);
    put(temporary, 0x10, 0);
    if (invalid_end) context.containers.invalid_parameter_00bf6713();
    const U first = word(vector, 4);
    if (first > word(vector, 8)) context.containers.invalid_parameter_00bf6713();
    void* const found = search.find_member_directory_00bb4f40(
        pointer(first), pointer(end), temporary);
    destroy_native_mpak_directory_00bb6500(temporary, context);
    const U current_end = word(vector, 8);
    if (word(vector, 4) > current_end) context.containers.invalid_parameter_00bf6713();
    if (!vector) context.containers.invalid_parameter_00bf6713();
    if (address(found) == current_end) return false;
    if (!vector) context.containers.invalid_parameter_00bf6713();
    if (address(found) >= word(vector, 8)) context.containers.invalid_parameter_00bf6713();
    put(provider, 0x3c, address(found));
    copy_name(output, found, context.strings);
    return true;
}

bool reject_native_mpak_probe_00bb40c0(const void*) noexcept { return false; }
bool reject_native_mpak_operation_00bb79e0(U, U, U, U) noexcept { return false; }
void noop_native_mpak_default_00bb79f0() noexcept {}
void* clear_native_mpak_result_00bb7a00(void* output, U) noexcept {
    put(output, 0x10, 0);
    put(output, 0xc, 0);
    put(output, 8, 0);
    put(output, 4, 0);
    put(output, 0, 0);
    return output;
}
} // namespace bsp
