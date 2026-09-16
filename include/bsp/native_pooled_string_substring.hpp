#pragma once

#include <cstdint>

namespace bsp {

class NativeStringStorage;
struct NativeStringRawPoolContext;

// Full 00426060, originally ECX destination, stack source, EAX destination,
// RET 4. Both pointers address actual eight-byte length/data headers. Clears
// the destination even on identity, abandoning any old buffer. Allocation may
// change either header; the subsequent source guard and copy use current fields.
// There is no output cleanup if construction throws, and no null-header guard.
void* copy_construct_native_string_header_00426060(void* actual_destination_header,
    const void* actual_source_header, NativeStringStorage& storage);

// Full 00469840, originally ECX source, stack output/start/count, EAX output,
// RET 0Ch. Start has native signed interpretation; additions and addresses use
// Win32 unsigned wrap. Captures source data before temporary allocation, performs
// strncpy (including padding), then copy-constructs the actual output header.
// On failure the current temporary header is destroyed. Normal temporary return
// uses the earlier data capture and current length. No existing output release.
// NativeStringStorage supplies the already-bound pool; its noexcept release and
// omitted zero-byte memcpy remain host boundaries, not the native ABI.
// Evidence and exception-state limits: docs/NATIVE_POOLED_STRING_SUBSTRING.md.
void* construct_native_string_substring_00469840(const void* actual_source_header,
    void* actual_output_header, std::uint32_t start, std::uint32_t count,
    NativeStringStorage& storage);


// Genuine raw pool variants: no noexcept storage adapter. Actual output and
// temporary ownership states are retained, with true-unwind second-failure
// termination. Raw byte copying retains native overlap behavior.
void* copy_construct_native_string_header_00426060(void*, const void*, NativeStringRawPoolContext&);
void* construct_native_string_substring_00469840(const void*, void*, std::uint32_t,
    std::uint32_t, NativeStringRawPoolContext&);

} // namespace bsp
