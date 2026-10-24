# Raw Icon allocation, state storage and terminal prerequisite

This packet implements source orchestration over the SAME original Icon
allocation. It does not activate a Page, Screen, retained type factory, GUI
manager, Icon application or new runtime switch. The existing GPU adapter and
`GuiIconRuntime` are unchanged. In particular, that runtime's semantic states
have not been adopted into these raw fields.

## Allocation and normal construction

AA1410 selects AB5C60 for a null source and **AB5D30** for a nonnull source.
The new split `create_native_gui_icon_*_00aa1410` arms use the existing actual
AB2B60/F8BDA0 pool route, return null on its null branch, and return the same
captured slot to that pool if construction throws after its constructor cleanup. The pool
must already be initialized through its existing native process producer.

The Icon payload is138h and the pool slot13Ch; the slab index at138 is separate.
AA6560's Progbar request140h still selects this same pool through AA14A0.
That pre-existing discrepancy is preserved: no resized pool or Progbar support
is introduced by this Icon-only packet.

AB5C60 first calls the real raw AA9390(type6), reads actual D7A24C once at
AB5C80, then publishes D5C4C0 and the original tail stores. EC isFFFF; F4/F8/FC
are null; bytes101/100 are0/1; ShaderName104/108 is empty;10C/110 are0;
114 is one; crop118/11C/120/124 is0/0/one/one;12C/134/128/130 are0.
F0, padding and all other constructor-unwritten bytes remain preimages. There
is no second base/layout/tree/model representation.

AB5D30 runs the real raw AA9520 first, then stamps D5C4C0, copies current EC,
constructs the SAME raw state vector and copies the remaining native fields.
ShaderName104/108 is reset rather than copied. AB5DCB..5DD6,5E0C..5E14 and
5E2F..5E35 use x87 for114/128/130. The current native table is captured between
FLD128 and FSTP128 at AB5E12. The EC word argument is zero-extended at AB5E28;
slot80 is read from that captured table at AB5E48, after the remaining stores.

The genuine selected target AB3CB0 remains a mandatory
`NativeGuiIconCurrentCalls::call_rebuild_current80` service. No raw GPU/state
authority is fabricated from `GuiIconRuntime`, and no empty rebuild is provided.
The actual table supplied by the caller must expose the real selected slot.
The supported raw AA9520 clone domain still requires the source's actual Model
and the existing canonical clone services; it is not a general all-widget copy.

## One raw state representation

`NativeGuiIconTailStorage`, `NativeGuiIconStateVectorStorage` and
`NativeGuiIconStateStorage` are borrowed native layout views, with Win32 size
and offset assertions. They allocate no state or ownership metadata. The actual
F0 header holds its unwritten proxy plus F4/F8/FC pointers; each40h record holds
actual texture0, native name length4/buffer8, remaining fields through3D and
unwritten3E/3F. Nothing is cast to a logical widget or native-string C++ object.

The raw AB3A70 copy stores texture0, zeroes its destination name, calls existing
actual-pool0041DD40 and copies by the reloaded native string lengths. C..28 are
DWORD copies;2C/30/34/38 use original x87 FLD/FSTP. It copies3C/3D and performs
one Interlocked-style increment on the SAME texture+04. Texture custody in the
canonical renderer registry is a caller precondition, not another owner map.

AB3280 decrements that actual texture+04 through the existing native-owner
release helper. Only zero resolves its canonical terminal; there is no extra
decrement. It clears record0 only after that terminal returns, then releases
the reloaded native name through the SAME actual pool. Native state0 name
cleanup also runs if texture release throws. Name destruction leaves the raw
header untouched, as the native body does.

AB58F0/AB5850 are qualified stock vector substitutions, with no reconstructed
vendor-library credit. They use compatible source CRT backing, actual40h
records and the above raw ownership operations. Copy allocates exact size,
publishes F4 and FC with F8 still at begin, and publishes F8 only after all
records complete. AB4BDA..4BF6 destroys the completed prefix in forward order
on failure; AB59B6 then calls AB5850 to free the still-unpublished backing.
The failed current record is not treated as a constructed state. Tidy captures
begin/end once, releases forward, frees CURRENT F4, then clears F4/F8/FC.
The unwritten F0 proxy is preserved; no host vector/proxy is installed.

## Derived, base and pool lifetime

`NativeGuiIconStorageContext` borrows the SAME lifetime, actual-owner registry,
actual string context, delayed service, shared singleton deletion map and native
Icon table. Before every destructive AB5700 getter, the source adapter requires
`singleton_deletion.gui_icon_delayed_loads == &delayed`. The new appended pointer
and D5C4B8 dispatch arm invoke that exact service's AB57C0 over the popped actual
owner and original flags. All prior binding offsets remain unchanged; binding
size changes212->216. No host constructs or activates this service here.

AB5E70's normal order is preserved: publish D5C4C0; genuine AB5700 getter;
AB50E0 cancellation for this SAME raw Icon; shader-string return; state-vector
destruction; raw AA9730. The delayed producer uses its existing actual F8BD98
cell and SAME01090AA0 manager/string domains, including real later process drain.
No private singleton/list or replacement manager is added. Queue insertion and
file delivery remain outside the existing delayed-service API; this packet does
not enable `DelayedTextureLoad=true` or manufacture pending entries.

Unwind maps DEEAE8 (AB5D30) and DEEB24 (AB5E70), inspected from original bytes,
select state2 name -> state1 vector (AB59D0 tail-jump to AB5850) -> state0 AA9730.
The destructor lowers its state before each normal member destruction; copy
construction has no delayed cancellation on unwind. The retained one-shot
acquisition frames record entry, phase, native EH state and C++ failure. Existing
raw-base and clone cleanup remain authoritative; failed clone acquisitions stay
in the caller's base frame. No replay/rollback of consumed native effects occurs.

AB6100 reads only flags' low byte AFTER AB5E70 returns and returns the SAME slot
through AB2150/F8BDA0 iff bit0. It neither tests nor decrements widget+04.
`NativeGuiIconReference` separately admits one completely constructed actual
allocation into the SAME canonical registry, borrowing actual+04 without a
store/retain. CURRENT0 BD30E0 freshly selects CURRENT4 AB6100(1) at already-zero;
direct scalar transport has no count gate. Successful or entered-failed scalar
retirement unbinds the same opaque identity without touching returned payload.
Flags0 retires payload metadata but leaves physical allocation with the caller.

## Remaining authority and verification limits

Full construction, actual allocator provenance, stable/disjoint operation frames,
all backing and transitive parent/child/timed/model/provider custody are caller
preconditions. Slot/profile/count checks alone cannot prove them. Calls must be
serialized, excluding destructive reentry, state-vector mutation and slot reuse
until canonical unbind. Entered failure preserves effects and prohibits retry;
retain the partial raw graph, delayed producer and dependencies independently.

The existing closed Group/ClipBox lifetime dispatcher still rejects an Icon
child. A future same-tree provider must supply its real current20/class0C/
delete04(1), actual lineage phase and mandatory timed-entry domain. Raw AA9730
continues to control self-detachment and child-loop progress. This packet adds
neither guessed list erasure nor alternate tree authority.

Future retained activation must adopt the SAME raw vector/name/texture fields
as the sole derived authority, compose the real current80/GPU services, and bind
the SAME raw Model at the native point. Copying the typed runtime into a second
raw object, independently owning its texture references, or routing through a
logical `GuiWidgetOwner` cast would violate this contract. The existing GPU
adapter's count1 retirement guard is not a zero-count raw scalar substitute.

Evidence: factory AA1410; defaults AB5C60..5D29; copy AB5D30..5E65;
destructor AB5E70..5EFE; scalar AB6100..611D; record AB3A70..3B25 and
AB3280..32FD; stock-vector AB58F0/AB5850/AB4B80; original EH data and cleanup
thunks CB7D40..7D8F. Ghidra was verified on configured `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`, before analysis. No Ghidra mutation or new
ledger reconstruction credit was needed. `scripts/build.ps1` compiled both new
source files in Win32 Release, linked `bsp_game`, and passed the two existing
CTest targets. Private CMake registration was restored byte-for-byte afterward;
no game or new test was run. This is source/build evidence, not original binary
ABI, live raw Icon, visual rendering, Page activation or all18-class proof.
