# Generic unit tick phase

`00953CC0..00953D9F` already had a reconstruction in
`tick_element_overrides`; this packet corrects that implementation instead of
adding a second module. Its final virtual runs when byte `unit+520h` is **clear**.
The old C++ returned on that condition. `00953D84 JNZ 00953D9A` establishes the
opposite gate. The earlier correction in `TICK_ELEMENT_OVERRIDES.md:322` had not
reached the source. The state field is now `suppress_1d8_520h`, initially false,
and `notify_code_528h` starts at the native constructor value -9 instead of -1.

| Routine | ABI | Coverage |
| --- | --- | --- |
| `00953CC0..00953D9F` | thiscall, ECX = tick node at unit+310h, one float, `RET 4` at 00953D9D | Complete control sequence and local state operations through the existing four explicit host boundaries; no runtime binding or binary-compatible object layout |
| `0095DC40..0095DDF8` | thiscall, ECX = unit, one float, `RET 4` | Read-only dependency assessment; not implemented by this packet |
| `006D1F20..006D1F22` | `RET 4`, one instruction | Original empty virtual body identified; no Ghidra function yet and no new implementation |

The existing public entry remains
`unit_tick_advance_sim_00953cc0(UnitTickAdvanceState&, float, UnitTickAdvanceHost&)`.
The former `unit_virtual_5c_notify` callback is now
`unit_virtual_5c_is_kind_of`: the actual leaf callees implement the class query.
The callback remains void because this caller discards its result. No other
current source or test implements this interface. Runtime integration remains
with the primary orchestrator; this packet does not change GameUnitsHost.

## Complete sequence and call contracts

ESI receives the node from ECX at `00953CC2`; EDI receives `node-310h` at
`00953CF1`. All unit virtual receivers are that unit, not the embedded node.

| Site | Target | Arguments and result |
| --- | --- | --- |
| `00953CDF` | unit vtable +5Ch | Query class 9 only if signed unit+528h >= 0; result discarded |
| `00953CFD` | unit vtable +1F0h | Unconditional unit call with float step |
| `00953D6E` | `00927F10` | One role dword, skipped for role 8; stdcall `RET 4`, no unit argument; false clears unit+520h |
| `00953D98` | unit vtable +1D8h | Float step, reached only with unit+520h clear |

Between the second and third calls, nonzero unit+634h is cleared if byte
unit+61h is clear. Each positive timer at +6F8h/+6FCh loses step, with no clamp
when it crosses zero. The original uses COMISS for the positive tests and x87
for subtraction. The fixture covers ordinary finite arithmetic; it does not
establish floating-point exception/status behavior for arbitrary inputs.

`00927F10..00927F26` loads the game global, then participant pointer
`game[18CCh + role*4]`, and returns its byte +9. Its ECX is overwritten by the
global load; the caller's incidental unit ECX is not an argument. An assigned
role therefore requires the existing real participant owner and availability
contract. Current role 0 must come from the canonical current-role owner
documented by `SHIP_AI_ROLE_OWNERS.md` and packet K, not a policy role array or
a second owner. Role 8 short-circuits this dereference.

## Producers and actual leaf targets

The common constructor `0095CC90` establishes these native fields. ESI is the
unit receiver, EBX is zero from `0095CCE0`, and XMM0 is zero from `0095CD7D`.
Their writes were checked in the complete saved listing, including intervening
register writes.

| Store | Native destination | Value |
| --- | --- | --- |
| `0095CDC1` | unit+528h | -9 |
| `0095CDD7` | byte unit+520h | 0 |
| `0095CE0B` | unit+634h | 0 |
| `0095CF50`, `0095CF58` | unit+6F8h, unit+6FCh | 0.0f |
| `00928630` constructor role fill | current role dwords +1ACh..+1CCh | 8 |

The direct producer and lifetime of byte +61h remain unresolved here. Its
existing semantic default is not evidence that every native unit has a clear
byte. The host must supply a valid current field value. The state is a semantic
projection with bool fields, not raw native storage; arbitrary nonzero byte
representations, aliasing through callbacks, and synchronized live-field
publication are outside the fixture's proof.

Constructor stores and original primary vtable words identify the three
generic leaf classes from the prior motion dispatch assessment:

| Leaf | Primary table / store | +5Ch | +1D8h | +1F0h |
| --- | --- | --- | --- | --- |
| LandVehicle | 00CFFDE0 / 0074DCCF | 0074DD90 | 006D1F20 | 0095DC40 |
| LandFort | 00CFF3F8 / 0074597D | 006F5890 | 006D1F20 | 0095DC40 |
| CommandBuilding | 00CFB028 / 006F564E | 006F58E0 | 006D1F20 | 0095DC40 |

The three +5Ch bodies are complete class/ancestor boolean queries with `RET 4`;
they do not send a notification. The +1D8h target is exactly `C2 04 00`, inclusive
end `006D1F22`, exclusive end `006D1F23`. Root independently verified this and
will define it under the Ghidra write lock during integration.

The shared +1F0h target is a real update, not an empty boundary. At `0095DC4B`
it calls `00927F30(unit, 4)`. That predicate accepts a local-player index only
in 0..7 and compares it to current role 4. Consequently role 4 == 8 guarantees
false for every possible local-index dword. The false arm at `0095DD71` stores
1.0f at unit+63Ch and returns without input/settings reads. A later binding can
use this established arm only with the actual role and +63Ch owner. The true
arm reads the input manager, mission clock and settings and updates +640h,
+644h and +63Ch; it remains unreconstructed. No local-player default or input
manager substitute is introduced here.

## Verification and limits

The ignored `local/make_generic_motion_probe.py` compared all 224 original
bytes to the live Ghidra program and generated the executable byte array.
Original executable SHA256:
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Routine SHA256:
`01aa42ffb00572401036c480701089492fda026d2b2ff8368c13a46fafac3265`.

`local/generic_motion_probe.cpp` executes the full original body and the rebuilt
C++ helper. It relocates only the direct role call. Its three callable virtual
slots and role callback are explicit observation seams, not a complete native
vtable or implementations of the original callees. Four cases cover constructor
defaults, suppression with role 8, unavailable assigned role clearing the gate,
and available assigned role retaining it while query/timer/flag operations run.
Both sides agree on nine callback events and all 7,424 compared bytes, with
zero mismatches and matching ESP before/after the original `RET 4` call.
No bytes are masked or normalized. The rest of each 740h-byte fixture is A5.
This proves the caller sequence for those cases, not real participant, input,
settings, virtual body, or game execution.

The MSVC Win32 Release build and both existing CTests passed. No tracked tests
were added. Source, binary, fixture, byte evidence and logs are hashed in
`local/generic_motion_artifact_manifest.json`. Ghidra was read-only. The existing
function name is preserved, with corrected gate/ABI evidence appended in the
name ledger; the reconstruction ledger records the complete sequence and its
explicit runtime boundaries.
