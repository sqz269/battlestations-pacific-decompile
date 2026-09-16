#include "bsp/native_particle_text_loader.hpp"

#include "bsp/native_string.hpp"
#include "bsp/native_vfs_name_resolution.hpp"
#include "bsp/native_vfs_runtime_bindings.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native particle text loading requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
void* at(const void* base, Word offset = 0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(base) + offset);
}
volatile Word& word(const void* base, Word offset = 0) noexcept {
    return *static_cast<volatile Word*>(at(base, offset));
}
void* pointer(const void* base, Word offset = 0) noexcept {
    return reinterpret_cast<void*>(word(base, offset));
}
} // namespace

std::uint32_t load_native_particle_text_buffer_00af5850(
    void* text, const char* name, void* volatile& actual_vfs_publication_0109ceec,
    NativeVfsRuntimeBindings& vfs, NativeVfsNameResolutionContext& name_resolution,
    NativeVfsNameResolutionAcquired& invocation, NativeStringRawPoolContext& strings) {
    // Native reuses its incoming name argument word at [ESP+10] for the
    // output-count pointer. Retain its input bits until a real read replaces it.
    Word actual_count = reinterpret_cast<Word>(name);
    void* const name_header = at(text, 0x0c);
    Word length = 0;
    if (name) {
        while (name[length] != '\0') ++length;
    }
    resize_native_string_header_0041dd40(name_header, strings, length, false);
    void* const current_name_bytes = pointer(name_header, 4);
    if (current_name_bytes) {
        const Word current_length = word(name_header);
        // BF7680's overlapping-range branch is part of the native primitive.
        if (current_length != 0) std::memmove(current_name_bytes, name, current_length);
    }

    void* const resolve_manager = actual_vfs_publication_0109ceec;
    (void)resolve_native_vfs_existing_name_00bdf4c0(
        resolve_manager, name_header, name_resolution, invocation);
    void* const open_manager = actual_vfs_publication_0109ceec;
    const Word open_target = word(pointer(open_manager), 4);
    void* const stream = vfs.open_manager_entry(open_target, open_manager, name_header, 0x32);

    const Word length_target = word(pointer(stream), 0x30);
    const Word extent = static_cast<Word>(vfs.stream_length_entry(length_target, stream));
    word(text) = extent;
    word(text, 8) = extent;
    word(text, 4) = 0;
    word(text, 0x18) = 0;
    // BF55BE is only JMP BF681B. The existing CRT allocation provider owns
    // this exact byte extent; no array header, terminator or zero fill is added.
    void* const allocation = singleton_lifetime_allocate({
        SingletonAllocationKind::object, extent, extent});
    const Word requested = word(text);
    word(text, 0x14) = reinterpret_cast<Word>(allocation);
    const Word read_target = word(pointer(stream), 0x24);
    vfs.source_read(read_target, stream, allocation, requested, &actual_count);
    if (actual_count != word(text)) {
        singleton_lifetime_free(pointer(text, 0x14));
        word(text, 0x14) = 0;
        return 0;
    }

    if (InterlockedDecrement(static_cast<volatile LONG*>(at(stream, 4))) == 0) {
        const Word table = word(stream);
        const Word terminal = word(reinterpret_cast<void*>(table));
        vfs.source_zero_reference(terminal, stream, table);
    }
    return actual_count;
}
} // namespace bsp
