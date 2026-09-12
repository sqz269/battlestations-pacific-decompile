# Actual VFS manager container dependency map

Read-only discovery `orch2_manager_containers_ap`, based on `116987ab`, after
the six allocation leaves were integrated. **The full manager is not source
ready.** This pass identifies independent raw-container packets and preserves
the missing-instruction and existing-source boundaries.

The report is
[native_vfs_manager_containers_packet_map.json](../reports/native_vfs_manager_containers_packet_map.json).
All 27 bounded logical code spans and seven EH data maps were compared between
live Ghidra memory and the installed executable. Queries used the read-only BSP
CLI, whose target verification checks `bsp`, `/battlestationspacific.exe`, x86
language and image base against `config/target.json`; project file is
`C:/Users/sqz269/bsp.gpr`. Installed PE:
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`.
No original file, Ghidra definition/comment/flow, C++, test or ledger was changed.
Names below describe behavior and are hypotheses, not recovered symbols.

## Independent implementation packets

"Ready" means sufficient static evidence and existing source services for a
bounded implementation. It does not mean reconstructed, build-tested or ABI /
game-validated. Claim explicit addresses/files before beginning any packet.

| Packet | Exact entries / logical byte counts | Total | Dependencies and conditions |
| --- | --- | ---: | --- |
| `native_vfs_plain_list_destruction4` | BDAED0[72], 7F8310[72], BDB3C0[5], 7F8770[5] | **154** | Smallest independent packet. Only existing returning `singleton_lifetime_free`; four raw gaps captured. No payload destruction or pool dependency. |
| `native_vfs_pair_vector5` | BDB850[118], BDCC60[175], BDCD10[221], BDEC70[101], BE0350[23] | **638** | Actual string resize/destruction and pool services, existing allocator/free, library memcpy. Implement complete grow/shrink/copy and native EH schedule; no rollback of copied values/new backing on reserve failure. |
| `native_vfs_request_list_lifetime4` | 4D2640[29], BE1220[154], BE19E0[62], BE1D60[29] | **274** | Actual 4D05E0 string-list clear, actual string destruction, pool/free. 4D2640 is required by BE1220 unwind, not an optional wrapper. |
| `native_vfs_string_subtree2` | BDF7E0[82], 4CEC60[82] | **164** | Actual pool/free plus self-recursion. Coordinate 4CEC60's existing semantic ledger record before adding its actual raw body. |

Suggested disjoint module basenames are `native_vfs_plain_list_destruction`,
`native_vfs_pair_vector`, `native_vfs_request_list_lifetime` and
`native_vfs_string_subtree`, each with matching header/source/doc/report files.
Use existing source registration convention; do not lease the shared registry.

Pool-dependent packets can use `ActualNativeStringPoolStorage`, which repeats
the actual getter/return operations. Its inherited `release` is `noexcept`:
failure while lazily recreating the pool terminates. That existing source
boundary does **not** provide the original throwing-getter/EH identity. Preserve
the recovered cleanup states and document this boundary; do not invent a new
pool, cached publication, private handler or synthetic CRT globals.

## Root routines, registers and producer writes

| Entry / inclusive logical end | Bytes | Original ABI / evidence | Coverage |
| --- | ---: | --- | --- |
| BE1DC0-BE1F5A | 411 | ECX manager, ESI captures it at BE1DD8; EBX becomes zero at BE1DE4; EDI current member. EAX=this at BE1F4C, RET. | Complete static body and EH read; source absent |
| BE1F60-BE21E2 | 643 | ECX manager captured in ESI at BE1F7A; EBX zero at BE1F8D; EBP=manager+3C during mount cleanup; RET, no stable EAX result established. | Complete disk logical body read; live stored listing ends BE2028 |
| BDB970-BDBA04 | 149 | One stack C-string input, RET4; incoming ECX/EDX unused. ECX/EDX are built from two local 8-byte headers. No stable EAX result established. | Complete static body/EH read; actual BEE390 dependency missing |

All root CALL rows, including their containing function and callee ABI, are in
the report. The three words pushed before each 419CC0 call are **pending
arguments for BD1510**, not getter arguments: 419CC0 consumes none and returns
the pool in EAX; ECX receives it; BD1510 consumes `(block,length+1,1)` with
RET0C. Native length addition wraps at 32 bits.

BE1DC0 calls BDA6F0 before initializing the following storage. The base's actual
publication/lifetime/critical-section behavior remains a separate contract.

| Manager storage | Native producer sites / initial writes |
| --- | --- |
| +00 | BE1DE6 writes D685B4 after base construction |
| +04,+08,+0C,+10,+14 | BE1DEC, BE1DEF, BE1DF6, BE1DF9, BE1DFC write zero |
| +18,+1C,+20 | BE1DFF writes FFFFFFFF; BE1E06 zero DWORD; BE1E09 zero byte |
| +24,+28,+2C | BE1E16/19/1C zero DWORDs; no separate member cleanup established |
| +30 list | BDA960 call BE1E1F; head +34 at BE1E24; count +38 at BE1E27 |
| +3C tree | BDABF0 call BE1E34; head +40; byte head+21=1 at BE1E3C; self-link +4/+0/+8 at BE1E43/49/4E; count +44=0 |
| +48,+4C,+50 | BE1E54/57/5A zero native string-vector header |
| +54 tree | BDABA0 call BE1E67; head +58; byte head+1D=1 at BE1E6F; self-link +4/+0/+8 at BE1E76/7C/81; count +5C=0 |
| +60 list | BDA980 call BE1E91; head +64 at BE1E96; count +68 at BE1E99 |
| +6C tree | 4C26B0 call BE1EA6; head +70; byte head+15=1 at BE1EAE; self-link +4/+0/+8 at BE1EB5/BB/C0; count +74=0 |
| +78,+79 | BE1ED0 zero byte; BE1ED3 one byte |
| +7C list | 7F82F0 call BE1ED7; head +80 at BE1EDC; count +84 at BE1EDF |
| +88,+8C,+90,+94,+98,+9C | BE1EE2/E8/EE, BE1EF4/FA, BE1F00 zero DWORDs |

The allocator/comparator words +30/+3C/+54/+60/+6C/+7C and padding such as
+21..+23 and +7A..+7B are not initialized here. The allocation leaves themselves
write nonsentinel tree nodes; the owner performs the sentinel writes above.
BDABA0 is also the physical-directory index allocation producer.

The constructor's last six calls are BDB970 at BE1F10/1A/24/2E/38/42, passing
D68464/D68454/D68444/D68438/D68414/D68400 respectively. Every call consumes its
one argument with RET4. Their discarded outputs do not make these calls no-ops.

## Normal destruction and complete cleanup contract

BE1F60 first writes D685B4. Nonnull current 109CEE8 receives current virtual0(1)
at BE1FA1; its concrete object contract is not established here. It then walks
all mounts, calling nonnull node+18 providers through their current virtual4(1)
at BE1FFD. It does not test node+1C as an ownership flag. BD97E0 advances the
iterator, and BF6713's invalid-iterator wrapper can return to the caller.

Member destruction order is +94 pair vector, +7C plain list, +6C tree, +60
payload list, +54 tree, +48 string vector, +3C mount tree, +30 factory list,
+0C string, then BDA790. Each list captures next before destruction, resets
the live head links/count before its walk, and rereads the current head where
the original does. Plain-list cleanup frees nodes and head without deleting
payload pointers. Mount providers are deleted in the earlier virtual pass;
BDF7E0 later releases only the key string and node allocation.

The pair-vector layout is `(backing,count,capacity)`, 10h-byte elements containing
two native strings at +0/+8. BDCC60 initializes/copies each string separately,
reloading source and destination fields after resize callbacks, and copies with
**memcpy**, not strncpy. BDB850 releases second then first, retaining headers.
BDEC70 uses signed comparisons, zeroes all four DWORDs of newly admitted slots,
decrements the current count before each removed-value destruction, and stores
the requested count last. BDCD10 clamps requested capacity to at least one,
uses a wrapping DWORD `capacity<<4`, and publishes new backing/capacity only
after old values and backing are destroyed. Its raw CDD0-CDD9 continuation is
necessary to recover that publication and balanced register restoration.

## Exception cleanup

Verified FuncInfo/map pairs: BE1DC0 E0107C/E010A0; BE1F60 E010F0/E01114.
Each has ten states chained to the preceding state. Constructor actions use
`[EBP-10]`; destructor actions use `[EBP-18]`.

| State | Member | Cleanup | Constructor / destructor action |
| ---: | --- | --- | --- |
| 0 | base | BDA790 | CC6910 / CC6990 |
| 1 | +0C string | 41DD20 | CC6918 / CC6998 |
| 2 | +30 list | BDB3C0 -> BDAED0 | CC6923 / CC69A3 |
| 3 | +3C tree | BE16C0 | CC692E / CC69AE |
| 4 | +48 vector | existing actual 4D0FA0 | CC6939 / CC69B9 |
| 5 | +54 tree | BE1700 | CC6944 / CC69C4 |
| 6 | +60 list | BE1D60 | CC694F / CC69CF |
| 7 | +6C tree | 4D74A0 | CC695A / CC69DA |
| 8 | +7C list | 7F8770 -> 7F8310 | CC6965 / CC69E5 |
| 9 | +94 vector | BE0350 | CC6970 / CC69F0 |

BDB970's E0047C/E00464 map has state0 -> -1 releasing source at EBP-1C;
state1 -> 0 releasing output at EBP-14; state2 -> -1 releasing output at
EBP-14. Normal code uses state0 after source construction and state2 after
canonicalization. If canonicalization throws, only source is cleaned; after it
returns, a throwing source release cleans output without releasing source again.

BDB850 (E00440/E00438) arms first-string cleanup while destroying the second.
BDCC60 (E006A8/E006A0) arms first-string cleanup only after that string's copy
completes, while constructing the second. Both actions call 41DD20 on the
captured pair+0, leaving headers unchanged.

BDCD10 (E006D4/E006CC) has one state armed around the current copied element.
CC6270 computes the copied-index address, pushes it and the current destination
address, calls **401130**, then adds eight to ESP. The verified whole 401130
body is **RET**. This is not completed-element or new-backing rollback; adding
such RAII cleanup would change the native failure schedule.

BE1220 (E00ED0/E00EC0) destroys +14 list while state1 is armed. State1 calls
4D2640 on payload+8, then state0 releases the base string through 41DD20.
Normal code lowers to state0 before destroying +8 list and to -1 before
releasing the base string. The previously unnamed 4D2640 wrapper is therefore
a required dependency of the complete payload destructor.

## Required raw intervals and source gaps

All inspected entry addresses already exist as Ghidra functions. There are no
new undefined function starts established here. The following **inclusive
intervals** are required continuations missing from their stored listings;
they were read from matching installed bytes without repairing Ghidra.

| Function | Required raw intervals |
| --- | --- |
| BE1F60 | BE2029-BE21E2 (442 bytes) |
| BDAED0 | BDAEF8-BDAF02; BDAF0C-BDAF17 |
| 7F8310 | 7F8338-7F8342; 7F834C-7F8357 |
| BE1220 | BE1259-BE12B9 |
| BE19E0 | BE1A10-BE1A1A |
| BE1D60 / 4D2640 | BE1D71-BE1D7C / 4D2651-4D265C |
| BDCD10 / BE0350 | BDCDD0-BDCDD9 / BE0362-BE0366 |
| BE16C0 / BE1700 / 4D74A0 | BE16E4-BE16F3 / BE1724-BE1733 / 4D74C4-4D74D3 |
| BDF7A0 / BDF7E0 / 4CEC60 | BDF7CC-BDF7D6 / BDF821-BDF82B / 4CECA1-4CECAB |

Alignment at BDCD6D-6F and the single-byte padding at BE0CAF, BE0D7F and
4D1ACF are recorded separately, not misrepresented as missing control flow.
4CEC60's RET4 starts at 4CECAF; its inclusive byte end is **4CECB1**, making
the logical body 82 bytes.

BE0C30, BE0D00 and 4D1A50 are complete 201-byte range-erase routines, ECX tree
plus five stack words `(output,first-owner,first-node,last-owner,last-node)`,
RET14 and EAX output. Their full-range branches call BDF7A0, BDF7E0 or 4CEC60;
their partial branches require respectively BD9860/BDFD80, BD97E0/BE0080,
and 4BE730/4CF8A0. Existing BD9860/BD97E0 raw iterators are reusable.
The 4BE730 body and full removal/balancing/EH bodies of BDFD80-BDFFFC,
BE0080-BE0311 and 4CF8A0-4CFB31 remain unaudited. Saved `STL_xlen_throw`
names on two removal functions do not establish library contracts. Do not
implement only full-range destruction and label the range routines complete.

BDF7A0 additionally calls 489D50 on its two-string payload at node+0C. The
current traffic ledger labels 489D50 a fragment. Coordinate an actual raw
pair cleanup with the BDB850 packet before composing that subtree family.

4CEC60's ledger currently says `reconstructed_build_tested` in
`src/mission_load_hosts.cpp`; the cited source instead forwards host calls at
lines 162 and 251 and exposes no address-matched raw subtree implementation.
That semantic host coverage does not supply this manager's native cleanup.

BEE390's existing `canonicalize_resource_path_00bee390_fragment` rejects
embedded NUL/high-bit input and operates on `std::string`. Its actual 387-byte
body uses raw headers, a 256-byte stack/heap threshold, NUL-terminated scratch
copy followed by counted traversal, and signed-byte CRT `tolower`. The CRT
function has a locale-dependent branch. A full actual BEE390 implementation,
including output construction and successful-only heap release, is required
before composing the 149-byte BDB970 wrapper. The existing BEE690/BEE780 actual
normalizers are different operations and cannot replace it.

## Validation boundary

`verify_report_calls.py` checked **97 live direct-call rows, zero failures**.
The separate raw/indirect rows never claim to belong to Ghidra's stored body;
in particular BE2029-BE21E2 does not acquire a stored-function attribution from
its contiguous disk location. Tail thunks and EH jumps have distinct transfer
records. No build or tests were needed for this documentation-only discovery.
Full-manager source, binary ABI and game validation remain unclaimed.

## AQ parent integration, 2026-09-12

Correction from docs/NATIVE_VFS_SEQUENCE_LIFETIME.md: the four plain-list wrappers/lifetimes and five string-pair/vector lifetime bodies are reconstructed and bounded-fixture checked. Root review corrected the second string data-pointer read order. Manager tree ownership and the complete BE1F60 tail remain open; discovery readiness does not establish their independence. Three stored Ghidra sequence tails remain incomplete after supported flow repair.
