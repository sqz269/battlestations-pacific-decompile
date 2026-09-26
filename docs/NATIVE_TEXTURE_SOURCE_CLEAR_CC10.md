# Restricted texture-source child reset at C303B0

`clear_native_texture_source_children_00c303b0` reconstructs the complete
C303B0..C30403 normal schedule as a new C++ source interface. The original84B/
36instructions take the actual receiver in ECX, no stack arguments, and plain
RET; no meaningful returned value is established. This packet does not claim
binary ABI, original private FH3/SEH/hardware-fault, or runtime equivalence.
C302F0 already has source/credit and is unchanged; C30360 is a separate80B
descriptor body ending at C303AF. Root owns future function creation/84B credit.

The native D64478/D644B4/D79B54 profiles share C303B0 at+28 and C302F0 at+2C.
Reset starts a DWORD index0 and compares it to the freshly loaded count with
unsigned branches. Each iteration freshly loads the array, current child and
captures that same slot. Null skips decrement, dispatch and clearing. Nonnull
children decrement their actual+4 once and dispatch CURRENT slot0 only at
zero, with the captured child receiver and no stack flags argument. After
normal return, clear the captured slot; increment with DWORD wrapping and
reload count. Finally call typed737390(header,0), then write actual+1C=0 only
after it returns. No per-child count decrement, direct leaf array free,
selected/rate reset, base/profile store or owner count operation was added.
Data/capacity stay stale on the ordinary nonnegative-capacity resize-to-zero
path. Existing typed737390 can reserve(header,0) when current signed capacity
is negative, retaining its allocation/copy/free/publication behavior. No new
capacity guard or normalization is introduced.

The caller must supply the separate live NativeTextureSourcePayload genuinely
placed at actual+8 by C30470, its already-live nested header, and reached live
void* slots from the typed reserve/resize/append path. Launder cannot adopt a
raw image or begin a lifetime. Keep payload/header alive through the final
flag store and each captured old slot alive through terminal return/clear,
even if a reentrant callback changes current header data/count. Subsequent
iterations reload both. No count, payload, header or slot lifetime starts here.

Every nonnull child separately requires caller-proven live std::atomic<int32_t>
at actual+4, a valid owned credit, and the SAME canonical companion/registry.
Address equality, positive count bits, canonical binding and supported profile
checks do not establish the atomic lifetime. Fresh B319B0/current named2D
B3F930->B34230->B34120 DWORD1 byte stores, unnamed factory/base-constructor
paths, raw B3F2B0 slab storage and Entry's reference borrow are not closed by
this packet. Their constructor lifetime audit/fix remains separate. This leaf
constructs/resets no count and adds no retain, fallback or child-factory repair.

Existing release_native_render_actual_owner decrements the same+4 before
zero-only companion resolution. NativeTextureLoadOwners supplies supported
current D61948/D61870/D618B0 zero-profile correspondence: CURRENT slot0
BD30E0 reloads the profile and forwards CURRENT scalar slot04(flags1), using
real B3F590/B3F410/B3F430 terminals and retained renderer/pool/serial/registry
contexts. C303B0 itself calls slot0 without flags. No arbitrary virtual callback
or slot04 substitution is admitted. Existing zero-companion callbacks are
noexcept and terminate on terminal failure; this is their provider contract,
not native exception equivalence or recoverable failed-return behavior.

The one-shot host diagnostic records the reached child, array, captured slot,
index and returned steps. A source zero-resolution exception can leave count0
with the slot uncleared; resize failure leaves initialized uncleared. Catch
only marks failed and rethrows; it frees/releases nothing and supplies no
rollback, retry, resume or normalization. Retain failed diagnostics and backing
until caller reconciliation and metadata-only acknowledgement. Reusing a
nonfresh operation throws before engine accesses. Unacknowledged failed/running
diagnostic destruction terminates. Unknown nonlocal escapes and hardware faults
have no new catch guarantee. Native unsigned negative-count bits remain intact.

C304A0's reverse loop, current count decrement, current array free and base
destruction remain separate; its null-child restriction was not changed.
Neither appended nulls nor any historical fixture acquire destructor/runtime
credit. No Lua, platform5030/MSG, source0/W, sampler or application activation
occurred, and no test or runtime fixture was added.

One strict MSVC Win32 Release build with /WX and /fp:strict passed all three
existing CTests. The new exact static-library member has16 complete physical
code sections,785 declared unique bytes, including compiler/support/padding.
The whole normal/catch/invalid-operation section is299B, with catch entry248
and normal RET247; separate EH29B gives328 unique related bytes, not an
entry-only slice or a sum of overlapping symbols. Complete section bytes,
symbols and relocations are in compiled_sections.json/compiled_clear.json;
compiled_schedule.json checks the full field/control/call schedule. Emitted
unsigned count80/JAE83, fresh data92/child95, captured slot110, null branch128
to177, genuine release145, captured clear167, increment177/back178, typed
resize197 and final+1C store216 preserve the reviewed events. Catch marks phase3
and rethrows; compiler EH uses CxxFrameHandler3 without private-native parity.

All declared nonempty physical sections/relocations in six existing selected
providers remain unchanged, including663 code sections35410B. COFF sections
with PointerToRawData0 have zero physical raw bytes; declared .bss storage is
not initialized-data evidence. Exact old/new objects and full library member
indices are preserved. The application map does not select this leaf, so there
is no linked leaf-body or application execution credit. The pre-sync evidence
helper's wrong old EXE path refusal was preserved, then corrected to Release;
it caused no production/build attempt. All readiness/revision/independent-review
archives remain immutable. The verification package is ignored local evidence
under local/cc10_texture_source_clear_implementation, with its frozen manifest.
