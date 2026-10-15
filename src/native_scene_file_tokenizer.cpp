#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#include "bsp/native_scene_file_tokenizer.hpp"
#include "bsp/native_crt_memset.hpp"
#include "bsp/native_vfs_runtime_bindings.hpp"
#include <cstring>
#include <new>
#include <type_traits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native scene file tokenizer requires MSVC Win32.
#endif
namespace bsp {
namespace {
using U = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(NativeString) == 8 && alignof(NativeString) == 4);
static_assert(std::is_nothrow_default_constructible_v<NativeString> &&
    std::is_trivially_destructible_v<NativeString>);
void* at(void* owner, U byte_offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<U>(owner) + byte_offset);
}
U word(const void* owner, U byte_offset) noexcept {
    U value;
    __asm {
        mov eax, owner
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov value, eax
    }
    return value;
}
void put(void* owner, U byte_offset, U value) noexcept {
    __asm {
        mov eax, owner
        mov edx, byte_offset
        mov ecx, value
        mov dword ptr [eax + edx], ecx
    }
}
void zero_byte(void* owner, U byte_offset) noexcept {
    __asm {
        mov eax, owner
        mov edx, byte_offset
        mov byte ptr [eax + edx], 0
    }
}
} // namespace

NativeSceneFileTokenizerVfsCalls::NativeSceneFileTokenizerVfsCalls(
    NativeVfsRuntimeBindings& bindings) noexcept : bindings_(bindings) {}
void* NativeSceneFileTokenizerVfsCalls::open(void* manager, const NativeString& name, U flags) {
    const auto table = capture_native_lua_vfs_table(manager);
    return bindings_.open(table, manager, name, flags);
}
std::int64_t NativeSceneFileTokenizerVfsCalls::stream_size_slot30(void* stream,
    NativeRetainedMemoryOwnerContext&) {
    const auto table = capture_native_lua_vfs_table(stream);
    return static_cast<std::int64_t>(bindings_.length(table, stream));
}
void NativeSceneFileTokenizerVfsCalls::stream_read_slot24(void* stream, void* destination,
    U count, U* actual, NativeRetainedMemoryOwnerContext&) {
    const auto table = capture_native_lua_vfs_table(stream);
    bindings_.read(table, stream, destination, count, actual);
}
void NativeSceneFileTokenizerVfsCalls::stream_zero_reference_slot0(void* stream,
    NativeRetainedMemoryOwnerContext&) {
    const auto table = capture_native_lua_vfs_table(stream);
    bindings_.zero_reference(table, stream);
}

void* construct_native_scene_file_tokenizer_008d9cf0(void* owner,
    NativeString& filename, const char* extra, NativeSceneFileTokenizerContext& c) {
    // These two states are the actual Original cleanup obligations, not a
    // stream/buffer transaction. Normal final argument release is state-1.
    int unwind_state = 0;
    NativeString* member = nullptr;
    try {
        put(owner, 0, 0x00d15fe4);
        member = ::new (at(owner, 0x820)) NativeString;
        unwind_state = 1;
        put(owner, 0x81c, 0);
        fill_native_crt_bytes_00bf79f0(at(owner, 5), 0, 0x400, c.actual_crt_feature_word_0109eea4);
        fill_native_crt_bytes_00bf79f0(at(owner, 0x405), 0, 0x400, c.actual_crt_feature_word_0109eea4);
        zero_byte(owner, 0x805); zero_byte(owner, 0x80a);
        zero_byte(owner, 0x809); zero_byte(owner, 0x808);
        put(owner, 0x814, 0); put(owner, 0x810, 0); put(owner, 0x80c, 0);
        put(owner, 0x818, 0);
        zero_byte(owner, 0x807); zero_byte(owner, 0x806);
        if (member != &filename) {
            const U captured_length = filename.length();
            resize_native_string_header_0041dd40(member, c.strings, captured_length, true);
            if (captured_length != 0) {
                // Reload both headers after resize; native BF7680 admits overlap.
                std::memmove(member->data(), filename.data(), member->length());
            }
        }
        void* const manager = c.actual_vfs_publication_0109ceec;
        void* const opened = c.calls.open(manager, *member, 0x32);
        put(owner, 0x828, reinterpret_cast<U>(opened));
        if (opened) {
            const U size = static_cast<U>(c.calls.stream_size_slot30(opened, c.memory));
            put(owner, 0x830, size);
            void* const buffer = c.calls.allocate_00bf55be(size);
            const U current_size = word(owner, 0x830);
            void* const current_stream = reinterpret_cast<void*>(word(owner, 0x828));
            put(owner, 0x82c, reinterpret_cast<U>(buffer));
            put(owner, 0x834, 0);
            c.calls.stream_read_slot24(current_stream, buffer, current_size, nullptr, c.memory);
            if (extra) {
                const U n = static_cast<U>(std::strlen(c.default_delimiters_00e0c944));
                const U e = static_cast<U>(std::strlen(extra));
                auto* const allocated = static_cast<char*>(c.calls.allocate_00bf55be(n + e + 2u));
                put(owner, 0x818, reinterpret_cast<U>(allocated));
                std::strcpy(allocated, c.default_delimiters_00e0c944);
                const U extra_bytes = static_cast<U>(std::strlen(extra)) + 1u;
                auto* const destination = reinterpret_cast<char*>(word(owner, 0x818));
                std::memcpy(destination + std::strlen(destination), extra, extra_bytes);
            } else {
                put(owner, 0x818, reinterpret_cast<U>(c.default_delimiters_00e0c944));
            }
        }
        unwind_state = -1;
        destroy_native_string_header_0041dd20(&filename, c.strings);
        return owner;
    } catch (...) {
        if (unwind_state >= 1) destroy_native_string_header_0041dd20(member, c.strings);
        if (unwind_state >= 0) destroy_native_string_header_0041dd20(&filename, c.strings);
        throw;
    }
}
} // namespace bsp
