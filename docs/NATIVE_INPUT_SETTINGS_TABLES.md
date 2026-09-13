# Native settings table parser

Addresses: 006A7BE0

`load_native_input_settings_tables_006a7be0` reconstructs the complete normal
7268-byte loader schedule over actual settings storage and tracked Lua objects.
It continues the script prefix documented in `NATIVE_INPUT_SETTINGS_STARTUP.md`
through table parsing and temporary-owner cleanup. The constructor prefix and
full settings singleton lifetime remain separate dependencies.

The original receives the settings allocation in ECX, takes no stack arguments,
and returns with RET. Its existing descriptive name is a hypothesis, not a
recovered symbol. The new C++ interface has explicit services and is not binary
ABI compatible with that entry.

## Data and call sequence

The existing prefix sets byte +4 before opening the persistent Lua owner at
+78 for ControlPresets.lua. It then opens a distinct temporary owner for
KeyboardSetup.lua, including the nested Inputs.lua request. A set byte +4 skips
all work. The complete parser retains this guard on failure and never changes
byte +5.

The parser destroys the existing device-tree subtree, resets its head links and
count, and erases the device-order vector through the original checked iterator
contract. It then consumes the temporary owner's globals in this order:

| Input | Actual settings output |
| --- | --- |
| KeyboardSetup | Device tree +08 and device-name order +14 |
| InputNames | String-keyed descriptor tree +24; writes descriptor words +00 and +0C |
| Conflicts.Groups | Checked nested string vectors at +30 |
| Conflicts.Pairs | Checked nested DWORD vectors at +40; subtracts one from each Lua index |
| DEVINPUTS | Byte +50, true only for a Boolean value that converts to true |
| ControllerInputNames | String-keyed outer tree +60 with signed-integer/string inner maps |

Each device is the existing 0x84-byte native mapped record. Inputs create code
vectors, two default binding descriptors, two reverse bits and display order.
Bare string entries only append display order. Sensitivities create category
and code records, append order, and copy the live one constant with MOVSS.
BaseSensitivities populate signed-key nested float maps. AxisPairs append two
pooled names per checked row; optional Min1SensHacks inserts signed set keys.

Base-scale conversion preserves the float spill and x87 store/ordered-zero
comparison. Ordered zero is replaced by the live CF7FE8 float (1e-8 in the
captured image); other values retain their converted representation. Iterator
and element bounds checks keep the original pointer-capture order. Conflict
string destinations are captured before Lua string conversion; controller
strings are converted before the mapped lookup. String overwrites retain the
existing native resize, allocation and release rules.

ControllerInputNames.lua executes in the same temporary Lua owner, after
DEVINPUTS. Its path is 0x2B bytes plus NUL. The parser reuses the unbound
InputNames key/value objects for controller iteration. Normal cleanup releases
controllers, groups, DEVINPUTS, name value/key, names and KeyboardSetup in that
order, then closes the temporary owner. The persistent owner survives.

## Required providers and uncertainty

`NativeInputKeyboardLibrary` and `NativeInputSettingsTableCalls` operate on the
actual tree/vector allocations, mapped records and pooled string keys. Every
method is required. This change does not supply a production STL provider or
successful fallback. Existing concrete checked-DWORD append, signed-key float
subscript, pooled-string and Lua routines are reused.

Two explicit inputs represent private native stack preimages: the high three
bytes of the default descriptor flag word and the opaque checked-vector word.
Private stack addresses, arbitrary aliasing, malformed raw storage, allocation
failure through the original FH3 frames, and hardware-fault behavior are not
validated. C++ cleanup preserves the implemented normal lifetimes; no complete
original exception-unwind equivalence is claimed.

The full native constructor/destructor used by the comparison are fixture
boundaries. Their execution does not provide reconstructed source lifetime or
singleton publication. Application wiring and gameplay remain unvalidated.

## Verification

The strict MSVC Win32 build and both existing CTests pass. Eight seed spans match
disk and Ghidra; all 326 recorded parser CALL rows pass the live ownership and
callee audit. `reports/native_input_settings_tables.json` records the evidence
and final combined source/archive identity.

One ignored manifested Win32 differential fixture executes the full original
constructor and loader against the source constructor prefix plus this parser.
Both consume unmodified installed fundamentals and four datatable scripts,
using isolated file streams and actual Lua 5.1.1 interpreters. Both populate
fresh raw settings through captured native container routines; no preseeded
maps or successful placeholder calls are used. The fixture compares 61,260
words across populated outputs and guard replay, including tree links, counts,
colors and shape, vector sizes/capacities, defined descriptors, used reverse
bits, pooled string contents and float bits. Opaque words, unused padding and
private descriptor flag bytes are excluded. The captured full destructor then
closes the persistent Lua owner and releases all pooled strings in both runs.

The fixture uses 416 disk/live-equal native spans and a private relocated data
image. Sixteen library bodies require bounded disk continuations after a free
call; those complete byte ranges were checked live and recorded separately.
They are not claimed as repaired saved Ghidra function ownership. Error-only
library paths terminate the fixture rather than returning invented success.
The original FH3 handlers are not executed. No running game is accessed.
