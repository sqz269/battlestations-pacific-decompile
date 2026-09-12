#include "bsp/native_mpkg_archive.hpp"
#include "bsp/native_mpkg_directory.hpp"
#include "bsp/native_mpkg_entry_vector.hpp"
#include "bsp/native_filestore_open.hpp"
#include "bsp/native_memory_stream.hpp"
#include "bsp/native_adopted_substream.hpp"
#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native MPKG archive storage requires MSVC Win32.
#endif

namespace bsp {
namespace {
void* at(void* p, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) + offset);
}
const void* at(const void* p, std::uint32_t offset) noexcept {
    return reinterpret_cast<const void*>(reinterpret_cast<std::uintptr_t>(p) + offset);
}
std::uint32_t word(const void* p, std::uint32_t offset = 0) noexcept {
    return *static_cast<const volatile std::uint32_t*>(at(p, offset));
}
void put(void* p, std::uint32_t offset, std::uint32_t value) noexcept {
    *static_cast<volatile std::uint32_t*>(at(p, offset)) = value;
}
void* pointer(const void* p, std::uint32_t offset = 0) noexcept {
    return reinterpret_cast<void*>(word(p, offset));
}
std::uint32_t method(const void* p, std::uint32_t offset) noexcept {
    return word(pointer(p), offset);
}
void release_path(void* header, NativeMpkgDirectoryServices& services) {
    void* const data = pointer(header, 4);
    if (!data) return;
    const auto bytes = word(header) + 1u;
    auto* const pool = services.string_pool_00419cc0();
    services.return_string_00bd1510(pool, data, bytes, 1);
}
void release_reference(void* owner, NativeAdoptedSubstreamDispatch& streams) {
    if (InterlockedDecrement(static_cast<volatile LONG*>(at(owner, 4))) == 0) {
        const auto table = word(owner);
        const auto entry = word(reinterpret_cast<const void*>(table));
        streams.source_zero_reference(entry, owner, table);
    }
}
void read_current_stream(void* archive, void* output, std::uint32_t count,
    NativeAdoptedSubstreamDispatch& streams) {
    void* const stream = pointer(archive, 0xc);
    const auto entry = method(stream, 0x24);
    streams.source_read(entry, stream, output, count, nullptr);
}
} // namespace

void* construct_native_mpkg_archive_00bb9920(void* archive,
    const void* name, NativeMpkgArchiveContext& context) {
    auto& directory = context.directory;
    void* const path = at(archive, 4);
    const bool same = path == name;
    put(path, 0, 0);
    put(path, 4, 0);
    if (!same) {
        resize_native_string_header_0041dd40(path, directory.strings, word(name), true);
        if (word(name) != 0) {
            const auto bytes = word(path);
            void* const destination = pointer(path, 4);
            const void* const source = pointer(name, 4);
            if (bytes != 0) std::memmove(destination, source, bytes);
        }
    }
    // DFE4DC/DFE4C4: 0 -> path; 1 -> vector; 2 -> raw backing free.
    // The path copy precedes state0, and vector stores cannot throw in this
    // source domain. None of these states owns a converted/decoded stream.
    put(archive, 0x1c, 0xffffffffu);
    put(archive, 0x28, 0);
    put(archive, 0x2c, 0);
    put(archive, 0x30, 0);
    try {
        void* const manager = context.actual_manager_publication_0109ceec;
        const auto open_entry = method(manager, 4);
        void* const source = context.services.open_manager_00bb99a0(open_entry, manager, name, 2);
        void* const converted = convert_native_stored_stream_00bef750(source, context.conversion);
        release_reference(source, directory.streams);
        const auto length_entry = method(converted, 0x30);
        const auto length = static_cast<std::uint32_t>(directory.services.source_length(length_entry, converted));
        void* const raw = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x10, 0x10});
        void* backing = nullptr;
        try {
            if (raw) backing = construct_native_memory_backing_008d43c0(
                raw, static_cast<std::int32_t>(length), context.conversion.memory_owners);
        } catch (...) {
            singleton_lifetime_free(raw);
            throw;
        }
        if (static_cast<std::int32_t>(length) > 0) {
            const auto whole_blocks = length / 0x2f1u;
            for (std::uint32_t i = 0; i < length; ++i) {
                const auto block = i / 0x2f1u;
                const auto block_size = block == whole_blocks ? length - whole_blocks * 0x2f1u : 0x2f1u;
                const auto index = block * 0x2f1u - i % block_size + block_size - 1u;
                auto* const destination = static_cast<volatile std::uint8_t*>(pointer(backing, 8));
                const auto* const input = native_memory_stream_data_00bef610(converted, nullptr);
                const auto source_byte = input[index];
                const auto key_byte = context.actual_xor_key_00e144f0[index % 0x219u];
                destination[i] = source_byte ^ key_byte;
            }
        }
        const auto delete_entry = method(converted, 4);
        context.services.delete_stream_00bb9aae(delete_entry, converted, 1);
        void* const decoded = create_native_memory_stream_from_backing_00bef6d0(
            backing, context.conversion.memory_owners);
        put(archive, 0xc, reinterpret_cast<std::uintptr_t>(decoded));
        release_reference(backing, directory.streams);
        find_native_mpkg_end_record_00bb87a0(archive, directory);
        void* const stream = pointer(archive, 0xc);
        const void* const seek_table = pointer(stream);
        const auto end_record = word(archive, 0x1c);
        const auto seek_entry = word(seek_table, 0x1c);
        directory.streams.source_seek(seek_entry, stream, end_record, 0, 0);

        // ESP+30 was the backing pointer and is reused by ALL header reads.
        // Preserve untouched bytes after short reads instead of zero-filling.
        std::uint32_t scratch = reinterpret_cast<std::uintptr_t>(backing);
        read_current_stream(archive, &scratch, 4, directory.streams);
        read_current_stream(archive, &scratch, 2, directory.streams);
        read_current_stream(archive, &scratch, 2, directory.streams);
        read_current_stream(archive, &scratch, 2, directory.streams);
        read_current_stream(archive, &scratch, 2, directory.streams);
        const auto count = scratch & 0xffffu;
        void* const size_stream = pointer(archive, 0xc);
        put(archive, 0x14, count);
        const auto size_read_entry = method(size_stream, 0x24);
        directory.streams.source_read(size_read_entry, size_stream, &scratch, 4, nullptr);
        put(archive, 0x20, scratch);
        read_current_stream(archive, &scratch, 4, directory.streams);
        put(archive, 0x24, scratch);
        read_current_stream(archive, &scratch, 2, directory.streams);
        const auto end_offset = word(archive, 0x1c);
        const auto directory_offset = word(archive, 0x24);
        const auto directory_size = word(archive, 0x20);
        const auto start = end_offset - directory_offset - directory_size;
        put(archive, 0x18, start);
        load_native_mpkg_directory_00bb9700(archive, directory);
    } catch (...) {
        destroy_native_mpkg_entries_00bb9900(at(archive, 0x28), directory.strings);
        release_path(path, directory.services);
        throw;
    }
    return archive;
}

void destroy_native_mpkg_archive_00bb9c10(void* archive, NativeMpkgArchiveContext& context) {
    auto& directory = context.directory;
    int state = 1;
    try {
        void* const stream = pointer(archive, 0xc);
        release_reference(stream, directory.streams);
        state = 0;
        resize_native_mpkg_entries_00bb94a0(at(archive, 0x28), 0, directory.strings);
        singleton_lifetime_free(pointer(archive, 0x28));
        state = -1;
        release_path(at(archive, 4), directory.services);
    } catch (...) {
        if (state >= 1) {
            state = 0;
            destroy_native_mpkg_entries_00bb9900(at(archive, 0x28), directory.strings);
        }
        if (state >= 0) release_path(at(archive, 4), directory.services);
        throw;
    }
}
} // namespace bsp
