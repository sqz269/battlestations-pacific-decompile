#include "bsp/native_mpak_provider.hpp"

#include "bsp/native_adopted_substream.hpp"
#include "bsp/native_file_provider_base.hpp"
#include "bsp/native_pak_registry.hpp"
#include "bsp/native_path_canonicalizer.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_string_vector.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

namespace bsp {
namespace {
using U = std::uint32_t;
static_assert(sizeof(void*) == 4);
static_assert(sizeof(CRITICAL_SECTION) == 0x18);
void* pointer(U value) noexcept { return reinterpret_cast<void*>(value); }
U address(const void* value) noexcept { return reinterpret_cast<U>(value); }
void* at(const void* owner, U offset) noexcept { return pointer(address(owner) + offset); }
U word(const void* owner, U offset = 0) noexcept {
    return *static_cast<const volatile U*>(at(owner, offset));
}
void put(void* owner, U offset, U value) noexcept {
    *static_cast<volatile U*>(at(owner, offset)) = value;
}
std::uint8_t byte(const void* owner, U offset = 0) noexcept {
    return *static_cast<const volatile std::uint8_t*>(at(owner, offset));
}
U little_word(const void* owner) noexcept {
    U value = byte(owner, 3);
    value = (value << 8) + byte(owner, 2);
    value = (value << 8) + byte(owner, 1);
    return (value << 8) + byte(owner);
}
U little_half(const void* owner) noexcept {
    const U high = byte(owner, 1);
    return (high << 8) + byte(owner);
}
void observe_marker(const void* cursor, const char* marker) noexcept {
    for (U index = 0; index < 4; ++index) {
        if (byte(cursor, index) != static_cast<std::uint8_t>(marker[index])) break;
    }
}
void return_string(void* block, U bytes, NativeMpakDirectoryContext& context) {
    auto* const pool = context.allocation_and_pool.string_pool_00419cc0();
    context.allocation_and_pool.return_string_00bd1510(pool, block, bytes, 1);
}
void release_current_string(void* header, NativeMpakDirectoryContext& context) {
    void* const block = pointer(word(header, 4));
    if (block) return_string(block, word(header) + 1u, context);
}
void clear_file_offsets(void* record, NativeMpakDirectoryContext& context) {
    void* const backing = pointer(word(record, 0x18));
    if (backing) context.allocation_and_pool.free_scratch_00bf65ac(backing);
    put(record, 0x18, 0);
    put(record, 0x1c, 0);
    put(record, 0x20, 0);
}
void clear_directory_members(void* record, NativeMpakDirectoryContext& context) {
    auto& names = *static_cast<NativeStringVectorStorage*>(at(record, 8));
    resize_native_string_vector_00427110(names, 0, context.strings);
    // The original BF6989 is the matching CRT array-free boundary.
    void* const backing = pointer(word(record, 8));
    context.allocation_and_pool.free_scratch_00bf65ac(backing);
}
void copy_name(void* destination, const void* source, NativeStringStorage& strings) {
    if (destination == source) return;
    resize_native_string_header_0041dd40(destination, strings, word(source), true);
    if (word(source) != 0) {
        const U count = word(destination);
        void* const output = pointer(word(destination, 4));
        const void* const input = pointer(word(source, 4));
        // BF7680 retains its CRT name but also supports backward overlap.
        std::memmove(output, input, count);
    }
}
void provider_unwind(void* owner, int state, NativeMpakProviderContext& context) {
    if (state >= 2) {
        // CC4723 / CC46A3 -> BB7580 -> BB71F0.
        context.directory.containers.destroy_directory_vector_00bb71f0(at(owner, 0x2c));
    }
    if (state >= 1) {
        context.directory.containers.destroy_file_vector_00bb6e60(at(owner, 0x1c));
    }
    destroy_native_file_provider_base_00bb5380(owner, context.strings);
}
// The installed 4254B0 body is RET. Preserve the caller's name-data load;
// this function performs the recovered no-op, without a fabricated log sink.
void trace_004254b0(const char*, const void* = nullptr) noexcept {}
} // namespace

void* read_native_mpak_string_00bb5630(void* cursor, void* output,
    NativeStringStorage& strings) {
    const U previous = word(cursor);
    const U text = previous + 1u;
    put(cursor, 0, text);
    const U prefix = byte(pointer(previous));
    construct_native_string_cstring_0041e870(output,
        static_cast<const char*>(pointer(text)), strings);
    put(cursor, 0, word(cursor) + prefix + 1u);
    return output;
}

void* construct_native_mpak_file_00bb6870(void* record, const void* name,
    U first, U second, std::uint8_t flag, NativeStringStorage& strings) {
    put(record, 0, 0);
    put(record, 4, 0);
    copy_name(record, name, strings);
    put(record, 0xc, second);
    *static_cast<volatile std::uint8_t*>(at(record, 0x10)) = flag;
    put(record, 8, first);
    put(record, 0x18, 0);
    put(record, 0x1c, 0);
    put(record, 0x20, 0);
    return record;
}

void destroy_native_mpak_file_00bb5430(void* record,
    NativeMpakDirectoryContext& context) {
    clear_file_offsets(record, context);
    release_current_string(record, context);
}

void destroy_native_mpak_directory_00bb6500(void* record,
    NativeMpakDirectoryContext& context) {
    try {
        clear_directory_members(record, context);
    } catch (...) {
        // CC44D0 -> 41DD20: only the embedded name is still armed.
        release_current_string(record, context);
        throw;
    }
    release_current_string(record, context);
}

void load_native_mpak_directory_00bb7c50(void* provider,
    NativeMpakDirectoryContext& context) {
    alignas(4) std::uint8_t header[16];
    void* const header_stream = pointer(word(provider, 0x14));
    const U header_entry = word(pointer(word(header_stream)), 0x24);
    context.streams.source_read(header_entry, header_stream, header, 16, nullptr);
    observe_marker(header, "MPAK");
    // All fields are read before allocation. The native instruction stream
    // interleaves their byte loads; short reads leave unspecified stack bytes.
    const U first_size = little_word(header + 4);
    const U second_size = little_word(header + 8);
    const U file_count = little_half(header + 12);
    const U directory_count = little_half(header + 14);
    const U directory_bytes = first_size + second_size;
    const U scratch_bytes = directory_bytes + 4u;
    void* const scratch = context.allocation_and_pool.allocate_scratch_00bf55be(scratch_bytes);
    void* const directory_stream = pointer(word(provider, 0x14));
    const U directory_entry = word(pointer(word(directory_stream)), 0x24);
    context.streams.source_read(directory_entry, directory_stream, scratch, scratch_bytes, nullptr);
    observe_marker(scratch, "TOC"); // includes the NUL fourth byte at D64237.
    U cursor = address(scratch) + 4u;
    NativeString name;
    NativeString empty;
    NativeString member;
    alignas(4) std::uint8_t record[0x24];
    int state = -1;
    try {
        void* const files = at(provider, 0x1c);
        for (U index = 0; index < file_count; ++index) {
            cursor += 2u; // native skips this field without reading it.
            const U second = little_word(pointer(cursor));
            cursor += 4u;
            const U first = little_word(pointer(cursor));
            cursor += 4u;
            const auto flag = static_cast<std::uint8_t>(byte(pointer(cursor)) != 0);
            ++cursor;
            read_native_mpak_string_00bb5630(&cursor, &name, context.strings);
            state = 0;
            construct_native_mpak_file_00bb6870(record, &name, first, second,
                flag, context.strings);
            state = 1;
            context.containers.append_file_00bb7a20(files, record);
            state = 0;
            destroy_native_mpak_file_00bb5430(record, context);
            const U offsets = little_half(pointer(cursor));
            cursor += 2u;
            for (U offset = 0; offset < offsets; ++offset) {
                U end = word(files, 8);
                U value = little_word(pointer(cursor));
                cursor += 4u;
                value += directory_bytes + 0x10u;
                if (word(files, 4) > end) context.containers.invalid_parameter_00bf6713();
                const U last = end - 0x24u;
                if (last > word(files, 8) || last < word(files, 4)) {
                    context.containers.invalid_parameter_00bf6713();
                }
                end -= 0x24u;
                if (end >= word(files, 8)) context.containers.invalid_parameter_00bf6713();
                const U base = word(pointer(end), 0x18);
                void* const values = pointer(end + 0x14u);
                const U count = base ? static_cast<U>(static_cast<std::int32_t>(
                    word(values, 8) - base) >> 2) : 0;
                if (base && count < static_cast<U>(static_cast<std::int32_t>(
                    word(values, 0xc) - base) >> 2)) {
                    const U destination = word(values, 8);
                    put(pointer(destination), 0, value);
                    put(values, 8, destination + 4u);
                } else {
                    void* const where = pointer(word(values, 8));
                    if (base > address(where)) context.containers.invalid_parameter_00bf6713();
                    alignas(4) U result_iterator[2];
                    context.containers.insert_offset_00a40d60(values, result_iterator,
                        values, where, &value);
                }
            }
            state = -1;
            release_current_string(&name, context);
        }
        const U marker = cursor;
        cursor += 4u;
        observe_marker(pointer(marker), "STOC");
        void* const directories = at(provider, 0x2c);
        for (U index = 0; index < directory_count; ++index) {
            put(&empty, 0, 0);
            put(&empty, 4, 0);
            resize_native_string_header_0041dd40(&empty, context.strings, 0, true);
            void* const empty_data = pointer(word(&empty, 4));
            const U empty_length = word(&empty);
            if (empty_data) std::memmove(empty_data, "", empty_length + 1u);
            state = 2;
            put(record, 0, 0);
            put(record, 4, 0);
            resize_native_string_header_0041dd40(record, context.strings, empty_length, true);
            if (empty_length != 0) {
                const U bytes = word(record);
                void* const data = pointer(word(record, 4));
                std::memmove(data, empty_data, bytes);
            }
            put(record, 8, 0);
            put(record, 0xc, 0);
            put(record, 0x10, 0);
            state = 3;
            context.containers.append_directory_00bb7ba0(directories, record);
            state = 4;
            clear_directory_members(record, context);
            state = 2;
            release_current_string(record, context);
            state = -1;
            if (empty_data) return_string(empty_data, empty_length + 1u, context);

            const U end = word(directories, 8);
            if (word(directories, 4) > end) context.containers.invalid_parameter_00bf6713();
            const U last = end - 0x14u;
            if (last > word(directories, 8) || last < word(directories, 4)) {
                context.containers.invalid_parameter_00bf6713();
            }
            if (last >= word(directories, 8)) context.containers.invalid_parameter_00bf6713();
            void* const directory = pointer(last);
            read_native_mpak_string_00bb5630(&cursor, &name, context.strings);
            state = 5;
            copy_name(directory, &name, context.strings);
            state = -1;
            release_current_string(&name, context);
            const U members = little_half(pointer(cursor));
            cursor += 2u;
            auto& names = *static_cast<NativeStringVectorStorage*>(at(directory, 8));
            for (U item = 0; item < members; ++item) {
                read_native_mpak_string_00bb5630(&cursor, &member, context.strings);
                state = 6;
                append_native_string_vector_004cdc20(names, member, context.strings);
                state = -1;
                release_current_string(&member, context);
            }
        }
        observe_marker(pointer(cursor), "RAWD");
        context.allocation_and_pool.free_scratch_00bf65ac(scratch);
    } catch (...) {
        switch (state) {
        case 1:
            destroy_native_mpak_file_00bb5430(record, context);
            [[fallthrough]];
        case 0:
        case 5:
            release_current_string(&name, context);
            break;
        case 3:
            destroy_native_mpak_directory_00bb6500(record, context);
            release_current_string(&empty, context);
            break;
        case 4:
            release_current_string(record, context);
            [[fallthrough]];
        case 2:
            release_current_string(&empty, context);
            break;
        case 6:
            release_current_string(&member, context);
            break;
        default:
            break;
        }
        // No native state owns the allocated directory scratch.
        throw;
    }
}

void* construct_native_mpak_provider_00bb8240(void* owner, const void* name,
    NativeMpakProviderContext& context) {
    construct_native_file_provider_base_00bb5590(owner, name, context.strings);
    int state = 0;
    try {
        put(owner, 0, 0x00d641f8);
        void* const registry = get_native_pak_registry_00736c30(context.registry);
        put(owner, 0x18, word(registry, 0xc));
        put(owner, 0x20, 0);
        put(owner, 0x24, 0);
        put(owner, 0x28, 0);
        put(owner, 0x30, 0);
        put(owner, 0x34, 0);
        put(owner, 0x38, 0);
        put(owner, 0x3c, 0);
        put(owner, 0x40, 0);
        const U name_data = word(name, 4);
        state = 2;
        trace_004254b0("+PAK begin %s", name_data ? pointer(name_data) : "");
        void* const manager = context.actual_manager_publication_0109ceec;
        const U target = word(pointer(word(manager)), 4);
        void* const stream = context.dispatch.open_manager_00bb82c8(target,
            manager, at(owner, 8), 2);
        put(owner, 0x14, address(stream));
        load_native_mpak_directory_00bb7c50(owner, context.directory);
    } catch (...) {
        provider_unwind(owner, state, context);
        throw;
    }
    return owner;
}

void destroy_native_mpak_provider_00bb7920(void* owner,
    NativeMpakProviderContext& context) {
    put(owner, 0, 0x00d641f8);
    void* const stream = pointer(word(owner, 0x14));
    int state = 2;
    try {
        if (InterlockedDecrement(static_cast<volatile LONG*>(at(stream, 4))) == 0) {
            const U table = word(stream);
            const U target = word(pointer(table));
            context.directory.streams.source_zero_reference(target, stream, table);
        }
        trace_004254b0("-PAK");
        state = 1;
        context.directory.containers.destroy_directory_vector_00bb71f0(at(owner, 0x2c));
        void* const begin = pointer(word(owner, 0x20));
        void* const files = at(owner, 0x1c);
        state = 0;
        if (begin) {
            void* const end = pointer(word(files, 8));
            context.directory.containers.destroy_file_range_00bb6220(begin, end, files, owner);
            void* const backing = pointer(word(files, 4));
            context.directory.allocation_and_pool.free_scratch_00bf65ac(backing);
        }
        put(files, 4, 0);
        put(files, 8, 0);
        put(files, 0xc, 0);
    } catch (...) {
        provider_unwind(owner, state, context);
        throw;
    }
    destroy_native_file_provider_base_00bb5380(owner, context.strings);
}

void* delete_native_mpak_provider_00bb7b80(void* owner, U flags,
    NativeMpakProviderContext& context) {
    destroy_native_mpak_provider_00bb7920(owner, context);
    if (flags & 1u) singleton_lifetime_free(owner);
    return owner;
}

void replace_native_mpak_cache_00bb82f0(const void* name,
    NativeMpakCacheContext& context) {
    void* allocation = nullptr;
    bool allocation_cleanup = false;
    try {
        void* const entered = context.actual_lock_publication_010904e0;
        EnterCriticalSection(static_cast<CRITICAL_SECTION*>(entered));
        put(entered, 0x18, word(entered, 0x18) + 1u);
        const void* const data = pointer(word(name, 4));
        if (!data || _stricmp(static_cast<const char*>(data), "") == 0) {
            context.actual_cached_provider_010904dc = nullptr;
        } else {
            allocation = singleton_lifetime_allocate({
                SingletonAllocationKind::object, 0x44, 0x44});
            allocation_cleanup = true;
            void* result = nullptr;
            if (allocation) result = construct_native_mpak_provider_00bb8240(
                allocation, name, context.provider);
            context.actual_cached_provider_010904dc = result;
        }
        void* const leaving = context.actual_lock_publication_010904e0;
        put(leaving, 0x18, word(leaving, 0x18) - 1u);
        LeaveCriticalSection(static_cast<CRITICAL_SECTION*>(leaving));
    } catch (...) {
        // State0 remains armed through publication and LeaveCriticalSection.
        if (allocation_cleanup) singleton_lifetime_free(allocation);
        throw;
    }
}

NativeMpakRuntimeProviderOperations::NativeMpakRuntimeProviderOperations(
    NativeMpakCacheContext& context) noexcept : context_(context) {}
void* NativeMpakRuntimeProviderOperations::construct_00bb8240(void* owner, const void* name) {
    return construct_native_mpak_provider_00bb8240(owner, name, context_.provider);
}
void NativeMpakRuntimeProviderOperations::replace_cached_00bb82f0(const void* name) {
    replace_native_mpak_cache_00bb82f0(name, context_);
}
} // namespace bsp
