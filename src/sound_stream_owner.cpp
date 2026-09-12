#include "bsp/sound_stream_owner.hpp"
#include "bsp/sound_alternate_owner.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/native_vfs_open_logging.hpp"
#include "bsp/singleton_lifetime.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Stream sound ownership requires MSVC Win32.
#endif
namespace bsp {
namespace {
template<class T> T read(const void* p, std::size_t offset) noexcept {
    T v; std::memcpy(&v, static_cast<const std::byte*>(p) + offset, sizeof v); return v;
}
template<class T> void write(void* p, std::size_t offset, T v) noexcept {
    std::memcpy(static_cast<std::byte*>(p) + offset, &v, sizeof v);
}
void* at(void* p, std::size_t offset) noexcept { return static_cast<std::byte*>(p) + offset; }
std::int32_t signed_word(std::uint32_t n) noexcept {
    std::int32_t v; std::memcpy(&v, &n, sizeof v); return v;
}
void* word_address(void* p, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) + offset);
}
void* allocate(std::uint32_t n) {
    return singleton_lifetime_allocate({SingletonAllocationKind::object, n, n});
}
void release_reference(void* object, SoundStreamOwnerContext& c) {
    if (InterlockedDecrement(reinterpret_cast<volatile LONG*>(at(object, 4))) == 0) {
        if (read<std::uint32_t>(object, 0) == 0x00d58f80)
            scalar_delete_sound_alternate_table_00a796f0(object, 1, c.table.strings);
        else c.references.zero_references_slot_00(object);
    }
}
void clear_reference(void* slot, SoundStreamOwnerContext& c) {
    if (void* const object = read<void*>(slot, 0)) {
        release_reference(object, c);
        write<void*>(slot, 0, nullptr); // after the current-vtable callback
    }
}
void root(void* p) noexcept { write<std::uint32_t>(p, 0, 0x00ceb130); }
void initialize_control(void* vector, std::uint32_t offset) noexcept {
    if (void* const record = word_address(read<void*>(vector, 0), offset)) {
        write(record, 0, 1.0f); write(record, 4, 1.0f); // +8 is never initialized
    }
}
// DEC264 destructor states. A secondary exception while unwinding terminates,
// as native C++ EH does; normal zero-reference callbacks may still throw.
void unwind_members(void* p, int state, SoundStreamOwnerContext& c) noexcept {
    if (state >= 5) destroy_sound_stream_controls_00a86990(at(p, 0x48));
    if (state >= 4) clear_reference(at(p, 0x3c), c);
    if (state >= 3) destroy_native_string_header_0041dd20(at(p, 0x34), c.table.strings);
    if (state >= 2) destroy_native_string_header_0041dd20(at(p, 0x2c), c.table.strings);
    if (state >= 1) destroy_native_string_header_0041dd20(at(p, 0x24), c.table.strings);
    if (state >= 0) root(p);
}
} // namespace

void reserve_sound_stream_controls_00a86540(void* vector, std::int32_t capacity) {
    if (capacity < 1) capacity = 1;
    if (read<std::int32_t>(vector, 8) >= capacity) return;
    void* const block = allocate(static_cast<std::uint32_t>(capacity) * 12u);
    std::uint32_t index = 0, offset = 0;
    while (signed_word(index) < read<std::int32_t>(vector, 4)) {
        if (void* const destination = word_address(block, offset)) {
            void* const source = word_address(read<void*>(vector, 0), offset);
            write(destination, 0, read<std::uint32_t>(source, 0));
            write(destination, 4, read<std::uint32_t>(source, 4));
            write(destination, 8, read<std::uint32_t>(source, 8));
        }
        ++index; offset += 12u;
    }
    singleton_lifetime_free(read<void*>(vector, 0));
    write(vector, 0, block); write(vector, 8, capacity);
}
void resize_sound_stream_controls_00a868b0(void* vector, std::int32_t count) {
    if (count > read<std::int32_t>(vector, 8)) reserve_sound_stream_controls_00a86540(vector, count);
    const auto requested = static_cast<std::uint32_t>(count);
    auto index = read<std::uint32_t>(vector, 4);
    const auto difference = requested - index;
    if (signed_word(difference) >= 4) {
        const auto groups = ((difference - 4u) >> 2) + 1u;
        auto offset = index * 12u;
        index += groups * 4u;
        for (std::uint32_t group = 0; group < groups; ++group) {
            initialize_control(vector, offset); initialize_control(vector, offset + 12u);
            initialize_control(vector, offset + 24u); initialize_control(vector, offset + 36u);
            offset += 48u;
        }
    }
    if (signed_word(index) < count) {
        auto offset = index * 12u;
        const auto remaining = requested - index;
        for (std::uint32_t n = 0; n < remaining; ++n, offset += 12u)
            initialize_control(vector, offset);
    }
    while (count < read<std::int32_t>(vector, 4))
        write(vector, 4, read<std::uint32_t>(vector, 4) - 1u);
    write(vector, 4, count);
}
void destroy_sound_stream_controls_00a86990(void* vector) {
    resize_sound_stream_controls_00a868b0(vector, 0);
    singleton_lifetime_free(read<void*>(vector, 0));
}
void* construct_sound_stream_table_00a79150(void* p, const NativeString& filename,
    SoundDialogTableContext& c) {
    root(p); write<std::int32_t>(p, 4, 1);
    write<std::uint32_t>(p, 0, 0x00d58f80);
    write<void*>(p, 8, nullptr); write<std::int32_t>(p, 0xc, 0);
    write<std::int32_t>(p, 0x10, 0); write<std::uint32_t>(p, 0x18, 0);
    write<std::uint8_t>(p, 0x1c, 0); write(p, 0x14, 1.0f);
    try { load_sound_dialog_table_00a87060(p, filename, c); }
    catch (...) {
        // DEADD4: A790F0(current vector) followed by BD30F0. The existing
        // A791D0 body is precisely that sequence, with no derived-vtable store.
        destroy_sound_alternate_table_00a791d0(p, c.strings); throw;
    }
    return p;
}
void* construct_sound_stream_00a877d0(void* p, NativeString& owned_name,
    void* owned_table, SoundStreamOwnerContext& c) {
    auto& strings = c.table.strings;
    root(p); write<std::int32_t>(p, 4, 1); write<std::uint32_t>(p, 0, 0x00d5b360);
    for (std::size_t offset = 8; offset != 12; ++offset) write<std::uint8_t>(p, offset, 0);
    write(p, 0xc, 1.0f); write(p, 0x14, 1.0f);
    for (std::size_t offset = 0x18; offset != 0x54; offset += 4)
        write<std::uint32_t>(p, offset, 0);
    int state = 7;
    NativeString definition, extension, substring, concatenated;
    void* partial_table = nullptr;
    try {
        copy_native_string_header_00be0a30_fragment(at(p, 0x24), strings, &owned_name);
        copy_native_string_header_00be0a30_fragment(at(p, 0x2c), strings, at(p, 0x24));
        c.host.resolve_stream_name_00bdf4c0(at(p, 0x2c));
        state = 8;
        extension.resize_0041dd40(strings, 4, true);
        char* const extension_data = extension.data();
        if (extension_data) std::memcpy(extension_data, ".def", extension.length() + 1u);
        state = 9;
        construct_native_string_substring_00469840(at(p, 0x2c), &substring, 0,
            read<std::uint32_t>(p, 0x2c) - 4u, strings);
        state = 10;
        concatenate_native_string_headers_004261a0(&substring, &concatenated, &extension, strings);
        state = 11;
        copy_native_string_header_00be0a30_fragment(&definition, strings, &concatenated);
        state = 10; destroy_native_string_header_0041dd20(&concatenated, strings);
        state = 9; destroy_native_string_header_0041dd20(&substring, strings);
        state = 8;
        // Native normal cleanup uses the pointer captured immediately after
        // extension allocation, with the CURRENT extension length.
        if (extension_data) strings.release(extension_data, extension.length() + 1u);
        if (owned_table) {
            void* const old = read<void*>(p, 0x3c);
            if (old != owned_table) {
                write(p, 0x3c, owned_table);
                InterlockedIncrement(reinterpret_cast<volatile LONG*>(at(owned_table, 4)));
                if (old) release_reference(old, c);
            }
        } else {
            partial_table = allocate(0x20);
            state = 12;
            void* const created = construct_sound_stream_table_00a79150(partial_table, definition, c.table);
            state = 8;
            clear_reference(at(p, 0x3c), c);
            write(p, 0x3c, created);
        }
        const auto controls = read<std::int32_t>(read<void*>(p, 0x3c), 0xc);
        resize_sound_stream_controls_00a868b0(at(p, 0x48), controls);
        if (read<std::int32_t>(p, 0x4c) > 0) {
            std::uint32_t iteration = 0, index = 0;
            do {
                void* const record = word_address(read<void*>(p, 0x48), index * 12u);
                ++iteration;
                write(record, 0, 0.0f); write(record, 4, 0.0f);
                index = static_cast<std::uint16_t>(iteration); // native MOVZX EAX,CX
            } while (signed_word(index) < read<std::int32_t>(p, 0x4c));
        }
        state = 7; destroy_native_string_header_0041dd20(&definition, strings);
        state = 0; destroy_native_string_header_0041dd20(&owned_name, strings);
        state = -1;
        if (owned_table) release_reference(owned_table, c);
    } catch (...) {
        // DEC304. State12's allocation cleanup returns directly to state8;
        // extension/substring/concat are already dead by the allocation call.
        const auto unwind = [&]() noexcept {
            if (state == 12) { singleton_lifetime_free(partial_table); state = 8; }
            if (state >= 11) destroy_native_string_header_0041dd20(&concatenated, strings);
            if (state >= 10) destroy_native_string_header_0041dd20(&substring, strings);
            if (state >= 9) destroy_native_string_header_0041dd20(&extension, strings);
            if (state >= 8) destroy_native_string_header_0041dd20(&definition, strings);
            if (state >= 2) unwind_members(p, state >= 7 ? 5 : state - 2, c);
            if (state >= 1) destroy_native_string_header_0041dd20(&owned_name, strings);
            if (state >= 0 && owned_table) release_reference(owned_table, c);
        };
        unwind(); throw;
    }
    return p;
}
void destroy_sound_stream_00a87390(void* p, SoundStreamOwnerContext& c) {
    auto& strings = c.table.strings;
    write<std::uint32_t>(p, 0, 0x00d5b360);
    alignas(4) std::byte builder[0x18];
    int state = 5;
    try {
        construct_native_log_builder_00426500(builder, strings); state = 6;
        append_native_log_cstring_00bd1a60(builder, "cStreamAudio::~cStreamAudio()", strings);
        append_native_log_header_00bd1a20(builder, at(p, 0x24), strings);
        state = 5; destroy_native_log_builder_00425f80(builder, strings);
        c.host.stop_stream_00a86bf0(p);
        resize_sound_stream_controls_00a868b0(at(p, 0x48), 0);
        state = 4; destroy_sound_stream_controls_00a86990(at(p, 0x48));
        state = 3; clear_reference(at(p, 0x3c), c);
        state = 2; destroy_native_string_header_0041dd20(at(p, 0x34), strings);
        state = 1; destroy_native_string_header_0041dd20(at(p, 0x2c), strings);
        state = 0; destroy_native_string_header_0041dd20(at(p, 0x24), strings);
        state = -1; root(p);
    } catch (...) {
        if (state >= 6) destroy_native_log_builder_00425f80(builder, strings);
        unwind_members(p, state >= 5 ? 5 : state, c); throw;
    }
}
void* scalar_delete_sound_stream_00a87b30(void* p, std::uint8_t flags, SoundStreamOwnerContext& c) {
    destroy_sound_stream_00a87390(p, c);
    if (flags & 1u) singleton_lifetime_free(p);
    return p;
}
} // namespace bsp
