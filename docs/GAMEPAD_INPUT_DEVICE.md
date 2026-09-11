# Canonical gamepad force binding

Addresses: 00A95D70, 00A95A80, 00A95890, 00A98CC0, 00A99940, 00A991F0, 00A9A5A0, 00A9A7C0, 00A9A9C0.

`GamepadInputDevice` connects the recovered force-request state to both actual
joystick and XInput device classes. Every object owns exactly one request map
and amplitude pair. `GamepadForceDeviceHost` resolves current input groups on
each request, binds that device to the canonical E12F2C flag, and dispatches
output to the existing XInput motor setter or joystick SetParameters routine.
Use its `context(next_id)` factory so the context and host share the flag.

XInput binds the same flag reference at construction; binding a different flag
is rejected. A joystick can stay unbound while its force state is empty and
zero. Its first force operation binds the flag before exposing the state. The
application must keep this flag, the clock, SDK interfaces, and all borrowed
services alive through device destruction. Directly populating an unbound
device's state violates the composition contract.

Base destruction runs after the complete derived destructor. It deletes owned
requests in key order and applies the recovered A95A80 check. With enabled,
nonzero output, native code reaches the base vtable's `_purecall`; C++ destruction
terminates explicitly on the corresponding guard. It never sends force output
through a destroyed derived object. A95890 explicit clear remains available
while the actual device is alive. No automatic clear or COM Release was added.
The explicit state teardown function keeps its catchable typed guard for focused
inspection; normal C++ destructors cannot propagate it.

Joystick `direction_mode` is canonical +B14 storage initialized to zero, as shown
by A99998. The actual setter receives references to existing effects, effect kind,
direction mode and activity deadline, and uses the same clock service as polling.

Correction from this composition to XINPUT_DEVICE.md and JOYSTICK_INPUT.md: their
earlier empty-force-tree boundary is closed for typed devices by the shared state
and recovered request pipeline. Native object layout, full ABI and gameplay
remain unvalidated. No new native function count is claimed for composition.

The saved Ghidra bodies at A95A80 and A991F0 still have short metadata boundaries
despite restored tail instructions. Source evidence includes the separately
inspected native tails; this change does not claim those metadata repairs finished.

Validation: strict MSVC Win32 build and both existing CTests passed. The existing
ignored XInput fixture was recompiled and extended with one composition scenario:
a request reaches the owned amplitude, right-motor storage, and recorded SDK call;
disable/clear update that same state, and a mismatched enable-flag binding is
rejected. All 131,072 signed16 cases still pass, with process exit 0. No hardware
calls were made. This does not test native purecall execution or live haptics.
