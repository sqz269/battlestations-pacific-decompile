# DirectInput enumeration and device attachment

Addresses: 00a98030, 00a982b0, 00a972e0, 00a97fa0, 00a962f0, 00a9a3e0, 00a9a290

`on_input_device_enumerated_00a98030` implements the callback's class dispatch,
product-name filtering, persistent instance-GUID sequence, and attachment through
the existing `InputDeviceTable`. `enumerate_direct_input_devices` calls the real
ANSI `IDirectInput8A::EnumDevices` with that context. The existing
`InputFocusBackendState.direct_input` host can call this adapter directly: it uses
the same owning slots and references the existing GUID vector rather than making
another backend or a temporary deduplication set. The established `00a983c0`
wrapper remains responsible for clearing the Xbox-presence flag before enumeration.

## Native sequence and corrected interpretation

The callback masks `DIDEVICEINSTANCEA::dwDevType` with FFh. Keyboard type13h only
creates a device when class0/slot0 is empty; mouse type12h uses class1/slot0. Both
construct first, then call the canonical attachment routine with requested slot0.
Attachment writes the returned device's own class row and its `assigned_slot`;
it does not activate a binding group or invoke the devices-changed callback.
Other types return DIENUM_CONTINUE except joystick14h and gamepad15h.

For joystick/gamepad, the callback first copies `tszProductName` through the
canonical NativeString allocator. It matches precisely `Xbox`, `XBOX`, or `XBox`,
plus `360`, using case-sensitive substring searches. A match sets backend+F4
before the GUID lookup, even if the GUID was already present. Lowercase `xbox`
does not match. The name is released before returning, including skipped devices.

The GUID comparison at 00A972E0 is equality of all sixteen bytes, not an ordering
predicate. A new GUID is appended through the native vector helper 00A97FA0.
Only after that append does the caller skip an Xbox360 device or examine a slot.
The **GUID's insertion index** is the class2 slot index; this is not a first-free
search. Thus a skipped Xbox device reserves an index, and an already occupied slot
also leaves its new GUID recorded. A later duplicate is skipped even if that slot
has become empty. This corrects the first-free wording in the earlier discovery
notes. Native code has no eight-slot bound check here; the typed adapter appends
the GUID first and then reports a non-Xbox index >=8 instead of reading out of row.

## Real keyboard and mouse construction

The new creation functions invoke actual SDK COM methods. Each allocates typed
storage before CreateDevice. Keyboard uses GUID_SysKeyboard, then c_dfDIKeyboard,
then reads the current platform HWND and calls SetCooperativeLevel(HWND,6).
Mouse uses GUID_SysMouse and c_dfDIMouse2; it sets no cooperative level during
construction. Its existing typed constructor queries SM_SWAPBUTTON and the double
click interval after SetDataFormat, matching native order. The unsigned interval
conversion remains in the canonical mouse-state implementation, including the
x87 2^32 correction and double-precision divide by1000.

Native CreateDevice, SetDataFormat, and keyboard cooperative HRESULTs are ignored.
The typed functions preserve that when an output pointer exists, but reject a
missing CreateDevice output instead of dereferencing native uninitialized storage.
No Acquire, AddRef, Release, or extra device initialization is inserted.

If object allocation returns null, native A980D5 (keyboard/mouse) or A98260
(joystick) still passes null to A904E0. That function loads the device at A904E1
and dereferences its vtable at A904E5 without a null check; it does not store or
ignore null. The typed creation helpers can likewise return null, but canonical
`InputDeviceTable::attach` rejects it and the enumeration adapter reports a host
error. This is an explicit boundary around a native invalid-pointer path.

SDK-linked `c_dfDIMouse2` equals native D795FC: header fields are
`24,16,2,20,11`; every one of its11 object descriptors matches. SDK-linked
`c_dfDIKeyboard` equals native D79804: header `24,16,2,256,256`; all256 objects
match. The comparison resolved each GUID's16 bytes and checked GUID nullness,
object offset, type, and flags, excluding relocatable pointer addresses. The
source links the actual SDK definitions through dinput8.lib and dxguid.lib.

The returned wrappers borrow their COM pointer. A successful CreateDevice has
provided a COM reference that the application must track and keep alive through
wrapper use, including focus resets that delete the wrapper. Native mouse deletion
A9A390 does not Release. Keyboard deletion A9A470 calls A95E60, which only changes
the vtable and tail-calls the already reconstructed BD30F0 base destruction; it
also does not Release. Do not attach COM Release to those native deletion paths.
An injected host exception after a COM reference has been supplied does not gain a
new native cleanup rule; external ownership remains the integration obligation.

The typed keyboard constructor supplies the three zeroed256-byte arrays from
A962F0. Native vtable writes and reference-count1 are not projected as binary
layout. Native mouse sample bytes are uninitialized; the caller supplies an
explicit typed initial sample. The typed mouse's array initialization occurs in
its canonical constructor after SDK setup; this is not an in-progress native
object-layout replacement. Joystick construction A99940 remains a required actual
polymorphic factory, including its COM setup, object enumeration, and lifetime.

## ABI, failure boundary, and analysis gaps

| Address | Original ABI and boundary |
| --- | --- |
| A98030 | stdcall(context, instance), RET8 at A982AA/3 bytes, end A982AC |
| A982B0 | stdcall(instance, context); six instructions, RET8 at A982BF/3, end A982C1, total12h bytes |
| A972E0 | ECX/EDX point to16 bytes; EAX equality boolean; RET at A9737C/1, end A9737C |
| A97FA0 | ECX vector header, source stack pointer; RET4 at A98020/3, end A98022; standard-container boundary |
| A962F0 | ECX keyboard base; EAX=this; RET at A96342/1, end A96342; typed array-init fragment |
| A9A3E0 | ECX allocated keyboard, DI8 stack pointer; EAX=this, RET4 at A9A468/3, end A9A46A |
| A9A290 | ECX allocated mouse, DI8 stack pointer; EAX=this, RET4 at A9A37C/3, end A9A37E |

At analysis time Ghidra had no function at A982B0. Its body is the separate
18-byte thunk described above; the live A98030 body ends at A982AC and does not
contain it. The earlier three-instruction description was incorrect. The adjacent
keyboard deleting-destructor listing omits the three-byte `ADD ESP,4` at
A9A48B..A9A48D after the incorrectly nonreturning BF65AC call. Disk bytes confirm
that instruction; its final RET4 is A9A491/3, end A9A493. These repair facts were
sent to the integrator; this packet did not mutate Ghidra.

The callback returns1 on normal paths. C++ host errors are captured into the
context and return0 to stop SDK enumeration; the bridge rethrows after EnumDevices
returns. It clears that host-only error at the start of a fresh enumeration.
No exception is allowed to unwind through the DLL callback boundary. Native
unchecked pointer/slot failures and proprietary vector allocation behavior are
not claimed as parity. The GUID vector uses `std::vector::push_back`; the SDK,
CRT comparison, and container implementation are not reconstructed here.

Win32 Release, both existing CTests, and all8 native seed comparisons passed.
One ignored local recording fixture passed11 enumeration items, class guards,
GUID-index gaps, exact Xbox casing, duplicate flag updates, occupied-slot handling,
same-table attachment, product-string cleanup and callback-error containment.
That executable also emitted the SDK format descriptors for the complete static
comparison above. It uses explicit COM/factory doubles and did not enumerate real
hardware or change focus/cooperative state. No hardware or gameplay validation
is claimed. Details are in `reports/input_enumeration.json`.
