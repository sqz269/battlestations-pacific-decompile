# Segment 72 multiplayer, lane oct18b-mpA

Assigned list: coordinator `local/codex_mpA.txt`, 47 rows, `00A47EE0..00A4D4E2`.
One game constructor is reconstructed in `src/online_cc13.cpp` and declared in
`include/bsp/online_cc13.hpp`. The other 46 rows are DLL import jumps; the
individual PE/import and existing-interface evidence is in
`OCT18B_MPA_IMPORTS.md`. No mpB rows are included.

## 00A47EE0 BSP_XLiveSystem_Construct

New C++ entry: `bsp::online_cc13::system_construct_00a47ee0`.
Original ABI: ECX = native 2218h-byte object, no stack arguments, EAX = object,
plain RET. Ghidra prints `__fastcall`; assembly has no EDX input. Evidence:
`00A47EFB MOV ESI,ECX`, base call `00A47F02`, final `00A4831D MOV EAX,ESI`,
`00A4832C RET`. No x87 instructions appear in the 251-instruction export.
The MSVC SEH frame is compiler machinery, not game logic.

The C++ body reuses existing reconstructions of base constructor `00994080`,
player-list construction `00A47B50` (same two stores and allocator as the four
inlined sequences), task construction `00A42510` and `00A43070`, embedded
constructors `00A49CF0`, `00A4A380`, `00A48F10`, `00A499D0`, friend clearing
`00A43FA0`, communication privilege `00A3EB30`, and native counted strings
`00408720` / destruction `004072D0`. Existing PlayerListCalls,
XLiveVoiceCalls and XenonCalls remain the corresponding service boundaries.
No host wiring is added.

Construction follows the listing order, without clearing unrelated padding or
allocator words. Important placements:

| Listing | Native storage and operation |
|---|---|
| 00A47F02..00A48024 | base, vtable D24878, participant list +DCh, four player lists +F4h/+100h/+10Ch/+118h, mask list +168h, flags/vector words |
| 00A48035 / 00A48045 | matchmaking +184h, host QoS +1AE8h |
| 00A4804A..00A480B3 | pair of vptrs +1B9Ch, then 3 arrays of 4 task vptrs, stride 4, +1BA4h/+1BB4h/+1BC4h |
| 00A480E7..00A48127 | voice +1C24h, then 16 strings +1D5Ch, stride 1Ch |
| 00A48190 | 16 participant-pair lists +1F3Ch, stride Ch; ctor A446A0 / dtor 4C8BF0 |
| 00A481AF | 4 task-pair records +1FFCh, stride 18h, only the first 8 bytes initialized |
| 00A481CE | 4 lists +2064h, stride Ch; ctor A446E0 / dtor A44340 |
| 00A481DE..00A48226 | session +2098h, stats list +2200h, friend reset and communication privilege byte +84h |
| 00A4822B..00A48255 | eight unaligned dwords, +1C02h through +1C1Eh, exactly 20h bytes |
| 00A48275..00A482DC | twelve counted strings appended to vector +1F1Ch, using table D24734..D24760 |
| 00A482DE..00A48310 | first four strings at +1D5Ch get voice_mask_none (15 bytes), then zero +9Ch for 40h bytes |

Read-only PE evidence from the configured original executable (also corroborated
by Ghidra bytes for D24724..D24763): the ordered names are
`voice_mask_none`, `voice_mask_anonymous`, `player_sun`, `player_jed`,
`player_jenna`, `player_sarama`, `player_angul`, `player_machiko`, `player_jinx`,
`player_amran`, `player_roosta`, `player_vorsh`. The second name requires heap
storage; the local native string is destroyed on both successful push and a
throwing push. Its allocator word stays unspecified as in the native local.

The original float data are retained as exact words. All five sources and the
mask pointer table are in read-only `.rdata` (PE characteristics 40000040h):

| Source | Destination | Bits | Value |
|---|---|---|---|
| D7A260 | +88h | BF800000 | -1.0 |
| CE74F8 | +1D40h | 3F4CCCCD | 0.8 |
| CE3800 | +1F2Ch | 3F000000 | 0.5 |
| CE4E0C | +1F30h | 3F7D70A4 | 0.99 |
| CE3958 | +1F38h | 40000000 | 2.0 |

## Remaining boundaries and uncertainty

XLiveSystemConstructCalls binds the existing STL constructors/allocators,
CRT array iterator, vector append and singleton lookup. The iterator contract
must destroy a partially constructed prefix before throwing. Callback numbers
are original addresses; they are never cast to executable pointers here.

The original handler `00CB49F6` loads function-info address `00DEA740` and
jumps to the CRT frame handler. The PE function info has magic 19930522h,
26 unwind states and unwind map DEA764h. Each state points back to its preceding
state (state 0 to -1). The implementation tracks the listing's completed-member
state and requires the explicit `unwind_failed_construction_00cb49f6` adapter to
execute that cleanup graph. It does not call the handler as an ordinary C++
function. Local temporary cleanup is performed directly; the outer adapter
sees state 18h after it, rather than state 19h (which includes the native
stack temporary). Adapter state -1 has no completed member to destroy.

Names describe inferred roles. The object remains raw native storage; this is
a new C++ interface, not a binary-compatible constructor. The host adapters and
exception path have no runtime/ABI proof. No message-handle arithmetic is changed.
No game or multiplayer session has been run for this packet.

## Validation

`verify_report_calls.py local/oct18b_mpa_calls.json`: 26 exact CALL sites
checked against live Ghidra, zero failures. The Markdown report checker also
passed (its range warning denotes the intentionally noncontiguous worklist).
Ghidra project/program verification passed; the constructor evidence comment
was appended under the write lock, previous annotations were saved, the
project was saved and its export refreshed. A separate read-only review found
no offset, stride, cleanup-state or temporary-lifetime blocker.

`./scripts/build.ps1` passed for MSVC Win32 Release, including both existing
CTest checks (`reconstructed_math`, `tool_tests`), zero failures. No tests were
added. This proves compilation and the existing suite, not execution of this
unwired constructor or its native ABI/EH adapter.
