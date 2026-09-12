# Raw DirectInput joystick

Addresses: 00A99940, 00A991F0, 00A99900, 00A99920, 00A98400, 00A98CC0,
00A98E30, 00A992F0, 00A98750, 00A98780, 00A98940, 00A98B90, 00A98BB0,
00A98BD0, 00A98C50, 00A99150, 00A99160, 00A992E0.

This packet reconstructs these bodies over one actual B48h allocation. The existing
typed `joystick_input` and `joystick_labels` records remain intact. The new raw
variants are separate ledger fragments; they do not reinterpret a typed joystick
as a native receiver. Public C++ names are hypotheses, not original symbols or
binary-compatible entry points. Ghidra access was read only in `bsp.gpr`, program
`/battlestationspacific.exe`, through the target-verifying BSP wrappers.

| Routine | Original ABI | Coverage |
| --- | --- | --- |
| A99940 constructor | ECX B48h; stack DI8*, instance*; EAX this; RET8 | Complete normal body; required actual base and services |
| A991F0 destructor | ECX this; RET | Complete normal body and declared C++ unwind |
| A99900 scalar destructor | ECX this; flags; EAX original; RET4 | Complete |
| A98E30 poll | ECX this; unread float; AL success; RET4 | Complete normal body; explicit stack preimage |
| A992F0 describe object | ECX this; instance*; EAX1; RET4 | Complete |
| A98B90 count / A99920 describe callback | stdcall instance*, raw this; RET8 | Complete normal body; host exception transport |
| A98400 reset feedback | ECX this; RET | Complete |
| A98CC0 force | ECX this; channel,float; RET8 | Complete normal body, both CRT conversion modes |
| A98750 relative / A98BD0 down | ECX this; code; AL; RET4 | Complete |
| A98C50 value | ECX this; code; ST0 float; RET4 | Complete |
| A98780/A98940 binding down/value | ECX this; 0Ch binding*; AL/ST0; RET4 | Complete |
| A98BB0 relative setter | ECX this; code,low byte; RET8 | Complete |
| A99150/A99160 metadata name ctor/dtor | ECX 1Ch row; RET; ctor EAX this | Complete |
| A992E0 product text | ECX this; EAX data or F8BC03; RET | Complete; missing native function definition reported |

The caller A98030 pushes B48h at A98235, allocates through BF681B, passes its current
manager+E0 and instance to A99940 at A98259, then attaches the same returned pointer
at A9826B. All allocations and scalar frees use the existing malloc/new-handler
domain. The constructor does not allocate a wrapper or maintain another owning
vector. `NativeJoystickStorage` contains bytes only; the caller owns its initial
preimage and allocation cleanup if construction throws.

The actual prefix is the runtime worker's 220h gamepad base. Derived state includes
device+220; eight-byte product string+224; counts+22C/+230; sixty used-slot bytes+234;
signed next axis+270; DIDATAFORMAT+274; validity byte+28C; 1Ch metadata array+290;
previous/current DWORD arrays+294/+298; ninety 0Ch bindings+29C; effect kind/mode at
B10/B14, force axes at B18/B1C/B20, effect pointers at B24/B28, inhibition/detection
at B2C/B30/B34, deadline+B38, Xbox byte+B3C, and triggers+B40/B44.

A99150 writes only the metadata row's eight-byte string. The constructor's later
loop clears byte+C; enumeration writes kind+8; successful GetProperty writes the
range flag, minimum+10, maximum+14 and wrapped span+18. Other bytes retain their
allocation preimage. Binding padding, object gaps, initial sample DWORDs, B34 and
first-poll B40/B44 are not defaulted. The additional native byte store at +6D4 is
preserved. Sample/metadata allocation uses native saturated size multiplication;
metadata has the actual preceding count cookie and reverse destruction order.

The two real SDK EnumObjects passes receive the exact raw receiver. A constructor
local TLS frame supplies strings and catches C++ exceptions at the stdcall callback;
it supports nested enumerations and adds no owner fields. After EnumObjects returns,
the captured exception is rethrown. There is no fabricated count-stability check.
Generated descriptors are the canonical +288 array, with null GUID, index*4, and
the SDK's current type/flags. The method for SetCooperativeLevel is captured before
the window getter, while its receiver is reloaded after the getter, matching assembly.

Poll clears validity, handles failed Poll by reloading the device and calling Acquire,
then copies current bytes to previous before GetDeviceState. The signed object scan
preserves deadzone and detection branches. Each virtual value query reloads the
profile, and the first absolute float above the live threshold updates the deadline.
The signed timestamp ratio is rounded to float before adding the live 60-second
constant. This scan precedes XInput; failed XInputGetState is not converted into an
exception or success default. A supplied 16-byte stack preimage is copied locally
before the real SDK call, then the native trigger bytes are consumed regardless of
HRESULT. That preimage is distinct from the raw object's B40/B44 bytes.

Force skips the clock for an absent initial effect, reads deadline after the clock
returns, and reloads the final effect pointer before SetParameters. Both deadzone
and force use the existing `native_crt_truncate_st0_00bf7420`, borrowing the live
0109EEA4 word at each conversion. Thus SSE2's double-spill/int32 conversion and the
BF7456 signed64-low-word fallback remain distinct. No C++ cast substitutes for them.
The product getter returns the actual data pointer or the caller's original live
F8BC03 empty-byte address; it never creates an empty string owner.

Constructor EH state0 cleans the base; state1 cleans product+224 then the base.
State2 frees the temporary metadata-cookie allocation while the nonthrowing A99150
array constructor runs. After member-array publication, later failures retain the
native arrays: the native EH map contains no derived destructor or array rollback.
The implementation preserves that allocation-retention boundary. This is not an
allocation-balanced constructor-failure claim.

Destructor order is previous, current, reverse metadata names and cookie, format,
effect Unload0/1, product, base. An effect-call exception skips subsequent effect calls
and still unwinds product/base. No device/effect Release is inserted. The shared
`NativeInputDeviceSdk` tracks each returned reference using the actual +220/B24/B28
output slots, including references acquired before a later constructor failure.
The host releases those references only after every native owner and callback has
finished borrowing. Scalar flags are tested after destruction, and free uses the
original allocation identity.

Primary repaired A991F0's previously excluded tail through RET A992DE (72 instructions,
zero gaps), CB6AD6's POP/RET tail through CB6AE0, and missing handlers CB6A66/CB6AE1.
The report records their exact bounds and unwind maps. A992E0 remains a requested
definition: final RET A992EF length1, end exclusive A992F0. All direct calls, tail
calls, caller attribution and SDK argument counts are recorded in the JSON report.

Verification: eight existing native seeds matched the installed binary. The strict
Win32 Release build and both existing CTests passed. One ignored manifested fixture
compared four metadata-constructor preimages, all 28 bytes each; 270 original-byte
query/down and x87 results; and eight original-force cases across both CRT modes,
including large magnitudes and direction negation. Four nonempty raw objects verified
descriptors, POV/axis mapping, padding, names4/4, polls4, state calls3, Acquire1,
explicit failed-XInput trigger bytes, and effect-pointer reload. Those poll/effect
calls use recording COM fixture objects. A separate phase called the actual system
DirectInput8/CreateDevice(SysKeyboard), tracked one returned reference and ended with
zero pending references. It does not establish physical joystick availability.

Full successful raw A99940/base teardown has not been fixture-tested here: the concrete
base A95D70/A95A80, current-profile dispatcher, current clock/window and live globals
are required parent composition. The ignored fixture leaves those base calls strict
and unreached. Raw control-name slot3C A99710 still has only the earlier typed label
path. This packet does not claim application or game integration, original FH3/SEH,
hardware-fault equivalence, or arbitrary mutation of saved native EH stack aliases.
