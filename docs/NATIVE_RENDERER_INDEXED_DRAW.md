# Native renderer indexed drawing

This packet implements full B24010 (331 bytes), viewport size-address leaf
B1F740 (4 bytes) and logical vertex-count leaf B48CD0 (4 bytes). It borrows
the stream without creating, retaining or destroying it, so its source does
not depend on the unresolved logical base retained-owner lifetime.

The original parent takes ECX renderer and five stack DWORDs: primitive
type, minimum vertex, vertex count, start index and primitive count, RET14h.
The C++ entry adds explicit synchronization/profile context. The logical
profile is the actual D61D6C table; current virtual+20 resolves to full
B48CD0, which reads DWORD+64. Foreign profiles are outside this concrete
contract. Descriptive names are hypotheses, not original symbols.

Renderer DWORD+1D90 inhibits first; byte+1D8A is read only if not inhibited.
Either gate returns before viewport access or guard entry. The subsequent
renderer+1904 read feeds B1F740. That leaf computes viewport+10 and does not
read viewport dimensions; its unused result must not create new pointee
accesses or cause removal of the original renderer field read.

The parent captures mode before optional guard entry and reloads it after
entry returns. Native B24072 reads the uninitialized saved-renderer word if
entry is skipped. The vertex-count TEST precedes cleanup arming at B2407D.
Zero vertices or zero primitives use the captured mode for normal leaving.
With both counts nonzero, a current nonnull stream0 is queried even when
its tag is ordinary. If requested vertices exceed the returned unsigned
count, renderer+1774 is reloaded and that current stream's tag+54 is checked.
Only tag40000001 rejects the draw. There is no replacement null check on
this second stream read.

The drawing path captures current device+1A10, then its current table,
then current base vertex+17BC and table+148. All requested DWORDs pass to
DrawIndexedPrimitive unchanged; base vertex keeps its signed 32-bit bits.
HRESULT is ignored and no statistics counter is incremented. The special
stream rejection and post-draw exit reload current mode before disarming
cleanup. No native binding cache or draw arguments are repaired.

Native CBCF78 uses FuncInfo DF55B8, map DF55B0 and guard funclet CBCF70
(EBP-14 to full B21110). Existing full enter/leave/unwind providers preserve
current lock/mode behavior. Entry exceptions occur before cleanup arming;
exceptions after arming use that original cleanup; normal leave exceptions
do not cause a second leave. Skipped uninitialized records are not repaired.

The strict MSVC Win32 main build and both existing CTests passed. The primary
independently replayed all three original bodies against the frozen actual
main library. All 16 pairs passed, including real HAL drawing and pixel
readback. Complete stream binding/ownership, original caller ABI and gameplay
remain separate. No permanent tests were added.


## Primary validation

The primary verified 75 immutable worker pins and seven unchanged current
provider source/header files. Thirteen fresh guarded live/installed-PE spans
cover 604 bytes; eight native seeds matched. The unchanged fixture linked
only frozen actual main library
`9a9de67d355ddadcfdd957b89d4479c0f057ffc159f327adf2e502ad31d663b9`.
Two exact archive objects and 317 complete COFF sections match all linked
bytes and relocations, including fixture code: 121 entries and 15,424 unique
code bytes. Thirty-three normal postimage stages plus two per termination
child preserve complete original/linked code, read-only data and actual tables.

All 249,641 compared DWORDs match literally across 125 snapshots: complete
renderer and logical storage, actual synchronization globals and critical
sections, real VB/IB binding getters, and sampled pixel readback. Cases cover
the inhibit/lost gates, poisoned stream behind zero counts, unsigned counts,
ordinary versus special tags, entry-time mode change, current device/table/
stream/base/lock mutation, and armed/unarmed cleanup exceptions. The changed
device case draws the actual triangle with base -1, minimum 1 and start 3;
the red pixel appears on device 1 while device 0 remains black.

All three complete originals execute at their original addresses in an
isolated reserved child, with the full original guards and FH3 map. The
D61D6C profile and its four-byte count getter remain byte-identical. There
is no getter callback or source-provider bridge. The 68 provider call proofs
resolve to actual HAL D3D9 and Windows critical-section implementations.
Both secondary-cleanup exceptions terminate with marker 73 after real Leave.

The actual absent-index draw returned S_OK without changing its black target.
Ignored HRESULT behavior is therefore established by complete code inspection,
without a negative-HRESULT runtime claim. A disabled-entry mode was never
changed to enable use of uninitialized guard fields. Pixel readback proves
a fixture draw effect; it does not establish game rendering or render parity.

The existing Ghidra names and comments were preserved, reviewed evidence
was appended and saved, and all three exports and complete reconstruction
records were refreshed.
