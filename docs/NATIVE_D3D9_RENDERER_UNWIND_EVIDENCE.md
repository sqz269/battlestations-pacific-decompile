# Native D3D9 renderer parent unwind evidence

This read-only packet recovers the exact FH3 state maps for constructor `00B32410` and destructor `00B32920`. It does not implement either parent or close the retained-failure boundary in `NativeSystemConstantRegistryOperation`.

Evidence is pinned to base `3c85892e` and immutable local captures. Every captured native span matched the active Ghidra program and installed PE: 11 spans / **6,978 bytes**. Configured analysis: `C:/Users/sqz269/bsp.gpr`, project `bsp`, program `/battlestationspacific.exe`, x86 at `00400000`. Each live `bsp.py ghidra bytes` call checks project name, program path, language and image base. No Ghidra mutation, C++ edit, test or build is part of this evidence packet.

## Native frames and metadata

| Parent | Full inclusive native body | Handler | FuncInfo | Unwind map |
|---|---|---|---|---|
| Constructor | B32410..B328F7 (1,256 bytes) | CBDEF5 | DF66EC | DF6710 |
| Destructor | B32920..B339C0 (4,257 bytes) | CBE07B | DF67F8 | DF681C |

Both handlers load the FuncInfo address into EAX and jump to `BF6B43` (existing label `FID_conflict:___CxxFrameHandler3`). Both FuncInfo records contain `19930522`, state count `1D`, the respective map pointer, five zero DWORDs, and final flags DWORD `1`. Thus each map has 29 `(signed next-state, action-address)` pairs; try-block and IP-map counts are zero.

Both entries take the actual renderer in ECX and use plain RET. The constructor returns that pointer in EAX. The funclet EBP is the FH3 establisher value corresponding to entry ESP; it is not the destructor body's freely reused EBP register. State is `[FH3 EBP-4h]`. Constructor root is captured at `[FH3 EBP-45Ch]`, equal to `[normal steady ESP+10h]`; destructor root is `[FH3 EBP-14h]`, also `[normal steady ESP+10h]`. Constructor allocation is `[FH3 EBP-460h]` / `[normal steady ESP+0Ch]`. Destructor nested-owner capture is `[FH3 EBP-10h]` / `[normal steady ESP+14h]`. Pushing one argument changes the displayed normal stack-state displacement by four bytes.

The earlier saved destructor ended at `B32D72`, immediately after a returning `_free` call. It omitted most member cleanup and the final base destructor. The immutable disk/live capture here reaches the actual `RET` at `B339C0` (1,255 instructions). The primary integrator repaired and refreshed Ghidra independently after this capture; the truncated body is never used as complete evidence.

## Shared states 0 through 24

For both parents, each row loads ECX from its root-capture slot, adds the listed offset (no add for zero), and tail-jumps to the target. State `s` transitions to `s-1`; state 0 transitions to -1. Unwind order is descending, beginning at the active state. The two funclet addresses differ because their captured-root stack slots differ.

| State | Root offset | Target | Constructor funclet | Destructor funclet | Next |
|---:|---:|---|---|---|---:|
| 0 | +0h | 00B284E0 | 00CBDD20 | 00CBDF00 | -1 |
| 1 | +1Ch | 008D4E60 | 00CBDD2B | 00CBDF08 | 0 |
| 2 | +28h | 0086AE00 | 00CBDD39 | 00CBDF13 | 1 |
| 3 | +19A0h | 00B27F50 | 00CBDD47 | 00CBDF1E | 2 |
| 4 | +19B0h | 00B29B20 | 00CBDD58 | 00CBDF2C | 3 |
| 5 | +19C4h | 00B29B40 | 00CBDD69 | 00CBDF3A | 4 |
| 6 | +19F4h | 00402F70 | 00CBDD7A | 00CBDF48 | 5 |
| 7 | +1A60h | 00B321D0 | 00CBDD8B | 00CBDF56 | 6 |
| 8 | +1A74h | 00B32370 | 00CBDD9C | 00CBDF64 | 7 |
| 9 | +1A98h | 00B32200 | 00CBDDAD | 00CBDF72 | 8 |
| 10 | +1AACh | 00B29B20 | 00CBDDBE | 00CBDF80 | 9 |
| 11 | +1AB8h | 00B29B40 | 00CBDDCF | 00CBDF8E | 10 |
| 12 | +1AC4h | 00B29B60 | 00CBDDE0 | 00CBDF9C | 11 |
| 13 | +1AD0h | 00B29B80 | 00CBDDF1 | 00CBDFAA | 12 |
| 14 | +1ADCh | 00B28090 | 00CBDE02 | 00CBDFB8 | 13 |
| 15 | +1AE8h | 00B280B0 | 00CBDE13 | 00CBDFC6 | 14 |
| 16 | +1AF4h | 00B280D0 | 00CBDE24 | 00CBDFD4 | 15 |
| 17 | +1B00h | 00737BF0 | 00CBDE35 | 00CBDFE2 | 16 |
| 18 | +1B0Ch | 00B280F0 | 00CBDE46 | 00CBDFF0 | 17 |
| 19 | +1B18h | 00B2F700 | 00CBDE57 | 00CBDFFE | 18 |
| 20 | +1CF4h | 00B29BA0 | 00CBDE68 | 00CBE00C | 19 |
| 21 | +1D00h | 00B29BE0 | 00CBDE79 | 00CBE01A | 20 |
| 22 | +1D0Ch | 00B29C20 | 00CBDE8A | 00CBE028 | 21 |
| 23 | +1D18h | 00B29C60 | 00CBDE9B | 00CBE036 | 22 |
| 24 | +1D2Ch | 00B5E2F0 | 00CBDEAC | 00CBE044 | 23 |

Three targets are five-byte tail thunks: `B321D0 -> B32030`, `B32200 -> B320F0`, and `B2F700 -> B2F690`. They do not change the receiver. Each byte string and ordered instruction sequence is included in the JSON report.

## Constructor allocation branches

States 25 through 28 each load EAX from `[FH3 EBP-460h]`, push it, call `BF65AC`, pop ECX to remove the argument, and RET. These are cdecl allocation frees, with no ECX owner receiver and no destructor call inside the funclet. Each next state is 24, so the chain is `allocation free -> 24 -> 23 -> ... -> 0 -> -1`.

| State | Bytes | Allocation call | Capture write | Arm write | Constructor call / target | Disarm to 24 | Funclet |
|---:|---:|---|---|---|---|---|---|
| 25 | 4CCh | 00B327E1 | 00B327E9 | 00B327EF | 00B327FB / 00B1BB90 | 00B32802 | 00CBDEBD |
| 26 | 2Ch | 00B3280A | 00B32812 | 00B32818 | 00B32824 / 00B585A0 | 00B3282B | 00CBDECB |
| 27 | 10h | 00B32833 | 00B3283B | 00B32841 | 00B3284D / 00B5BF70 | 00B32854 | 00CBDED9 |
| 28 | 24h | 00B328A2 | 00B328AA | 00B328B0 | 00B328BC / 00B33DA0 | 00B328C5 | 00CBDEE7 |

All four allocations call `BF681B` while state 24 is active. State 25/26/27/28 is armed only after the allocation returns and its pointer is captured. A null result still reaches the corresponding state write but skips the constructor call. A constructor exception first passes through that callee's own native unwind, then reaches the parent allocation free.

The successful Lua, shader-state and system-registry allocations share and overwrite one capture slot. Their returned pointers are discarded by this parent after publication through the child constructors. Returning to state 24 does **not** add a later destructor/free for those completed singleton owners. Parent-wide rollback of previously completed singleton owners would add behavior absent from this map. State 28 is disarmed before publishing the control worker at root+1970h (`B328CD`).

Constructor direct state writes are `0, 2, 19, 23, 24, 25, 24, 26, 24, 27, 24, 28, 24` at the exact IPs recorded in JSON. Intermediate map states still participate in unwind even when no direct state write appears between non-throwing initializations. Initial state -1 precedes `B283F0`; state 0 is written at `B32448` after the base returns.

## Destructor nested branches and normal disarming

| State | Funclet | ECX from capture | Absolute renderer offset | Target | Next |
|---:|---|---|---:|---|---:|
| 25 | CBE052 | `[FH3 EBP-10h]+44h` | +1B5Ch | B29E40 | 18 |
| 26 | CBE05D | `[FH3 EBP-10h]+4` | +1A9Ch | B317C0 | 8 |
| 27 | CBE068 | `[FH3 EBP-10h]+4` | +1A64h | B316A0 | 6 |
| 28 | CBE073 | `[FH3 EBP-14h]` | +0 | B33D90 | -1 |

- State 25: `B32E1C` captures root+1B18h. `B32E26` arms 25 before `B2AE20(root+1B68h,0)` at `B32E2B` and its current-data free at `B32E34`. `B32E41` lowers to 18 before destruction of the remaining DWORD vector at root+1B5Ch. The funclet destroys only that remaining vector, then continues with state 18; it does not repeat the entire state-19 owner destructor.
- State 26: `B3359A` captures root+1A98h. After profile D5F04C is written, `B335A6` arms 26 before `B31750` at `B335AB`. `B335B6` lowers to 8 before `B30410(root+1A9Ch,0)` at `B335BB` and its current-data free. The unwind target `B317C0` is exactly 23 bytes: resize zero via B30410, reload CURRENT data, free via BF6989, return without clearing pointer/capacity.
- State 27: `B335E1` captures root+1A60h. After profile D5F024 is written, `B335ED` arms 27 before `B31630` at `B335F2`. `B335FD` lowers to 6 before `B30270(root+1A64h,0)` at `B33602` and its current-data free. Thus failure of the already-disarmed resize/free does not repeat state 27.
- State 28: after writing base profiles D5E628 and D5E76C, `B33996` arms 28 before `B25FE0(root+0Ch)` at `B3399E`. `B339A5` stores EBP=-1 before the explicit `B33D90(root)` call at `B339A9`; this avoids repeating primary-base destruction. The ordinary body reestablishes EBP=-1 at B336C1, with B33945 restoring it after its later copy-loop scratch use.

State 24 is first armed at `B32955`. It remains active through the outer destructor work until `B32D33` lowers to 23 before `B5E2F0(root+1D2Ch)`. The body then lowers before successive member destruction. All state writes, including the three nested branches and final -1, are recorded in JSON. State 5 is only a metadata intermediate: the normal section destruction runs under state 6, then `B3363E` lowers directly to 4 before the next vector cleanup. This preserves the native sequence rather than inventing a write of 5.

## Source cleanup composition at the pinned revisions

| Route | Existing source and remaining boundary |
|---|---|
| Both state 0; destructor state 28 | `native_renderer_base_lifetime.cpp`: actual B284E0/B33D90 and actual +0Ch singleton route. B284E0 already protects primary cleanup while B25FE0 runs. Source adds explicit publication context. |
| Both state 7; destructor state 27 | `native_vertex_declaration_registry_lifetime.cpp`: actual B32030 and B316A0 compose through the B321D0 thunk. Existing actual owner/string contexts are required. |
| Both state 24 | `native_renderer_worker_lifetime.cpp`: full actual B5E2F0 body. Its extra EDX string context and existing noexcept cleanup boundary remain explicit. |
| Allocation funclets | `singleton_lifetime_free` is a real shared CRT free service. It is usable only after the child's cleanup and ownership requirements have been satisfied. |
| Constructor state 26 child | `native_shader_state_definitions.cpp`: actual B585A0 via SoundLifetimeAccess, explicit member/base `__finally` cleanup. Parent allocation remains 2Ch despite the 28h accessed source prefix. |
| Constructor state 25 child | Sibling commit `9447b175`: actual 4CCh Lua owner and armed Lua/base cleanup. Pending primary integration at the audit base; source/secondary-exception ABI limits remain. |
| Constructor state 28 child; normal root+1970h deletion | Sibling commit `6feace26`: actual 24h control worker lifetime. Reached thread providers, retained process context and parent integration remain requirements. |
| Both state 8; state-26 dependency | B32370/B30410 are another worker's in-flight packet. They are not counted as integrated source in this audit. B317C0 remains its own resize0/free wrapper. |
| Both state 9; destructor state 26 | B32200 -> B320F0 and full B31750/B317C0 are not supplied by `EffectCache::clear_00b31750_fragment`; that projected fragment does not prove actual owner-layout cleanup. |
| Both state 19; destructor state 25 | Actual B2AE20/B23120 and B236B0 helpers exist, but B2F700 -> B2F690 and B29E40/B260B0 exact cleanup bodies are still separate work. |
| Other vector/section funclets | Exact native targets are established above. Similar typed vectors, pooled lock methods and reserve helpers do not establish these exact raw destruction entries. No missing cleanup is replaced by a stub. |

These are source-composition observations, not a claim that original FH3 handlers, original exception objects, SEH faults, secondary cleanup exceptions, virtual profiles or the complete parent are interchangeable with the source interfaces. Ordered chains assume each prior cleanup returns. No gameplay or runtime renderer creation/destruction was exercised.

## Precise system-registry conflict

`B32833` allocates the 10h registry owner, `B3283B` captures it, `B32841` arms state 27 and `B3284D` calls B5BF70. On native unwind, CBDED9 frees that exact pointer through BF65AC and continues with state 24. The parent funclet does not call a registry destructor, inspect a host operation, unregister a published owner or implement child recovery; the native callee must have completed its own unwind before the parent allocation is freed.

Current `NativeSystemConstantRegistryLifetimeBinding::begin` admits a fresh operation by retaining its owner and marking it running. Each admitted source failure marks the operation failed and rethrows while retaining its array child, temporary names/records, unpublished storage, input pointers and binding guard. `NativeSystemConstantRegistryOperation::~NativeSystemConstantRegistryOperation` terminates for running/failed phases. The binding destructor terminates with a live guard, and the canonical terminal callback terminates if asked to retire the guarded owner. The documented contract requires these objects and their domains to stay alive for explicit later recovery; no such recovery is implemented.

Therefore simply wrapping current B5BF70 in the original parent state-27 allocation cleanup frees an owner that its source continuation still owns. Putting the operation or binding on the parent's unwinding automatic stack instead terminates before a normal cleanup chain can finish. Keeping a dangling operation after free, clearing its guard/phase, replaying construction or inventing rollback does not resolve the conflict. A BF681B failure before B5BF70 admission is different: parent state remains 24 and no new admitted operation exists.

The outstanding work is substantive inner registry failure/unwind recovery compatible with subsequent parent free, or an explicitly incomplete retained parent continuation keeping every guarded dependency alive. This evidence packet implements neither and leaves original parent exception compatibility open.

## Reproducible evidence

The report contains both maps, every funclet byte string and instruction sequence, exact per-state receiver/capture and next state, all parent state-write IPs, complete chain orders, source snapshots and native span SHA-256 values. Immutable local manifests:

- `J:\PROG\battlestations-pacific-decompile-orch5-native-renderer-unwind-evidence\local\output\renderer_unwind_capture_20260914\manifest.json`; SHA-256 `8d4383c6ebcb360ff2a0187b7e9e3819b549e77ac23b944be0532a85c2b9803e`.
- `J:\PROG\battlestations-pacific-decompile-orch5-native-renderer-unwind-evidence\local\output\renderer_unwind_sources_20260914\manifest.json`; SHA-256 `f8ebf4003f452ddd0007e2feb7ed844158376fff1908b92cfcaa088e2a0e7d7e`.

Local binaries/listings and source snapshots are ignored evidence inputs; the committed JSON preserves their manifest values. Source snapshots from siblings are read directly from the named commits. The original source/analysis state is preserved. The evidence checks only equality of these native bytes and the stated source contracts; no test suite was added or run for these documentation/report changes.
