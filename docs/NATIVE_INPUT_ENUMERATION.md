# Raw input enumeration and attachment

Addresses: 00A904E0, 00A98030, 00A982B0.

The new entries operate on the existing F8h backend allocation, its device table,
and its actual GUID header at E4h. They retain the earlier typed implementation
as a separate interface. A904E0 is native thiscall(backend,slot,device), RET8;
A98030 is stdcall(backend,instance), RET8; A982B0 swaps the callback arguments
and calls A98030 before RET8. Added source services change the callable ABI.

Attachment captures the device profile and slot08 class call before resolving
the requested slot. It writes the resolved index at device+08 and the same
pointer into the backend table. There is no retain, replacement deletion, or
range clamp. An automatic request scans eight cells; a full row stores at
index8. Callers must supply storage covering the resulting native address.

Enumeration checks the low type byte at instance+24. Keyboard and mouse guard
backend+04/+24 before allocating 310h/23Ch. Each constructor receives the current
backend+E0 interface after allocation. A returned device is attached after the
allocation cleanup state has been cleared. A constructor exception frees the
captured allocation after its own member unwind; attachment failure does not.

Joystick/gamepad handling uses the existing eight-byte NativeString and sized
storage services. It captures the product pointer, tests the three case-specific
Xbox spellings plus 360, and writes F4h before searching GUIDs. The loop reads the
actual vector begin/end and compares sixteen bytes using the existing A972E0
implementation. A duplicate GUID returns without creating a device. A new GUID
is appended before the Xbox exclusion and occupied-slot checks. Its index is
used directly, including beyond the eight nominal joystick cells; no clamp is
introduced. A97FA0 and its iterator/allocation callees remain recognized STL
contracts: this packet requires an adapter for that same raw header and does
not add a second owning vector or port their library implementation.

The DECE58 FH3 descriptor points to the four-state map at DECE38: states0/1
free the keyboard/mouse allocation through CB69E0/CB69EB; state2 destroys the
local string through CB69F6 ->41DD20; state3 frees the joystick allocation
through CB69FE then proceeds to state2. The three free funclets' missing POP/RET
tails and the CB6A09 handler were repaired/defined under the primary write lock,
saved, and exported. All four now have complete gap-free bodies.

The real EnumDevices adapter passes the raw backend as the SDK opaque argument.
A synchronous thread-local frame borrows the source services, supports nested
enumerations, catches C++ callback exceptions, returns STOP, and rethrows after
the SDK call. This transport is a source boundary; original FH3/SEH and hardware
fault compatibility are not claimed. The callback must be used through that
adapter. The returned HRESULT is preserved for the backend caller to ignore.

The Win32 build and both existing tests passed. One ignored focused probe passed
the Xbox flag-before-append sequence, duplicate GUID rejection, occupied slot
preservation, and a real DirectInput EnumDevices callback whose intentional
constructor exception was caught and rethrown outside the SDK. The two GUIDs
lived in the backend's actual header; its append service was a bounded fixture,
so it does not establish the production STL adapter. The SDK reference tracker
was empty after explicit release. Independent source review found no further
caller schedule defect after aligning device scalar frees with the same CRT
allocation family. Required raw device providers,
the production GUID append adapter, and their application binding remain explicit
dependencies. No full backend startup, physical input delivery, or gameplay
validation is claimed by this caller packet.
