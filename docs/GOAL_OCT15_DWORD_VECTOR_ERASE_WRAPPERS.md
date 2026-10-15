# Actual checked DWORD erase wrappers

Both complete wrappers use the published actual `004685A0` range erase. Prior
placeholders `STL_inst_00468af0` and `STL_inst_00468c90` are retained in evidence
and annotation prior-value logs. Descriptive clear/embedded-clear names are
hypotheses, without a nominal C++ receiver/container class or recovered symbol.
No indexed/unindexed whole implementation existed; the C90 application projection
remains separate. No production hook, callback or interface is replaced.

| Entry | Inclusive native body | Bytes / instructions | Captured control |
| --- | --- | --- | --- |
| `00468AF0` | `00468AF0..00468B28` | 57 / 26 | ECX |
| `00468C90` | `00468C90..00468CC9` | 58 / 26 | ECX+8 modulo32 |

Total native coverage is 115 bytes / 52 instructions, five blocks and three
physical outgoing transfers per wrapper. Original ABI has ECX receiver, incoming
EDX unused, no stacked arguments, plain RET, and preserved EBX/ESI/EDI. EAX is
the actual child's selected current output-pointer result. It is not interpreted
as an owner/control result, nor promised always to select the initial scratch.

Each body captures initial control.end+8 before any diagnostic. Compare CURRENT
begin+4 unsigned with that captured end; report if begin>end. After any returning
report, capture FRESH begin+4 and compare it unsigned against FRESH current end+8;
report again if begin>end. Retain original end and that captured begin through
the second report. Do not reread either boundary before argument publication.
Both diagnostics may return or throw; no validity-only shortcut or added owner
check is supplied.

After those steps, write all five actual child DWORDs in native PUSH order:
captured initial end at child+10h, control at+Ch, captured begin at+8, control
at+4, then actual private output address at+0. Invoke the complete actual erase
Source with captured control and the live child parameter backing. Its captured
owner/current nodes/late CURRENT output and node-before-owner publication remain
composed, including returning diagnostics and SDK behavior. Return its actual
pointer result without replacing it by control or the initially supplied output.
Shared ordinary Source mechanics create no native inter-wrapper transfer.

`NativeSceneDwordEraseWrappersFrame` binds actual_output8 and actual_child_parameters20.
These buffers and binding metadata are stable/disjoint; described control/payload
aliases retain every ordered access/store. Only reached raw accesses require
valid backing. Output is not zero-initialized, snapshotted or owned by the wrapper.
Native entryESP=P has uninitialized output8 at P-8, a 12-byte saved-register gap,
child parameters20 at P-40 and the child return slot at P-44. The native diagnostic
temporary stack can overlap future child argument addresses. Those hidden helper/
register-save/frame coincidences, return/EH/spill/argument-home aliases and native
dangling-frame lifetime are outside this ordinary Source interface. The explicit
buffers are not equated with compiler stack addresses. No asynchronous mutation,
local EH/cleanup, allocation, ownership or rollback is introduced.

The only concrete providers are the accepted real BF6713 five-zero-word/current
SDK shim and the whole actual 685A0 range erase. No incomplete provider, invented
callback or stub is required. Current CRT/SDK policy is composed; original static
CRT/global/errno/exception/register/stack/fault/SEH/instruction/binary identity and
game behavior remain unproved.

| Original outgoing transfer | Role |
| --- | --- |
| `00468B00 CALL 00BF6713` | AF0 first returning diagnostic |
| `00468B0D CALL 00BF6713` | AF0 second returning diagnostic |
| `00468B1D CALL 004685A0` | AF0 actual erase |
| `00468CA1 CALL 00BF6713` | C90 first returning diagnostic |
| `00468CAE CALL 00BF6713` | C90 second returning diagnostic |
| `00468CBE CALL 004685A0` | C90 actual erase |

| Selected original incoming transfer | Continuation |
| --- | --- |
| `004F6105 CALL 00468AF0` | EAX is later set from ESI |
| `004D54A4 CALL 00468C90` | EAX is overwritten by a field load |

Both parent contexts were checked without inferring a semantic return type;
receipt: `local/goal_oct15_dword_erase_wrappers_parent_context.json`.

## Verification

Whole native ABI/listings, parent setups and actual providers were reviewed.
All 115 live bytes match the installed PE; eight actual native transfer rows
remain separate from ordinary Source composition. Locked naming preserves old
values/comments/evidence, saves the project and refreshes exports with readback
recorded in the report. Independent primary review of all 52 native operations
and the complete Source/header passed with the recorded SHA256 hashes asserted.
The primary will append unique CMake registration at its next available turn
and run the required combined normal Win32 build and existing checks; our
non-owning registration queue was removed at its request. Build and focused
compiler review remain pending. No new tests are added; packet-specific native differential/game and
original ABI remain unproved.
