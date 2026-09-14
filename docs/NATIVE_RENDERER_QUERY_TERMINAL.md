# Actual query terminal and renderer unregister

These providers close the actual D62AD0 query lifetime path used by the renderer
destructor. They operate on raw query fields and the current renderer's +19A0
array. The existing projected D3D9QueryRegistry and generic reference-counted
callback APIs remain unchanged; no retained operation or private owner is added.

| Entry | Full inclusive native range | ABI |
| --- | --- | --- |
| B25290 raw removal | B25290..B252F6, 103 bytes | ECX array, stack pointer-to-DWORD, RET4/full EAX0 or1 |
| B27CF0 unregister | B27CF0..B27D3D, 78 bytes | ECX renderer, stack query, RET4; EAX unstable across leave |
| B5FD40 base wrapper | B5FD40..B5FD4A, 11 bytes | ECX query, tail BD30F0 |
| B5FDA0 destructor | B5FDA0..B5FE13, 116 bytes | ECX query, RET |
| B5FE40 scalar | B5FE40..B5FE5D, 30 bytes | ECX query, stack flags, EAX original address, RET4 |
| BD30E0 concrete query adapter | BD30E0..BD30ED, 14 bytes | ECX query, RET; no flags/refcount decrement |

All 352 owned bytes, plus 234 bytes of reused bodies, profile data and FH3
metadata, were compared between the existing live bsp.gpr analysis and the
installed PE. B25290 is identical to the full 103-byte B25300 body without any
operand mask, so its distinct-address API reuses the substantive existing raw
provider. B5FD40's source is a naked profile stamp and tail to the existing exact
seven-byte BD30F0 provider; BD30F0 itself is not copied.

Removal captures data, count and wrapping end=data+count*4. Unsigned empty or
backward ranges return0 without reading the target cell. A nonempty search
captures the target once and finds its first match. The native signed wrapping
index preserves its -1 sentinel. A non-last match is overwritten with the last
captured cell; only then is CURRENT header count decremented. This ordering
matters when array data aliases the count field. Capacity and the stale final
cell remain, with no query or COM ownership operation.

B27CF0 enters the existing actual optional guard before forming/removing the
query argument on renderer+19A0. It consults current exit mode before leaving.
There is no native EH registration or RAII leave, so none is added. Isolated
native MOVs copy the ignored saved-result DWORD, including padding, to B33B00
without an indeterminate C++ scalar evaluation, matching the existing raw
physical-unregister provider's mechanism.
Entry-disabled/exit-enabled transitions that expose unwritten native guard state
remain outside the valid native domain; no guard initialization or repair occurs.

B5FDA0 stamps D62AD0, captures query+10 and arms base cleanup before any COM
dispatch. Nonnull captured COM uses its current vtable slot8 Release, ignores
the result, and clears CURRENT query+10 only after return. A provider's replacement
COM field is therefore cleared on return; it is retained if the provider throws.
No extra AddRef/Release, refcount decrement, state or samples write occurs.

After Release/clear, the destructor reloads the actual F8D394 publication and
the selected renderer's CURRENT profile. The supported immutable D5F0A8 view
contains B27CF0 at slot28. The source reads that slot and directly executes the
substantive raw unregister provider on the captured current renderer. The context
also borrows actual synchronization bytes0108D6DC..E3; it owns no global or list.
Renderer/profile publication is not captured before Release.

Normal completion disarms cleanup before base restoration. B5FD40 stamps
D62AB0 and tails BD30F0, which stamps CEB130 and touches nothing else. Native
FH3 handler CC1288 loads FuncInfo DF9FF0; its one unwind row at DF9FE8 sends
state0 to -1 through CC1280. That action loads captured owner[EBP-10] and tails
B5FD40. The source invokes the same concrete base wrapper on C++ unwind, without
retrying Release, unregister, or allocation release and without rolling back
provider publications or field writes.

B5FE40 calls the complete destructor, then frees the allocation iff flags bit0
is set, finally returning the original address even when freed. Native free
BF65AC and the previously used BF6989 belong to the same chain: BF6989 jumps to
BF65AC, which jumps to native CRT free BF9DC8. The source reuses the existing
shared lifetime free service. A destructor exception skips scalar free and retains
the allocation for the caller's exceptional ownership obligations.

The additional concrete BD30E0 API returns immediately for null. Otherwise it
reloads CURRENT query profile and reads actual D62AD0 slot4=B5FE40, invoking
flags1. It does not reread slot0 or decrement references: the renderer parent
selects the slot0 terminal after its own real InterlockedDecrement. The existing
generic BD30E0 callback API remains available for other users. Finite profile
views contain original numeric code words, never original-EXE call addresses.

The scalar listing omits ADD ESP,4 at B5FE55..B5FE57 after returning free call
B5FE50. Full disk/live bytes include that instruction and RET4 through B5FE5D.
Root owns this returning override repair, names, preserved old comments, save and
refreshed exports after release. The worker makes no Ghidra mutations.

Verification includes eight native seeds, strict Win32 build and both CTests,
native call-site audit, and one focused fixture using copied original entries,
actual storage and real Win32 optional guards. The fixture's relocated callable
profile tables are explicit native-test bindings; the source uses the finite
original-token views. Original FH3 handler literals are explicitly fail-fast
relocated for normal execution only. Source-only COM exceptions check retained
publications, skipped clears/unregister/free and base restoration; they do not
establish original FH3/SEH equivalence. Valid raw extents, shared allocation
ownership, profile domains and lifetime remain caller obligations.

The immutable archive separates source/library/tool/probe inputs from measured
loaded runtime DLLs and actual guard-provider modules. Canonical paths are measured
in the x86 process; later disk/archive hashes are not mapped-image hashes.
Neither these local fixtures nor the build establish game integration or gameplay.
