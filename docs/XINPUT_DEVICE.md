# XInput controller device

Addresses: 00a9a5a0, 00a9a5f0, 00a9a600, 00a9a610, 00a9a660, 00a9a790, 00a9a7c0, 00a9a7f0, 00a9a9c0, 00a9aa40.

`XInputDevice` supplies the controller implementation allocated four times by
00A982D0. It uses real XINPUT1_3 ordinal2/3 through `XInputLibrary`, or an explicit
recording host in a fixture. `XInputFocusDeviceHost` composes with the existing
keyboard/mouse and joystick adapters; ordinary binding queries already accept
its canonical `InputStateDevice` base. No original game address is executed.

The native object is240h with vtableD5BB48, index+220, actual16-byte XINPUT_STATE
at+228, four-byte vibration at+238 and connection byte+23C. Constructor stores
clear state/vibration/connection. The field+224 is not consumed by this packet.
The common base's history arrays are reused. Its force-request tree is empty
on construction; this API does not expose arbitrary A954C0 force requests.
Construction/deletion are therefore explicitly typed projections over that
empty-base boundary, not full arbitrary native owner replacements.

| Address | Body and original ABI |
| --- | --- |
| A9A5A0 | Derived construction; ECX object, stack index, EAX object, RET4; empty common-base projection |
| A9A5F0 | Identifier always0; ECX ignored, EAX0, RET |
| A9A600 | Vtable then common-base destruction; ECX object, tail jump; empty-base projection |
| A9A610 | Button/activity query; ECX object, stack code, EAX bool, RET4 |
| A9A660 | Analog/button value; ECX object, stack code, ST0 float, RET4 |
| A9A790 | Clear vibration then actual SetState; ECX object, EAX SDK result, RET |
| A9A7C0 | Scalar deletion; ECX object, stack flag byte, original object in EAX, RET4; empty-base projection |
| A9A7F0 | Poll; ECX object, ignored float stack argument, AL bool, RET4 |
| A9A9C0 | Motor amplitude; ECX object, motor/value stack, RET8 |
| A9AA40 | Construct control label; output/code stack, ECX unused, EAX output, RET8 |

Poll passes the durable state directly to GetState. It always sets connection
to `result != 1167`, even on other errors, then returns false for every nonzero
result without clearing samples or sending vibration. Successful reads send
vibration next (zeroing it first when the actual globalE12F2C is false), ignore
SetState's result, and normalize all four stick components. Failed SDK writes
are not replaced with invented successful data.

Each signed16 component strictly between -6553 and6553 becomes zero; other
components subtract the signed cutoff, multiply by the exact double
327675/262144 atD5BB90, truncate, and retain the low signed16 result. The x87
multiply observes the caller's control word. Runtime0109EEA4 selects the
original double-spill/SSE2 conversion or an extended truncation path. The latter
uses hardware truncation for this bounded finite domain; it does not copy CRT
code or claim identical x87 exception-status side effects to the CRT fallback.
Both paths preserve the caller's control word.

Trigger codes12/13 divide bytes by255 with the native x87 float spill. Other
codes below16 test the current E12F40 mask table; its zero-extended WORD comparison
against -1 cannot take the apparent skip branch. Codes60..63 scale signed axes
by1/32768 and negate Y. Query threshold is the native double representation of
0.1f for codes below16, otherwise0.5, with equality accepted. Unknown codes
return zero. The common activity scan tests codes0..89 in order. +1C and slot
reset retain their already recovered base no-op/zero behavior.

Motor0 writes the RIGHT word and motor1 the LEFT. The native operation multiplies
the float by65535, switches x87 rounding to truncation, stores signedDWORD,
retains its low word and restores the control word. It neither clamps nor takes
an absolute value. Unsupported motor IDs do nothing. Destruction does not call
the separate stop-vibration helper. Label tables E12F60/E12FA0 and the fallback
`Unknown` use the real native narrow-string constructor, including overwrite
semantics for a previously live destination.

Validation: strict MSVC Win32 Release and both existing CTests passed. One ignored
fixture checked all65536 stick inputs in each conversion mode against an
independent integer/rational oracle at x87 precision53/round-nearest. It also
checked durable SDK buffers/order, thresholds, Y sign, motor ordering and wrap,
disabled vibration, preserved failed-read state and current labels. No actual
controller state or vibration call was made. Import ordinals and installed
XINPUT1_3 exports were checked; its SDK implementation was not reconstructed.

Seven missing entries were defined from reviewed native bytes; the function
bodies stop at their real terminators. The stop-vibration disassembly window
included two trailing INT3 bytes, but readback confirmed its actual body ends
atA9A7B1. One false-no-return continuation after free in A9A7C0 was repaired
without changing the callee flag. Descriptive names remain hypotheses.

Evidence: `reports/xinput_device.json`, `reports/xinput_function_definitions.json`,
`reports/xinput_flow_repairs.json`, `local/xinput-device-fixture.log` and
`local/xinput-device-build.log`. Remaining work includes shared gamepad force
requests/owner lifetime and executable input-owner binding. This is not original
ABI, live-hardware or gameplay validation.

## Correction from docs/GAMEPAD_INPUT_DEVICE.md

The typed joystick and XInput classes now share a GamepadInputDevice base with
one canonical force-request map and amplitude pair per device. The recovered
request pipeline drives the real motor/effect setters, and base teardown follows
derived cleanup. Enabled nonzero amplitudes at base destruction preserve the
native purecall failure boundary. This closes the earlier empty-tree projection
for typed objects; it does not establish native ABI or gameplay parity. Joystick
+B14 direction mode is initialized to zero at A99998. See GAMEPAD_FORCE_REQUESTS.md
and GAMEPAD_INPUT_DEVICE.md for the request and lifetime contracts.
