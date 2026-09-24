# B107F0 post664 continuation (CC10)

This is only `[00B1297A,00B12C9A)`: 800 bytes, 187 complete instructions,
36 direct caller calls. The last instruction is the five-byte `CALL B4E2B0`
at B12C95 (inclusive last byte B12C99). B12C9A `PUSH 20h` belongs to the next
allocation and is excluded. The containing original ABI remains ECX=service,
three borrowed DWORD argument cells and eventual RET0C. This fragment reads
none of those argument cells and implements no native function return.

`continue_native_render_resource_init_00b1297a_fragment` consumes the exact
completed post65C state once. The state checks the predecessor phase/site,
original entry, argument cells and unchanged continuation-context object,
plus each retained successor identity back to the entry. Its independent
post20 construction block and seven raw name headers must remain alive with
all predecessor states, original providers and parameter backing storage
through dependent stages and callbacks. The predecessor header changes only
by adding one successor identity and two phases.

## Native construction and publication

The BF681B object request is exactly 20h. Its actual result becomes EDI and
ESP14, EH state 86, then EBX becomes 100h at B12997 **before either branch**.
On a non-null result, construct the raw `fakemotionblur.mshd` header from
D5E240; OR the full DWORD ESP10 mask with EBX, set state 87, and call the
existing genuine B4E470 constructor with count 3 and null optional input.
Its actual result, including null on the allocation-null path, is retained.

At B129D2 EBP becomes FFFFFFFF before the DWORD mask test and before publishing
the result to current service+664 at B129D9. Only after that store is the EH
state disarmed at B129DF. EDI remains the raw allocation; it is not reset.
If mask100 is set, capture the raw name data first, clear bit100 with a DWORD
operation, read current length+1 only for non-null data, obtain the actual
current string pool and return the captured bytes. The raw header stays stale.
No texture binding or additional parameter/default appears in this interval.

## Six borrowed parameter registrations

Each row constructs its own raw name before the indicated captures, then
loads the current post material through B4CBA0, registers a borrowed record,
captures name data before disarming, and performs the same conditional pool
return. Scalar B18B20 uses one DWORD; float2 B18B00 uses two; all use matrix0.
Receiver reads remain independent even if an earlier callback replaced +664.

| Literal | Source | Capture order after name | EH | Registration |
| --- | --- | --- | --- | --- |
| D5E230 cCameraVelocity | service+1A8, 2 DWORDs | source B12A26 before current664 B12A2D | 89 | B12A4D -> B18B00 |
| D5E220 cCameraSpeed | service+1B0, 1 DWORD | source B12A8F before current664 B12A9E | 90 | B12AB6 -> B18B20 |
| D5E214 cMinSpeed | service+238, 1 DWORD | current664 B12AF8 before source B12AFE | 91 | B12B1F -> B18B20 |
| D5E208 cMaxBlur | service+240, 1 DWORD | source B12B61 before current664 B12B68 | 92 | B12B88 -> B18B20 |
| D5E1FC cDivider | service+23C, 1 DWORD | source B12BCA before current664 B12BD9 | 93 | B12BF1 -> B18B20 |
| D5E430 cSceneColorSampleOffset | service+4, 2 DWORDs | current664 B12C33 before EBX=source B12C39 | 94 | B12C57 -> B18B00 |

D5E430 is the existing original continuation `parameter_names[0]` view. The
other six views are caller-supplied original literal bytes, not new globals.
Seven distinct NativeString headers represent native baseline length/data
spills 1A0/1A4, 1B0/1B4, 1C0/1C4, 1D0/1D4, 1DC/1E0, 1EC/1F0 and 1FC/200.
This correspondence does not claim physical native private-stack aliasing.

After the last name return, capture current service+1D4 at B12C88 **before**
current service+664 at B12C8E. Call the full existing B4E2B0 frame assignment
at B12C95 using the actual entry/current profiles/imports and increment cell.
Its incoming publication/retain and old-owner release remain authoritative.

## Providers and retained frontier

All callees already exist: BF681B object allocation, 41E870 raw string
construction, 419CC0 current pool, BD1510 return, canonical B4E470 post20,
B4CBA0 material getter, B18B00/B18B20 borrowed records and B4E2B0 frame
assignment. No new callee, wrapper, generic companion, registry, owner counter
or nested-holder registration is introduced. The construction block retains
the true canonical post20 companion and its actual acquired children.

At B12C9A: ESI is the same service; the new state's EBX is service+4 from
B12C39; continuation EBP is FFFFFFFF; EDI and ESP14 are the raw +664 allocation;
mask is zero and EH state is -1. Original/aligned/half-dimension spills and
the three original borrowed argument cells are unchanged. Inherited EBX228
and EBP648 are admitted as the exact predecessor frontier, then replaced at
their native sites. There is no added retain for borrowed parameter storage.

Failures retain actual publication, raw name/mask state, latest capture and
provider acquisitions, and fail the consumed chain. This source fragment
does not add rollback, free or retry; provider failure behavior remains its
own contract. A null allocation follows the native null publication and
subsequent current-field use rather than gaining a new early-return policy.
External quiescence requirements for block reset remain in force. Existing
bloom+430..43F extent and distortion cleanup-preimage admission limits remain.
No full initializer, service-wide lifetime/teardown, native FH3/SEH, binary ABI,
private-stack alias, composed runtime or game-validation claim is made.

## Evidence and validation

Read-only Ghidra wrappers verified `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`. All 800 live caller bytes match the installed PE,
SHA256 `f3c8d2b5afee26f6cd3f04c8224d7ecff3757657e7fb113829f745cab70a7bcb`.
The 88-byte new literal region D5E1FC..D5E253 also matches the PE, SHA256
`bbafaf195f5d4674ba9fcc50019db31386d20bdffaa39c60818ecf53808f3964`.
Full listing, raw batch outputs and handoff remain under ignored
`local/render_init_post664_*` and `local/render_init_next_post4_*`; prior
packet artifacts remain unchanged. The JSON report contains all 36 exact
address/native caller rows: 36 checked, zero failed. `scripts/build.ps1` passed
MSVC Win32 Release `/MD /W4 /WX`, including the new source and all three
existing CTests (`reconstructed_math`, `native_math_differential`, `tool_tests`).
These checks establish static and build evidence only. No new test/probe was
added.
