#pragma once

namespace bsp {
struct NativeStringRawPoolContext;

// Original005CF090: ECX destination raw8, stack SOURCE RAW8 header, RET4.
// Capture source length first; zero reads no destination/native pool state.
// Nonzero captures old destination length, performs real raw resize(sum,true),
// then reads CURRENT source DATA BEFORE destination DATA and copies captured
// sourceLen*2 bytes at captured oldDstLen*2. All arithmetic is Win32 DWORD wrap.
// No NUL copy, data-NULL guard, self-return, temporary owner or local cleanup.
// Caller proves reached headers/copy/alias/provider lifetimes. Source backing
// must survive real resize unless its actual header is updated by that resize.
// Escapes propagate with partial native ownership retained; no rollback/retry.
// Ordinary void Source boundary, not Original register/FH3/SEH/runtime proof.
void append_native_wide_string_header_005cf090(void* actual_destination,
    const void* actual_source_header8, NativeStringRawPoolContext& actual_strings);
} // namespace bsp
