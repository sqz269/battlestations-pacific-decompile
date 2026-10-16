# Actual-header wide assignment 139

`assign_native_wide_string_header_004c5e20(void* dst8, const void* src8,
NativeStringStorage& storage)` now provides direct actual-header assignment using
the genuine existing raw 4C53E0 resize and SAME retained storage. It returns the
same destination identity. The old typed API/algorithm remains unchanged; this
adds a raw input/provider capability, not a new algorithm recovery, callback
facade, string owner or native Text implementation.

The worker retained baseline `0ca0434edf4b30a7ef01d1b7cad150cd01baff88` without
merge. Only the two existing wide-string files, this document/report and owned
4C5E20 address-shard rows changed. The TU was already registered to `bsp_core`
at `cmake/startup.cmake:1775`; no CMake or host activation change was needed.

## Exact behavior and domain

Original `[004C5E20,004C5E59)` is 57 bytes / 24 instruction starts, SHA256
`6741d4cc2dc99f68b8f6c489424b29e05eabf1f2167e586b5c903929eaf024b1`.
Native ECX is destination, one stack pointer is source, EAX returns the SAME
destination and RET4 starts at 4C5E56. ESI/EDI are preserved. There is no local
FH3 frame, acquired owner, cleanup or rollback.

The new Source entry follows the complete native schedule:

1. Compare pointer identity before **any** header read. Self assignment returns
   immediately without initialization, resize or release (4C5E28).
2. Otherwise capture source length, then directly call real
   `resize_native_wide_string_header_004c53e0` with preserve 1 and the SAME supplied
   storage (4C5E2C/31). No generic resize callback or fallback is used.
3. Reread CURRENT source length after resize returns. Zero skips both data
   pointer reads (4C5E36).
4. Nonzero captures CURRENT source data, then CURRENT destination data
   (4C5E3B/3E).
5. Load one unsigned WORD, store it, advance both cursors by two, test the
   captured unit and repeat through and including NUL (4C5E41..50). Neither the
   captured length nor a host memcpy bounds this stream.
6. Return the original destination identity.

Headers must be actual initialized DWORD-aligned 8B `{length DWORD, data DWORD}`
representations in the existing MSVC Win32 raw-view/alias domain. Reached backing
must remain valid through NUL under the native valid overlap/alias contract.
Volatile DWORD/WORD accesses make the required current reads and memory operand
order explicit. They are not an atomicity or concurrent mutation guarantee.
The implementation does not reinterpret inputs as `NativeWideString`,
`std::u16string` or an unrelated Text owner. The existing typed API stays separate
and byte-identical; matching physical shape alone does not prove backing provenance.

Production borrows the existing `GameNativeStringProcess::strings()` actual
storage and keeps that SAME owner, headers and reached backing alive through
callbacks and failure. A caller-supplied reference does not validate foreign
heap/pool provenance or create a new process owner. No private global, manager,
counter, header default, temporary copy, callback shim or automatic cleanup is
introduced.

The unchanged resize defines equal/zero/nonzero behavior, DWORD wrapping size
arithmetic, allocation/preservation, post-callback current-header reloads, old
backing release and terminator publication. This entry adds no bounds check,
overflow/allocation repair, local catch, rollback or retry. Allocation exceptions
propagate through that existing Source domain; `NativeStringStorage::release`
remains noexcept. Invalid/retired backing and destructive provider reentry are
outside the admitted graph, not repaired by rediscovery or a default result.

## Static and build evidence

Both complete old Source files remain literal byte prefixes: the new declaration
and body are appended in reopened namespaces. Before editing, the prior canonical
wide object, Core archive, game and two existing check executables/log were copied,
along with shared exports and the old typed/provider inputs. A fresh old strict
object was compiled before the append.

Before/after strict objects contain the same nine old emitted bodies: five
public/external functions and four internal helpers. Every complete body byte and
every relocation offset/type/target symbol matches exactly. The only new emitted
function is this assignment, 74 bytes, with one genuine resize REL32 relocation
at +19h. The existing resize and raw copy/construct/destroy bodies were not altered.

Root reviewed the full appended header/CPP and `strict.asm:506-587`. Emitted
offsets are self CMP/JE +0A/+0C before first header read +0E; resize CALL +18;
current length +20; source data +25 before destination +28; WORD load +30,
store +3C, test +40; same-destination result +45. The compiler schedules pointer
LEAs before the store using `[updated_output-2]`, preserving the original output
address and load-before-store effects. It also uses the new Source argument/RET
ABI and loop padding. Native register placement, exact instruction/fault timing,
private CRT/FH3/SEH or binary compatibility is not claimed.

Strict MSVC Win32 `/MD /EHsc /std:c++17 /O2 /Gy /W4 /WX /fp:strict /permissive-`
compilation passed. One canonical `./scripts/build.ps1` run compiled the registered
TU and passed all three existing checks in 7.77 seconds. No new behavioral test,
probe, Native differential or execution of this new entry was performed. The
complete Original/listing/call proof is reused from captured 136 and checked
against unchanged current Original; fresh post-annotation bytes are also retained.
All live batches verify `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`.

Only 4C5E20 was annotated through the write-locked tool with old history retained,
project saved and owned exports refreshed. `BSP_WideString_Assign` is preserved.
The old typed reconstruction/name evidence remains; one distinct raw-provider
record is appended and only owned name evidence is extended. No unrelated body,
prototype, local, label, library name or shard row is changed.

This closes the small actual-header assignment prerequisite only. Raw
ABA8D0/ABB1D0, cursor, whole Text copy and genuine Font/Text/current-dispatch/layout
composition remain the separate boundaries recorded in 136. There is no new
binder, production capability, normal native Text activation or game-validation
claim. Final Source, before/after objects, operands, Original proof, logs and
annotation history are sealed under
`local/cc12_native_wide_string_raw_assign_139/artifact_manifest.json` and held for
Root capture before reuse.
