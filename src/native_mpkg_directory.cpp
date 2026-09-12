#include "bsp/native_mpkg_directory.hpp"

#include "bsp/native_mpkg_entry_vector.hpp"

#include <cstddef>
#include <cstring>

namespace bsp {
namespace {
using U = std::uint32_t;
void* pointer(U value) noexcept { return reinterpret_cast<void*>(value); }
U address(const void* value) noexcept { return reinterpret_cast<U>(value); }
void* at(const void* owner, U offset) noexcept { return pointer(address(owner) + offset); }
U word(const void* owner, U offset = 0) noexcept {
    return *static_cast<const volatile U*>(at(owner, offset));
}
void put(void* owner, U offset, U value) noexcept {
    *static_cast<volatile U*>(at(owner, offset)) = value;
}
std::uint16_t half(const void* owner, U offset) noexcept {
    return *static_cast<const volatile std::uint16_t*>(at(owner, offset));
}
void put_half(void* owner, U offset, std::uint16_t value) noexcept {
    *static_cast<volatile std::uint16_t*>(at(owner, offset)) = value;
}
std::uint8_t byte(U value) noexcept {
    return *static_cast<const volatile std::uint8_t*>(pointer(value));
}
U take_word(void* reader) noexcept {
    const U source = word(reader, 8);
    put(reader, 8, source + 4u);
    U result = byte(source + 3u);
    result = (result << 8) + byte(source + 2u);
    result = (result << 8) + byte(source + 1u);
    return (result << 8) + byte(source);
}
std::uint16_t take_half(void* reader) noexcept {
    const U source = word(reader, 8);
    put(reader, 8, source + 2u);
    const U high = byte(source + 1u);
    const U low = byte(source);
    return static_cast<std::uint16_t>((high << 8) + low);
}
void release_current_string(void* header, NativeMpkgDirectoryServices& services) {
    void* const block = pointer(word(header, 4));
    if (block == nullptr) return;
    const U bytes = word(header) + 1u;
    auto* const pool = services.string_pool_00419cc0();
    services.return_string_00bd1510(pool, block, bytes, 1);
}
} // namespace

void find_native_mpkg_end_record_00bb87a0(void* archive,
    NativeMpkgDirectoryContext& context) {
    void* const length_owner = pointer(word(archive, 0xc));
    const U length_target = word(pointer(word(length_owner)), 0x30);
    const U length = static_cast<U>(context.services.source_length(length_target, length_owner));
    put(archive, 0x10, length);
    const U count = length > 0xffffu ? 0xffffu : length;
    void* const storage = context.services.allocate_00bf55be(count);
    void* const seek_owner = pointer(word(archive, 0xc));
    const U current_length = word(archive, 0x10);
    const U seek_target = word(pointer(word(seek_owner)), 0x1c);
    (void)context.streams.source_seek(seek_target, seek_owner, current_length - count, 0, 0);
    void* const read_owner = pointer(word(archive, 0xc));
    const U read_target = word(pointer(word(read_owner)), 0x24);
    context.streams.source_read(read_target, read_owner, storage, count, nullptr);
    if (count > 4u) {
        U cursor = address(storage) + count - 2u;
        U distance = 4;
        do {
            if (byte(cursor - 2u) == 0x50 && byte(cursor - 1u) == 0x4b &&
                byte(cursor) == 5 && byte(cursor + 1u) == 6) {
                put(archive, 0x1c, word(archive, 0x10) - distance);
                break;
            }
            ++distance;
            --cursor;
        } while (distance < count);
    }
    context.services.free_00bf65ac(storage);
}

void read_native_mpkg_header_00bb8850(void* reader, void* header, U kind) {
    put(header, 0, kind);
    put(header, 4, take_word(reader));
    if (kind == 0) put_half(header, 8, take_half(reader));
    put_half(header, 0xa, take_half(reader));
    put_half(header, 0xc, take_half(reader));
    put_half(header, 0xe, take_half(reader));
    put(header, 0x10, take_word(reader));
    put(header, 0x14, take_word(reader));
    put(header, 0x18, take_word(reader));
    put(header, 0x1c, take_word(reader));
    put_half(header, 0x20, take_half(reader));
    put_half(header, 0x22, take_half(reader));
    if (kind == 0) {
        put_half(header, 0x24, take_half(reader));
        put_half(header, 0x26, take_half(reader));
        put_half(header, 0x28, take_half(reader));
        put(header, 0x2c, take_word(reader));
        put(header, 0x30, take_word(reader));
    }
}

void* read_native_mpkg_name_00bb9090(void* reader, void* output, U count,
    NativeStringStorage& strings) {
    put(output, 0, 0);
    put(output, 4, 0);
    resize_native_string_header_0041dd40(output, strings, count, true);
    void* const fill_data = pointer(word(output, 4));
    if (fill_data != nullptr) std::memset(fill_data, 0x20, word(output));
    for (U index = 0; index < count; ++index) {
        const auto value = byte(word(reader, 8) + index);
        const U destination = word(output, 4);
        *static_cast<volatile std::uint8_t*>(pointer(destination + index)) = value;
    }
    put(reader, 8, word(reader, 8) + count);
    return output;
}

void parse_native_mpkg_directory_entry_00bb95b0(void* archive, void* reader,
    NativeMpkgDirectoryContext& context) {
    alignas(4) std::byte header[0x34];
    alignas(4) std::byte entry[0x24];
    alignas(4) std::byte temporary[8];
    read_native_mpkg_header_00bb8850(reader, header, 0);
    put(entry, 0, 0);
    put(entry, 4, 0);
    void* captured_entry_data = nullptr; // EBX until BB961F.
    int state = 0;
    try {
        void* const name = read_native_mpkg_name_00bb9090(reader, temporary,
            half(header, 0x20), context.strings);
        state = 1;
        if (static_cast<void*>(entry) != name) {
            resize_native_string_header_0041dd40(entry, context.strings, word(name), true);
            const U current_name_length = word(name);
            captured_entry_data = pointer(word(entry, 4));
            if (current_name_length != 0) {
                const U count = word(entry);
                const void* const from = pointer(word(name, 4));
                if (count != 0) std::memmove(captured_entry_data, from, count);
            }
        }
        state = 0;
        release_current_string(temporary, context.services);
        const U prefix = word(archive, 0x18);
        const U relative_offset = word(header, 0x30);
        put(entry, 8, prefix + relative_offset);
        const U extra = half(header, 0x22);
        const U combined = extra + prefix + half(header, 0x20);
        put_half(entry, 0x14, half(header, 0xe));
        put(entry, 0x1c, word(header, 0x1c));
        const U comment = half(header, 0x24);
        const U data_offset = combined + relative_offset + 0x1eu;
        put(reader, 8, word(reader, 8) + comment + extra);
        put(entry, 0x10, data_offset);
        put(entry, 0x18, word(header, 0x18));
        const U crc = word(header, 0x14);
        *static_cast<volatile std::uint8_t*>(at(entry, 0xc)) = 0;
        put(entry, 0x20, crc);
        append_native_mpkg_entry_00bb9520(at(archive, 0x28), entry, context.strings);
        state = -1;
        if (captured_entry_data != nullptr) {
            const U bytes = word(entry) + 1u;
            auto* const pool = context.services.string_pool_00419cc0();
            context.services.return_string_00bd1510(pool, captured_entry_data, bytes, 1);
        }
    } catch (...) {
        // DFE430: state1->0 CC4828/41DD20; state0->-1 CC4820/BB9030.
        if (state >= 1) release_current_string(temporary, context.services);
        if (state >= 0) release_current_string(entry, context.services);
        throw;
    }
}

void load_native_mpkg_directory_00bb9700(void* archive,
    NativeMpkgDirectoryContext& context) {
    void* const seek_owner = pointer(word(archive, 0xc));
    const U table = word(seek_owner);
    const U offset = word(archive, 0x24);
    const U seek_target = word(pointer(table), 0x1c);
    (void)context.streams.source_seek(seek_target, seek_owner, offset, 0, 0);
    const U length = word(archive, 0x10) - word(archive, 0x24);
    void* const read_owner = pointer(word(archive, 0xc));
    alignas(4) U reader[3]; // Native actual-count slot is not initialized.
    void* const allocation = context.services.allocate_00bf55be(length);
    put(reader, 0, address(allocation));
    const U read_target = word(pointer(word(read_owner)), 0x24);
    context.streams.source_read(read_target, read_owner, allocation, length,
        static_cast<U*>(at(reader, 4)));
    put(reader, 8, word(reader));
    try {
        U index = 0;
        while (index < word(archive, 0x14)) {
            parse_native_mpkg_directory_entry_00bb95b0(archive, reader, context);
            ++index;
        }
        context.services.free_00bf65ac(pointer(word(reader)));
    } catch (...) {
        // DFE464: state0->-1 CC4840/BB8570 frees CURRENT reader base.
        context.services.free_00bf65ac(pointer(word(reader)));
        throw;
    }
}
} // namespace bsp
