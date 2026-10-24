# Raw Sound widget source prerequisite, 2026-10-24

This packet adds the complete normal raw storage/factory/lifetime schedules
for class14 Sound over its same actual108h allocation. It is a source
prerequisite, not activation, Page delivery, a complete Sound property/play
implementation, all18 admission, or an original binary ABI replacement.
Existing typed widget code and factory registration are unchanged.

## Storage and original evidence

The existing `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, was verified before analysis. Original assembly
was checked for the register inputs, current virtual calls and reference
transport; reconstructed source was inspected first.

| Address | Original interface / storage | Source composition |
| --- | --- | --- |
| AA1890 | ECX optional actual Sound source; EAX destination or null; plain RET. Allocate via ACD7F0 before deciding default versus copy. | Same canonical Sound pool slot, constructed in place. Null allocation skips construction. |
| ACD7F0 | `MOV ECX,F8C0EC; JMP ACD6B0`. Incoming108h size is ignored. | Reuses `allocate_gui_widget_pool_slot<GuiSoundPoolTraits>()`, with explicit prior process-pool startup. |
| ACDB30 | ECX fresh destination, EAX same, plain RET. AA9390(type14), D5CFD0, seven zero DWORDs EC..104. | Genuine raw base and derived stores on that same allocation. |
| ACDBA0 | ECX destination, stack actual source, EAX destination, RET4. | DEFAULT AA9390(type14), not AA9520. Empty strings, copy F4, then exact ACD8E0/ACD9E0 reference assignments. |
| ACD8E0 / ACD9E0 | ECX destination pointer cell, EDX source pointer cell, EAX destination cell, plain RET. | Preserves the complete normal temporary/assignment/additional-retain/two-release sequence inside the raw copy; no ordinary +1 substitution. |
| ACDC40 | ECX actual Sound, plain RET. | Sample release; current instance stop+08(0); current instance release; strings100 thenF8; current ref-member releases; genuine AA9730. |
| ACDEF0 | ECX Sound, stack flags, EAX original identity, RET4. | Read flags LOWBYTE after destructor; bit0 returns same slot through existing F8C0EC pool. |

`GuiSoundPoolTraits` already owns the actual distinct F8C0EC pool in the
canonical allocator-list domain. Its native object is108h, slot10Ch and slab
trailer108h. This packet adds no pool or startup owner. A caller must complete
the existing explicit process initialization before factory/scalar use.

Raw Sound uses the existing full AA9390 base constructor on its actual
allocation, including actual+04 initialized to1 and genuine native containers.
There is no `GuiWidgetOwner`/`SoundInstance` cast, private tree, mirrored native
field set, parallel reference counter, copied allocator trailer or forged
callable vtable. Constructor-unwritten base bytes stay unwritten. D5CFD0 is
the actual numeric profile, not a host function-table address.

Derived layout: EC is an actual sample reference; F0 is an actual tracked
instance reference; F4 is the copied DWORD; F8/FC is the SoundEfx NativeString;
100/104 is Category NativeString. Copy leaves both strings empty and preserves
the default base. Historical name evidence saying it copies both strings is
superseded by ACDBE2..ACDBF4 zero stores and ACDC15/ACDC22 pointer assignments.

## Exact reference and destruction schedule

ACD8E0/ACD9E0 compare current destination/source values first. If different,
capture CE221C once, retain the old destination temporarily, reread source
then destination, publish incoming, retain it, release the captured replaced
pointer, reread/retain current destination, then release the held pointer twice.
Every reached decrement reloads CURRENT CE2220. With stable pointer cells and
differing initial pointers, incoming gains two references and old loses two.
Equal initial pointers return without changes.
The nonzero NEG/SBB/TEST constants are null predicates, not global owner reads.
These operations are preserved individually; diagnostic metadata owns no credit.

ACDC40 stamps D5CFD0, captures EC, then captures CE2220 once for all four
normal release sites. Zero dispatch goes through
`dispatch_native_render_actual_owner_zero`: canonical lookup only after the
real raw+04 decrement observes zero, borrowing that SAME actual count and
requiring its nonthrowing current-profile terminal. The registry contains
sample/instance companions supplied by the caller; this packet does not invent
them, enroll the widget, allocate resources or provide missing audio lifetimes.

The current F0 profile/table+08 is captured for stop(0). A required
`NativeGuiSoundStopBindings` implementation resolves its SAME live table and
executes the genuine captured stop target. Unknown targets must reject; no
default success or numeric host call exists. F0 is reloaded AFTER stop returns,
then released and cleared. Both strings use the same raw419CC0/BD1510 context,
leave headers untouched, and return100 beforeF8. Their callbacks may replace
EC/F0: normal member cleanup rereads and releases those current values before
AA9730. The two native conditional/unconditional early slot-clear stores are
retained. Scalar deletion performs no widget count decrement or gate.

## Borrowing, failure and API contract

Prepare one `NativeGuiSoundStorageContext` and fresh externally held
`NativeGuiSoundStorageAcquired` before any entry. The context borrows the same
actual widget/node/container lifetime context, raw string publication/gate/
manager cells, canonical actual-owner registry, genuine stop provider, live
one word and real stdcall CE221C/CE2220 import cells. It creates none of them.
The source entry points are `construct_native_gui_sound_00acdb30`,
`copy_native_gui_sound_00acdba0`, `destroy_native_gui_sound_00acdc40`,
`scalar_delete_native_gui_sound_00acdef0`, and `make_native_gui_sound_00aa1890`.

All raw/candidate backing, context/provider/import cells and diagnostic records
are address-stable, externally serialized and disjoint from each other and
retired backing/private Original scratch. Metadata resolution is side-effect-free.
Existing provider contracts supply genuinely constructed base/children/timed
entries and matching current terminals. Counts have the native credits required
by the reached operations; no synthetic count repairs or retains are admitted.

An entered C++ interruption is sticky. The record retains captured identities,
returned import results, release/zero phases and its native call site, while
the caller retains the actual slot and unresolved ownership graph independently.
There is no rollback, refcount normalization, constructor/destructor replay,
partial-Sound scalar deletion, implicit AA9730 or automatic pool return.
AA9390 retains responsibility for its own established allocation-failure cleanup.
Discarding an entered/failed record terminates before dependency drain; use the
application's retained-process exit policy. A fresh record cannot authorize
reentry on an interrupted allocation. Successful scalar results are identity
only after possible slot return, never a surviving object borrow.

This normal-body Source composition deliberately does not claim native factory/
copy/destructor FH3 unwind or its partial-failure cleanup timing. Unknown audio
targets, actual Sound companion admission, properties/PlayByDefault/update,
original CRT/heap/register ABI, malformed or concurrent storage and game/runtime
parity remain outside this prerequisite.

## Validation

`scripts/build.ps1` passed with the new translation unit compiled under MSVC
Win32 Release and `bsp_game.exe` linked. Both existing CTest checks passed:
`reconstructed_math` 0.31 s and `tool_tests` 10.75 s; total 11.10 s. The private
capture is `local/gui-sound-build.log`. No new tests, probes or game runs were
added; actual Sound operations have not been exercised in a fixture/runtime.

Private source registration used the installed CMake's project-specific
`CMAKE_PROJECT_bsp_reconstruction_INCLUDE` cache hook, then restored its prior
absent value in a finally block. `CMakeLists.txt` and `cmake/startup.cmake`
remained byte-identical before/after. The ignored restoration evidence is
`local/gui-sound-cmake-restoration.json`; no tracked CMake file was edited.
The AC-band constructor/copy/destructor/scalar/reference/allocator evidence
comments were saved to the verified BSP project and affected exports refreshed.
Prior annotations are retained in
`local/ghidra-annotations-20261024T114752Z.json`.
