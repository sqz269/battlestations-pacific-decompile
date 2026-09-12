#include "bsp/sound_dialog_table.hpp"
#include "bsp/sound_sample.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>

namespace bsp {
namespace {
template<class T> T read(const void* p, std::uint32_t n) noexcept {
    T value; std::memcpy(&value, static_cast<const unsigned char*>(p) + n, sizeof value); return value;
}
template<class T> void write(void* p, std::uint32_t n, T value) noexcept {
    std::memcpy(static_cast<unsigned char*>(p) + n, &value, sizeof value);
}
void* at(void* p, std::uint32_t n) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) + n);
}
std::int32_t signed_word(std::uint32_t bits) noexcept {
    std::int32_t value; std::memcpy(&value, &bits, sizeof value); return value;
}
bool keyword(NativeTextTokens& tokens, const char* value) {
    return _stricmp(tokens.peek_00bee8e0().c_str(), value) == 0;
}
struct TemporaryRecord {
    alignas(4) unsigned char bytes[0x14];
    NativeStringStorage& strings;
    ~TemporaryRecord() { destroy_sound_dialog_record_00a77ea0(bytes, strings); }
};
}

void destroy_sound_dialog_record_00a77ea0(void* record, NativeStringStorage& strings) noexcept {
    destroy_native_string_header_0041dd20(record, strings);
}

void* copy_sound_dialog_record_00a77ef0(void* destination, const void* source,
    NativeStringStorage& strings, const std::array<std::uint32_t, 4>& counts) {
    write<std::uint32_t>(destination, 0, 0);
    write<char*>(destination, 4, nullptr);
    const auto format = read<std::uint32_t>(source, 0x10);
    write(destination, 0x10, format);
    const auto initial_count = counts[format];
    try {
        write(destination, 8, initial_count);
        if (destination != source) {
            resize_native_string_header_0041dd40(destination, strings,
                read<std::uint32_t>(source, 0), true);
            if (read<std::uint32_t>(source, 0) != 0) {
                const auto length = read<std::uint32_t>(destination, 0);
                const auto data = read<const char*>(source, 4);
                const auto target = read<char*>(destination, 4);
                if (length) std::memmove(target, data, length);
            }
        }
        write(destination, 8, read<std::uint32_t>(source, 8));
        write(destination, 0xc, read<std::uint32_t>(source, 0xc));
    } catch (...) {
        // DEABEC state0 -> CB4D40 -> native string destructor0041DD20.
        destroy_native_string_header_0041dd20(destination, strings);
        throw;
    }
    return destination;
}

void reserve_sound_dialog_records_00a78dc0(void* header, std::int32_t requested,
    NativeStringStorage& strings, const std::array<std::uint32_t, 4>& counts) {
    if (requested < 1) requested = 1;
    if (read<std::int32_t>(header, 8) >= requested) return;
    const auto bytes = static_cast<std::uint32_t>(requested) * 0x14u;
    void* const allocation = singleton_lifetime_allocate({SingletonAllocationKind::pointer_slots, bytes, bytes});
    for (std::uint32_t i = 0; signed_word(i) < read<std::int32_t>(header, 4); ++i) {
        if (auto* destination = at(allocation, i * 0x14u))
            copy_sound_dialog_record_00a77ef0(destination,
                at(read<void*>(header, 0), i * 0x14u), strings, counts);
    }
    // DEAD38/CB4E30 only calls no-op00401130 on a failed placement copy.
    // It does not delete the allocation or destroy completed records.
    for (std::uint32_t i = 0; signed_word(i) < read<std::int32_t>(header, 4); ++i)
        destroy_sound_dialog_record_00a77ea0(at(read<void*>(header, 0), i * 0x14u), strings);
    singleton_lifetime_free(read<void*>(header, 0));
    write(header, 0, allocation);
    write(header, 8, requested);
}

void load_sound_dialog_table_00a87060(void* table, const NativeString& filename,
    SoundDialogTableContext& context) {
    // Native state0 guards scanner construction only. Once constructed it has
    // no parser EH cleanup; an exception after this point retains the scanner.
    auto* const scanner = context.scanner.open_scanner_00bef2e0(filename).release();
    auto& tokens = scanner->tokens;
    for (;;) {
        const auto& token = tokens.peek_00bee8e0();
        if (tokens.eof_at_token_start() || (token.empty() && !tokens.quoted())) break;
        if (keyword(tokens, "Channels")) {
            tokens.accept_00bee800();
            while (!keyword(tokens, "end")) {
                TemporaryRecord temporary{{}, context.strings};
                write(temporary.bytes, 0xc, context.temporary_first_channel_word);
                bool ignored_success = false;
                const char* const text = tokens.read_string_00bef020(ignored_success).c_str();
                const auto length = static_cast<std::uint32_t>(std::strlen(text));
                if (length) {
                    char* const data = context.strings.allocate(length + 1u);
                    write(temporary.bytes, 4, data);
                    write(temporary.bytes, 0, length);
                    data[length] = 0;
                    std::memmove(data, text, length);
                }
                std::uint32_t format = 0;
                if (keyword(tokens, "mono")) format = 1;
                else if (keyword(tokens, "stereo")) format = 2;
                else if (keyword(tokens, "51")) format = 3;
                if (format) {
                    tokens.accept_00bee800();
                    const auto count = context.format_counts_00e12ef0[format];
                    write(temporary.bytes, 0x10, format);
                    write(temporary.bytes, 8, count);
                }
                auto* const header = at(table, 8);
                if (read<std::uint32_t>(table, 0xc) == read<std::uint32_t>(table, 0x10)) {
                    auto capacity = signed_word(read<std::uint32_t>(table, 0x10) * 2u);
                    if (capacity <= 1) capacity = 1;
                    reserve_sound_dialog_records_00a78dc0(header, capacity, context.strings,
                        context.format_counts_00e12ef0);
                }
                auto* const row = at(read<void*>(header, 0), read<std::uint32_t>(header, 4) * 0x14u);
                if (row) copy_sound_dialog_record_00a77ef0(row, temporary.bytes,
                    context.strings, context.format_counts_00e12ef0);
                write(header, 4, read<std::uint32_t>(header, 4) + 1u);
                auto* const last = at(read<void*>(header, 0), read<std::uint32_t>(header, 4) * 0x14u - 0x14u);
                write(last, 0xc, read<std::uint32_t>(table, 0x18));
                write(table, 0x18, read<std::uint32_t>(table, 0x18) + read<std::uint32_t>(temporary.bytes, 8));
            }
            tokens.accept_00bee800();
        } else if (keyword(tokens, "Volume")) {
            tokens.accept_00bee800();
            bool ignored_success = false;
            write(table, 0x14, tokens.read_float_00bef170(ignored_success));
        } else if (keyword(tokens, "Loop")) {
            tokens.accept_00bee800();
            write<std::uint8_t>(table, 0x1c, 1);
        }
    }
    delete scanner;
}
} // namespace bsp
