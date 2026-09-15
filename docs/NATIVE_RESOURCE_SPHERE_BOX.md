# Native sphere to bounds projection

Addresses: 00B7D160, 00B7D220.

| Routine | Coverage | Entry and return |
| --- | --- | --- |
| B7D160 | Complete, 191 bytes | ECX output six floats; EDX sphere four floats; EAX output; RET |
| B7D220 | Complete, 62 bytes | ECX output; sphere pointer at entry ESP+4; EAX output; RET4 |

The sphere is {center X, center Y, center Z, radius}; the output is minimum
XYZ followed by maximum XYZ. The raw leaf computes all components before writing
the output, preserving overlap with the sphere. It retains center values on the
x87 stack, rounds maxima through float temporaries, and reloads radius through
x87 float stores before subtracting it. Minimum output copies use FLD/FSTP;
maximum copies use MOVSS. The wrapper computes into its own six-float stack
temporary, then copies all six output components with ordered FLD/FSTP pairs.

`native_resource_sphere_box.cpp` spells out the complete Win32 instruction
schedule in MSVC inline assembly. No FP mode change, null guard, radius clamp,
allocation or unresolved provider is introduced. The caller's x87 state is an
input. Replacing this schedule with center +/- radius expressions or memcpy
would change intermediate conversions and potentially signaling-NaN, denormal,
status-flag and unmasked-exception behavior. Exact exception context identity
between different executable addresses is outside this packet.

The wrapper's C++ declaration uses an ignored EDX parameter so __fastcall calls
put its actual sphere argument on the stack and retain native RET4 cleanup.
The leaf uses the native ECX/EDX arrangement directly. This establishes explicit
entry contracts, not a game installation or drop-in binary replacement claim.

The existing `sphere_to_box_00b7d160` in `structured_hierarchy.cpp` remains a
typed fragment. Its shorter x87 schedule and C++ array copy do not substitute
for these raw bodies. No typed caller is rerouted as part of this packet.

All current inbound direct calls were inspected: B7D22E calls the leaf, and
B7EDF6 in B7EB90 calls the wrapper after reading a sphere at item+5Ch. The latter
copies the returned bounds into item+6Ch. Live containing-function queries and
complete PE/live byte comparisons accompany the report. Existing descriptive
Ghidra names are correct and retained; this packet makes no Ghidra writes.

The full strict Win32 Release build and both existing CTests passed. The current
object's complete 191-byte leaf equals the installed/live original byte for
byte. Its 62-byte wrapper equals the original except for the sole four-byte
CALL relocation, whose COFF target is the reconstructed leaf. The sections
have exactly those lengths. Current compiler command/read/write records bind
the source and header to this object, and its unique current archive member
matches the object bytes. No new test or native/source execution was added.

Validation is recorded in `reports/native_resource_sphere_box.json`. The two
bodies do not complete B7EB90 or establish game/runtime behavior.
