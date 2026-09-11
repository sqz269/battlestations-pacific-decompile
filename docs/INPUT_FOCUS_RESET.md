# Input focus reset and backend device update

Addresses: 00beca40, 00a9a140, 00a90ee0, 00a91620, 00a92840,
00bebf30, 00bec230, 00a983c0, 00a918a0, 00a97390, 00a90490,
00a93eb0, 00a9a390. Source: `src/input_focus_reset.cpp`.

The focus reset removes the first active mouse, deletes the mouse-class slots,
enumerates devices, activates mouse slot0 through the backend's device-ID filter,
and retargets cached mouse bindings. It does not clear keyboard state or call
the ordinary device reset slot. The first vector at +94/+98 belongs to class1
mouse: the established class-group headers begin at +6C with a 24h stride.
Keyboard is class0. This conclusion comes from addressing and binding-class
tests, independently of old descriptive labels.

## Shared state and ownership

`InputFocusBackendState` references the existing `InputDeviceTable` and
`InputBindingDeviceGroups`. The fixed table at +04 contains 3x8 owning slots;
the active-device vectors at +6C/class*24h are separate borrowed-pointer lists.
Class records also carry a desired active count at +68/class*24h and a checked
ID vector at +7C/class*24h (begin at +80). The view references those counts,
filters, backend+64, dirty byte+D4, optional callback+D8 and Xbox-detected byte+F4.
No alternate device table, duplicate groups or shadow singleton is created.

The new `InputDeviceTable::slot_reference` exposes one actual slot for recovered
ownership operations. It validates class/index for the typed domain; existing
attach/read/reset behavior is unchanged. A deleting callback executes before
that slot is cleared, so even a callback-written replacement is subsequently
cleared, as at BEBF51. The device host must provide the original deleting-slot
semantics, not merely detach a pointer.

For the reconstructed mouse type, A9A390 changes base vtables and conditionally
frees the object. It does **not** release the DirectInput COM pointer. Its typed
implementation therefore preserves external COM ownership and frees only the
ordinary-new `MouseInputDevice` projection when flags bit0 is set. Flags0
destroys the projection in place. Keyboard/gamepad deletion remains a required
device host operation. The existing device types already designate their COM
pointers as borrowed.

## Native routines and ordering

| Address | ABI and established behavior |
| --- | --- |
| 00BECA40 | ECX unused, RET. Initial absent backend returns; later singleton reads are required. End BECB14/1 byte. |
| 00A90EE0 | ECX backend, device stack, RET4 at A90F8D/3. Query device class+8; erase first active match; dirty before erase; callback afterward. |
| 00BEBF30 | ECX backend, class stack, RET4 at BEBF61/3. Eight slot deleting calls+4(flags1), each followed by clearing that same slot. |
| 00A983C0 | ECX backend, RET at A983DF/1. Clear +F4 and call DirectInput8 EnumDevices(type0, callback A982B0, this, flags1). |
| 00A91620 | ECX backend, class/slot stack, RET8 at A917D3/3. Filter device IDs; append only if accepted and not already active. |
| 00A92840 | ECX action manager, replacement device stack, RET4 at A92903/3. Retarget every cached class1 pointer. |
| 00A918A0 | ECX backend, float seconds stack, RET4 at A91921/3. Prepass then class/slot-ordered polling and conditional activation. |
| 00A90490 | ECX backend, class stack, AL bool, RET4 at A904D1/3. Exact active-count equality with requested count. |
| 00A97390 | ECX unused, RET at A97390/1. Verified no-op prepass. |
| 00A93EB0 | ECX unused, EAX zero, RET at A93EB2/1. Verified keyboard/mouse identifier slot+34. |
| 00A9A390 | ECX mouse, flags stack, EAX original pointer, RET4 at A9A3B1/3. Base destruction then conditional free. |
| 00BEC230 | ECX platform, EAX HWND+30, RET at BEC233/1. |
| 00A9A140 | ECX mouse, flags stack, RET4 at A9A172/3. Actual DirectInput SetCooperativeLevel, then configured byte+234=1. |

BECA40 loads backend F8BBF4 initially, before deletion, before enumeration,
before activation, and before fetching the final mouse. These reads cannot be
coalesced: the optional backend callbacks and device operations may replace
the current singleton. The final mouse pointer is pushed before the lazy
004BEC00 action-manager getter, so it remains captured even if that getter
changes the active groups. The getter remains a required genuine singleton
and lifetime service; it is not substituted with an independently allocated
action table.

A90EE0 invokes callback+D8 with device class in ECX and -1 in EDX. A91620
invokes it with class in ECX and the active-vector count captured before append
in EDX. Both are fastcall-style register arguments with no stack arguments.
Dirty+D4 is set before mutation. The callback pointer is loaded after mutation.
Pointer erasure preserves order. Active insertion uses standard vector storage
in the projection; native A91430/A91260 are checked STL insertion and growth,
not additional game rules or a library implementation to reproduce here.

A91620 scans the entire accepted-ID list even after finding a match. An ID -1
accepts without invoking the device; every other entry invokes device+34 and
compares the result. An empty list rejects. Already-active candidates produce
no append, dirty store or callback. A null slot can pass a wildcard filter and
be appended as native permits; querying its identity is outside the valid domain.

A92840 visits every 30h action and every 34h binding, regardless of enabled or
resolved flags. Class1 primaries change cached pointer+0C; class1 required and
forbidden 14h modifiers change cached pointer+08. It does not change device
indices, resolved flags, class, values or modifier storage. Null replacement
is meaningful and is written to those same cached fields.

A918A0 invokes the derived backend prepass+10 (A97390, verified RET), then walks
classes0..2 and slots0..7. Every nonnull device is polled through+10 with the
float argument loaded/stored by x87. Poll result is ignored. If the active count
already equals the requested count, activation is skipped. Otherwise class2
needs activity+28 only when backend+64 is false; classes0/1 and a true +64
activate without this query. The candidate slot is reloaded by A91620 after
polling; the captured polled device remains the activity-query target.

## Actual platform and device boundaries

Mouse cooperative mode uses the SDK `IDirectInputDevice8A::SetCooperativeLevel`
with the supplied actual HWND and flags. The caller's flags pass through intact;
the cursor policy passes6 and ordinary mouse polling passes5. Native captures
the COM table, calls the pure HWND getter BEC230, then calls COM+34 with this,
HWND, flags. HRESULT is ignored and configured+234 is set even on failure.

`KeyboardMouseFocusDeviceHost` calls the existing real keyboard and mouse poll
methods. The same platform byte+170 suppresses keyboard state; the same
`Win32PlatformState::settings_changed_2c` reaches `MouseInputPlatform` and is
consumed by mouse polling. Parent integration owns that canonical platform field.
Both keyboard D5B904 and mouse D5B8B0 tables have A93EB0 at+34, proving ID zero.
Unsupported device operations require an explicit delegate and otherwise throw;
no fabricated joystick state, polling or deletion is provided.

A983C0 uses the established `DirectInputHost::enum_devices(0,1)` contract.
The concrete SDK host must enumerate with the actual callback behavior over
these same backend slots. A98030 includes keyboard/mouse creation and joystick
GUID/Xbox filtering. That existing constructor/enumeration boundary is retained;
this packet does not replace it with an empty callback or claim complete
joystick construction. Reset does not AddRef or recreate the DirectInput8 object.

## Verification and limits

Ghidra was read only against the existing BSP project and program, checked by
bsp.py before live queries/exports. Initially absent definitions were reported
to the primary: A983C0..A983DF (32 bytes), A918A0..A91923 (132 bytes), A97390
(one byte), and A93EB0..A93EB2 (three bytes). The A9A390 listing also omitted
A9A3AB..A9A3AD, live bytes `83 c4 04`, after the wrongly no-return free call;
that is `ADD ESP,4` before the inspected return. No worker Ghidra writes occurred.

The typed routines require stable valid class/vector storage while native
device queries use captured iterator addresses. Callback-driven singleton
replacement is preserved at the inspected reload points. Native malformed
iterator handlers, bad class/slot accesses, initial-null active mouse and later
null-backend dereferences become explicit host exceptions. Those guards and
standard container allocation are not original failure or replacement ABIs.
No new polling policy or blanket state reset is added.

The Win32 Release build and both existing CTests passed; verified native seed
bytes match the installed image. One local fixture exercises reentrant focus
reset: a removal callback switches backend, a deleting callback overwrites its
slot, activation scans after a match and changes backend again, and the lazy
manager getter changes the final group. It verifies five backend reads, the
post-callback slot clear, two identity queries, both change callbacks, and the
captured pre-getter mouse in primary/modifier bindings. No test suite was added.
Commands, logs and exact status are recorded in `reports/input_focus_reset.json`.
No live game, hardware-input or cursor-state validation is implied.

## Integration from docs/PLATFORM_SERVICES.md

The parent now composes these routines through PlatformServices and XLiveManagerRuntime, with original-ordinal SDK forwarding and shared canonical state. Input lookup/reset/tick use one published pointer. The combined build and existing tests passed; installed XLive loader probes failed before pretranslation. Required owner construction, game dependencies and runtime validation remain explicit in docs/PLATFORM_SERVICES.md.
