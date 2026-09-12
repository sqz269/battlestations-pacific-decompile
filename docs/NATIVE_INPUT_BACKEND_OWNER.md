# Native input backend ownership

Packet `orch4_native_input_backend_owner_ac`, read-only against
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`.

The source owns actual native F8h storage and registers that same address through
the application's existing raw14h singleton manager. It does not publish an
`InputFocusBackendState` pointer or create a parallel semantic domain. Required
raw device providers remain explicit: this is not yet a complete application
input runtime. `reports/native_input_backend_owner.json` carries the CALL audit,
full bounds, old/proposed names, provider contracts and validation receipts.

## Recovered entries

All rows implement complete normal control flow over raw storage with the
required providers described below. C++ exception cleanup follows the decoded
native state maps within the stated source allocation/provider domain. These
interfaces add a context argument and are not binary ABI replacements.

| Native entry | Inclusive end | Native receiver / cleanup | Source behavior |
|---|---|---|---|
| A908A0 | A90930 | ECX raw owner; RET; EAX=this | D5B5F4; capture section, publish F8BBF4, second manager/register |
| A90940 | A909D8 | ECX raw owner; RET | D5B5F4; capture section, second manager/unregister current publication; clear it; CE3818 |
| A909E0 | A909FD | ECX owner, stack flags; RET4 | Base destruction then free iff flags bit0; return captured address |
| A914C0 | A914FA | ECX raw24h class header; RET; EAX=this | Six zero stores; preserve remaining words |
| A91570 | A91619 | ECX raw owner; RET; EAX=this | Base construction, D5B5F8, three forward class constructors, slots/flags/counts |
| A90DC0 | A90DFA | ECX raw24h class header; RET | Free second buffer then first; clear six words |
| A90E00 | A90EBB | ECX raw owner; RET | D5B5F8; 3x8 reset/reload/ref-release; reverse class destruction; base destruction |
| A91150 | A9116D | ECX owner, stack flags; RET4 | Groups destruction then conditional free; return captured address |
| A982D0 | A983B0 | ECX actual F8h; RET; EAX=this | Groups construction, D5B72C, real DI8 Create/AddRef, required enumeration, four raw240h XInput constructions/attachments |
| A97C00 | A97C56 | ECX owner, stack flags; RET4 | D5B72C; GUID buffer cleanup, groups destruction, conditional owner free |
| A97B00 | A97B29 | ECX actual10h GUID header at backend+E4; RET | Free header+4, zero +4/+8/+C; preserve +0 |

`create_native_input_backend` is a host allocation wrapper, not another native
entry claim. It uses the existing malloc/free allocation domain for exactly
F8h, calls A982D0, and frees its captured allocation if construction throws.
Scalar flags1 requires that same allocation domain. Flags0 supports borrowed
native storage. No destructor calls DirectInput `Release`.

## Storage and publication

Application instructions 0073DD6C/71 allocate F8h; 0073DD8C/8E pass the result
in ECX. Base A908A0 publishes exactly that address at A908F6. The raw manager
obtained before registration is borrowed through `SoundLifetimeAccess`; the
source explicitly rejects its semantic-domain alternative. The captured first
manager section is retained even if a callback changes publication. A second
getter is evaluated before reading the current F8BBF4 registration argument.
Destruction similarly unregisters the current publication, not a cached `this`.

| Native fields | Producer behavior |
|---|---|
| +0 | D5B5F4 -> D5B5F8 -> D5B72C, unwind/destruction ultimately CE3818 |
| +04..+60 | 24 owning device pointers, all zeroed in class/slot order |
| +64 | Zero byte; +65..67 untouched |
| +68 + class*24h | Requested active count, zeroed after the three class constructors |
| Each class +08/+0C/+10, +18/+1C/+20 | Zeroed by A914C0; buffers freed in reverse member order by A90DC0 |
| Each class +04/+14 | Untouched native leading header words |
| +D4, +DC, +DD | Zero bytes; +D5..D7 and +DE..DF untouched |
| +D8 | Zero callback word |
| +E0 | Zero before actual DirectInput8Create output; remains after native destruction |
| +E4 | Untouched leading GUID-vector word |
| +E8/+EC/+F0 | GUID buffer pointers zeroed before enumeration and during destruction |
| +F4 | Zero byte after GetModuleHandleA, before DirectInput8Create; trailing padding untouched |

Device +4 really is an intrusive count: A95D9F writes1 in the common gamepad
constructor called by A9A5A0; A9A2C1 does the same in the mouse producer.
A90E00 calls reset on the current slot, reloads that slot at A90E53, decrements
the reloaded device's actual LONG at +4, then calls its current slot zero only
at zero. That virtual takes ECX only, with **no stack flags**. The owning slot
is cleared after the callback, including any replacement the callback wrote.
Reset may instead clear the slot, in which case no release occurs. The source
does not replace these operations with unconditional C++ deletion.

## Required services and actual SDK work

`NativeInputBackendDirectInput` executes actual `DirectInput8Create` using
version800h, SDK `IID_IDirectInput8A` and the exact owner+E0 output. Its actual
COM `AddRef` receives the captured interface; A982D0 reloads owner+E0 afterward.
The original HRESULT is ignored by the native control flow. The adapter retains
it for inspection and records a nonnull output even on error. A missing output
is an explicit source error before the original null dereference at AddRef.

The adapter reserves its recording capacity before each SDK acquisition and
does not AddRef for bookkeeping. `release_tracked_references` is an explicit
host-only reverse release after owners, devices and callbacks stop borrowing.
It neither follows stale owner pointers nor resets a freed owner's +E0. It
tracks only these Create/AddRef acquisitions: raw factory references and their
failure cleanup belong to the required providers. The existing typed factory
exception gap remains a boundary, not an ownership claim made by this module.

| Required provider | Verified native contract |
|---|---|
| EnumDevices slot10 | Actual interface, type0, callback A982B0, same raw backend context, flags1. A982B0 swaps its two stack arguments into A98030, RET8. Provider must contain/rethrow callback failures outside the SDK. |
| A9A5A0 | ECX supplied raw240h allocation; stack controller index0..3, RET4, EAX constructed pointer. Calls common A95D70 and stamps D5BB48. Provider owns its constructor-member cleanup. |
| A904E0 | ECX raw backend; stack requested slot then device, RET8. Query class at device slot8, scan eight columns for -1, write device+8 and the raw table slot. No retain. Other enumeration callers pass0 or GUID-derived index; interface preserves the signed slot input. |
| Device slot14 | ECX current raw device, no stack input. Known targets include A93E80 bare RET and A98400's actual three force-feedback reset calls. |
| Device slot0 | ECX captured raw device after actual +4 decrement reaches0; no stack input. Provider resolves the current native profile and performs its full zero-reference operation. |

No provider has a default successful implementation. Existing typed devices,
groups, factory contexts and reference vectors are not cast into raw layouts.
GamePlatformServices must eventually bind views of the same canonical owners.
The primary integrator owns raw singleton profile admission and application
wiring; this packet does not edit those shared dispatchers.

## Exceptions and flow qualifications

The FH3 maps were read directly: DEC87C/DEC8B0 contain captured guard cleanup
then root profile reset; DEC970 contains base destruction; DEC8E4 contains
reverse class destruction then base destruction; DECE7C contains current240h
allocation free, GUID-header destruction, and groups destruction in that order.

Registration failure releases the captured section and resets `this` to CE3818,
but leaves the native publication. The host allocation wrapper may subsequently
free the captured F8h; it does not silently repair the surviving publication.
After base construction, A914C0 is only raw stores and cannot throw a C++
exception in this source domain. Original hardware faults/FH3 spill aliases
are outside the interface.

On a reset/zero-reference callback exception, A90E00 stops the device walk and
unwinds class headers then the base. It does not clean up unvisited devices.
A982D0 restores state1 before attachment: a constructed but unattached device
is not newly freed when attachment throws. A9A5A0 failure instead frees that
current raw240h allocation before GUID/groups cleanup. A secondary cleanup
exception during source C++ unwinding terminates.

The primary repaired A90DC0 and A97C00 free fallthrough before this packet.
Additional live gaps contain `83 C4 04` (`ADD ESP,4`): A909F5..F7,
A91165..67 and A97B10..12. Their inclusive routine ends are listed above; the
source includes the continuation. EH helper CB6A36 ends with `59 C3` at
CB6A3F..40 after its free call. This worker made no Ghidra mutation. The
application's callback004B4630 is now a separately defined genuine RET; it is
not part of this packet's address claims or constructor body.

## Validation boundary

Build, eight native seeds, two existing CTests, mechanical CALL checks and the
single ignored manifested probe are recorded in the companion JSON report.
The probe uses explicitly synthetic raw device profiles to exercise nonempty
reference release, slot replacement and exceptional cleanup. Its SDK arm uses
actual DirectInput Create/AddRef but deliberately fails before enumeration.
No successful full raw-device startup, hardware enumeration, controller sample,
application execution, or game behavior is claimed.
