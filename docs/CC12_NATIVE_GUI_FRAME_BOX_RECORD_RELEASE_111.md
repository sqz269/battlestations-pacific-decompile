# Native FrameBox record release 111

Whole AD0860 `[AD0860,AD08DE)` (126 bytes, 42 starts) is reconstructed over
existing genuine raw services in `native_gui_frame_box_records.hpp/.cpp`.
The new explicit Source API is
`release_native_gui_frame_box_record_00ad0860(record, actual_owners,
actual_strings, operation)`. Original ABI is ECX actual2Ch record, no stacked
arguments, RET; this four-argument C++ API is not a binary replacement.
The existing Ghidra name `BSP_GuiTextureRef_Release_00ad0860` is preserved as
an earlier descriptive hypothesis.

## Domain and ownership

The only established zero terminal family is 110's completed canonical
registered2D creator, current profile D61948 with actual slot00 BD30E0 and
slot04 B3F590, the SAME texture context and actual pool. Caller supplies the
SAME renderer canonical registry through `NativeRenderActualOwners&`, plus
SAME `NativeStringRawPoolContext`/publication/gate/raw-manager cells. This is
an explicit caller provenance precondition, not a runtime claim that arbitrary
companions, profiles or virtual adapters are safe. The nonzero decrement result
performs no companion lookup. No new owner, fake terminal or default service
is supplied.

Caller prepares the full graph and this SAME persistent operation outside the
failing scope BEFORE entry. Record/resource/count/name storage, the canonical
registry/companion, renderer/texture/pool and VFS/string/publication/manager
services remain live through their applicable actions. Metadata/bindings stay
stable, disjoint from raw payload and provider scratch, with external lifetime
synchronization. Registry locking and an atomic count do not establish that
whole graph lifetime. Do not discover failed ownership from container links.

`NativeGuiFrameBoxRecordReleaseOperation` is noncopyable/nonmovable Source
scheduling metadata with read-only diagnostics. Fresh admission precedes raw
reads; entered/returned/failed are sticky and there is no reset, retry or
rollback API. Destroying entered/failed metadata terminates without inspecting
the graph; fresh/returned destruction reads metadata only. This is an explicit
**Source orchestration rule**, not Original object ownership or destructor
behavior. The operation owns no graph and cannot repair or count-gate one.
Captured pointers may be freed/reused diagnostic identities after a terminal;
neither metadata nor a stale record pointer is a healthy retry capability.
A different fresh operation must not replay the same released/failed lifetime.

Phase flags describe Source scheduling, not a Native instruction or exception
origin. `zero_dispatch_entered` includes canonical registry lookup;
`normal_name_return_entered` includes getter plus return, so the getter may
fail before BD1510. Consumers must interpret later completion flags separately.

## Body and failure order

The actual resource DWORD is captured **before** the persistent name action is
armed. Only in the captured-nonnull branch does the body perform a single real
Win32 InterlockedDecrement on actual resource+04. Observed zero invokes existing
`dispatch_native_render_actual_owner_zero`, which does not decrement again.
After that service returns, record+0 is cleared; the captured-null branch does
not clear it. No resource payload is read after a successful terminal.

The body next captures current name DATA at record+8, consumes the name action,
and only for nonnull data captures CURRENT record+4 length plus1 (DWORD wrap).
It uses genuine419CC0 through the supplied actual publication/raw-manager cells,
then genuineBD1510 with the captured data/size and actual live gate. The header
is left unchanged. No speculative cleanup or extra length read is added.

```text
00ad08c1 CALL 00419cc0
00ad08c8 CALL 00bd1510
```

Original AD0890 uses import cell CE2220 for the count service. AD08A0 is current
resource table slot00, with no stacked flags. The Source zero service resolves
and validates the same actual+04 companion; for the proven2D family its current
terminal is the real B3F590 path, whose internal flags1 belongs to the inherited
BD30E0 terminal route. It is not a direct flags1 substitution at AD0860.

A C++ resource-service escape marks the operation failed and consumes the name
action **before** genuine41DD20 on the live record+4 header. That cleanup failure
terminates; it never retries a potentially entered getter/return. After a normal
name getter escape the action is already consumed, so the catch does not invoke
41DD20 again. A failed resource service may leave record+0 referring to an
already-zero owner; a successful terminal can free/reuse it before the record
clear. No count rollback, slot return, resource retry or implicit graph cleanup
is invented. Failure remains externally retained even if name cleanup succeeds.

The native EH action is reused from 98: CB90BB -> DF043C, maxState1,
DF0434 state0 -> CB90B0 -> live record+4 destruction41DD20. This packet does not
claim private Native FH3/double-exception ABI equivalence. Existing canonical
Entry terminals are noexcept and terminate on terminal failure; those failures
cannot unwind into record name cleanup. Registry/identity errors can escape
before the Entry terminal. The explicit nested Source catch also terminates
on an escaping cleanup exception. `/EHsc` C++ exceptions, not arbitrary SEH or
faulting/dead-storage behavior, are the supported exception boundary.

## Verification and limits

Baseline: Root `d251a7b85c5c8ab6be9eed7ed7df55c58af1aa06` merged with checked
exit. The whole126B/42-start live body matches the installed Original and prior98
receipt, SHA-256 `5e96ab380ada5815df5ce8ce7611ba1f9695126612c335867cda66eb4a51a805`.
Each live/annotation/export batch verifies `C:/Users/sqz269/bsp.gpr`,
`/battlestationspacific.exe`. Original installation remains
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

Fresh MSVC Win32 `/MD /O2 /EHsc /fp:strict /W4 /WX` compilation passed. Full
optimized primary/catch and metadata-destructor slices were independently
reviewed by Root. The release COMDAT is323B including catches/padding; metadata
destructor17B. Sole LOCK XADD+DEC is the Win32 Source service, not Original IAT
or private-ABI identity. Important compiler limit: internal C++ EH state is
stored at +40 **before** actual resource capture+47, while the explicit
persistent name action is armed only at +4C. The claimed capture-before-action
schedule concerns that explicit metadata; it is not exact Native FH3/SEH state
placement. No C++ throwing operation occurs in that capture/arm gap.

Other inspected offsets: zero-only service+6E, nonnull-only clear+7A,
name DATA+84, consumed metadata+8A, current length+94, getter+A7/return+B8;
failure action consumed+112 before41DD20+121. Metadata destructor reads only
phase and terminates for entered/failed. Both complete103 file byte prefixes
and its full163B optimized copy code remain unchanged.

Required `./scripts/build.ps1` passed; existing3/3 checks passed in8.51s.
Those checks do not execute this new release path. No new test, callback
fixture, probe, relocated Original comparison or runtime/gameplay was run.
No tidy, vector, factory, rebuild, GUI activation, graph retirement, Native CRT
or private exception/ABI completion is claimed. No CMake/Main changes were made.
Only AD0860 is annotated under the write lock with old-value history/save,
preserving name/comments and refreshing exports; its unique raw reconstruction
record is appended without replacing older records.

The companion report and
`local/cc12_native_gui_frame_box_record_release_111/manifest.json` freeze final
Source, strict object/assembly, existing-check log, bytes/history/refreshed
exports and build libraries. No build or artifact replacement follows sealing
until Root has captured them.
