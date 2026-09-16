#define _CRT_SECURE_NO_WARNINGS
#include "bsp/native_particle_texture_names_raw.hpp"

#include "bsp/native_particle_type_property.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/native_string.hpp"

#include <cstdio>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle texture-name raw helpers require MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;

void* at(void* value, Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(value) + offset);
}
const void* at(const void* value, Word offset) noexcept {
    return reinterpret_cast<const void*>(reinterpret_cast<std::uintptr_t>(value) + offset);
}
template<class T> T read(const void* value, Word offset = 0) noexcept {
    T result;
    std::memcpy(&result, at(value, offset), sizeof result);
    return result;
}
template<class T> void write(void* value, Word offset, T field) noexcept {
    std::memcpy(at(value, offset), &field, sizeof field);
}
void clear_header(void* header) noexcept {
    write<Word>(header, 0, 0);
    write<char*>(header, 4, nullptr);
}

// A native EH ownership state is armed only after its constructor returns.
// Normal destruction clears the state first. A second pool-getter failure while
// unwinding reaches C++ termination, matching the established raw-string policy.
class RawHeaderOwner {
public:
    explicit RawHeaderOwner(NativeStringRawPoolContext& strings) noexcept
        : strings_(strings) {}
    RawHeaderOwner(const RawHeaderOwner&) = delete;
    RawHeaderOwner& operator=(const RawHeaderOwner&) = delete;
    ~RawHeaderOwner() noexcept {
        if (armed_) destroy_native_string_header_0041dd20(header_, strings_);
    }
    void* header() noexcept { return header_; }
    const void* header() const noexcept { return header_; }
    void arm() noexcept { armed_ = true; }
    void release() {
        armed_ = false;
        destroy_native_string_header_0041dd20(header_, strings_);
    }
private:
    alignas(4) unsigned char header_[8]{};
    NativeStringRawPoolContext& strings_;
    bool armed_ = false;
};

class OutputUnwind {
public:
    OutputUnwind(void* header, NativeStringRawPoolContext& strings) noexcept
        : header_(header), strings_(strings) {}
    ~OutputUnwind() noexcept {
        if (armed_) destroy_native_string_header_0041dd20(header_, strings_);
    }
    void arm() noexcept { armed_ = true; }
    void dismiss() noexcept { armed_ = false; }
private:
    void* header_;
    NativeStringRawPoolContext& strings_;
    bool armed_ = false;
};

// BE0A30-shaped assignment emitted inline by 4CAD40/AEFB20. Source fields are
// reloaded after resize; destination/source identity skips every operation.
void assign_current_header(void* destination, const void* source,
    NativeStringRawPoolContext& strings) {
    if (destination == source) return;
    resize_native_string_header_0041dd40(destination, strings,
        read<Word>(source), true);
    if (read<Word>(source) != 0) {
        const Word count = read<Word>(destination);
        void* output = read<void*>(destination, 4);
        const void* input = read<const void*>(source, 4);
        if (count != 0) std::memmove(output, input, count);
    }
}

// 426060-shaped output construction emitted inline by AF3B50. Clearing happens
// before the identity branch and abandons any old output allocation.
void* copy_construct_current_header(void* destination, const void* source,
    NativeStringRawPoolContext& strings) {
    const bool identical = destination == source;
    clear_header(destination);
    if (!identical) {
        resize_native_string_header_0041dd40(destination, strings,
            read<Word>(source), true);
        if (read<Word>(source) != 0) {
            const Word count = read<Word>(destination);
            void* output = read<void*>(destination, 4);
            const void* input = read<const void*>(source, 4);
            if (count != 0) std::memmove(output, input, count);
        }
    }
    return destination;
}

std::int32_t initial_substring_position(const void* value,
    const void* search) noexcept {
    const char* pattern = read<const char*>(search, 4); // 4CAD60
    const char* data = read<const char*>(value, 4);     // 4CAD63
    if (!data || !pattern) return -1;
    const char* match = std::strstr(data, pattern);
    if (!match) return -1;
    return static_cast<std::int32_t>(match - read<const char*>(value, 4));
}

std::int32_t next_substring_position(const void* value, const void* search,
    const void* replacement, std::int32_t prior) noexcept {
    Word start = read<Word>(replacement) + static_cast<Word>(prior);
    const char* data = read<const char*>(value, 4);
    const char* pattern = read<const char*>(search, 4);
    if (!data || !pattern) return -1;
    if (static_cast<std::int32_t>(start) < 0) start = 0;
    else if (start > read<Word>(value)) return -1;
    const char* match = std::strstr(data + start, pattern);
    if (!match) return -1;
    return static_cast<std::int32_t>(match - read<const char*>(value, 4));
}

std::int32_t last_index(const char* value) noexcept {
    return static_cast<std::int32_t>(static_cast<Word>(std::strlen(value)) - 1u);
}

void strip_query_prefix(void* query, Word start,
    NativeStringRawPoolContext& strings) {
    RawHeaderOwner suffix(strings);
    construct_native_string_substring_00469840(query, suffix.header(), start,
        0x7fffffffu, strings);
    suffix.arm();
    assign_current_header(query, suffix.header(), strings);
    suffix.release();
}

void* current_item(void* manager, Word index) noexcept {
    void* rows = read<void*>(manager, 4);
    return read<void*>(rows, index * 4u);
}

void* scan_items(void* manager, const void* query,
    const char* null_pattern) {
    Word index = 0;
    while (static_cast<std::int32_t>(index) < read<std::int32_t>(manager, 8)) {
        void* item = current_item(manager, index);
        if (matches_native_particle_atlas_item_00aee0f0(
                item, query, null_pattern))
            return current_item(manager, index); // AEFE15/AEFEF1 reload +4/item.
        ++index;
    }
    return nullptr;
}
} // namespace

void resize_native_particle_string_fill_0043bbf0(void* header,
    Word length, std::int8_t fill, NativeStringRawPoolContext& strings) {
    const Word old_length = read<Word>(header);
    resize_native_string_header_0041dd40(header, strings, length, true);
    const Word current_length = read<Word>(header);
    if (old_length < current_length) {
        char* current_data = read<char*>(header, 4);
        std::memset(at(current_data, old_length), fill,
            current_length - old_length);
    }
}

void replace_native_particle_string_substrings_004cad40(void* value,
    const void* search, const void* replacement, Word count,
    NativeStringRawPoolContext& strings) {
    std::int32_t position = initial_substring_position(value, search);
    while (position != -1) {
        RawHeaderOwner suffix(strings), prefix(strings), head(strings), joined(strings);
        construct_native_string_substring_00469840(value, suffix.header(),
            read<Word>(search) + static_cast<Word>(position), 0x7fffffffu, strings);
        suffix.arm();
        construct_native_string_substring_00469840(value, prefix.header(), 0,
            static_cast<Word>(position), strings);
        prefix.arm();
        concatenate_native_string_headers_004261a0(prefix.header(), head.header(),
            replacement, strings);
        head.arm();
        concatenate_native_string_headers_004261a0(head.header(), joined.header(),
            suffix.header(), strings);
        joined.arm();
        assign_current_header(value, joined.header(), strings);
        joined.release();
        head.release();
        prefix.release();
        suffix.release();

        position = next_substring_position(value, search, replacement, position);
        --count;
        if (count == 0) return;
    }
}

void* construct_native_particle_texture_stem_00af37d0(void* output,
    const char* filename, NativeStringRawPoolContext& strings) {
    OutputUnwind output_unwind(output, strings);
    output_unwind.arm(); // AF37F5: output cleanup state precedes 41E870.
    construct_native_string_header_0041e870(output, strings, filename);

    RawHeaderOwner needle(strings);
    construct_native_string_header_0041e870(needle.header(), strings, ".");
    needle.arm();
    const std::int32_t dot = reverse_find_native_string_header_00467cf0(
        output, needle.header(), 0x7fffffffu);
    needle.release();
    if (dot != -1)
        resize_native_particle_string_fill_0043bbf0(output,
            static_cast<Word>(dot), 0x20, strings);
    output_unwind.dismiss();
    return output;
}

void* construct_native_particle_texture_extension_00af38b0(void* output,
    const char* filename, NativeStringRawPoolContext& strings) {
    std::int32_t dot = 0;
    for (std::int32_t index = last_index(filename); index > 0; --index) {
        if (filename[index] == '.') { dot = index; break; }
    }
    char buffer[256];
    std::strcpy(buffer, filename + dot);
    return construct_native_string_header_0041e870(output, strings, buffer);
}

void* construct_native_particle_texture_prefix_00af3960(void* output,
    const char* stem, NativeStringRawPoolContext& strings) {
    char buffer[256];
    std::strcpy(buffer, stem);
    Word zeros = 0;
    for (std::int32_t index = last_index(buffer); index > 0; --index) {
        if (buffer[index] == '0' && ++zeros == 3) {
            buffer[index] = '\0';
            break;
        }
    }
    return construct_native_string_header_0041e870(output, strings, buffer);
}

void* find_native_particle_atlas_item_00aefb20(void* manager,
    const char* filename, NativeStringRawPoolContext& strings,
    const char* null_pattern) {
    if (!filename || !*filename) return nullptr;

    RawHeaderOwner query(strings);
    construct_native_particle_texture_stem_00af37d0(
        query.header(), filename, strings);
    query.arm();
    {
        RawHeaderOwner slash(strings), backslash(strings);
        construct_native_string_header_0041e870(slash.header(), strings, "/");
        slash.arm();
        construct_native_string_header_0041e870(backslash.header(), strings, "\\");
        backslash.arm();
        replace_native_particle_string_substrings_004cad40(query.header(),
            backslash.header(), slash.header(), 0x7fffffffu, strings);
        backslash.release();
        slash.release();
    }

    // AEFC43 deliberately dereferences query.data without a null guard.
    while (*read<const char*>(query.header(), 4) == '/')
        strip_query_prefix(query.header(), 1, strings);
    lowercase_native_string_header_004bcc00(query.header());

    if (void* result = scan_items(manager, query.header(), null_pattern)) {
        query.release();
        return result;
    }

    std::int32_t slash_position;
    {
        RawHeaderOwner slash(strings);
        construct_native_string_header_0041e870(slash.header(), strings, "/");
        slash.arm();
        slash_position = reverse_find_native_string_header_00467cf0(
            query.header(), slash.header(), 0x7fffffffu);
        slash.release();
    }
    if (slash_position >= 0) {
        strip_query_prefix(query.header(),
            static_cast<Word>(slash_position) + 1u, strings);
        if (void* result = scan_items(manager, query.header(), null_pattern)) {
            query.release();
            return result;
        }
    }
    query.release();
    return nullptr;
}

std::int32_t count_native_particle_texture_frames_00af3a20(
    const char* filename, NativeParticleTextureNamesRawContext& context) {
    RawHeaderOwner stem(context.strings);
    construct_native_particle_texture_stem_00af37d0(
        stem.header(), filename, context.strings);
    stem.arm();
    const char* stem_text = read<const char*>(stem.header(), 4);
    if (!stem_text) stem_text = context.empty_stem_00f8c2c1;
    if (!has_native_particle_frame_suffix_00af3750(stem_text)) {
        stem.release();
        return 0;
    }

    RawHeaderOwner prefix(context.strings);
    construct_native_particle_texture_prefix_00af3960(
        prefix.header(), stem_text, context.strings);
    prefix.arm();
    std::int32_t index = 0;
    char buffer[32];
    for (;;) {
        const char* prefix_text = read<const char*>(prefix.header(), 4);
        if (!prefix_text) prefix_text = context.empty_stem_00f8c2c1;
        std::sprintf(buffer, "%s%03d", prefix_text, index);
        void* manager = context.actual_atlas_manager_00f8c26c; // AF3AAC reload.
        if (!find_native_particle_atlas_item_00aefb20(manager, buffer,
                context.strings, context.null_pattern_00e17bf0))
            break;
        ++index;
    }
    prefix.release();
    stem.release();
    return index;
}

void* construct_native_particle_texture_frame_name_00af3b50(void* output,
    const char* filename, std::int32_t index,
    NativeParticleTextureNamesRawContext& context) {
    RawHeaderOwner stem(context.strings), prefix(context.strings),
        extension(context.strings), number(context.strings), head(context.strings),
        joined(context.strings);
    construct_native_particle_texture_stem_00af37d0(
        stem.header(), filename, context.strings);
    stem.arm();
    const char* stem_text = read<const char*>(stem.header(), 4);
    if (!stem_text) stem_text = context.empty_stem_00f8c2c1;
    construct_native_particle_texture_prefix_00af3960(
        prefix.header(), stem_text, context.strings);
    prefix.arm();
    construct_native_particle_texture_extension_00af38b0(
        extension.header(), filename, context.strings);
    extension.arm();

    char buffer[32];
    std::sprintf(buffer, "%03d", index);
    construct_native_string_header_0041e870(number.header(), context.strings, buffer);
    number.arm();
    concatenate_native_string_headers_004261a0(prefix.header(), head.header(),
        number.header(), context.strings);
    head.arm();
    concatenate_native_string_headers_004261a0(head.header(), joined.header(),
        extension.header(), context.strings);
    joined.arm();
    copy_construct_current_header(output, joined.header(), context.strings);

    OutputUnwind output_unwind(output, context.strings);
    output_unwind.arm(); // AF3C88: only after output construction returns.
    joined.release();
    head.release();
    number.release();
    extension.release();
    prefix.release();
    stem.release();
    output_unwind.dismiss();
    return output;
}
} // namespace bsp
