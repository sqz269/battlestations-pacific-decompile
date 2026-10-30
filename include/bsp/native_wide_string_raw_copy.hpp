#pragma once

namespace bsp {
struct NativeStringRawPoolContext;

// Actual raw8 004C8DD0 through SAME AA8/AA4/AA0 pool cells. Compare identity
// BEFORE clearing length then data, including self identity. Otherwise captured
// source length -> real raw resize(preserve1) -> CURRENT source length gate ->
// CURRENT source data THEN destination data -> forward WORD stream through NUL.
// All raw accesses are fenced byte copies; no typed object/backing cast, counted
// copy, adapter, allocation owner, Frame, validation, rollback or cleanup added.
// Caller retains valid reached headers/backing/pool/code and partial-failure
// custody, with legal alias lifetimes. Ordinary Source ABI, not Native thiscall,
// FH3/SEH/hardware/current-CRT identity, startup or runtime qualification.
void* copy_construct_native_wide_string_header_004c8dd0(void* actual_header,
    const void* actual_source, NativeStringRawPoolContext&);
} // namespace bsp
