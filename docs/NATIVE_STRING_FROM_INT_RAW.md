# Raw integer-to-string construction

`construct_native_string_from_int_004260b0` is dormant Core Source for the
game-owned 004260B0 body (Original ECX output8, stack signed32, EAX output8,
RET4). The ordinary C++ interface is not a binary replacement. The prior
main-menu routine and every existing caller remain unchanged.

The immutable context borrows the SAME `NativeStringRawPoolContext` and the
actual mapped CE3A34 three-byte `%d\0` span. A Game caller may obtain that span
from its real read-only-data service; Core introduces no Game dependency,
formatter callback or private format cell. The shape/content check precedes
effects, but matching bytes do not prove Original-data provenance. The caller
must prove the actual mapping, extent, lifetime and disjoint backing.

The output length/data are zeroed before current CRT formatting into a 52-byte
buffer. Original calls BF7A6A `sprintf`; this Source uses current SDK `snprintf`
with the actual span, ignores its result, and admits only signed32 `%d` (at most
11 characters plus NUL). Arbitrary formats and CRT error/locale equivalence
are unsupported. No CRT/vendor body is reconstructed or credited.

Only after formatting are the private aligned raw8 temporary fields zeroed.
The real raw 41DD40 resizes it, then data is captured before length and the
buffer is copied. This interval has no selected local cleanup. After that copy
the self comparison precedes arming state0. Output resize uses the captured
length; its copy reads CURRENT output length then data and uses captured source
data. Header fields use fenced byte copies, not typed primitive lvalues.

Normal return disarms state-1 before any captured-data return. For nonnull data,
captured length+1 wraps as DWORD before actual 419CC0, then BD1510 receives the
captured data/size and the live AA4 cell. Getter failure is not retried. Selected
C++ unwind consumes state0 to -1 before actual 41DD20 on the CURRENT private
temporary header; a secondary cleanup failure terminates. Output is never locally cleaned up.
There is no temporary-header read after normal pool return.

The selected map is independently evidenced: C5E818 selects D84D24
(19930522, maxState1, map D84D1C); state0 maps to previous -1/action C5E810,
which addresses EBP-48 then jumps to 41DD20. This is not inferred from the old
typed menu catch. Saved assembly has a three-byte alignment gap; FH3 frames,
private spills, hardware/SEH faults and Original ABI identity remain unproved.

The caller supplies live raw output8, real three cells/provider graph and a
noninvalidating, synchronous domain. Output/context/format/provider backing
must be disjoint; no callback may retain/invalidate the private buffer/temp or
concurrently mutate them. Overlap-safe copies retain the established omission
of zero-byte copies after required loads. Lower raw resize/destroy/getter/pool
contracts remain prerequisites. Failure retains output/provider debt; there is
no rollback, retry or invented owner. No caller, parser/loading, consumer,
runtime or startup authority follows from this provider; `source_ready=false`.
