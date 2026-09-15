#pragma once

namespace bsp {
struct NativeStringRawPoolContext;

// Complete B1BF50..B1BF67 (24 bytes). ECX is actual command storage, the
// public stack word points to three DWORDs, RET4. The unused source EDX slot
// keeps that original stack argument. Each source read follows the preceding
// command+1C/+20 store, preserving overlap; EAX ends with the third word and
// EDX with the second. No command construction, retention or validation.
void __fastcall set_native_render_command_metadata_00b1bf50(
    void* actual_command, void* unused_source_edx,
    const void* actual_three_word_source) noexcept;

// Complete B1D910..B1D945 (54 bytes), with an explicit source pool binding.
// Native ECX=command, stack=actual8h source header, RET4; source ABI differs.
// Destination is the existing length/data header at command+14. Exact header
// identity returns before any header/provider read. Otherwise resize from the
// captured source length, preserve1, then re-read source length. If nonzero,
// capture current destination length, source data, destination data IN ORDER;
// overlap-capable copy uses destination length without an added terminator.
// A zero-count library call is omitted only AFTER all three argument captures.
// Actual command/header/buffer backing and the same01090AA8/AA4/AA0 pool cells
// must remain valid. No owner, pool, rollback or cleanup is introduced here.
// Raw pool getter failures propagate with reached stores intact. This is not
// native FH3/SEH, fault, concurrent-memory or game validation.
void set_native_render_command_diagnostic_00b1d910(void* actual_command,
    const void* actual_source_header, NativeStringRawPoolContext& actual_pool);
} // namespace bsp
