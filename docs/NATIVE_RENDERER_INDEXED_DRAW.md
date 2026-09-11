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

The strict MSVC Win32 main build and both existing CTests passed.
Original-body fixture validation is pending. This
does not claim complete stream binding/ownership, original caller ABI or
gameplay. No permanent tests were added.
