# Raw scene release and owned layer retirement (packet 76)

Packet 76 closes the whole raw `00B72250` leaf and connects the owned arm of
`00AC5480` to it. It removes the unused abstract raw scene-release callback.
The existing typed `NativeGuiSceneOwner` overload remains separate and unchanged.
This is a Source interface over required live services, with a controlled normal
Original comparison. It does not provide the complete manager or scene subtree,
retain a Source layout/ClipBox, or activate native Text.

Baseline: Root merge `843fe9618953a64e5e011597c96c265346e36fa7`, containing
packet 73. Ownership: `00B72250`, `00AC5480`, the existing layer-lifetime header
and implementation, and this packet's receipt files. No CMake, Main, constructor,
loader, current24, raw tree, or factory changes.

## Whole Original body and Source boundary

Verified BSP wrapper batches used `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. The whole leaf is `[00B72250,00B7226C)`:
28 bytes, 14 instructions, SHA-256
`1f665fbb8e540be92480153e4cea8ecb980ddd9a505d43e3783604e482d00db7`.
Its ABI is ECX = actual nonnull scene, no stack arguments. It saves ESI, calls
the **current** stdcall LONG decrement target from `00CE2220` with `scene+4`,
then tests the returned LONG. Nonzero restores ESI and returns. Zero reads the
current DWORD profile at `scene+0`, then its current table DWORD0, restores ESI,
and jumps to that captured target with the same scene in ECX. There is no local
EH frame, count admission policy, extra retain, or read of scene storage after
the terminal dispatch.

`release_native_gui_scene_storage_00b72250` borrows the same
`NativeGuiLayerLifetimeContext` as the parent. The actual decrement pointer cell
is read only at the reached call. A required side-effect-free metadata service
resolves the captured numeric profile to the same live table; DWORD0 is read
only after a zero decrement, then passed to the required terminal-call service.
Neither service has a default implementation. The raw scene prefix is aligned,
initialized Win32 DWORD/LONG storage; it is not a cast from `SceneResource`,
`NativeGuiSceneOwner`, or another Source companion. Context/table metadata must
survive terminal retirement and must not alias its freed backing. Callers remain
externally serialized and supply valid reached targets/storage.

The Source function is an ordinary C++ call followed by return, whereas Original
zero dispatch is a tail jump. No Original register, private stack, nonlocal
exception, FH3/SEH, or fixed-VA replacement ABI is claimed. The leaf adds no
cleanup; ordinary Source failures propagate into the parent's existing cleanup
state. A null resolved table fails explicitly rather than inventing a target.

The owned parent still obtains the actual manager before reloading `page+F0`,
removes that live store, then reloads `page+EC` and calls this raw leaf. It clears
`page+EC` only after normal return. The shared arm retains its original inline
decrement/current-profile/current0 sequence. Name capture, raw string return,
state1 name-then-base cleanup, state0 base-only cleanup, state-1 no-repeat, and
the scalar wrapper's post-destructor LOWBYTE flags read are unchanged from
[packet 73](CC12_NATIVE_GUI_LAYER_LIFETIME_73.md). No extra child/ancestor retain
or Source-layout count gate is inferred.

## Focused verification

The fresh translation unit compiles with MSVC Win32 `/MD /O2 /W4 /WX`. The
canonical `scripts/build.ps1` and its three existing checks pass. This worker's
baseline CMake does not yet register the layer-lifetime translation unit;
therefore its strict direct compile and fresh-object probe establish the new
body's compiled execution. Root owns registration. No persistent tests were
added and no fake providers were introduced merely to link.

The manifested Win32 probe links that fresh production object with the actual
canonical `bsp_core`, Lua, zlib and system libraries. It reuses the two packet-73
normal cases: owned name/store/scene reload, and shared zero retirement. The
owned Original parent now calls a copied **whole 28-byte Original B72250**;
only its `CE2220` operand is relocated to the live fixture cell. Source calls the
new production leaf. The real OS `InterlockedDecrement` export is supplied.
Both normal pairs pass, comparing all 292 page bytes after only the declared
store/name pointer normalizations and checking the same provider order.

Fixture pages come from the real initialized layer pool and whole `AA3840`
default constructor, with valid initialized reached retirement fields. They do
not come from an incomplete `AC6600` prefix. Actual raw string-pool and whole
empty-base/container bodies execute. Manager/store removal and current scene0
remain controlled services over live DWORD/LONG prefixes; terminal `VirtualFree`
checks the count and makes an after-retirement scene read observable. These
cases do not prove actual populated scene/manager bodies or lifetime admission.

The same two Source-only cases remain: current flags change during an earlier
callback and cause a real pool return after destruction; and an owned store
callback throws, completing raw name/base cleanup without scene terminal or
slot return. They are not Native exception comparisons. Copied Original parent
FH3 handler `CB8836` stays unchanged, unbound and unreached; exceptions are
excluded from those copied frames.

All 269 copied parent/wrapper/leaf bytes independently match live Ghidra and the
unchanged installed PE. The audit contains 83 instructions and 56 operand rows.
Ten named call/IAT relocations change 40 operand bytes; the other 229 bytes,
including all constants/internal branches, remain unchanged. No constant
instruction patches were made. The JSON receipt records complete operand maps,
artifact hashes and this precise proof boundary. The live report verifier also
checks all eight declared direct CALL sites with zero failures; indirect IAT
and register dispatch remain covered by the separate listing/operand audit.

## Remaining raw manager removal frontier (read-only)

`AA4B30` is still only available as the typed
`remove_gui_camera_store_00aa4b30(GuiCameraStoreMap&, GuiCameraStore*)` Source
projection. Its live whole raw listing is now `[AA4B30,AA4C1D)`: 237 bytes,
81 instructions, no listed gaps. ECX is the raw manager, one stack store
argument is consumed by RET4. Its installed bytes match live Ghidra, SHA-256
`225015b168fc419a580bb64eea3f0b881aaefbe282bb18ecfa029f4429fa371b`.

The manager+8 raw tree iterator checks owner/current head, searches payload at
node+10, and uses the already recovered whole raw `004BDA70` increment routine.
On a match it reads the current payload's +18 name provider through `B6D800`,
logs via `004254B0`, reloads/validates the payload, and calls `_free` at
`AA4BF6` if nonnull. The listing **continues** at `AA4BFB`, clears node+10 at
`AA4BFE`, and calls `AA46B0` at `AA4C0E` before the epilogue. The historical
free no-return gap is absent from this listing. This is not a claim that every
Ghidra flow attribute was repaired: the wrapper could report listing coverage
but not exact no-return properties, and packet 76 makes no repair there.

`AA46B0` has no reconstruction record. Its historical name
`STL_xlen_throw_00aa46b0` is not evidence of a throw-only body: the whole live
extent is 679 bytes/231 instructions and references iterator increment,
min/rightmost, both rotations, free, and the invalid-iterator exception path.
The actual raw erase continuation and its initialized tree/provider contracts
must be established before claiming whole raw manager removal. No AA4B30,
AA46B0, name, type or flow mutation is included here. Packet 76 closes the scene
release dependency, not that independent manager/tree frontier or ancestor
storage transport.
