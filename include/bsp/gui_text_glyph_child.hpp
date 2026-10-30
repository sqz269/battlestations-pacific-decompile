#pragma once
#include "bsp/native_string_pool_storage.hpp"
#include <array>
#include <cstdint>

namespace bsp {
class GuiTextLifetime;
struct FontGlyphData;

// Original caller values for the ten DWORD stack arguments to00AB98F0, in
// order, plus its SAME canonical ECX owner. Borrowed evidence/continuation
// state only: constructing this record neither repeats the mapped writer nor
// creates a child. At needs_glyph_child the first missing instruction remains
// AB9D33, immediately before native Text pool allocation00AB78A0.
// The caller retains its stable float3 allocation and both mapped streams.
struct GuiTextGlyphChildCallFrame {
    GuiTextLifetime& parent_ecx;
    const FontGlyphData* glyph_1;
    const std::array<float, 3>* position_2;
    void* vertex_stream_3;
    std::uint16_t* indices_4;
    const void* unused_5;
    const void* unused_6;
    std::uint32_t quad_index_7;
    std::uint32_t height_word_8; // Only low16 is consumed before the child tail.
    std::uint32_t first_vertex_9;
    std::uint32_t code_unit_word_10; // Only low16 is consumed.

    // Both known callers pass position_2->data() for slots2/5/6. The FULL
    // callee never reads slots5/6: these are not extra output pointers.
    // Actual arg2 is read for initial x/y and reread for x atAB9F5E/9F62.
    static constexpr std::uint32_t pending_native_address = 0x00ab9d33u;
};

// Complete00AB81C0 ordinary valid-storage body. Native ECX points to the actual
// eight-byte UTF16 header {DWORD code-unit length, DWORD buffer}; one code-unit
// DWORD stack argument, EAX same header, RET4. Uses the existing ACTUAL string
// pool bridge, not a new allocator or a NativeString byte-length reinterpretation.
// Writes zero header first (constructor, not assignment), allocates4, preserves
// the native current-header copy/release order across pool callbacks, publishes
// length1/data, writes UTF16 terminator then low16 code unit. No implicit owner,
// destructor, nullable-buffer fallback, native SEH or Text-child completion.
// Caller later releases a nonnull buffer with current length*2+2 through the
// SAME actual pool bridge, as AB9E8A/AB9E91 does.
void* construct_gui_text_code_unit_00ab81c0(void* actual_utf16_header,
    std::uint32_t code_unit_word, ActualNativeStringPoolStorage&);

// SAME complete body through the caller's actual AA8/AA4/AA0 raw pool cells.
// Resolve00419CC0 before allocating4 and again before EVERY nonnull return,
// including large/disabled returns. The returning getter may throw: this
// overload does not inherit NativeStringStorage::release's noexcept boundary.
// Zero length then data BEFORE the first getter; after allocation reread the
// CURRENT unsigned length/data. Copy one WORD for length>=1; length0 still
// reads CURRENT data but omits native memcpy(...,0), as other raw wide helpers.
// Capture CURRENT old data then wrapping length*2+2 BEFORE the returning getter.
// Capture the by-value argument's lowWORD only AFTER return, then publish the
// captured allocation, length1, terminator, and reload CURRENT published data
// for the final code-unit store. Zero and all upper-bit patterns are admitted.
// Caller retains valid ordinary8B header/backing, actual pool/manager cells and
// their full lifetime graph through normal return or escape. No typed owner,
// new pool, local EH/cleanup, retry, rollback or exception-origin inference.
// This ordinary C++ interface does not expose the original Native stack home;
// inherited raw-pool/CRT/FH3/fault/secondary-exception and ABI limits remain.
void* construct_gui_text_code_unit_00ab81c0(void* actual_utf16_header,
    std::uint32_t code_unit_word, NativeStringRawPoolContext&);
} // namespace bsp
