# Raw locale table-name registration

`register_native_locale_table_name_00aa0d30` is a qualified ordinary C++
bridge for the complete two-instruction Original routing body:
`00AA0D30 ADD ECX,4; 00AA0D33 JMP 00450540`.
The descriptive Source name is a hypothesis, not a recovered symbol.

The bridge performs wrapping Win32 `manager+4` address arithmetic and directly
calls the existing `append_native_string_header_vector_00450540`. Its zero EDX
argument belongs only to that Source helper's explicitly unused EDX parameter.
The original reaches the same checked string-vector append with its incoming
EDX unconsumed and its original stack argument intact. This new API adds the
explicit `NativeStringRawPoolContext&`; it claims no Original tail-stack,
calling-convention, EAX, native fault or FH3/SEH equivalence.

The caller owns a live actual manager/table-name vector (header at manager+4,
begin/end/capacity at +8/+C/+10), the borrowed actual eight-byte name header and
reached data, and the SAME retained raw string-pool context. Keep those inputs
and their providers valid through the callee's operations and failures. No
semantic `LocaleTables`, `NativeString` view or local header is substituted.

The existing checked append retains its actual capture/read/copy/store and
allocation order, deep-copy and alias behavior, normal-return obligations and
failure policy. Its closure includes the existing raw uninitialized-fill and
insert-one paths, actual pool services and the qualified potentially returning
`invoke_native_invalid_parameter_00bf6713` import. Current Source pool/CRT
behavior does not prove Original allocator, invalid-parameter handler or vendor
library equivalence. This bridge adds no validation, owner, provider, getter,
publication, state, cleanup or exception interception.

Native startup calls this registration at `0073E103` after language selection
(`0073E08F`) and before non-forced reload (`0073E135`). Registration alone says
nothing about loaded raw map/sidecar vectors, canonical F8BC4C publication,
uppercase readiness or Text admission. No startup/caller is added here.

Evidence and validation are recorded in
`reports/native_locale_table_name.json`: saved two-instruction assembly,
existing callee Source, and one isolated MSVC Win32 object compilation. Root
owns build-list/ledger integration and any later coupled checks. No new test,
probe executable, game run, Ghidra mutation, native ABI or runtime claim is
part of this packet.
