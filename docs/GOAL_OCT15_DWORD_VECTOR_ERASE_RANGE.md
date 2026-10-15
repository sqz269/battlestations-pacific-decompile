# Actual checked DWORD vector range erase

Whole `004685A0..004685F9` is 90 bytes / 37 instructions, eight blocks and two
outgoing native transfers. The prior placeholder `STL_inst_004685a0` is retained
in evidence and the annotation prior-value log. The descriptive hypothesis
`STL_CheckedDwordVector_EraseRange_004685A0` identifies the established behavior,
without claiming a recovered symbol or nominal C++ container class. Existing
frontend/HUD/powerup projections remain separate; no prior indexed or unindexed
whole actual-storage Source was found.

Original ABI is ECX captured control and five stacked DWORDs: output, first_owner,
first_node, last_owner, last_node; RET14h and raw EAX output pointer. The ordinary
Source interface binds these live cells at offsets0/4/8/C/10 in
`actual_parameters20`. Its binding metadata is stable and disjoint from described
payload/cells. Only reached raw accesses need valid backing; an equal-node path
need not access control.end. Described argument/control/output aliases remain
possible. No implicit compiler/native private frame, argument home, return slot,
EH record, spill or asynchronous alias is added beyond the explicit cells.

Capture first_owner before diagnostics and retain it. If zero, do not read
last_owner; otherwise read current last_owner and compare it with captured owner.
`004685B3 CALL 00BF6713` reports on zero/mismatch and may return or throw. There
is no owner==control check or additional validation. Control also stays captured
before reporting; after any returning diagnostic read CURRENT first_node then CURRENT
last_node. If equal, skip current end+8 read, movement and end publication entirely.

If unequal, read CURRENT control.end+8, subtract captured last_node modulo32,
reproduce SAR2 bits and scale by4 modulo32. Capture first_node+bytes before the
SDK. Only signed-positive word counts reach the real SDK; zero/negative skip it
but STILL publish the wrapped captured result into captured control.end+8.
`004685DF CALL 00BF67A7` is the actual native SDK transfer; status is ignored.
The Source composes the accepted complete positive kernel
`move_native_scene_dword_range_00467d90(last_node,current_end,first_node)` with
the same raw arithmetic, current SDK and captured-result admission. This is a
Source implementation fact: the owned native body has no transfer to that helper.
No private argument-home coincidence is inferred from that ordinary interface.

Only after the movement/end step, read CURRENT output pointer from the explicit
parameter backing. Write output.node+4 from captured first_node BEFORE writing
output.owner+0 from the pre-diagnostic captured owner, then return captured output.
Both stores use captured values even if diagnostics, SDK movement or earlier
stores changed the original argument cells. A returning SDK may change the output
slot before its late read. No revalidation, rollback, ownership, allocation, pool,
destruction or local EH/cleanup scope is introduced.

Real providers are accepted `invoke_native_invalid_parameter_00bf6713` (its actual
five-zero-word shim and current SDK policy) and the positive raw movement kernel.
No callback, stub, nominal container or unresolved service is invented. Current
SDK/CRT handler, reporting, returning and throwing behavior is composed. Original
static CRT/global/errno/exception/register/stack/private-frame/fault/SEH/binary
identity and game behavior remain unproved.

Two bounded incoming setups were checked in full context:

| Native transfer | Pushed last_node,last_owner,first_node,first_owner,output | Control and cleanup |
| --- | --- | --- |
| `00468B1D CALL 004685A0` | EBX,ESI,EDI,ESI,private output address | ECX=ESI; child RET14h cleans the five words |
| `00468CBE CALL 004685A0` | EDI,ESI,EBX,ESI,private output address | ECX=ESI; child RET14h cleans the five words |

Other observed xrefs are not used to infer a nominal owner or stronger validity
domain. The preflight receipt is
`local/goal_oct15_scene_dword_erase_range_preflight_callers.json`.

## Verification

Whole native assembly/prototype, live dependencies and actual provider Source
were reviewed. All 90 live bytes match the installed PE. The two owned and two
selected incoming native transfer rows are separate from Source composition.
The primary independently reviewed every native instruction, the complete Source/
header and positive provider. Recorded Source/header hashes are asserted against
that review before recording it; no Source mismatch or edit was requested.
Locked annotation preserves prior name/comments/evidence, saves the project and
refreshes the affected export with readback recorded in the report. Shared CMake
registration was appended at this worker's queue turn, after removing only its
own queue request and immediately verifying the registry is unleased. No exclusive
registry lease was taken. Required
normal Win32 build, existing checks and focused compiler inspection are pending.
No new tests are added; packet-specific native differential/game and original ABI
remain unproved.
