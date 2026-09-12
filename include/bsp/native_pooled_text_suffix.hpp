#pragma once

#include "bsp/native_pooled_text.hpp"

namespace bsp {

// Complete AF4450. ECX actual4h header, stack text, RET4; EAX copied count.
// Accept TAB and signed ASCII 20h..7Eh. Null/empty/rejected prefix leaves the
// header alone. Scan, release old bytes, allocate count+1, publish, strncpy,
// then reload the header for the terminator. The genuine CRT strncpy boundary
// requires nonoverlapping byte ranges; header alias/publication order is kept.
// Uses the application's SAME NativeStringStorage as native_pooled_text.hpp.
std::uint32_t assign_native_pooled_text_suffix_00af4450(void* actual_header,
    const char* text, NativeStringStorage& storage);

// Complete AF44C0. ECX actual4h line, stack output/index, RET8; EAX output.
// CONSTRUCTS output through a temporary and AEE2E0, with no old-output release.
// Search accepts signed 20h..7Eh and tests the index BEFORE skipping spaces:
// index0 preserves leading spaces; index1 skips the first run of spaces.
// Once selected, AF4450 also copies tabs and the remaining printable suffix.
// Missing/negative indices or a nonprintable first byte construct null.
void* get_native_pooled_text_suffix_00af44c0(const void* actual_line,
    void* actual_output, std::int32_t index, NativeStringStorage& storage);

} // namespace bsp
