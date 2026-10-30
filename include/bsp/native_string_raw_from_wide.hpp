#pragma once

namespace bsp {
struct NativeStringRawPoolContext;

// Whole00436630 normal constructor over actual raw8 and raw WORD-source bytes.
// Clear LENGTH/DATA before full-WORD scan; wrapping byte difference uses SAR1.
// Real RAW41DD40, then CURRENT destination DATA once and BYTE-only low-byte
// streaming through low-byte0. Retain full resize length and untouched tail.
// No wchar/semantic projection, input guard, old-buffer release, local cleanup
// or Frame. Caller retains backing/cells/providers and partial failure debt.
void* construct_narrow_from_wide_00436630(void* actual_header8,
    const void* actual_wide_source, NativeStringRawPoolContext&);
} // namespace bsp
