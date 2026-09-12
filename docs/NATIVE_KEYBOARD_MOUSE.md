# Raw keyboard and mouse devices

Addresses: 00A962F0, 00A9A3E0, 00A9A290, 00A9A470, 00A9A390,
00A95E60, 00A99EF0, 00A99E90, 00A9A4A0, 00A9A180, 00A95E70,
00A95E90, 00A99F70, 00A99FE0, 00A96350, 00A9A0F0, 00A9A140.

`native_keyboard_mouse.hpp/.cpp` adds raw variants alongside the existing typed
`input_enumeration`, `input_device_state` and `input_focus_reset` implementations.
The allocation bytes are canonical: no `InputDevice` projection or duplicate
reference count exists. These are source C++ entry points, not binary replacements.
Ghidra was read only in project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`; `bsp.py ghidra` verifies that target on every query.

| Routine | Native ABI | Coverage |
|---|---|---|
| A962F0 | ECX storage; EAX same storage; RET | complete |
| A9A3E0 / A9A290 | ECX storage; one stack DI8 pointer; EAX storage; RET4 | complete normal body and base-only C++ EH |
| A95E60 / A99EF0 | ECX storage; tail BD30F0; RET | complete |
| A9A470 / A9A390 | ECX storage; low flag byte in DWORD stack slot; EAX captured allocation; RET4 | complete |
| A96350 / A9A0F0 | ECX unused; EAX class 0 / 1; RET | complete |
| A99E90 | ECX storage; RET | complete, required current-profile query dispatch |
| A95E70 / A99F70 | ECX storage; stack code; AL result; RET4 | complete |
| A95E90 / A99FE0 | ECX storage; stack code; ST0 float result; RET4 | complete |
| A9A4A0 / A9A180 | ECX storage; ignored float stack argument; AL result; RET4 | complete |
| A9A140 | ECX mouse; stack flags; RET4 | complete |

## Allocation and deletion evidence

A98030 allocates exactly 310h at A98068..75 and 23Ch at A980A8..B5.
It reads the backend's live +E0 DirectInput pointer immediately before each
constructor (A98085..8E / A980C5..CE). Both constructors clean one argument with
RET4. Caller allocation failure and attachment remain the enumeration packet.

| Offset | Producer and retained bytes |
|---|---|
| +0 / +4 | sole raw profile / initial native refcount 1 |
| +8..+B | unwritten in both constructors |
| +C..+10B / +10C..+20B | two zeroed 256-byte histories |
| keyboard +20C..+30B | zeroed 256-byte sample |
| keyboard +30C / mouse +20C | actual SDK output slot; no preliminary null write |
| mouse +210 | only this valid byte becomes 0; +211..+213 retained |
| mouse +214..+227 | entire DIMOUSESTATE2 allocation preimage retained |
| mouse +228/+22C/+230 | wrapping accumulation words zeroed in reverse order |
| mouse +234/+235 | cooperation byte 0 after SetDataFormat; swap byte from real GetSystemMetrics(23) |
| mouse +236/+237 | unwritten padding |
| mouse +238 | real GetDoubleClickTime converted with signed FILD, conditional live 2^32 bias, live double divisor, FSTP float |

A962F0 stamps CEB130 then D5B6C4; final keyboard profile is D5B904 and mouse
profile D5B8B0. The data-format structures at D79804/D795FC match the SDK's
keyboard 100h / mouse2 14h formats; GUID bytes D78EFC/D78EEC match the SDK names.
Three memset calls in A962F0 share ADD ESP,24h; mouse's two share ADD ESP,18h.

Both device profiles' slot 0 is BD30E0: no arguments, null-safe, capture current
vtable, push 1, call slot 4, RET. Slot 4 is A9A470 / A9A390 and takes flags.
The keyboard scalar stamps D5B904, invokes A95E60, tests only flag bit 0, then
optionally frees the captured allocation. Mouse scalar stamps D5B638 and invokes
BD30F0 before the same flag test. A95E60 and A99EF0 are profile-reset tails,
not adjusting-this thunks: each writes D5B638 then jumps to BD30F0, which writes
CEB130. They perform no COM Release, Unacquire, refcount decrement or sample clear.
The common root/no-argument dispatch is owned by the primary integration packet.

Scalar flags 1 require storage from ordinary operator new; source operator delete
frees the same identity and the function returns its captured value. Flags 0/2
retain allocation storage. No second owning wrapper may free it again.

## Constructor failure

Mouse handler CB6AF8 loads FuncInfo DECF7C, whose map at DECF74 has one state:
0 -> -1, cleanup CB6AF0. Keyboard handler CB6B18 loads DECFA8, map DECFA0,
0 -> -1, cleanup CB6B10. Cleanup tails CB6AF3 -> A99EF0 and CB6B13 -> A95E60
reload ECX from [EBP-10h]. The state is active before the real CreateDevice call.
It covers subsequent SDK, platform and Win32 calls. No member cleanup releases
the returned COM device. Source catches reset only the base profile and rethrow.

At inspection, the two handler starts were missing functions. Bytes establish
CB6AF8..CB6B02 and CB6B18..CB6B22 (exclusive): MOV EAX,FuncInfo then a five-byte
JMP BF6B43 at CB6AFD / CB6B1D. Parent was notified; workers made no Ghidra writes.
Live flow inspection found A9A470 has 12 instructions, no gaps; ADD ESP,4 at
A9A48B is present and RET4 at A9A491 has length 3, end-exclusive A9A494.

`NativeInputDeviceSdk` forwards the actual output slot and tracks the genuine
returned reference. Its acquisition-bookkeeping allocation can throw before the
SDK call; this source boundary still unwinds the completed native base. Every
nonnull SDK output must denote its returned valid COM reference. Native null
dereference paths instead report an explicit source binding error. Callers must
supply readable preimages and enough raw storage; keyboard codes are unchecked.
The SDK tracker outlives raw deletion and all other borrowers; explicit host
`release_tracked_references()` follows those native operations. No release was
added to native destruction or constructor EH.

## Polling, callbacks and numeric behavior

Keyboard clears its sample before checking the COM pointer. A failed Poll calls
Acquire on the reloaded +30C pointer and returns false without history updates.
After successful Poll, GetDeviceState's result is ignored, each returned byte is
shifted right 7, then the current platform publication's +170 byte can clear the
sample. The inline history loop reloads the current profile for every query.

Mouse clears only valid+210 before its null test. Initial cooperation uses flags5
and marks +234 even on SDK failure. The later platform publication is reloaded,
its settings byte +2C cleared before real Win32 queries. Poll failure calls
Acquire and returns false. GetDeviceState success writes valid=1; failure leaves
the current valid byte alone after the SDK call. Both paths add the retained or
partially written sample with 32-bit wrap and update history, then return true.

A9A140 takes arbitrary flags; its sole direct caller BECBB7 supplies 6, whereas
the initial mouse-poll arm supplies 5 inline. Keyboard construction also uses 6.
These paths capture the COM vtable before BEC230, read slot34 from that captured
table afterward, and reload the member COM pointer for the this argument.
The source calls the existing concrete platform-window getter on the current
`Win32PlatformState` publication; no fake platform or cached window is created.

A99E90 performs previous-store, virtual query, current-store for each byte in
order. `NativeInputHistoryHost` is a required shared raw dispatcher; its query20
and value24 methods inspect the actual current profile on each call, including
recursive keyboard-value->query and mouse-axis-query->value calls. Other profile
providers belong to their packets; an unsupported profile is not mapped to zero.

Mouse button swap applies only to codes0/1. Axis codes8/9 use CVTSI2SS; wheel10
negates in integer width first, retaining INT_MIN wrap. Inversion reads the live
negative-zero word before SUBSS. Scaling is FLD live float / FDIV live double /
FMUL sample / FSTP float. Unknown codes also scale zero, retaining 0*Inf/NaN.
Axis down treats NaN as true (FUCOMIP/LAHF/TEST AH,44h/JNP). Keyboard and mouse
button float values read live D7A24C, not a substituted literal 1.

## Scope and validation

This packet does not implement the entire device vtable. Shared refcount/delete
dispatch, reset leaves and backend enumeration/attachment are separate packets.
Device naming, activity/selection slots and mouse additional property slots need
subsequent raw bindings; their typed counterparts remain unchanged. There
is no fallback InputDevice object and no full application input-startup claim.

Validation evidence is recorded in `reports/native_keyboard_mouse.json` and
ignored `local/native_keyboard_mouse_ad_*` logs: verified eight original-byte
seeds, Win32 build and existing CTests, CALL-row audit, and one manifested probe
using real DirectInput acquisition plus raw preimage/history/deletion checks.
The probe's null-platform exception tests source failure recovery after real
acquisition; it does not claim to inject a native DLL C++ exception or validate
focused hardware input delivery.

The final Win32 build passed both existing CTests; all eight seeds matched and
the CALL audit passed 21 direct/tail rows with zero failures. The real SDK probe
passed with three CreateDevice calls, three raw frees, zero references after
explicit release, 256 interleaved history calls and one real shared BD30E0
zero-ref dispatch. Both real hardware polls returned false. Initial build and
probe logs are preserved separately; the final log uses the shared root source.
