# Native gamepad and XInput storage

This packet reconstructs the actual 220h common gamepad prefix and 240h XInput
allocation in `native_gamepad_xinput.hpp/.cpp`. It preserves the existing typed
`GamepadInputDevice`, force-request and `XInputDevice` implementations. Those
types are not valid raw receivers. Names are hypotheses, not recovered symbols.

| Entry (inclusive end) | Coverage | Original calling contract |
| --- | --- | --- |
| A95D70–A95E32 | Complete control flow, required raw dispatch | ECX actual220h prefix; RET; EAX=this |
| A95A80–A95BC4 | Complete normal and member-unwind source paths | ECX prefix; RET |
| A95E40–A95E5D | Complete, including qualified internal three-byte gap | ECX prefix; flags stack; RET4; EAX captured pointer |
| A954C0–A95672 | Complete raw nonempty force pump, required request/device dispatch | ECX prefix; float seconds stack; RET4 |
| A95890–A9595C | Complete raw nonempty clear | ECX prefix; RET |
| A95BD0–A95BD5 | Complete | ECX unconsumed; EAX2; RET |
| A9A5A0–A9A5E3 | Complete actual240h constructor | ECX owner; index stack; RET4; EAX=this |
| A9A600–A9A60A | Complete profile store and tail call | ECX owner; tail A95A80 |
| A9A7C0–A9A7E3 | Complete scalar destructor | ECX owner; flags stack; RET4; EAX captured pointer |
| A9A5F0–A9A5F2 | Complete | ECX unconsumed; EAX0; RET |
| A9A610–A9A659 | Complete current-slot query | ECX owner; code stack; RET4; EAX0/1 |
| A9A660–A9A78F | Complete value query | ECX owner; code stack; RET4; x87 ST0 after float spill |
| A9A790–A9A7B1 | Complete SDK stop | ECX owner; RET; EAX SDK result |
| A9A7F0–A9A9B7 | Complete SDK poll and x87 normalization | ECX owner; unused float stack; RET4; AL result |
| A9A9C0–A9AA3A | Complete motor conversion | ECX owner; channel,float stack; RET8 |
| A9AA40–A9AAA5 | Complete borrowed-table name assignment | ECX unconsumed; actual8h output,code stack; RET8; EAX output |

Source contexts change these ABIs. The packet makes no original-vtable/FH3/SEH
binary compatibility or game-validation claim. Every owned entry has a known
start; the JSON report carries every direct CALL and the A9A606 tail JMP.

A982D0 pushes 240h at A98360, allocates through BF681B at A98365 and supplies
that same EAX as ECX to A9A5A0 at A9837D, with EDI index0..3. A98390 attaches
the returned pointer. The constructor calls A95D70 on the same address. Its
actual +4 reference count is initialized to1. Assigned slot+8 remains untouched.
Two 100h history buffers at +C and +10C are zeroed. The raw tree uses allocator
word+20C (untouched), sentinel+210 and count+214. Amplitudes are +218/+21C.
XInput adds index+220, untouched+224, the actual 10h XINPUT_STATE at+228,
four-byte XINPUT_VIBRATION at+238, connected byte+23C and untouched padding.
No shadow vectors, maps, device wrappers or private singleton domain exist.

The tree node is18h: left/parent/right at0/4/8, unsigned key+C, request pointer
+10, color+14 and nil byte+15. The recognized STL instances A94230/A94730/
A948C0/A94E90/A95330/A95A40 reuse existing raw services 869A20/86AA60/86AC00/
86E8A0/86EE50/86FDE0. Normalized listings match exactly for the four complete
leaf/range pairs; erase's common212-instruction prefix differs only in the FH3
handler address. Its remaining19 instructions and the destructor's remaining7
instructions were checked from aligned disk bytes against existing source.
These remain library contracts; this packet does not port STL/CRT code.

The pump snapshots both amplitudes, clears live values, and advances its raw
iterator before request update. Each subsequent virtual operation reloads the
request pointer from the original node. Expiration deletes the current payload
before erasing that node; a two-child erase keeps the captured successor valid.
Survivors call channel before value. Values are spilled to binary32 before an
ordered x87 maximum comparison. The native channel index is unchecked.
Disabled feedback still ages and expires requests. Changed output includes
unordered comparisons and reloads the current device profile for each channel.

A95890 deletes payloads in unsigned-key order while nodes remain present, frees
the nodes, resets the sentinel and pumps zero against the current derived
profile. A95A80 instead first stamps D5B670. Its +38 word is the actual
BF698E purecall. The source invokes the real CRT `_purecall` when enabled old
amplitudes are nonzero/NaN; it does not substitute a derived output or a success
guard. The process/handler behavior belongs to the linked CRT domain.

The constructor's FH3 CB68B6/DECC3C map DECC2C and destructor's
CB6896/DECC08 map DECBF8 both unwind the completed tree first, then A93E70.
Tree unwind frees nodes/head without re-entering request destructors. A93E70
stamps D5B638 then calls the shared complete BD30F0 root reset. A95A80 moves to
state0 before its final tree destruction. Scalars free only after normal return,
using `singleton_lifetime_free`; exact-size producers use the same allocation
domain. No extra native reference decrement or SDK release is invented.

`NativeGamepadDispatch` requires real raw request slots00/04/08/0C/10 and current
device slots24/38. D0DB78/8C/A0 table words and constant request bodies
8724E0/872310/872320/872330/A93F90 confirm delete/channel/value/expired/update
contracts; both destructor callers pass flags1, the pump passes its caller's
float unchanged. Existing typed requests cannot supply these raw methods.
The raw request producer/insertion bridge remains unresolved and explicit.
Backend attachment, raw refcount-zero dispatch and history callers belong to
the application integration packets. No application startup completion is claimed.

`NativeXInputContext` borrows the caller-selected existing `XInputLibrary`, common
feedback byte, existing mask/name tables and actual mutable CRT mode DWORD.
C2F166/C2F16C are the XInput GetState/SetState import thunks. SDK writes target
the actual raw fields. Poll sets connected for every result except1167, returns
false on any nonzero result, and only then sends vibration/normalizes axes.
It preserves the multiplier on x87 through the first three conversions and
uses FIMUL for the fourth, calling the existing BF7420 converter with the live
mode address each time. Motor0 writes right+23A, motor1 left+238; conversion
multiplies by65535 before changing rounding mode, truncates, stores low16 and
restores the control word. It neither clamps nor takes absolute value.

Primary repaired/exported A95A80 through RET A95BC4, A94730 through RET4
A94762..64, A94E90 through RET0C A95134..36 and A95A40 through RET A95A73.
A95E40's internal A95E55..57 ADD ESP4 is a qualified aligned disk decode after
its _free call; the live function's outer RET4 boundary is A95E5B..5D. Existing
library providers still have excluded live tails86EB11..86EB46 and
86FE04..86FE13; their already reconstructed complete source is reused.
All Ghidra work by this worker was read-only after project/program verification.

Validation: eight seeds matched disk; Win32 Release build and both existing
CTests passed. `verify_report_calls.py` checked47 owned direct/tail rows and
three incoming caller rows with zero
failures; ten indirect rows retain their explicit slot contracts. The ignored
manifested `local/native_gamepad_probe.cpp` covers a three-node tree, callback
pointer replacement, two-child expiration erase, derived clear and request
exception unwind. It reports3 updates,1 expired deletion,2 survivors,3 total
payload deletes,4 force writes and zero nodes/head after teardown. An explicit
scripted SDK fixture checks actual output pointers and both conversion modes
at signed-axis/deadzone endpoints. This is not controller evidence. The real
installed 32-bit XInput1_3.dll separately returns160 for index4: pollfalse,
connected1, stop160. No valid controller was driven or claimed tested. No new
permanent tests were added.
