#pragma once

namespace bsp {
struct NativeStringRawPoolContext;

// Whole004C5E20 normal assignment over actual raw8 headers and SAME raw cells.
// Identity returns before any header/pool read. Otherwise captured source
// LENGTH -> genuine RAW4C53E0(preserve1) -> CURRENT source LENGTH gate ->
// CURRENT source DATA FIRST, then destination DATA -> streamed WORDs including
// NUL. No clear, count bound, input guard, copy-constructor or local cleanup.
// Caller retains all reached backing/provider lifetimes and partial debt.
// Ordinary MSVC Win32 interface, not Original registers/RET4/fault/stack ABI.
void* assign_native_wide_string_header_004c5e20(void* actual_header8,
    const void* actual_source8, NativeStringRawPoolContext&);
} // namespace bsp
