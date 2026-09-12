#pragma once

#include "bsp/native_string.hpp"

namespace bsp {

// Actual four-byte line/token owner, unlike NativeString's length/pointer pair.
// No implicit cleanup: use AEE2A0 with the application's SAME string storage.
struct NativePooledTextStorage { char* data; };

// Native stack text argument, RET4 (stdcall-compatible); text must be nonnull.
// All allocations/releases use strlen+1 through the canonical pool domain.
void release_native_pooled_text_bytes_00aee1e0(char* text,
    NativeStringStorage& storage) noexcept;

// ECX header, RET. Releases captured pointer then clears header, including
// changes made by the release callback. Null data skips the pool entirely.
void destroy_native_pooled_text_00aee2a0(void* actual_header,
    NativeStringStorage& storage) noexcept;

// ECX destination, stack source header, RET4, EAX destination. Constructor:
// overwrites destination without releasing; reloads source AFTER publishing
// allocation. No self-alias special case or allocation-failure fallback.
void* copy_construct_native_pooled_text_00aee2e0(void* actual_destination,
    const void* actual_source, NativeStringStorage& storage);

// ECX destination, stack bytes, RET4, EAX destination. Null creates null;
// empty text allocates one byte. Does not release an old destination.
void* construct_native_pooled_text_00af5660(void* actual_destination,
    const char* text, NativeStringStorage& storage);

// ECX destination, stack source header, RET4, EAX destination. Releases old
// destination BEFORE reading source. Null source leaves the old pointer in the
// header; self/byte aliasing can access returned storage. No safety repair.
void* assign_native_pooled_text_00af56c0(void* actual_destination,
    const void* actual_source, NativeStringStorage& storage);

// ECX header, stack text, RET4, EAX copied length. Copy only 21h..7Eh;
// null/empty/nonprintable prefix preserves an existing destination unchanged.
std::uint32_t assign_native_pooled_text_prefix_00aee340(void* actual_header,
    const char* text, NativeStringStorage& storage);

// ECX line, stack output/index, RET8, EAX output. CONSTRUCTS output, so caller
// supplies dead/unowned header storage. Token selection precedes space skip:
// leading spaces produce null token0. Missing tokens also construct null.
// Native's temporary allocation, output copy and temporary release are kept.
void* get_native_pooled_text_token_00aee3c0(const void* actual_line,
    void* actual_output, std::int32_t index, NativeStringStorage& storage);

// ECX header, stack bytes, RET4, AL boolean. Current CRT case-insensitive
// compare boundary; null arguments remain unsupported as in native callers.
bool equal_native_pooled_text_00aedf80(const void* actual_header,
    const char* text);

// ECX buffer, RET, EAX buffer. Actual initialization leaves +8 extent untouched.
void* initialize_native_text_buffer_00af5600(void* actual_buffer) noexcept;
// ECX buffer, RET. Clears only cursor+4 and auxiliary+18h.
void rewind_native_text_buffer_00af55f0(void* actual_buffer) noexcept;

// ECX buffer, stack output, RET4, AL boolean. Actual buffer has signed cursor
// +4, signed extent+8, data pointer+14h. scratch is the application's shared
// 00F8C2C8 binding, with sufficient writable capacity; no bound/reentrancy policy
// is added. Returns false without modifying output at absent data/EOF. Signed
// bytes <=20h trimmed at front; interior bytes <20h discarded; final cursor
// passes LF or extent. Constructs a temporary, assigns output, releases temp.
bool read_native_text_buffer_line_00af5740(void* actual_buffer,
    void* actual_output, NativeStringStorage& storage, char* scratch);

} // namespace bsp
