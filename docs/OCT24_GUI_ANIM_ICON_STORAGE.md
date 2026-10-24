# Raw AnimIcon allocation, state and lifetime prerequisite

This packet composes the existing SAME raw Icon base into actual AnimIcon
storage. It adds no Page, Screen, manager, retained type factory, input route or
runtime switch. `GuiIconRuntime` and the GPU adapters remain unchanged: there is
no second state vector, logical-widget cast, frame mirror or model owner.

## Matching actual storage

AA2340 passes144h to AD3960. That allocator selects the existing F8C1D4 process
pool:144h payload,148h slot,4 slots per slab. The separate slab-index DWORD144
is preserved. Unlike Progbar's alias documented separately, all AnimIcon tail
stores stay inside its genuine payload.

AD2D50 calls full raw AB5C60 first. At AD2D58 it reloads live CE38C8 (observed
bits41F00000,30fps), sets bytes140/141/142 to1, stamps D5BCE0, stores FPS138,
clears last-step milliseconds13C and writes type60=12. Byte143 is unwritten.
The source API borrows the SAME live constant and Icon one-value cells rather
than introducing a process global or a replacement constructor.

AA1C60 calls full raw Icon AB5D30, including its required genuine current80,
then stamps D5BCE0, copies FPS138 with original x87 FLD/FSTP, copies time13C
as a DWORD and copies bytes140/141/142. It does not rewrite source type60 or
copy byte143. There is no additional derived owning subobject or derived unwind
body. Icon retains its own constructor cleanup and failed clone acquisitions.

The split `create_native_gui_anim_icon_*_00aa2340` source arms use the SAME
initialized process pool, return null for its null branch, and return only the
captured failed slot after the constructor's existing cleanup on an exception.
No private allocator or larger backing is substituted.

## Actual frame state and loaded order

All frame state is the SAME inherited F0 header, F4/F8/FC40h state vector and
actual texture/name ownership implemented by the raw Icon packet. Copy uses
AB3A70's one texture+04 increment and native name copy per actual state;
destruction uses AB3280's one release and native name return. There is no second
AnimIcon frame array, texture retain sequence or semantic-state adoption here.

AD2FF0 always calls real AB10F0 first. That mandatory provider performs SAME
AA7170 (current60(0), raw bounds) and freshly selected current88(0,0,1) over this
actual object. After return, the outer hook reloads current forward142. Nonzero
returns immediately, without reading a state count or another native table.

On zero142, it reloads current F4; null means count0 without reading F8. Nonnull
uses the native signed wrapping pointer difference and SAR6 of F8-F4. It then
captures the current table and freshly selects its+88 entry, passing the full
DWORD count-1, mode0 and ratio1. Empty reverse therefore still calls with
FFFFFFFF; there is no invented empty-frame skip. Live D5BCE0+88 is AB1710.

`NativeGuiAnimIconLoadedCalls` requires genuine raw AB10F0 and selected AB1710
providers, including any reached current80/GPU services. It supplies no empty
callback or typed `GuiIconRuntime` substitute. Unknown current profiles/targets
reject in this closed AnimIcon source API. Original AD3040 animation-clock/update
activation and raw property/frame production are outside this packet; there is
no claim that loaded orchestration alone enables animation.

## Derived and canonical terminal lifetime

AD2DC0 stamps D5BCE0 then tail-jumps to full AB5E70. The reused raw Icon path
stamps its own D5C4C0 and performs actual delayed getter/cancellation, native
shader-string return, actual state vector destruction and raw AA9730 in order.
The SAME shared singleton deletion arm must already point to the SAME delayed
service before AB5700. Its real F8BD98 producer,01090AA0 manager and string
domain remain retained through normal process drain. No substitute singleton,
private list or delayed-entry producer is introduced.

AA1D50 tests only flags' low byte after the full destructor. Flags1 returns the
SAME F8C1D4 slot via AD3420/AD32B0; flags0 leaves destroyed physical storage
with the caller. It does not test or decrement widget+04. One-shot retained
frames record whether Icon returned, pool return entered, completion or failure.
There is no second Icon destructor or guessed derived rollback on failure.

`NativeGuiAnimIconReference` is the sole canonical companion for the actual
allocation. It borrows SAME actual+04 in the SAME renderer registry, without a
store/retain or an additional Icon companion. Current D5BCE0 slots0/4 are
BD30E0/AA1D50. At already-zero, BD30E0 freshly selects current4(1); direct
scalar transport has no count gate. Successful terminal return unbinds only the
opaque identity, without reading returned native storage.

Entered failure retains the canonical binding in a failed phase, rejects replay,
prevents normal metadata destruction and blocks identity reuse. The failed raw
graph/providers require explicit process disposition. Keeping that opaque
binding requires no further payload/count dereference, even if a terminal
provider's effects are uncertain. No destructor silently retries or rolls back.

## Caller authority and proof limits

The producer proves full raw construction, actual matching pool provenance and
all subobject/backing/model/provider custody; profile/count checks cannot prove
them. Raw actual tables, contexts, operation frames and flags stay stable and
disjoint from native payload. Serialize callbacks and exclude destructive
reentry, state-vector mutation and slot reuse until canonical unbind.

The existing closed Group/ClipBox dispatcher does not admit AnimIcon children.
Future same-tree current20/class0C/delete04(1), real type-lineage phase, timed
entries and complete Page services remain mandatory genuine providers. Raw
AA9730 continues to control detachment and child-loop progress. No alternate
tree, list-progress repair, state conversion or retained activation is added.

Evidence: AA2340, AD3960, AD2D50..2D96, AA1C60..1CB4, AD2DC0..2DC6,
AA1D50..1D69, AD2FF0..3033, existing pool traits, SAME raw Icon implementation
and live D5BCE0/CE38C8 bytes. Analysis verified the configured
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`. The new C++
interfaces are source orchestration, not original register/FH3/SEH ABI shims.
Malformed storage, destructive reentry, concurrent mutation and raw hardware
fault behavior are not qualified. `scripts/build.ps1` compiled both new source
files in Win32 Release, linked `bsp_game`, and passed the two existing CTest
targets. Temporary CMake registration was restored byte-for-byte afterward.
Seven bounded composition/audit ledger fragments distinguish this source/build
proof from the unsupported Progbar lifetime. No Ghidra mutation, game or new
test was run; no original ABI, visual/animation or Page activation claim is made.
