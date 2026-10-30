#pragma once

namespace bsp {
struct NativeStringRawPoolContext;

// Whole004C5E60 normal constructor over actual raw8 storage. Clear length/data
// before the source BYTE scan; real raw4C53E0 then CURRENT destination data;
// stream unsigned BYTE -> WORD through NUL, preserving wrapping/overlap epochs.
// No input guard, old-buffer release, local cleanup, rollback or output Frame.
// Caller retains actual source/output/cells/provider extents and failure debt.
// Ordinary MSVC Win32 Source interface, not Original thiscall/FH3/fault ABI.
void* construct_native_wide_string_header_004c5e60(void* actual_header8,
    const char* actual_source, NativeStringRawPoolContext&);
} // namespace bsp
